#define _USE_MATH_DEFINES
#include "LayersWidget.h"
#include "Common/Common/ResourceDefine.h"
#include "GSMedia/ThreadManager/ThreadManagerExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"

#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "Model/ModelModel/ModelModelExport.h"
#include "Model/ScreenModel/ScreenModelExport.h"

#include "UICommon/menuex.h"
#include "UICommon/Messagebox.h"
#include "UICommon/RenameDialog.h"
#include "UICommon/Colordialog.h"
#include "UICommon/UITranslate.h"
#include "ObjectWidget.h"
#include "LayerWidget.h"
#include "ClipTransFxWidget.h"
#include "TimelineDragSnapCache.h"
#include "ObjectDragMime.h"
#include <QDrag>
#include <QDragLeaveEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QResizeEvent>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QtGlobal>
#include <QPainter>
#include <QCoreApplication>
#include <set>
#include <map>
#include <algorithm>
#include <QApplication>
#include "TimelineWinMacros.h"
//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end
namespace
{
class CBoxSelectOverlayWidget : public QWidget
{
public:
	explicit CBoxSelectOverlayWidget(CLayersWidget* pLayers)
		: QWidget(pLayers)
		, m_pLayers(pLayers)
	{
		setAttribute(Qt::WA_TransparentForMouseEvents);
		setAttribute(Qt::WA_NoSystemBackground);
	}

protected:
	void paintEvent(QPaintEvent*) override
	{
		if (!m_pLayers)
			return;

		const QRect rectSelect = m_pLayers->BoxSelectRect();
		if (rectSelect.isNull())
			return;

		QPainter p(this);
		p.setRenderHint(QPainter::Antialiasing);
		p.setBrush(Qt::NoBrush);
		p.setPen(QPen(Qt::white, nSizeDefine_2));
		p.drawRect(rectSelect);
	}

private:
	CLayersWidget* m_pLayers = nullptr;
};
}

CLayersWidget::CLayersWidget(QWidget* pParent) :
     QWidget(pParent)
{
	setMouseTracking(true);
	setAcceptDrops(true);
	setFocusPolicy(Qt::StrongFocus);

	m_pBoxSelectOverlay = new CBoxSelectOverlayWidget(this);
	RefreshBoxSelectOverlay();
}
CLayersWidget::~CLayersWidget()
{
}

void CLayersWidget::UpdateTranslateText()
{
}

void CLayersWidget::UpdateView()
{
	CheckLayers();

	int YOffset = g_pContentViewData->GetYOffset();
	const int w = width();
	auto vecLayerIDs = g_pTimeLineModel->GetAllLayerIDs();
	int cumY = 0;
	for (int i = 0; i < (int)m_vecPLayers.size() && i < (int)vecLayerIDs.size(); ++i)
	{
		m_vecPLayers[i]->UpdateView(vecLayerIDs[i], i);
		const int layerH = m_vecPLayers[i]->GetRequiredHeight();
		m_vecPLayers[i]->setGeometry(0, cumY + YOffset, w, layerH);
		m_vecPLayers[i]->LayoutObjects();
		cumY += layerH;
	}
	update();
	RefreshBoxSelectOverlay();
}

void CLayersWidget::RelayoutLayersGeometry()
{
	static bool s_bRelayouting = false;
	if (s_bRelayouting)
		return;
	s_bRelayouting = true;

	const int YOffset = g_pContentViewData->GetYOffset();
	const int w = width();

	std::vector<int> heights;
	heights.reserve(m_vecPLayers.size());
	for (auto* pLayer : m_vecPLayers)
		heights.push_back(pLayer ? pLayer->GetRequiredHeight() : SingleLayerHeight);

	bool bAnyChanged = false;
	int cumY = 0;
	for (size_t i = 0; i < m_vecPLayers.size(); ++i)
	{
		auto* pLayer = m_vecPLayers[i];
		if (!pLayer)
			continue;
		const int layerH = heights[i];
		const QRect newGeo(0, cumY + YOffset, w, layerH);
		if (pLayer->geometry() != newGeo)
		{
			pLayer->setGeometry(newGeo);
			bAnyChanged = true;
		}
		cumY += layerH;
	}

	for (auto* pLayer : m_vecPLayers)
	{
		if (pLayer)
			pLayer->LayoutObjects();
	}

	s_bRelayouting = false;

	if (!bAnyChanged)
		return;
	update();
	emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_VerScroll);
}

void CLayersWidget::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	RefreshBoxSelectOverlay();
}

void CLayersWidget::RefreshBoxSelectOverlay()
{
	if (!m_pBoxSelectOverlay)
		return;

	m_pBoxSelectOverlay->setGeometry(rect());
	m_pBoxSelectOverlay->raise();
	m_pBoxSelectOverlay->update();
}

void CLayersWidget::wheelEvent(QWheelEvent* e)
{
	__super::wheelEvent(e);

	if (e->modifiers() & Qt::ControlModifier)
	{
		int horDelta = 0;
		bool zoomed = false;
		const int mouseX = mapFromGlobal(e->globalPos()).x();
		if (g_pContentViewData->HandleTimelineCtrlWheelZoom(e->angleDelta().y(), mouseX, horDelta, zoomed))
		{
			if (horDelta != 0)
				emit signalWheel(horDelta, 0);
			if (zoomed)
				emit signalUpdateView(TimelineZoomRefreshViewMask);
			e->accept();
			return;
		}
	}
	else if (e->modifiers() & Qt::ShiftModifier)
	{
		if (g_pContentViewData->HandleTimelineShiftWheelScroll(e->angleDelta().y()))
		{
			emit signalUpdateView(TimelineZoomRefreshViewMask);
			e->accept();
			return;
		}
	}
	else
	{
		emit signalWheel(0, -e->delta() / 12);
	}
}

void CLayersWidget::PreviewTimelineObjectDrag(const QMimeData* pMime, const QPoint& posInLayer)
{
	int oldSnapPos = -1;
	const bool bOldSnap = g_pContentViewData->GetSnap(oldSnapPos);
	const bool bOldDragPreview = g_pContentViewData->HasObjectDragPreview();
	const auto oldDragTime = g_pContentViewData->GetObjectDragPreviewTime();

	CObjectDragMime::PreviewObjectDragSnap(pMime, posInLayer, this);

	int newSnapPos = -1;
	const bool bNewSnap = g_pContentViewData->GetSnap(newSnapPos);
	const bool bNewDragPreview = g_pContentViewData->HasObjectDragPreview();
	const auto newDragTime = g_pContentViewData->GetObjectDragPreviewTime();
	if (bOldSnap == bNewSnap && oldSnapPos == newSnapPos
		&& bOldDragPreview == bNewDragPreview && oldDragTime == newDragTime)
		return;

	emit signalUpdateView(WndType::WndType_NoniusView | WndType::WndType_TimeLineTool | WndType::WndType_LayersControl);
}

void CLayersWidget::ClearObjectDragClipPosPreview()
{
	g_pContentViewData->ClearObjectDragPreview();
	emit signalUpdateView(WndType::WndType_NoniusView | WndType::WndType_TimeLineTool | WndType::WndType_LayersControl);
}

bool CLayersWidget::BlockLayerContextMenu()
{
	if (!m_bBlockNextLayerContextMenu)
		return false;
	m_bBlockNextLayerContextMenu = false;
	return true;
}

void CLayersWidget::dragEnterEvent(QDragEnterEvent * event)
{
	if (event->mimeData()->hasFormat(g_TimelineObjectDragFormat))
	{
		event->setDropAction(Qt::MoveAction);
		event->accept();
	}
}

void CLayersWidget::dragMoveEvent(QDragMoveEvent * event)
{
	if (event->mimeData()->hasFormat(g_TimelineObjectDragFormat))
	{
		if (auto* pLayerWidget = qobject_cast<CLayerWidget*>(childAt(event->pos())))
		{
			const QPoint posInLayer = pLayerWidget->mapFrom(this, event->pos());
			PreviewTimelineObjectDrag(event->mimeData(), posInLayer);
		}
		else
		{
			ClearObjectDragClipPosPreview();
		}
		event->acceptProposedAction();
	}
}

void CLayersWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
	TimelineDragSnapCache::Clear();
	g_pContentViewData->SetSnap(false);
	ClearObjectDragClipPosPreview();
	QWidget::dragLeaveEvent(event);
}

void CLayersWidget::dropEvent(QDropEvent * event)
{
	if (!event->mimeData()->hasFormat(g_TimelineObjectDragFormat))
		return;

	if (auto* pLayerWidget = qobject_cast<CLayerWidget*>(childAt(event->pos())))
	{
		const QPoint posInLayer = pLayerWidget->mapFrom(this, event->pos());
		if (pLayerWidget->TryCommitObjectDrop(event->mimeData(), posInLayer))
			event->acceptProposedAction();
	}
	ClearObjectDragClipPosPreview();
}

void CLayersWidget::mousePressEvent(QMouseEvent * e)
{
	__super::mousePressEvent(e);

	m_MouseStartPoint = e->pos();
	m_MouseStartTime = e->timestamp();

	if (e->button() == Qt::LeftButton)
	{
		m_operateMode = OperateType_Normal;
		m_MouseFirstMove = true;

		if (!childAt(e->pos()))
		{
			CClipTransFxWidget::ClearRecentPressState();
			g_pSelectController->ClearTransFxSelect();
			g_pSelectController->ClearTimeLineLayerSelect();
			g_pSelectController->ClearTimeLineObjectSelect();
			emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_LayersView | WndType::WndType_TimeLineTool);
			emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
		}
	}
	else if (e->button() == Qt::RightButton)
	{
		m_bRightPanMoved = false;
		m_bBlockNextLayerContextMenu = false;
		m_MouseFirstMove = true;
	}
}

void CLayersWidget::mouseMoveEvent(QMouseEvent * e)
{
	__super::mouseMoveEvent(e);

	m_MouseMoveOffset = e->pos() - m_MouseStartPoint;

	if (e->buttons() & Qt::LeftButton)
	{
		if (!m_MouseFirstMove || m_MouseMoveOffset.manhattanLength() > nSizeDefine_10)
		{
			m_MouseFirstMove = false;

			switch (m_operateMode)
			{
				case CLayersWidget::OperateType_Normal:
				{
					BoxSelect();
					emit signalUpdateView(WndType::WndType_LayersView);
				}
				break;
				default:
					break;
				}
		}
	}
	else if (e->buttons() & Qt::RightButton)
	{
		if (!m_MouseFirstMove || m_MouseMoveOffset.manhattanLength() > nSizeDefine_10)
		{
			m_MouseFirstMove = false;
			m_bRightPanMoved = true;

			setCursor(Qt::ClosedHandCursor);

			emit signalWheel(-m_MouseMoveOffset.x(), -m_MouseMoveOffset.y());

			m_MouseStartPoint = e->pos();

			emit signalUpdateView(WndType::WndType_LayersView | WndType::WndType_TimeLineScale);
		}
	}
}

void CLayersWidget::mouseReleaseEvent(QMouseEvent * e)
{
	__super::mouseReleaseEvent(e);

	const bool bDidDrag = (m_MouseMoveOffset.manhattanLength() > nSizeDefine_10);
	const bool bTransFxClick = CClipTransFxWidget::WasRecentPressHandledAt(e->globalPos());

	if (e->button() == Qt::LeftButton)
	{
		switch (m_operateMode)
		{
			case CLayersWidget::OperateType_Normal:
			{
				BoxSelectEnd();
			}
			break;
			default:
				break;
			}

		m_operateMode = OperateType_Normal;
		m_MouseMoveOffset = QPoint();
		m_MouseFirstMove = true;

		emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_LayersView |
			WndType::WndType_TimeLineTool | WndType_NoniusView |
			WndType::WndType_HorScroll);

		emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
		// 仅在实际拖动后刷新属性；转场点击后的 release 不应切走属性页
		if (bDidDrag && !bTransFxClick)
			emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeDataChange);
	}
	else if (e->button() == Qt::RightButton)
	{
		unsetCursor();
		m_MouseFirstMove = true;

		const bool bDragged = m_bRightPanMoved || (m_MouseMoveOffset.manhattanLength() > nSizeDefine_10);
		m_bRightPanMoved = false;
		m_MouseMoveOffset = QPoint();

		// 右键拖动后释放：不弹菜单，只吞掉后续Layer context menu
		if (bDragged)
		{
			m_bBlockNextLayerContextMenu = true;
			e->accept();
			return;
		}

		e->accept();
		return;
	}
}

void CLayersWidget::BoxSelect()
{
	int x = std::min(m_MouseStartPoint.x(), m_MouseStartPoint.x() + m_MouseMoveOffset.x());
	int y = std::min(m_MouseStartPoint.y(), m_MouseStartPoint.y() + m_MouseMoveOffset.y());
	int w = abs(m_MouseMoveOffset.x());
	int h = abs(m_MouseMoveOffset.y());

	m_RectSelect = QRect(x, y, w, h);
	RefreshBoxSelectOverlay();

	g_pSelectController->ClearTimeLineLayerSelect();
	g_pSelectController->ClearTimeLineObjectSelect();
	auto LayerIds = g_pTimeLineModel->GetAllLayerIDs();
	const int boxTop = y - g_pContentViewData->GetYOffset();
	const int boxBottom = y + h - g_pContentViewData->GetYOffset();
	for (int k = 0; k < (int)LayerIds.size(); k++)
	{
		const int layerTop = GetLayerTopY(k);
		const int layerH = GetLayerRequiredHeight(k);
		if (layerTop + layerH <= boxTop || layerTop >= boxBottom)
			continue;
		auto pLayer = g_pTimeLineModel->GetLayer(LayerIds[k]).lock();
		if (pLayer)
		{
			if (pLayer->IsLock()) continue;

			auto ObjectIds = pLayer->GetAllObjectIDs();
			for (int t = 0; t < (int)ObjectIds.size(); t++)
			{
				auto pObject = pLayer->GetObject(ObjectIds[t]).lock();
				if (pObject)
				{
					if (g_pContentViewData->TimeToPosition(pObject->GetStartTime()) < x + w - g_pContentViewData->GetXOffset() &&
						g_pContentViewData->TimeToPosition(pObject->GetStartTime() + pObject->GetDuration()) > x - g_pContentViewData->GetXOffset())
					{
						g_pSelectController->AddTimeLineObjectSelect(ObjectIds[t]);
						g_pSelectController->AddTimeLineLayerSelect(LayerIds[k]);
					}
				}
			}
		}
	}

	emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_LayersView |
		WndType::WndType_TimeLineTool);

	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
}

void CLayersWidget::BoxSelectEnd()
{
	m_RectSelect = QRect();
	RefreshBoxSelectOverlay();
}

void CLayersWidget::CheckLayers()
{
	std::vector<QUuid> vecLayerIDs = g_pTimeLineModel->GetAllLayerIDs();
	int diff = (int)vecLayerIDs.size() - (int)m_vecPLayers.size();
	if (diff > 0)
	{
		for (int i = 0; i < diff; ++i)
		{
			auto* p = new CLayerWidget(this, this);
			connect(p, &CLayerWidget::signalUpdateView, this, &CLayersWidget::signalUpdateView);
			p->show();
			m_vecPLayers.push_back(p);
		}
	}
	else if (diff < 0)
	{
		for (int i = 0; i < -diff; ++i)
		{
			CLayerWidget* p = m_vecPLayers.back();
			m_vecPLayers.pop_back();
			delete p;
		}
	}
	RefreshBoxSelectOverlay();
}

std::vector<CObjectWidget*> CLayersWidget::GetObjects() const
{
	std::vector<CObjectWidget*> result;
	for (auto* pLayer : m_vecPLayers)
	{
		if (!pLayer)
			continue;
		for (auto* pObj : pLayer->GetObjects())
			result.push_back(pObj);
	}
	return result;
}

CLayerWidget* CLayersWidget::GetLayerWidget(int layerIndex) const
{
	if (layerIndex < 0 || layerIndex >= (int)m_vecPLayers.size())
		return nullptr;
	return m_vecPLayers[layerIndex];
}

CLayerWidget* CLayersWidget::GetLayerWidget(const QUuid& layerId) const
{
	for (auto* pLayer : m_vecPLayers)
	{
		if (pLayer && pLayer->GetLayerId() == layerId)
			return pLayer;
	}
	return nullptr;
}

std::set<std::chrono::milliseconds> CLayersWidget::CollectSnapTimes(const QVector<QUuid>& excludeIds) const
{
	if (TimelineDragSnapCache::IsActive())
		return TimelineDragSnapCache::Get();

	std::set<QUuid> excludeSet(excludeIds.begin(), excludeIds.end());
	std::set<std::chrono::milliseconds> snapTimes;
	auto layerIds = g_pTimeLineModel->GetAllLayerIDs();
	for (const auto& layerId : layerIds)
	{
		auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
		if (!pLayer)
			continue;
		for (const auto& objId : pLayer->GetAllObjectIDs())
		{
			if (excludeSet.find(objId) != excludeSet.end())
				continue;
			auto pObj = pLayer->GetObject(objId).lock();
			if (!pObj)
				continue;
			snapTimes.insert(pObj->GetStartTime());
			snapTimes.insert(pObj->GetStartTime() + pObj->GetDuration());
		}
	}
	if (!g_pPlayerController->IsPlaying())
		snapTimes.insert(g_pPlayerController->GetCurTime());
	return snapTimes;
}

void CLayersWidget::OnObjectDragCommitted()
{
	UpdateView();
	for (auto* pLayer : m_vecPLayers)
	{
		if (pLayer)
			pLayer->RefreshTransFxForLayer();
	}
	NotifyAfterObjectDrag();
}

void CLayersWidget::NotifyAfterObjectDrag()
{
	emit signalUpdateView(WndType::WndType_LayerLabels
		| WndType::WndType_NoniusView
		| WndType::WndType_TimeLineTool
		| WndType::WndType_HorScroll);
	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
}

int CLayersWidget::GetLayerRequiredHeight(int layerIndex) const
{
	auto* pLayer = GetLayerWidget(layerIndex);
	return pLayer ? pLayer->GetRequiredHeight() : SingleLayerHeight;
}

int CLayersWidget::GetTotalLayersHeight() const
{
	int total = 0;
	for (auto* pLayer : m_vecPLayers)
	{
		if (pLayer)
			total += pLayer->GetRequiredHeight();
	}
	return total;
}

std::vector<int> CLayersWidget::GetLayerHeights() const
{
	std::vector<int> heights;
	heights.reserve(m_vecPLayers.size());
	for (auto* pLayer : m_vecPLayers)
		heights.push_back(pLayer ? pLayer->GetRequiredHeight() : SingleLayerHeight);
	return heights;
}

int CLayersWidget::GetLayerTopY(int layerIndex) const
{
	int top = 0;
	for (int i = 0; i < layerIndex && i < (int)m_vecPLayers.size(); ++i)
	{
		if (m_vecPLayers[i])
			top += m_vecPLayers[i]->GetRequiredHeight();
	}
	return top;
}

int CLayersWidget::GetLayerIndexAtY(int y) const
{
	const int localY = y - g_pContentViewData->GetYOffset();
	if (localY < 0)
		return -1;
	int cum = 0;
	for (int i = 0; i < (int)m_vecPLayers.size(); ++i)
	{
		const int h = m_vecPLayers[i] ? m_vecPLayers[i]->GetRequiredHeight() : SingleLayerHeight;
		if (localY >= cum && localY < cum + h)
			return i;
		cum += h;
	}
	return -1;
}
