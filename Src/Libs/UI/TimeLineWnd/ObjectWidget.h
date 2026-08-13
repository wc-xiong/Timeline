#pragma once
#include "ContentViewData.h"
#include <QWidget>
#include <QPoint>
#include <QUuid>
#include <QMouseEvent>
#include <chrono>
#include <memory>
#include <set>
#include <vector>

class ITimeLineTransFx;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QPaintEvent;
class CClipWidget;
class CClipFxWidget;
class CClipTransFxWidget;
class CLayersWidget;
class ITimeLineTransFx;
class CObjectWidget : public QWidget
{
	Q_OBJECT
public:
	CObjectWidget(QWidget* pParent = 0);
	~CObjectWidget();

	void	InitWnd();
	void	SetObjectId(const QUuid& id);
	QUuid	GetObjectId() const;
	void	SyncGeometryFromModel();
	/**
	 * 布局 CClip 与本素材关联的全部 CClipTransFxWidget。
	 * @param leftHalfPx/rightHalfPx 均 >=0 且 previewTransFxId 有效时为该转场的拉伸预览宽度
	 * @param previewJunctionLayerPx >=0 时固定交界点 J 的图层 X（拉伸预览用）
	 * @param previewTransFxId 非空时仅对该转场应用预览参数，其余转场按模型默认布局
	 */
	void LayoutChildWidgets(int leftHalfPx = -1, int rightHalfPx = -1, int previewJunctionLayerPx = -1,
		const QUuid& previewTransFxId = QUuid());
	int CalcTotalHeight() const;
	void ReleaseTransFxWidget();
	const std::vector<CClipTransFxWidget*>& GetClipTransFxWidgets() const { return m_transFxWidgets; }
	/**
	 * @brief 供 CClipWidget 绘制路径关键帧交互/预览态
	 * @param selectedLocalMs 输出：按下选中（或拖动中的预览）本地时刻
	 * @param bHasSelected 输出：是否处于按下/拖动交互选中（仅交互期间，松手清除）
	 * @param previewLocalMs 输出：拖动预览本地时刻
	 * @param bShowPreview 输出：是否正在拖动预览
	 * @param originLocalMs 输出：按下时的原始本地时刻（拖动中用于隐藏原菱形）
	 * @param bInteraction 输出：是否处于路径关键帧按下交互（蓝拖拽态，含未超阈值）
	 * @return 有交互选中或正在预览时返回 true
	 */
	bool	GetPathKeyframeDrawState(std::chrono::milliseconds& selectedLocalMs, bool& bHasSelected,
		std::chrono::milliseconds& previewLocalMs, bool& bShowPreview,
		std::chrono::milliseconds& originLocalMs, bool& bInteraction) const;

signals:
	void	signalUpdateView(int nFlag);

protected:
	void paintEvent(QPaintEvent* event) override;
	bool eventFilter(QObject* watched, QEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void dropEvent(QDropEvent* event) override;
	void InitSnapTimes();
	bool StartObjectDrag(QMouseEvent* event);

private:
	CLayersWidget* FindLayersWidget() const;
	bool	IsClickOnObjectByModel(const QPoint& localPos) const;
	bool	TryHandleTransFxMousePress(const QPoint& posInObject, QMouseEvent* event);
	bool	ShouldYieldPressToTransFx(QMouseEvent* event) const;
	void	ForwardMouseEventToLayers(QMouseEvent* event);
	void	UpdateHoverCursor(const QPoint& localPos);
	bool	IsParentLayerLocked() const;
	CClipTransFxWidget* EnsureTransFxWidget();
	void	LayoutOneTransFxWidget(const std::shared_ptr<ITimeLineTransFx>& pTransFx,
		int leftHalfPx, int rightHalfPx, int previewJunctionLayerPx);
	void	OnClipFxHeightChanged();
	void	OnClipKeyframeMarkersChanged();

	bool	IsDescendantOfClipFx(QObject* watched) const;
	void	InstallClipFxMouseEventForward();
	void	ShowObjectContextMenu(const QPoint& globalPos);
	void	ToggleClipFxTrackVisibility();

	/**
	 * @brief 尝试在路径片段上命中关键帧并进入交互（选中，尚未提交拖动）
	 * @param localPos ObjectWidget 局部坐标
	 * @return 命中并进入交互返回 true
	 */
	bool	TryBeginPathKeyframeInteraction(const QPoint& localPos);
	/**
	 * @brief 超过拖动阈值后更新路径关键帧预览本地时刻并重绘
	 * @param localPos 当前鼠标局部坐标
	 */
	void	UpdatePathKeyframeDragPreview(const QPoint& localPos);
	/**
	 * @brief 结束路径关键帧交互：可选提交移动、Seek 游标并刷新视图
	 * @param bCommitMove true 时若发生拖动则写回模型
	 */
	void	FinishPathKeyframeInteraction(bool bCommitMove);
	/**
	 * @brief 清除路径关键帧按下/拖动交互态
	 * @param bKeepSelection true 时保留选中高亮时刻（当前松手路径传 false）
	 */
	void	ClearPathKeyframeInteraction(bool bKeepSelection);
	/**
	 * @brief 将路径关键帧本地时刻帧对齐并钳到 [0, duration]
	 * @param localMs 待钳位的片段本地时刻
	 * @return 对齐并钳位后的本地时刻
	 */
	std::chrono::milliseconds ClampPathKeyframeLocalMs(const std::chrono::milliseconds& localMs) const;

private:
	QUuid						m_objId;
	CClipWidget*				m_pClip;
	CClipFxWidget*				m_pClipFx;
	std::vector<CClipTransFxWidget*> m_transFxWidgets; // 父控件均为 CLayerWidget
	QPoint		m_dragStartPos;
	QPoint		m_pressHotSpot;
	QUuid		m_originalLayerId;
	std::chrono::milliseconds	m_originalStartTime{ 0 };
	std::chrono::milliseconds	m_originalDuration{ 0 };
	int			m_originalX = 0;

	enum EResizeSide { RS_None, RS_Left, RS_Right };
	bool		m_bResizing = false;
	bool		m_bDragStarted = false;
	EResizeSide	m_resizeSide = RS_None;
	QPoint		m_resizeStartPos;
	QPoint		m_resizeStartGlobalPos;
	int			m_resizeStartWidth;
	std::set<std::chrono::milliseconds> m_setSnapTimes;

	/** @brief 是否处于路径关键帧按下交互（含未超阈值的短按） */
	bool		m_bPathKfInteraction = false;
	/** @brief 是否已超过拖动阈值、进入预览移动 */
	bool		m_bPathKfDragActive = false;
	/** @brief 是否有路径关键帧交互选中（仅按下/拖动期间为 true，松手清除；紫选中由游标重合决定） */
	bool		m_bPathKfHasSelection = false;
	/** @brief 路径关键帧按下时的局部坐标 */
	QPoint		m_pathKfPressPos;
	/** @brief 当前选中（拖动中等同预览）的片段本地时刻 */
	std::chrono::milliseconds	m_pathKfSelectedLocal{ 0 };
	/** @brief 按下命中时的原始片段本地时刻 */
	std::chrono::milliseconds	m_pathKfOriginalLocal{ 0 };
	/** @brief 拖动预览的片段本地时刻 */
	std::chrono::milliseconds	m_pathKfPreviewLocal{ 0 };
};