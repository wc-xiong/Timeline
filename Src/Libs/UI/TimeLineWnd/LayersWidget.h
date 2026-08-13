#pragma once
#include "ContentViewData.h"
#include "UICommon/BaseFrame.h"

#include "UICommon/SvgImageButton.h"
#include "UICommon/LineEditEx.h"

#include <QLabel>
#include <QVBoxLayout>
#include <set>
#include <chrono>
#include <vector>

class CObjectWidget;
class CLayerWidget;
class QMimeData;
class CLayersWidget :public QWidget
{
	Q_OBJECT
public:
	CLayersWidget(QWidget* pParent = 0);
	~CLayersWidget();

	void	UpdateTranslateText();
	void	UpdateView();
	// 仅重算图层几何，不创建/销毁 ObjectWidget（避免 ClipFx 高度变更时递归 CheckObject）
	void	RelayoutLayersGeometry();
	const QRect& BoxSelectRect() const { return m_RectSelect; }

	std::vector<CObjectWidget*> GetObjects() const;
	std::set<std::chrono::milliseconds> CollectSnapTimes(const QVector<QUuid>& excludeIds) const;

	CLayerWidget* GetLayerWidget(int layerIndex) const;
	CLayerWidget* GetLayerWidget(const QUuid& layerId) const;

	int GetLayerRequiredHeight(int layerIndex) const;
	int GetTotalLayersHeight() const;
	std::vector<int> GetLayerHeights() const;
	int GetLayerTopY(int layerIndex) const;
	int GetLayerIndexAtY(int y) const;

	void	OnObjectDragCommitted();
	void	NotifyAfterObjectDrag();
	void	PreviewTimelineObjectDrag(const QMimeData* pMime, const QPoint& posInLayer);
	void	ClearObjectDragClipPosPreview();
	bool	BlockLayerContextMenu();
	/** @brief 设置下一次图层右键菜单是否吞掉（素材菜单关闭后防止再弹复制/粘贴） */
	void	SetBlockNextLayerContextMenu(bool bBlock) { m_bBlockNextLayerContextMenu = bBlock; }

protected:
	void	resizeEvent(QResizeEvent* event) override;
	void	wheelEvent(QWheelEvent* e)override;
	void	dragEnterEvent(QDragEnterEvent* event) override;
	void	dragMoveEvent(QDragMoveEvent* event) override;
	void	dropEvent(QDropEvent* event) override;
	void	dragLeaveEvent(QDragLeaveEvent* event) override;
	void	mousePressEvent(QMouseEvent* event);
	void	mouseMoveEvent(QMouseEvent* event);
	void	mouseReleaseEvent(QMouseEvent* event);
protected:
	void	BoxSelect();
	void	BoxSelectEnd();
	void	CheckLayers();
	void	RefreshBoxSelectOverlay();
signals:
	void	signalWheel(int, int);
	void	signalUpdateView(int);
	void	signalDeleteKey();

private:
	enum EOperateType
	{
		OperateType_Normal,
	}m_operateMode;
	QPoint m_MouseStartPoint = QPoint();
	QPoint m_MouseMoveOffset = QPoint();
	long m_MouseStartTime = 0;
	bool m_MouseFirstMove = true;
	QRect m_RectSelect = QRect();
	std::vector<CLayerWidget*> m_vecPLayers;
	QWidget* m_pBoxSelectOverlay = nullptr;

	bool m_bRightPanMoved = false;				// 记录图层右键移动
	bool m_bBlockNextLayerContextMenu = false;	// 阻塞图层的右键菜单
};