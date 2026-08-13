#pragma once
#include "TimeLineModelGlobal.h"
#include "Common/Common/ITimeLineInterface.h"
#include "Common/Common/QUuidEx.h"

#include <map>
#include <vector>
#include <unordered_map>
#include <shared_mutex>

/**
 * @brief 属性关键帧轨道类型编码基准偏移量
 *
 * CTimeLineObjectFx::m_type 在属性关键帧轨道上取值为 PropertyKeyframeTypeBase + propId，
 * 其中 propId 对应 EMediaPropertyId 等媒体属性枚举。基准值 1000 用于与 EFxType（0~3，如
 * Position/Scale/Rotate/Size）区分，避免类型 ID 冲突。
 */
static constexpr int PropertyKeyframeTypeBase = 1000;

/**
 * @brief 属性关键帧出段曲线模式
 *
 * 描述当前关键帧到下一关键帧之间的插值曲线类型。曲线由起点关键帧的 outCurve 决定；
 * 终点关键帧的 outCurve 不参与本段计算，但终点的 leftHandle 可在混合段中塑形曲线。
 */
enum class EPropertyKeyframeCurveMode : uint8_t
{
	Line = 0,      // 直线插值：起点与终点值之间线性过渡
	AutoBezier,    // 自动贝塞尔：系统自动计算并镜像左右手柄，保证 C1 连续
	FreeBezier,    // 自由贝塞尔：用户可独立拖动左右手柄，完全自定义曲线形状
};

/**
 * @brief 属性关键帧贝塞尔控制手柄
 *
 * 手柄以所属关键帧为锚点，通过时间/数值偏移定义三次贝塞尔曲线的控制点位置。
 * 左手柄影响进入该关键帧的曲线段，右手柄影响离开该关键帧的曲线段。
 */
struct SPropertyKeyframeHandle
{
	/** @brief 相对关键帧锚点的时间偏移（毫秒）；负值表示向左（更早），正值表示向右（更晚） */
	float timeOffsetMs = 0.f;
	/** @brief 相对关键帧锚点数值的偏移量，与属性实际量纲一致（如 0~1 透明度、角度等） */
	float valueOffset = 0.f;
	/** @brief 手柄是否已展开/有效；false 时由 CheckKeyframeHandles 按默认比例自动初始化 */
	bool  bOpen = false;
};

/**
 * @brief 属性关键帧完整节点数据
 *
 * 单个关键帧时间点的完整描述，包含属性值、出段曲线类型及左右贝塞尔手柄。
 * 存储于 CTimeLineObjectFx::m_mapKeyframeNodes，键为 Clip 本地时间（毫秒）。
 */
struct SPropertyKeyframeNode
{
	/** @brief 该时间点的属性采样值（float 标量，如音量、透明度、位置分量等） */
	float value = 0.f;
	/** @brief 从本关键帧到下一关键帧的出段曲线模式 */
	EPropertyKeyframeCurveMode outCurve = EPropertyKeyframeCurveMode::Line;
	/** @brief 左手柄：影响从前一关键帧进入本关键帧的曲线形状（首帧通常为空） */
	SPropertyKeyframeHandle leftHandle;
	/** @brief 右手柄：影响从本关键帧到下一关键帧的曲线形状（末帧通常为空） */
	SPropertyKeyframeHandle rightHandle;
};

/**
 * @brief 判断出段曲线模式是否为贝塞尔类（自动或自由）
 * @param mode 出段曲线模式
 * @return 若为 AutoBezier 或 FreeBezier 返回 true，否则返回 false
 */
inline bool IsPropertyKeyframeBezierCurve(EPropertyKeyframeCurveMode mode)
{
	return mode == EPropertyKeyframeCurveMode::AutoBezier
		|| mode == EPropertyKeyframeCurveMode::FreeBezier;
}

/**
 * @brief 一维三次贝塞尔曲线求值
 *
 * 使用标准 Bernstein 基函数计算 B(t) = (1-t)³P0 + 3(1-t)²tP1 + 3(1-t)t²P2 + t³P3。
 *
 * @param p0 曲线起点值（t=0）
 * @param p1 第一控制点值
 * @param p2 第二控制点值
 * @param p3 曲线终点值（t=1）
 * @param t  归一化参数，取值范围 [0, 1]
 * @return t 处的插值结果
 */
inline float EvalPropertyKeyframeCubicBezier1D(float p0, float p1, float p2, float p3, float t)
{
	const float u = 1.f - t;
	return p0 * u * u * u + 3.f * p1 * u * u * t + 3.f * p2 * u * t * t + p3 * t * t * t;
}

/**
 * @brief 两相邻关键帧之间曲线段求值
 *
 * 根据起点/终点的 outCurve 与手柄数据，在 t∈[0,1] 处计算插值结果。
 *
 * 规则：
 * - 仅当起点与终点的 outCurve 均为 Line 时，使用线性插值；
 * - 否则使用三次贝塞尔：P0=起点值，P3=终点值；
 * - 起点为贝塞尔时 P1=起点值+右手柄 valueOffset，否则 P1=起点值；
 * - 终点为贝塞尔时 P2=终点值+左手柄 valueOffset，否则 P2=终点值；
 * - 混合情形（如起点 Line + 终点 Bezier）时，终点左手柄仍可塑形整段 A→B 曲线。
 *
 * @param nStart 段起点关键帧节点
 * @param nEnd   段终点关键帧节点
 * @param t      段内归一化时间，0 对应起点，1 对应终点
 * @return 该时刻的属性插值
 */
inline float EvaluatePropertyKeyframeSegmentValue(
	const SPropertyKeyframeNode& nStart,
	const SPropertyKeyframeNode& nEnd,
	float t)
{
	const bool bStartBezier = IsPropertyKeyframeBezierCurve(nStart.outCurve);
	const bool bEndBezier = IsPropertyKeyframeBezierCurve(nEnd.outCurve);
	if (!bStartBezier && !bEndBezier)
		return nStart.value + (nEnd.value - nStart.value) * t;

	const float p0 = nStart.value;
	const float p3 = nEnd.value;
	const float p1 = bStartBezier
		? (nStart.value + nStart.rightHandle.valueOffset)
		: nStart.value;
	const float p2 = bEndBezier
		? (nEnd.value + nEnd.leftHandle.valueOffset)
		: nEnd.value;
	return EvalPropertyKeyframeCubicBezier1D(p0, p1, p2, p3, t);
}

class TIMELINEMODEL_EXPORT CTimeLineObjectFx : public ITimeLineObjectFx
{
	INIT_ICOPYINTERFACE(CTimeLineObjectFx)
public:
	CTimeLineObjectFx();
	CTimeLineObjectFx(int type);
	virtual ~CTimeLineObjectFx();
	CTimeLineObjectFx(const CTimeLineObjectFx& other);
	CTimeLineObjectFx& operator=(const CTimeLineObjectFx& other);

	virtual int GetType()override;
	
	virtual void AddValue(const std::chrono::milliseconds& ms, const FxData& v)override;
	virtual void DelereValue(const std::chrono::milliseconds& ms)override;
	virtual void ClearValue()override;
	virtual FxData GetValue(const std::chrono::milliseconds& ms)override;

	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root)override;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator)override;

	/**
	 * @brief 读取属性关键帧轨上全部 (时间, float) 采样点
	 *
	 * 仅适用于 m_type >= PropertyKeyframeTypeBase 的属性关键帧轨道；
	 * 对普通特效轨则过滤 m_mapValues 中的 float 变体项。
	 *
	 * @return 按时间升序的 (毫秒, 属性值) 列表
	 */
	std::vector<std::pair<std::chrono::milliseconds, float>> GetFloatKeyframes() const;
	/**
	 * @brief 读取属性关键帧轨上全部完整节点
	 *
	 * 返回每个关键帧时间点的 SPropertyKeyframeNode（含曲线模式与手柄），
	 * 供编辑器展示与曲线编辑使用。
	 *
	 * @return 按时间升序的 (毫秒, 节点) 列表
	 */
	std::vector<std::pair<std::chrono::milliseconds, SPropertyKeyframeNode>> GetKeyframeNodes() const;
	/**
	 * @brief 读取指定时间点的关键帧节点
	 * @param ms      查询时间（Clip 本地毫秒）
	 * @param outNode 输出：该时间点的完整节点数据
	 * @return 该时间点存在关键帧返回 true，否则 false
	 */
	bool GetKeyframeNodeAt(const std::chrono::milliseconds& ms, SPropertyKeyframeNode& outNode) const;
	/**
	 * @brief 写入或覆盖指定时间点的关键帧节点
	 * @param ms   目标时间（Clip 本地毫秒）
	 * @param node 完整节点数据（值、出段曲线、左右手柄）
	 */
	void SetKeyframeNodeAt(const std::chrono::milliseconds& ms, const SPropertyKeyframeNode& node);
	/**
	 * @brief 将关键帧从旧时间移动到新时间
	 * @param oldMs 原关键帧时间
	 * @param newMs 目标时间；若新时间已有关键帧则移动失败
	 * @return 移动成功返回 true，源不存在或目标冲突返回 false
	 */
	bool MoveKeyframe(const std::chrono::milliseconds& oldMs, const std::chrono::milliseconds& newMs);
	/**
	 * @brief 修改指定时间点关键帧的属性值
	 * @param ms    关键帧时间；若不存在则隐式创建节点（默认直线曲线）
	 * @param value 新的 float 属性值
	 */
	void SetKeyframeValue(const std::chrono::milliseconds& ms, float value);
	/**
	 * @brief 设置指定关键帧的出段曲线模式
	 *
	 * 修改后会自动调用 CheckKeyframeHandles 重建/清理手柄。
	 *
	 * @param ms   关键帧时间
	 * @param mode 新的出段曲线模式（Line / AutoBezier / FreeBezier）
	 */
	void SetKeyframeCurveMode(const std::chrono::milliseconds& ms, EPropertyKeyframeCurveMode mode);
	/**
	 * @brief 获取指定关键帧的出段曲线模式
	 * @param ms 关键帧时间
	 * @return 该关键帧的 outCurve；时间点不存在时返回 Line
	 */
	EPropertyKeyframeCurveMode GetKeyframeCurveMode(const std::chrono::milliseconds& ms) const;
	/**
	 * @brief 设置指定关键帧的左或右手柄
	 * @param ms     关键帧时间
	 * @param bLeft  true 设置 leftHandle，false 设置 rightHandle
	 * @param handle 手柄数据（时间偏移、数值偏移、展开标志）
	 */
	void SetKeyframeHandle(const std::chrono::milliseconds& ms, bool bLeft, const SPropertyKeyframeHandle& handle);
	/**
	 * @brief 校验并自动修正指定关键帧的手柄状态
	 *
	 * 直线模式时清空双手柄；贝塞尔模式时为首/末/中间帧按默认比例初始化未展开的手柄；
	 * AutoBezier 模式下镜像左右手柄以保持平滑。首帧无左手柄，末帧无右手柄。
	 *
	 * @param ms 待校验的关键帧时间
	 */
	void CheckKeyframeHandles(const std::chrono::milliseconds& ms);
	/**
	 * @brief 获取当前关键帧在轨道上的前一关键帧时间
	 * @param ms 当前关键帧时间（须为已存在的关键帧）
	 * @return 前一关键帧时间；当前为首个关键帧或 ms 不存在时返回 -1ms
	 */
	std::chrono::milliseconds GetPreKeyframeTime(const std::chrono::milliseconds& ms) const;
	/**
	 * @brief 获取当前关键帧在轨道上的后一关键帧时间
	 * @param ms 当前关键帧时间（须为已存在的关键帧）
	 * @return 后一关键帧时间；当前为末个关键帧或 ms 不存在时返回 -1ms
	 */
	std::chrono::milliseconds GetNextKeyframeTime(const std::chrono::milliseconds& ms) const;
	/**
	 * @brief 判断指定时间之前是否还存在其他关键帧
	 *
	 * 语义与 UI 导航 lower_bound 一致：不要求 ms 本身为关键帧。
	 *
	 * @param ms 参考时间（Clip 本地毫秒）
	 * @return 存在更早的关键帧返回 true
	 */
	bool HasPreKeyframe(const std::chrono::milliseconds& ms) const;
	/**
	 * @brief 判断指定时间之后是否还存在其他关键帧
	 *
	 * 语义与 UI 导航 upper_bound 一致：不要求 ms 本身为关键帧。
	 *
	 * @param ms 参考时间（Clip 本地毫秒）
	 * @return 存在更晚的关键帧返回 true
	 */
	bool HasNextKeyframe(const std::chrono::milliseconds& ms) const;
	/** @brief 判断本轨道是否为属性关键帧轨（m_type >= PropertyKeyframeTypeBase） */
	bool IsPropertyKeyframeTrack() const { return m_type >= PropertyKeyframeTypeBase; }

	bool IsEmpty() const;
	bool HasValueAt(const std::chrono::milliseconds& ms) const;
	/** @brief 枚举经典 Fx / 属性轨上全部关键帧本地时间（升序） */
	std::vector<std::chrono::milliseconds> GetAllKeyframeTimes() const;

protected:
	/**
	 * @brief 在任意时间点求值属性关键帧轨道（含段内插值）
	 *
	 * ms 恰为关键帧时间时返回该点 value；否则在前一与后一关键帧之间
	 * 按 EvaluatePropertyKeyframeSegmentValue 计算曲线插值。无关键帧或
	 * 查询时间在首帧之前时返回 0.f。
	 *
	 * @param ms 求值时间（Clip 本地毫秒）
	 * @return 该时刻的属性 float 值
	 */
	float EvaluatePropertyKeyframeValue(const std::chrono::milliseconds& ms) const;

	mutable std::shared_mutex	m_mtx;
	int m_type = 0;
	std::map<std::chrono::milliseconds, FxData> m_mapValues;	//以开始时间为键存储特效数据
	/** @brief 属性关键帧轨专用存储：键为 Clip 本地时间（毫秒），值为完整关键帧节点 */
	std::map<std::chrono::milliseconds, SPropertyKeyframeNode> m_mapKeyframeNodes;
};

class TIMELINEMODEL_EXPORT CTimeLineObject : public ITimeLineObject
{
	INIT_ICOPYINTERFACE(CTimeLineObject)
public:
	CTimeLineObject();
	CTimeLineObject(const QUuid& id, int type);
	virtual ~CTimeLineObject();
	CTimeLineObject(const CTimeLineObject& other);
	CTimeLineObject& operator=(const CTimeLineObject& other);

	virtual QUuid GetId()override;
	virtual int GetType()override;

	virtual std::chrono::milliseconds GetStartTime()override;
	virtual void SetStartTime(const std::chrono::milliseconds& ms)override;

	virtual std::chrono::milliseconds GetDuration()override;
	virtual void SetDuration(const std::chrono::milliseconds& msPlayTime)override;

	virtual void AddFx(int type)override;
	virtual void DelereFx(int type)override;
	virtual void ClearFx()override;
	virtual std::weak_ptr<ITimeLineObjectFx> GetFx(int type)override;

	virtual QString GetDisplayName()override;
	virtual void SetDisplayName(const QString& name)override;

	/** @brief 路径相机片段是否在 3D 视图显示路径折线 */
	virtual bool IsShowPath() const override { return m_bShowPath; }
	/** @brief 设置路径相机片段是否显示路径折线 */
	virtual void SetShowPath(bool b) override { m_bShowPath = b; }

	/**
	 * @brief 获取指定媒体属性的属性关键帧轨道（只读弱引用）
	 * @param propId 媒体属性 ID（EMediaPropertyId 等）
	 * @return 已存在轨道的 weak_ptr；未创建过该属性轨时返回空 weak_ptr
	 */
	std::weak_ptr<CTimeLineObjectFx> GetPropertyKeyframeTrack(int propId) const;
	/**
	 * @brief 获取或懒创建指定媒体属性的属性关键帧轨道
	 *
	 * 若轨道不存在则创建 CTimeLineObjectFx，其 m_type 为 EncodePropertyKeyframeType(propId)，
	 * 并登记到 m_hashPropertyKeyframes。
	 *
	 * @param propId 媒体属性 ID
	 * @return 属性关键帧轨道的 shared_ptr（永不为空）
	 */
	std::shared_ptr<CTimeLineObjectFx> EnsurePropertyKeyframeTrack(int propId);
	/**
	 * @brief 在指定属性轨上添加或更新关键帧
	 * @param propId      媒体属性 ID
	 * @param clipLocalMs Clip 本地时间（毫秒）
	 * @param value       关键帧属性值
	 */
	void AddPropertyKeyframe(int propId, const std::chrono::milliseconds& clipLocalMs, float value);
	/**
	 * @brief 删除指定属性轨上某一时间点的关键帧
	 *
	 * 若删除后该轨为空，则从 m_hashPropertyKeyframes 中移除该属性轨。
	 *
	 * @param propId      媒体属性 ID
	 * @param clipLocalMs 要删除的关键帧时间
	 */
	void DeletePropertyKeyframe(int propId, const std::chrono::milliseconds& clipLocalMs);
	/**
	 * @brief 判断指定属性轨在给定时间是否有关键帧
	 * @param propId      媒体属性 ID
	 * @param clipLocalMs 查询时间
	 * @return 该时间点存在关键帧返回 true
	 */
	bool HasPropertyKeyframeAt(int propId, const std::chrono::milliseconds& clipLocalMs) const;
	/**
	 * @brief 获取指定属性轨上全部关键帧时间列表
	 * @param propId 媒体属性 ID
	 * @return 按时间升序的 Clip 本地毫秒列表；轨不存在或为空时返回空 vector
	 */
	std::vector<std::chrono::milliseconds> GetPropertyKeyframeTimes(int propId) const;
	/**
	 * @brief 汇总 Clip 上全部属性轨的关键帧时间（去重）
	 *
	 * 遍历 m_hashPropertyKeyframes 中所有非空轨道，合并各轨关键帧时间并去重，
	 * 供 Clip 缩略图时间轴上的关键帧标记绘制使用。
	 *
	 * @return 去重后的升序时间列表
	 */
	std::vector<std::chrono::milliseconds> GetAllPropertyKeyframeTimes() const;
	/** @brief 判断 Clip 是否至少有一条非空的属性关键帧轨 */
	bool HasAnyPropertyKeyframes() const;
	/** @brief 用 RapidJSON 数组整体替换属性关键帧轨（网络同步） */
	bool ReplacePropertyKeyframesFromRapidJson(const rapidjson::Value& propertyKeyframesArray);
	/**
	 * @brief 判断指定属性关键帧轨是否为空（未创建或无任何关键帧）
	 * @param propId 媒体属性 ID
	 * @return 轨不存在或无关键帧返回 true
	 */
	bool IsPropertyKeyframeTrackEmpty(int propId) const;
	/**
	 * @brief 移动指定属性轨上的关键帧时间
	 * @param propId 媒体属性 ID
	 * @param oldMs  原关键帧时间
	 * @param newMs  目标时间
	 */
	void MovePropertyKeyframe(int propId, const std::chrono::milliseconds& oldMs, const std::chrono::milliseconds& newMs);
	/**
	 * @brief 修改指定属性轨上某一关键帧的数值
	 * @param propId      媒体属性 ID
	 * @param clipLocalMs 关键帧时间
	 * @param value       新的属性值
	 */
	void SetPropertyKeyframeValue(int propId, const std::chrono::milliseconds& clipLocalMs, float value);
	/**
	 * @brief 设置指定属性轨上某一关键帧的出段曲线模式
	 * @param propId      媒体属性 ID
	 * @param clipLocalMs 关键帧时间
	 * @param mode        出段曲线模式
	 */
	void SetPropertyKeyframeCurveMode(int propId, const std::chrono::milliseconds& clipLocalMs, EPropertyKeyframeCurveMode mode);
	/**
	 * @brief 获取指定属性轨上某一关键帧的出段曲线模式
	 * @param propId      媒体属性 ID
	 * @param clipLocalMs 关键帧时间
	 * @return 出段曲线模式；轨或关键帧不存在时返回 Line
	 */
	EPropertyKeyframeCurveMode GetPropertyKeyframeCurveMode(int propId, const std::chrono::milliseconds& clipLocalMs) const;
	/**
	 * @brief 设置指定属性轨上某一关键帧的左或右手柄
	 * @param propId      媒体属性 ID
	 * @param clipLocalMs 关键帧时间
	 * @param bLeft       true 设置左手柄，false 设置右手柄
	 * @param handle      手柄数据
	 */
	void SetPropertyKeyframeHandle(int propId, const std::chrono::milliseconds& clipLocalMs, bool bLeft, const SPropertyKeyframeHandle& handle);
	/**
	 * @brief 校验并自动修正指定属性轨上某一关键帧的手柄
	 * @param propId      媒体属性 ID
	 * @param clipLocalMs 关键帧时间
	 */
	void CheckPropertyKeyframeHandles(int propId, const std::chrono::milliseconds& clipLocalMs);
	/**
	 * @brief 读取指定属性轨上某一时间点的完整关键帧节点
	 * @param propId      媒体属性 ID
	 * @param clipLocalMs 关键帧时间
	 * @param outNode     输出：完整节点数据
	 * @return 读取成功返回 true
	 */
	bool GetPropertyKeyframeNode(int propId, const std::chrono::milliseconds& clipLocalMs, SPropertyKeyframeNode& outNode) const;
	/**
	 * @brief 获取指定属性轨上当前关键帧的前一关键帧时间
	 * @param propId      媒体属性 ID
	 * @param clipLocalMs 当前关键帧时间
	 * @return 前一关键帧时间；无效时返回 -1ms
	 */
	std::chrono::milliseconds GetPrePropertyKeyframeTime(int propId, const std::chrono::milliseconds& clipLocalMs) const;
	/**
	 * @brief 获取指定属性轨上当前关键帧的后一关键帧时间
	 * @param propId      媒体属性 ID
	 * @param clipLocalMs 当前关键帧时间
	 * @return 后一关键帧时间；无效时返回 -1ms
	 */
	std::chrono::milliseconds GetNextPropertyKeyframeTime(int propId, const std::chrono::milliseconds& clipLocalMs) const;
	/**
	 * @brief 判断指定属性轨上，给定时间之前是否还有关键帧
	 * @param propId      媒体属性 ID
	 * @param clipLocalMs 参考时间
	 * @return 存在更早关键帧返回 true
	 */
	bool HasPrePropertyKeyframe(int propId, const std::chrono::milliseconds& clipLocalMs) const;
	/**
	 * @brief 判断指定属性轨上，给定时间之后是否还有关键帧
	 * @param propId      媒体属性 ID
	 * @param clipLocalMs 参考时间
	 * @return 存在更晚关键帧返回 true
	 */
	bool HasNextPropertyKeyframe(int propId, const std::chrono::milliseconds& clipLocalMs) const;

	/** @brief 获取 Clip 特效/属性关键帧轨道在时间线 UI 中是否可见 */
	bool IsClipFxTrackVisible() const { return m_bClipFxTrackVisible; }
	/** @brief 设置 Clip 特效/属性关键帧轨道在时间线 UI 中的可见性（打开工程时始终为折叠，需手动展开） */
	void SetClipFxTrackVisible(bool bVisible) { m_bClipFxTrackVisible = bVisible; }

	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root)override;
	virtual void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator)override;

protected:
	/**
	 * @brief 将媒体属性 ID 编码为 CTimeLineObjectFx 轨道类型
	 * @param propId EMediaPropertyId 等媒体属性枚举值
	 * @return PropertyKeyframeTypeBase + propId，用于 m_type 与 JSON 序列化
	 */
	static int EncodePropertyKeyframeType(int propId) { return PropertyKeyframeTypeBase + propId; }

private:
	mutable std::shared_mutex	m_mtx;
	std::chrono::milliseconds m_startTime{0};
	std::chrono::milliseconds m_duration{0};
	QUuid m_id;
	int m_type = ObjectType_Media;
	QString m_displayName; // 片段显示名（路径相机片段等）
	/** @brief Clip 特效/属性关键帧轨道在时间线 UI 中的可见性标志，默认 true */
	bool m_bClipFxTrackVisible = false;
	/** @brief 路径相机片段是否在 3D 显示路径折线，默认 true */
	bool m_bShowPath = true;

	std::unordered_map<int, std::shared_ptr<ITimeLineObjectFx>> m_hashFxs;	//以类型EFxType为键存储特效类,供路径相机使用
	/** @brief 属性关键帧轨集合：键为 propId（EMediaPropertyId），值为对应 float 关键帧轨道 */
	std::unordered_map<int, std::shared_ptr<CTimeLineObjectFx>> m_hashPropertyKeyframes;
};