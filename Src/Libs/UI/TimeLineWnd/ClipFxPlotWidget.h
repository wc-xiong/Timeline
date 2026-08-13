#pragma once
#include <QWidget>
#include <QUuid>
#include <chrono>
#include <map>
#include <optional>

#include "ClipFxDefine.h"
#include "Common/MediaPropertyKeyframeIds.h"
#include "Model/TimeLineModel/TimeLineObject.h"
/**
 * @brief 单属性关键帧曲线 Plot 控件
 *
 * 嵌入 CClipFxWidget 树形列表的 Plot 行，绘制关键帧节点、段间插值曲线及贝塞尔控制柄。
 * 支持拖拽移动关键帧、调整左右手柄，并通过信号通知父级更新选中状态与 Clip 缩略图标记。
 */
class CClipFxPlotWidget : public QWidget
{
	Q_OBJECT
public:
	CClipFxPlotWidget(const QUuid& objectId, EMediaPropertyId propId, QWidget* pParent = nullptr);
	/**
	 * @brief 设置 Plot 绘制区域宽度
	 * @param widthPx 像素宽度，随树视口宽度变化由 CClipFxWidget 调用
	 */
	void SetPlotWidth(int widthPx);
	/** @brief 请求重绘曲线（关键帧数据或选中态变更后调用） */
	void UpdatePlot();
	/**
	 * @brief 获取绑定的媒体属性 ID
	 * @return 构造时传入的 EMediaPropertyId
	 */
	EMediaPropertyId GetPropertyId() const { return m_propId; }
	/**
	 * @brief 设置当前选中的关键帧时间
	 * @param keyTime Clip 本地毫秒时间；std::nullopt 表示无选中
	 */
	void SetSelectedKeyframe(std::optional<std::chrono::milliseconds> keyTime);
	/**
	 * @brief 获取当前选中的关键帧时间
	 * @return 选中关键帧的 Clip 本地毫秒时间，未选中时为 std::nullopt
	 */
	std::optional<std::chrono::milliseconds> GetSelectedKeyframe() const { return m_selectedKeyTime; }
	/** @brief 清除选中态并重绘（不发射 signalSelectionCleared） */
	void ClearSelection();

signals:
	/**
	 * @brief 用户点击选中某关键帧菱形节点
	 * @param propId 本 Plot 绑定的属性 ID
	 * @param keyTimeMs 被选中关键帧的 Clip 本地时间（毫秒）
	 */
	void signalKeyframeSelected(EMediaPropertyId propId, qint64 keyTimeMs);
	/**
	 * @brief 用户点击 Plot 空白区域取消选中
	 * @param propId 本 Plot 绑定的属性 ID
	 */
	void signalSelectionCleared(EMediaPropertyId propId);
	/** @brief 拖拽提交后关键帧数据已写入 TimeLineObject 模型 */
	void signalDataChanged();

protected:
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	QSize sizeHint() const override;

private:
	/**
	 * @brief 绘制一帧所需的坐标系与数值范围上下文
	 *
	 * 将 Clip 本地时间与属性浮点值映射到 Plot 像素坐标。
	 */
	struct SPlotContext
	{
		int plotTop = 0;                              // Plot 内绘区顶部 Y 坐标（像素）
		int plotH = 0;                                // Plot 内绘区高度（像素）
		float minV = 0.f;                             // 当前属性值映射到 Plot 的下限
		float maxV = 1.f;                             // 当前属性值映射到 Plot 的上限
		std::chrono::milliseconds objectStart{ 0 };   // 素材在时间线全局时间轴上的起始时间
	};

	/**
	 * @brief 鼠标拖拽过程中的临时状态
	 *
	 * 含 previewNodes 预览集，释放前不写回模型。
	 */
	struct SDragState
	{
		bool active = false;                          // 是否已进入有效拖拽（超过 CLIPFX_DRAG_START_THRESHOLD）
		EClipFxHitTarget target = EClipFxHitTarget::None; // 当前拖拽目标：节点 / 左手柄 / 右手柄
		std::chrono::milliseconds keyTime{ 0 };       // 当前操作的关键帧时间（拖拽中可能变化）
		std::chrono::milliseconds originalKeyTime{ 0 }; // 鼠标按下时的原始关键帧时间
		float originValue = 0.f;                      // 按下时关键帧的属性值
		QPoint pressPos;                              // 鼠标按下时的本地坐标
		SPropertyKeyframeHandle originHandle;         // 拖拽手柄时的初始柄数据（timeOffset / valueOffset）
		std::map<std::chrono::milliseconds, SPropertyKeyframeNode> previewNodes; // 拖拽预览用的关键帧节点副本
	};

	/**
	 * @brief 构建当前帧绘制上下文
	 * @param ctx 输出的 SPlotContext
	 * @return 对象与属性有效且可绘制时返回 true
	 */
	bool BuildPlotContext(SPlotContext& ctx) const;
	/**
	 * @brief 从 TimeLineObject 加载本属性全部关键帧节点
	 * @return 以 Clip 本地毫秒时间为键的节点 map
	 */
	std::map<std::chrono::milliseconds, SPropertyKeyframeNode> LoadNodes() const;
	/**
	 * @brief Clip 本地时间转换为 Plot 内 X 坐标
	 * @param ctx 绘制上下文
	 * @param clipLocalMs Clip 本地毫秒时间
	 * @return Plot 内像素 X
	 */
	int TimeToPlotX(const SPlotContext& ctx, std::chrono::milliseconds clipLocalMs) const;
	/**
	 * @brief 贝塞尔手柄时间偏移转换为 Plot X 坐标
	 * @param ctx 绘制上下文
	 * @param keyTime 所属关键帧时间
	 * @param timeOffsetMs 手柄相对关键帧的时间偏移（毫秒）
	 * @return 手柄中心 Plot X
	 */
	int HandlePlotX(const SPlotContext& ctx, std::chrono::milliseconds keyTime, float timeOffsetMs) const;
	/**
	 * @brief Plot X 坐标反算为 Clip 本地时间
	 * @param ctx 绘制上下文
	 * @param x Plot 内像素 X
	 * @return 对齐后的 Clip 本地毫秒时间
	 */
	std::chrono::milliseconds PlotXToTime(const SPlotContext& ctx, int x) const;
	/**
	 * @brief 属性浮点值转换为 Plot Y 坐标
	 * @param ctx 绘制上下文
	 * @param value 属性值
	 * @return Plot 内像素 Y（值越大越靠上）
	 */
	int ValueToPlotY(const SPlotContext& ctx, float value) const;
	/**
	 * @brief Plot Y 坐标反算为属性浮点值
	 * @param ctx 绘制上下文
	 * @param y Plot 内像素 Y
	 * @return 限制在 [minV, maxV] 内的属性值
	 */
	float PlotYToValue(const SPlotContext& ctx, int y) const;
	/**
	 * @brief 鼠标位置命中检测
	 * @param ctx 绘制上下文
	 * @param pos 鼠标本地坐标
	 * @param nodes 当前关键帧节点集
	 * @param outKeyTime 命中关键帧时输出其时间
	 * @return 命中的交互目标类型
	 */
	EClipFxHitTarget HitTest(const SPlotContext& ctx, const QPoint& pos,
		const std::map<std::chrono::milliseconds, SPropertyKeyframeNode>& nodes,
		std::chrono::milliseconds& outKeyTime) const;
	/**
	 * @brief 绘制入段曲线（从前一关键帧到当前节点）
	 * @param painter Qt 画笔
	 * @param ctx 绘制上下文
	 * @param t0 段起点时间
	 * @param n0 段起点节点
	 * @param t1 段终点时间
	 * @param n1 段终点节点
	 */
	void DrawIncomingCurveSegment(QPainter& painter, const SPlotContext& ctx,
		const std::chrono::milliseconds& t0, const SPropertyKeyframeNode& n0,
		const std::chrono::milliseconds& t1, const SPropertyKeyframeNode& n1) const;
	/**
	 * @brief 绘制出段曲线（从当前节点到下一关键帧）
	 * @param painter Qt 画笔
	 * @param ctx 绘制上下文
	 * @param t0 段起点时间
	 * @param n0 段起点节点
	 * @param t1 段终点时间
	 * @param n1 段终点节点
	 */
	void DrawOutgoingCurveSegment(QPainter& painter, const SPlotContext& ctx,
		const std::chrono::milliseconds& t0, const SPropertyKeyframeNode& n0,
		const std::chrono::milliseconds& t1, const SPropertyKeyframeNode& n1) const;
	/**
	 * @brief 绘制由起点右手柄与终点左手柄共同控制的双柄贝塞尔段
	 * @param painter Qt 画笔
	 * @param ctx 绘制上下文
	 * @param t0 段起点时间
	 * @param n0 段起点节点
	 * @param t1 段终点时间
	 * @param n1 段终点节点
	 */
	void DrawDualHandleCurveSegment(QPainter& painter, const SPlotContext& ctx,
		const std::chrono::milliseconds& t0, const SPropertyKeyframeNode& n0,
		const std::chrono::milliseconds& t1, const SPropertyKeyframeNode& n1) const;
	/**
	 * @brief 将 m_drag.previewNodes 写回 TimeLineObject 并同步属性面板
	 *
	 * 拖拽释放且数据有效变更时调用，随后发射 signalDataChanged。
	 */
	void CommitDrag();
	QUuid m_objectId;                               // 绑定的时间线 Clip 对象 UUID
	EMediaPropertyId m_propId = EMediaPropertyId::VolumeIntensity; // 本 Plot 编辑的媒体属性 ID
	std::optional<std::chrono::milliseconds> m_selectedKeyTime; // 当前选中的关键帧 Clip 本地时间
	SDragState m_drag;                              // 鼠标拖拽交互的完整临时状态
};