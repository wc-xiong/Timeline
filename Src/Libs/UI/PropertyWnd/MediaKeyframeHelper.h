#pragma once
#include "MediaPropertyFrame.h"
#include "Model/TimeLineModel/TimeLineObject.h"

#include <QUuid>
#include <vector>
#include <utility>
#include <optional>
#include <chrono>

class CMediaNode;
class CMediaPropertyFrame;
class CPropertyKeyFrameBar;
/**
 * @brief 媒体属性关键帧助手
 *
 * 桥接三层数据：
 * 1. 时间线 CTimeLineObject — 属性关键帧轨道存储与插值
 * 2. CMediaNode — 音量/位置/色彩效果等渲染参数
 * 3. CFxProperty — 裁剪/羽化/抠像等 XEffect 因子
 *
 * 属性面板 CMediaPropertyFrame 与 ClipFx 时间线面板均通过本类读写关键帧。
 */

class CMediaKeyframeHelper
{
public:
	static CTimeLineObject* GetTimelineObject(const QUuid& mediaId);
	static std::shared_ptr<CMediaNode> GetMediaNode(const QUuid& mediaId);
	/**
	 * @brief 判断全局播放游标是否落在指定素材的有效时间范围内
	 * @param mediaId 目标素材 ID
	 * @param playheadMs 全局游标时间（毫秒）；为 nullopt 时使用当前全局播放头位置
	 * @return 游标处于 [start, start+duration) 区间内返回 true，否则返回 false
	 */
	static bool IsPlayheadOnObject(const QUuid& mediaId,
		std::optional<std::chrono::milliseconds> playheadMs = std::nullopt);
	/**
	 * @brief 将全局游标时间转换为 clip 本地时间，并按项目帧率对齐
	 * @param mediaId 目标素材 ID
	 * @param playheadMs 全局游标时间（毫秒）；为 nullopt 时使用当前全局播放头位置
	 * @return 对齐后的 clip 本地时间（相对于素材起始点的毫秒偏移）
	 */
	static std::chrono::milliseconds GetAlignedClipLocalMs(const QUuid& mediaId,
		std::optional<std::chrono::milliseconds> playheadMs = std::nullopt);
	static std::vector<EMediaPropertyId> GetGroupPropertyIds(EMediaPropertyGroupId groupId);
	static bool HasGroupKeyframes(const CTimeLineObject* pObject, EMediaPropertyGroupId groupId);
	static QString GetPropertyDisplayName(EMediaPropertyId propId);
	static QString GetGroupDisplayName(EMediaPropertyGroupId groupId);
	static void GetPropertyValueRange(EMediaPropertyId propId, float& outMin, float& outMax);
	static float ReadPropertyValue(CMediaPropertyFrame* pFrame, EMediaPropertyId propId);
	/**
	 * @brief 确保 CFxProperty 中存在与指定属性对应的 XEffect 特效因子
	 * @details 针对裁剪（Cut）、羽化（Feather）、抠像（Keying）等属性，
	 *          若因子尚未创建则按给定 value 初始化，保证关键帧写入后渲染链路可用
	 * @param mediaId 目标素材 ID
	 * @param propId 属性 ID，用于映射到对应的 EEffectType 因子
	 * @param value 因子初始值（通常来自属性面板当前显示值）
	 * @param pFrame 属性面板帧指针，用于读取 UI 状态辅助因子创建
	 */
	static void EnsureFxFactorForProperty(const QUuid& mediaId, EMediaPropertyId propId, float value,
		CMediaPropertyFrame* pFrame);
	/**
	 * @brief 在当前游标时刻切换（添加或删除）指定属性的关键帧
	 * @param mediaId 目标素材 ID
	 * @param propId 属性 ID
	 * @param value 写入关键帧的数值（来自属性面板当前显示值）
	 * @param pFrame 属性面板帧指针，用于读取 UI 值及刷新关键帧栏状态
	 * @return 操作成功返回 true；游标不在素材内或图层锁定时返回 false
	 */
	static bool TogglePropertyKeyframe(const QUuid& mediaId, EMediaPropertyId propId, float value,
		CMediaPropertyFrame* pFrame);

	/** @brief 是否为缩放宽/高属性 */
	static bool IsScaleSizePropertyId(EMediaPropertyId propId);
	/** @brief 缩放宽高配对属性；非缩放属性返回自身 */
	static EMediaPropertyId GetPairedScalePropertyId(EMediaPropertyId propId);
	/** @brief 媒体是否开启缩放等比例锁定 */
	static bool IsScaleAspectLocked(const QUuid& mediaId);
	/**
	 * @brief 按 SizeKeepAspectRatio（宽/高基准）由一侧算出另一侧尺寸
	 * @param editedProp 刚编辑的一侧（ScaleWidth 或 ScaleHeight）
	 * @param editedValue 该侧新值
	 */
	static float ComputePairedScaleValue(const QUuid& mediaId, EMediaPropertyId editedProp, float editedValue);
	/**
	 * @brief Toggle 宽/高关键帧后：锁定时使配对轨与本侧「同有/同无」
	 * @details 有本侧则补齐配对点（按比例取值）；无本侧则删除配对点
	 */
	static void SyncPairedScaleKeyframeAfterToggle(const QUuid& mediaId, EMediaPropertyId propId,
		CMediaPropertyFrame* pFrame = nullptr);
	/**
	 * @brief Plot 拖拽提交后：锁定时同步配对轨的时间与按比例数值
	 * @param originalKeyTime 拖拽前关键帧时间
	 * @param newKeyTime 拖拽后关键帧时间
	 * @param editedValue 本侧新值
	 */
	static void SyncPairedScaleKeyframeAfterDrag(const QUuid& mediaId, EMediaPropertyId propId,
		const std::chrono::milliseconds& originalKeyTime,
		const std::chrono::milliseconds& newKeyTime,
		float editedValue);

	/**
	 * @brief 分组关键帧 Toggle：全有则全删，否则为缺失项补齐
	 */
	static bool ToggleGroupKeyframes(const QUuid& mediaId, EMediaPropertyGroupId groupId,
		CMediaPropertyFrame* pFrame = nullptr);
	/**
	 * @brief 重置属性分组：清空组内全部关键帧，并将属性恢复为出厂默认值
	 * @param mediaId 目标素材 ID
	 * @param groupId 属性分组 ID
	 * @param pFrame 属性面板帧指针，用于同步滑条/SpinBox/CheckBox 显示
	 * @return 操作成功返回 true；素材无效或分组无属性时返回 false
	 */
	static bool ResetPropertyGroup(const QUuid& mediaId, EMediaPropertyGroupId groupId,
		CMediaPropertyFrame* pFrame);
	/**
	 * @brief 在游标处写入属性值：已有关键帧则更新，否则添加新关键帧
	 * @param mediaId 目标素材 ID
	 * @param propId 属性 ID
	 * @param value 待写入的浮点数值
	 * @param pFrame 属性面板帧指针
	 * @param playheadMs 全局游标时间；为 nullopt 时使用当前播放头位置
	 * @return 写入成功返回 true；游标不在素材内时返回 false
	 */
	static bool ApplyPropertyValueAtPlayhead(const QUuid& mediaId, EMediaPropertyId propId, float value,
		CMediaPropertyFrame* pFrame, std::optional<std::chrono::milliseconds> playheadMs = std::nullopt);
	static void NotifyClipFxPropertyKeyframeChanged(const QUuid& mediaId, EMediaPropertyId propId,
		const std::chrono::milliseconds& clipLocalMs);
	/**
	 * @brief 将属性面板所有滑条/SpinBox 重置为特效默认值
	 * @details 默认值来自 CMediaNode / CEffectProperty / CFxProperty 因子；
	 *          无对应因子时使用 XEffectCommon 结构体构造的出厂默认值
	 * @param pFrame 待重置的属性面板帧指针
	 * @param mediaId 目标素材 ID，用于读取节点上的实际默认参数
	 */
	static void LoadFxPropertyDisplayDefaults(CMediaPropertyFrame* pFrame, const QUuid& mediaId);
	/**
	 * @brief 在指定属性的关键帧轨道上执行上一帧/下一帧导航
	 * @param mediaId 目标素材 ID
	 * @param propId 属性 ID
	 * @param op 导航操作类型（上一关键帧 / 下一关键帧）
	 * @param outTargetClipLocalMs 可选输出：跳转后的 clip 本地时间，供刷新 Pre/Next 按钮可用状态
	 * @return 找到目标关键帧并成功跳转返回 true；无更多关键帧时返回 false
	 */
	static bool NavigatePropertyKeyframe(const QUuid& mediaId, EMediaPropertyId propId, EKeyFrameOp op,
		std::optional<std::chrono::milliseconds>* outTargetClipLocalMs = nullptr);
	/**
	 * @brief 刷新单个属性行关键帧栏的 Toggle/Pre/Next 按钮状态
	 * @param pFrame 属性面板帧指针
	 * @param propId 属性 ID
	 * @param pBar 该属性行右侧的关键帧操作栏
	 * @param playheadMs 参考游标时间；为 nullopt 时使用当前播放头位置
	 */
	static void RefreshKeyframeToggleState(CMediaPropertyFrame* pFrame, EMediaPropertyId propId,
		CPropertyKeyFrameBar* pBar, std::optional<std::chrono::milliseconds> playheadMs = std::nullopt);
	/**
	 * @brief 刷新属性分组标题行关键帧栏的 Toggle 按钮状态（分组级关键帧）
	 * @param pFrame 属性面板帧指针
	 * @param groupId 属性分组 ID
	 * @param pBar 分组标题右侧的关键帧操作栏
	 * @param playheadMs 参考游标时间；为 nullopt 时使用当前播放头位置
	 */
	static void RefreshGroupKeyframeToggleState(CMediaPropertyFrame* pFrame, EMediaPropertyGroupId groupId,
		CPropertyKeyFrameBar* pBar, std::optional<std::chrono::milliseconds> playheadMs = std::nullopt);
	/**
	 * @brief 刷新属性面板内所有属性行及分组标题的关键帧栏状态
	 * @param pFrame 属性面板帧指针；游标移动或关键帧变更后统一调用
	 */
	static void RefreshAllKeyframeToggleStates(CMediaPropertyFrame* pFrame);
	/**
	 * @brief 在 clip 本地时间对指定属性的关键帧轨道进行插值求值
	 * @param pObject 时间线对象指针，持有属性关键帧轨道
	 * @param propId 属性 ID
	 * @param clipLocalMs 求值时刻的 clip 本地时间（毫秒）
	 * @param fallback 轨道为空或该时刻无关键帧时使用的回退默认值
	 * @return 插值后的浮点属性值
	 */
	static float EvaluatePropertyFloat(const CTimeLineObject* pObject, EMediaPropertyId propId,
		const std::chrono::milliseconds& clipLocalMs, float fallback);
	/**
	 * @brief 将关键帧插值结果写回 CMediaNode / CEffectProperty，供播放时实时渲染
	 * @param mediaId 目标素材 ID
	 * @param timelineMs 当前全局时间线时间（毫秒）
	 * @param pFrame 属性面板帧指针，用于同步 UI 显示（可选路径）
	 */
	static void ApplyEvaluatedPropertiesToMediaNode(const QUuid& mediaId,
		const std::chrono::milliseconds& timelineMs, CMediaPropertyFrame* pFrame);
	/**
	 * @brief 判断指定属性是否允许在 ClipFx Plot 纵轴方向拖拽改值
	 * @param propId 属性 ID
	 * @return 允许纵向拖拽返回 true；PosX/Y/Z 等仅支持横向时间定位的属性返回 false
	 */
	static bool IsPropertyVerticalDragAllowed(EMediaPropertyId propId);
	/**
	 * @brief 在播放头时刻读取指定属性的插值数值
	 * @param mediaId 目标素材 ID
	 * @param propId 属性 ID
	 * @param fallback 无关键帧轨道时的回退默认值
	 * @return 当前游标时刻的插值属性值
	 */
	static float ReadPropertyValueAtPlayhead(const QUuid& mediaId, EMediaPropertyId propId, float fallback = 0.f);
	/**
	 * @brief ClipFx 时间线面板专用：在当前游标时刻切换指定属性的关键帧
	 * @param mediaId 目标素材 ID
	 * @param propId 属性 ID
	 * @param pFrame 可选的属性面板帧指针，用于同步面板 UI 状态
	 * @return 操作成功返回 true
	 */
	static bool TogglePropertyKeyframeForClipFx(const QUuid& mediaId, EMediaPropertyId propId, CMediaPropertyFrame* pFrame = nullptr);
	/**
	 * @brief 刷新 ClipFx 时间线面板中指定属性行的关键帧栏状态
	 * @param mediaId 目标素材 ID
	 * @param propId 属性 ID
	 * @param pBar ClipFx 面板中该属性对应的关键帧操作栏
	 * @param refKeyTime 参考关键帧时间；为 nullopt 时使用当前游标对齐后的 clip 本地时间
	 */
	static void RefreshClipFxKeyframeBar(const QUuid& mediaId, EMediaPropertyId propId, CPropertyKeyFrameBar* pBar,
		std::optional<std::chrono::milliseconds> refKeyTime = std::nullopt);
	/**
	 * @brief 为指定时刻的关键帧设置插值曲线模式（线性/贝塞尔/自动贝塞尔等
	 * @param mediaId 目标素材 ID
	 * @param propId 属性 ID
	 * @param keyTime 目标关键帧的 clip 本地时间（毫秒）
	 * @param mode 曲线模式枚举值
	 * @return 设置成功返回 true；关键帧不存在或模式不合法时返回 false
	 */
	static bool ApplyPropertyKeyframeCurveMode(const QUuid& mediaId, EMediaPropertyId propId,
		const std::chrono::milliseconds& keyTime, EPropertyKeyframeCurveMode mode);
	/**
	 * @brief 判断指定关键帧是否可使用自动贝塞尔（Auto Bezier）曲线模式
	 * @param pObject 时间线对象指针
	 * @param propId 属性 ID
	 * @param keyTime 目标关键帧的 clip 本地时间（毫秒）
	 * @return 同属性轨道上至少有一侧相邻关键帧时返回 true
	 */
	static bool CanUseAutoBezierCurve(const CTimeLineObject* pObject, EMediaPropertyId propId,
		const std::chrono::milliseconds& keyTime);
	/**
	 * @brief 判断指定关键帧是否可使用自由贝塞尔（Free Bezier）曲线模式
	 * @param pObject 时间线对象指针
	 * @param propId 属性 ID
	 * @param keyTime 目标关键帧的 clip 本地时间（毫秒）
	 * @return 同属性轨道上前后均有相邻关键帧（夹在中间）时返回 true
	 */
	static bool CanUseFreeBezierCurve(const CTimeLineObject* pObject, EMediaPropertyId propId,
		const std::chrono::milliseconds& keyTime);
	/**
	 * @brief 将全局播放头跳转到指定素材的 clip 本地时间位置
	 * @param mediaId 目标素材 ID
	 * @param clipLocalMs 目标 clip 本地时间（毫秒），将转换为全局时间线时间后执行 Seek
	 */
	static void SeekToClipLocalTime(const QUuid& mediaId, const std::chrono::milliseconds& clipLocalMs);
	/**
	 * @brief 注册当前活动的 CMediaPropertyFrame 实例（构造时注册，析构时注销）
	 * @param pFrame 属性面板帧指针；传入 nullptr 表示注销当前注册
	 */
	static void RegisterMediaPropertyFrame(CMediaPropertyFrame* pFrame);
	/**
	 * @brief 获取当前已注册的 CMediaPropertyFrame 实例
	 * @return 已注册的属性面板帧指针；无注册实例时返回 nullptr
	 */
	static CMediaPropertyFrame* GetRegisteredMediaPropertyFrame();
	static void SetPropertyFrameDisplayValue(CMediaPropertyFrame* pFrame, EMediaPropertyId propId, float value);
	/**
	 * @brief 对单个属性从时间线关键帧轨道求值，并刷新属性面板对应控件的显示
	 * @param pFrame 属性面板帧指针

	 * @param mediaId 目标素材 ID

	 * @param propId 属性 ID

	 * @param clipLocalMs 求值时刻的 clip 本地时间；为 nullopt 时使用当前游标对齐时间

	 */
	static void SyncPropertyFrameFromTimeline(CMediaPropertyFrame* pFrame, const QUuid& mediaId, EMediaPropertyId propId,
		std::optional<std::chrono::milliseconds> clipLocalMs = std::nullopt);
	/**
	 * @brief 游标在素材内时，将所有可关键帧属性同步为关键帧插值结果
	 * @details 无关键帧轨道的属性使用 GetPropertyDefaultValue 作为 fallback 显示默认值
	 * @param pFrame 属性面板帧指针
	 * @param mediaId 目标素材 ID
	 * @param playheadMs 全局游标时间；为 nullopt 时使用当前播放头位置
	 */
	static void SyncKeyframeEvaluatedValuesAtPlayhead(CMediaPropertyFrame* pFrame, const QUuid& mediaId,
		std::optional<std::chrono::milliseconds> playheadMs = std::nullopt);
	/**
	 * @brief 若已注册属性面板且当前选中指定素材，同步单属性控件的显示值
	 * @param mediaId 目标素材 ID
	 * @param propId 属性 ID
	 * @param clipLocalMs 求值时刻的 clip 本地时间；为 nullopt 时使用当前游标对齐时间
	 */
	static void SyncActivePropertyFrame(const QUuid& mediaId, EMediaPropertyId propId,
		std::optional<std::chrono::milliseconds> clipLocalMs = std::nullopt);
};