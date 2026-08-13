#pragma once
#include <QWidget>
#include <QUuid>
#include <chrono>
class QMouseEvent;
class QShowEvent;
class QResizeEvent;
// 转场在重叠区的显示侧：Front=左素材半场，Back=右素材半场
enum class ETransFxDisplaySide
{
	Front,
	Back
};

/** 片段转场特效控件：与对侧 CClipTransFxWidget 在交界处拼成完整转场区域 */
class CClipTransFxWidget : public QWidget
{
	Q_OBJECT
public:
	CClipTransFxWidget(QWidget* pParent = 0);
	~CClipTransFxWidget();

	void SetTransFxId(const QUuid& id);
	QUuid GetTransFxId() const { return m_transFxId; }
	void SetDisplaySide(ETransFxDisplaySide side);
	// 布局/绘制上下文：重叠区起止、模型单场最大时长、本素材起始时间
	void SetLayoutContext(
		std::chrono::milliseconds overlapStart,
		std::chrono::milliseconds overlapDuration,
		std::chrono::milliseconds transDuration,
		std::chrono::milliseconds objectStartTime);
	void UpdateView();
	// 由素材控件在点击穿透时调用，避免重复 sendEvent 导致双重处理
	void HandleMousePress(QMouseEvent* event);
	// 同一次点击可能被转场与素材重复处理，用于抑制后续的素材选中
	static bool WasRecentPressHandledAt(const QPoint& globalPos);
	static void ClearRecentPressState();

protected:
	void showEvent(QShowEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

private:
	void CommitResize();
	class CLayerWidget* FindLayerWidget() const;

private:
	QUuid m_transFxId;
	ETransFxDisplaySide m_side = ETransFxDisplaySide::Front; // 显示侧：Front=左素材半场，Back=右素材半场
	bool m_bBorderLeft = true;   // 是否绘制左边框
	bool m_bBorderTop = true;    // 是否绘制上边框
	bool m_bBorderRight = true;  // 是否绘制右边框
	bool m_bBorderBottom = true; // 是否绘制下边框
	std::chrono::milliseconds m_overlapStart{ 0 };		// 重叠区起始时间（rightStart，即右素材起点）
	std::chrono::milliseconds m_overlapDuration{ 0 };	// 两相邻素材的重叠总时长（leftEnd - rightStart）
	std::chrono::milliseconds m_transDuration{ 0 };		// 转场模型 ITimeLineTransFx::Duration（完整重叠时长）
	std::chrono::milliseconds m_objectStartTime{ 0 };	// 当前半场所属素材的起始时间
	bool m_bResizing = false;							// 是否处于转场拉伸状态
	int m_resizeStartLeftWidth = 0;						// 拉伸开始时左半场像素宽度 Wl
	int m_resizeStartRightWidth = 0;					// 拉伸开始时右半场像素宽度 Wr
	int m_resizePeakLeftWidth = 0;						// 拉伸过程中左半场曾达到的最大像素宽度
	int m_resizePeakRightWidth = 0;						// 拉伸过程中右半场曾达到的最大像素宽度
	int m_resizeMaxTotalOverlapPx = 0;					// 重叠区总像素宽度上限 Wl+Wr（等于左素材时长对应像素宽）
	int m_resizeStartJunctionLayerPx = 0;				// 拉伸开始时交界点 J 在图层坐标系中的 X
	int m_resizeStartOverlapStartLayerPx = 0;			// 重叠区起点 rightStart 在图层坐标系中的 X
	std::chrono::milliseconds m_resizeStartLeftEnd{ 0 };// 拉伸开始时左素材结束时间 leftEnd
	int m_previewLeftWidth = -1;						// 拉伸预览左半场像素宽度，-1 表示未在预览
	int m_previewRightWidth = -1;						// 拉伸预览右半场像素宽度，-1 表示未在预览
	QPoint m_resizeStartGlobalPos;						// 拉伸开始时鼠标的全局屏幕坐标
	enum EResizeSide { RS_None, RS_Left, RS_Right };	// 拉伸柄侧：无 / 左缘 / 右缘
	EResizeSide m_resizeSide = RS_None;					// 当前正在拖拽的拉伸柄
};