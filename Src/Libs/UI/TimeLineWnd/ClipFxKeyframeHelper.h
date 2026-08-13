#pragma once
#include "Common/MediaPropertyKeyframeIds.h"
#include "Model/TimeLineModel/TimeLineObject.h"
#include <QUuid>
#include <chrono>
#include <optional>

class CPropertyKeyFrameBar;
/**
 * @brief ClipFx 时间线面板关键帧助手（静态工具类）
 *
 * 与 CMediaKeyframeHelper 功能类似，但不依赖 PropertyWnd 头文件，
 * 专供 TimeLineWnd 内 ClipFx 曲线编辑与关键帧导航使用。
 * 属性面板双向同步通过 SyncActivePropertyFrame 转发至 CMediaPropertyFrame。
 */
class CClipFxKeyframeHelper
{
public:
	/**
	 * @brief 判断播放游标是否落在指定素材的时间范围内
	 * @param mediaId 时间线对象（Clip）UUID
	 * @return 游标时间与对象 [start, end) 相交时返回 true
	 */
	static bool IsPlayheadOnObject(const QUuid& mediaId);
	/**
	 * @brief 获取当前游标对应的 Clip 本地时间（帧对齐）
	 * @param mediaId 时间线对象 UUID
	 * @return 相对于素材入点的对齐毫秒时间；对象无效时返回 0ms
	 */
	static std::chrono::milliseconds GetAlignedClipLocalMs(const QUuid& mediaId);
	/**
	 * @brief 查询指定属性在 Plot 中的数值映射范围
	 * @param propId 媒体属性 ID
	 * @param outMin 输出属性最小值（Plot 底部）
	 * @param outMax 输出属性最大值（Plot 顶部）
	 */
	static void GetPropertyValueRange(EMediaPropertyId propId, float& outMin, float& outMax);
	/**
	 * @brief 判断 Plot 上是否允许垂直拖拽改变属性值
	 * @param propId 媒体属性 ID
	 * @return 连续数值型属性（如音量、位置）返回 true；离散/只读属性返回 false
	 */
	static bool IsPropertyVerticalDragAllowed(EMediaPropertyId propId);
	/**
	 * @brief 在指定 Clip 本地时间求值属性的浮点结果（含插值）
	 * @param pObject 时间线对象指针
	 * @param propId 媒体属性 ID
	 * @param clipLocalMs Clip 本地毫秒时间
	 * @param fallback 无关键帧或对象无效时的默认返回值
	 * @return 插值后的属性浮点值
	 */
	static float EvaluatePropertyFloat(const CTimeLineObject* pObject, EMediaPropertyId propId,
		const std::chrono::milliseconds& clipLocalMs, float fallback = 0.f);
	/**
	 * @brief 读取游标当前位置处属性的浮点值
	 * @param mediaId 时间线对象 UUID
	 * @param propId 媒体属性 ID
	 * @param fallback 游标不在对象上或无数据时的默认值
	 * @return 游标时刻的属性值
	 */
	static float ReadPropertyValueAtPlayhead(const QUuid& mediaId, EMediaPropertyId propId, float fallback = 0.f);
	/**
	 * @brief 在当前游标对应的 Clip 本地时间添加或删除关键帧
	 * @param mediaId 时间线对象 UUID
	 * @param propId 媒体属性 ID
	 * @return 操作成功（数据已写入模型）时返回 true
	 */
	static bool TogglePropertyKeyframe(const QUuid& mediaId, EMediaPropertyId propId);
	/** @brief 是否为缩放宽/高属性 */
	static bool IsScaleSizePropertyId(EMediaPropertyId propId);
	/** @brief 缩放宽高配对属性；非缩放属性返回自身 */
	static EMediaPropertyId GetPairedScalePropertyId(EMediaPropertyId propId);
	/** @brief 媒体是否开启缩放等比例锁定（KeepAspectRatio） */
	static bool IsScaleAspectLocked(const QUuid& mediaId);
	/**
	 * @brief 按 SizeKeepAspectRatio 由一侧算出另一侧尺寸
	 */
	static float ComputePairedScaleValue(const QUuid& mediaId, EMediaPropertyId editedProp, float editedValue);
	/**
	 * @brief Toggle 宽/高关键帧后：锁定时使配对轨与本侧「同有/同无」
	 */
	static void SyncPairedScaleKeyframeAfterToggle(const QUuid& mediaId, EMediaPropertyId propId);
	/**
	 * @brief Plot 拖拽提交后：锁定时同步配对轨的时间与按比例数值
	 */
	static void SyncPairedScaleKeyframeAfterDrag(const QUuid& mediaId, EMediaPropertyId propId,
		const std::chrono::milliseconds& originalKeyTime,
		const std::chrono::milliseconds& newKeyTime,
		float editedValue);
	/**
	 * @brief 刷新属性行关键帧导航栏按钮状态
	 * @param mediaId 时间线对象 UUID
	 * @param propId 媒体属性 ID
	 * @param pBar 待更新的 CPropertyKeyFrameBar 控件指针
	 */
	static void RefreshClipFxKeyframeBar(const QUuid& mediaId, EMediaPropertyId propId, CPropertyKeyFrameBar* pBar);
	/**
	 * @brief 对指定关键帧应用插值曲线模式
	 * @param mediaId 时间线对象 UUID
	 * @param propId 媒体属性 ID
	 * @param keyTime 目标关键帧 Clip 本地毫秒时间
	 * @param mode 线性 / 自动贝塞尔 / 自由曲线 等模式
	 * @return 模式已成功写入节点时返回 true
	 */
	static bool ApplyPropertyKeyframeCurveMode(const QUuid& mediaId, EMediaPropertyId propId,
		const std::chrono::milliseconds& keyTime, EPropertyKeyframeCurveMode mode);
	/**
	 * @brief 判断指定关键帧是否可使用自动贝塞尔模式
	 * @param pObject 时间线对象指针
	 * @param propId 媒体属性 ID
	 * @param keyTime 待检查的关键帧 Clip 本地时间
	 * @return 同属性轨道上至少有一侧相邻关键帧时返回 true
	 */
	static bool CanUseAutoBezierCurve(const CTimeLineObject* pObject, EMediaPropertyId propId,
		const std::chrono::milliseconds& keyTime);
	/**
	 * @brief 判断指定关键帧是否可使用自由贝塞尔模式
	 * @param pObject 时间线对象指针
	 * @param propId 媒体属性 ID
	 * @param keyTime 待检查的关键帧 Clip 本地时间
	 * @return 同属性轨道上前后均有相邻关键帧（夹在中间）时返回 true
	 */
	static bool CanUseFreeBezierCurve(const CTimeLineObject* pObject, EMediaPropertyId propId,
		const std::chrono::milliseconds& keyTime);
	/**
	 * @brief 将播放游标 Seek 到指定 Clip 本地时间
	 * @param mediaId 时间线对象 UUID
	 * @param clipLocalMs 目标 Clip 本地毫秒时间（内部转换为全局时间）
	 */
	static void SeekToClipLocalTime(const QUuid& mediaId, const std::chrono::milliseconds& clipLocalMs);
	/**
	 * @brief 同步已注册的 CMediaPropertyFrame 单属性显示
	 *
	 * 实现于 .cpp 以避免 TimeLineWnd 与 PropertyWnd 头文件循环依赖。
	 * @param mediaId 时间线对象 UUID
	 * @param propId 需同步的属性 ID
	 * @param clipLocalMs 可选的关键帧 Clip 本地时间；省略时仅刷新当前游标对应值
	 */
	static void SyncActivePropertyFrame(const QUuid& mediaId, EMediaPropertyId propId,
		std::optional<std::chrono::milliseconds> clipLocalMs = std::nullopt);
};