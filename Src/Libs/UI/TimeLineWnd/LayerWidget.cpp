#define _USE_MATH_DEFINES		//解决qmath.h和math.h的头文件冲突
#include "LayerWidget.h"
#include "LayersWidget.h"
#include "ObjectWidget.h"
#include "ClipTransFxWidget.h"
#include "ObjectDragMime.h"
#include "TimeLineOverlapHelper.h"
#include "PathCameraClipHelper.h"
#include "TimeLineGlobalDefine.h"
#include "Common/Common/ResourceDefine.h"
#include "Common/Common/CommonDefine.h"
#include "Common/Common/ChannelMap.h"
#include "Common/Common/TransationTypeDefine.h"

#include "GSMedia/ThreadManager/ThreadManagerExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/TimeLineModel/ConvertHelper.h"
#include "Model/ModelModel/ModelModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Common/Common/IModelDataInterface.h"
#include "Model/GlobalModel/LogModel.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Model/GlobalModel/SoftwareSettingData.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "Model/ScreenModel/ScreenModelExport.h"

#include "UICommon/UITranslate.h"
#include "UICommon/menuex.h"
#include "UICommon/ShowMessageWidget.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QPainter>
#include <QMimeData>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDataStream>
#include <QtMath>
#include <utility>
#include <map>
#include <set>
#include <algorithm>
#include <QContextMenuEvent>
#include <string>
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

CLayerWidget::CLayerWidget(CLayersWidget* pLayersWidget, QWidget* pParent)
	: QWidget(pParent)
	, m_pLayersWidget(pLayersWidget)
{
	setAcceptDrops(true);
	setMouseTracking(false);
	InitLockPixmap();
}

CLayerWidget::~CLayerWidget()
{
}

void CLayerWidget::InitLockPixmap()
{
	int tileSize = SingleLayerHeight - nSizeDefine_1;
	int lineWidth = nSizeDefine_2;
	int spacing = nSizeDefine_2;
	m_pixmapLock = QPixmap(tileSize, tileSize);
	m_pixmapLock.fill(Qt::transparent);

	QPainter tilePainter(&m_pixmapLock);
	tilePainter.setRenderHint(QPainter::Antialiasing, true);
	double diagonal = qSqrt(tileSize * tileSize + tileSize * tileSize);
	tilePainter.translate(tileSize / 2, tileSize / 2);
	tilePainter.rotate(45.0);
	tilePainter.translate(-diagonal / 2, -diagonal / 2);
	int stripeCount = qCeil(diagonal / (lineWidth + spacing)) + 2;
	for (int i = 0; i < stripeCount; ++i)
	{
		int pos = i * (lineWidth + spacing);
		tilePainter.fillRect(QRect(0, pos, diagonal, lineWidth), QColor(255, 255, 255, 90));
	}
}

void CLayerWidget::UpdateView(const QUuid& layerId, int layerIndex)
{
	m_layerId = layerId;
	m_layerIndex = layerIndex;
	CheckObject();
	RefreshTransFxForLayer();
	update();
}

int CLayerWidget::GetRequiredHeight() const
{
	int layerH = SingleLayerHeight;
	for (auto* pWndObj : m_vecObjects)
	{
		if (pWndObj)
			layerH = std::max(layerH, pWndObj->CalcTotalHeight());
	}
	return layerH;
}

void CLayerWidget::LayoutObjects()
{
	if (m_layerId.isNull())
		return;

	int XOffset = g_pContentViewData->GetXOffset();
	auto pLayer = g_pTimeLineModel->GetLayer(m_layerId).lock();
	if (!pLayer)
		return;

	for (auto* pWndObj : m_vecObjects)
	{
		if (!pWndObj)
			continue;
		QUuid objId = pWndObj->GetObjectId();
		auto pObject = pLayer->GetObject(objId).lock();
		if (!pObject)
			continue;
		int x = g_pContentViewData->TimeToPosition(pObject->GetStartTime()) + XOffset;
		int w = g_pContentViewData->TimeToPosition(pObject->GetDuration());
		const int objH = pWndObj->CalcTotalHeight();
		// 先定位素材，再布局子控件；否则TransFx会使用旧的y()导致右半场错位
		pWndObj->setGeometry(x, height() - objH, w, objH);
		pWndObj->LayoutChildWidgets();
	}
}

void CLayerWidget::CheckObject()
{
	auto pLayer = g_pTimeLineModel->GetLayer(m_layerId).lock();
	if (!pLayer)
		return;

	// 媒体与 3D 相机时间线对象均需创建 ObjectWidget
	std::vector<QUuid> objectIds;
	for (const auto& objId : pLayer->GetAllObjectIDs())
	{
		auto pObj = pLayer->GetObject(objId).lock();
		if (!pObj)
			continue;
		const int nType = pObj->GetType();
		if (nType == ITimeLineObject::ObjectType_Media ||
			nType == ITimeLineObject::ObjectType_3DModel)
			objectIds.push_back(objId);
	}

	std::set<QUuid> setExistingIds;
	for (auto* pObj : m_vecObjects)
	{
		QUuid id = pObj->GetObjectId();
		if (!id.isNull())
			setExistingIds.insert(id);
	}

	for (auto it = m_vecObjects.begin(); it != m_vecObjects.end(); )
	{
		CObjectWidget* pObj = *it;
		QUuid id = pObj->GetObjectId();
		bool bNeedDelete = id.isNull() ||
			(std::find(objectIds.begin(), objectIds.end(), id) == objectIds.end());
		if (bNeedDelete)
		{
			if (!id.isNull())
				g_pTimeLineModel->DeleteTransFxByObject(id);
			pObj->ReleaseTransFxWidget();
			pObj->deleteLater();
			it = m_vecObjects.erase(it);
		}
		else
		{
			++it;
		}
	}

	for (const auto& id : objectIds)
	{
		if (setExistingIds.find(id) != setExistingIds.end())
			continue;
		auto* pObj = new CObjectWidget(this);
		m_vecObjects.push_back(pObj);
		pObj->SetObjectId(id);
		pObj->show();
		connect(pObj, &CObjectWidget::signalUpdateView, this, &CLayerWidget::signalUpdateView);
	}
}

void CLayerWidget::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);
	if (m_layerId.isNull())
		return;

	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);

	int drawWidth = g_pContentViewData->TimeToPosition(g_pTimeLineModel->GetDuration()) + g_pContentViewData->GetXOffset();
	drawWidth = std::min(width(), drawWidth);

	QRect rc(0, 0, drawWidth, height() - nSizeDefine_1);
	QColor clr(g_pSelectController->IsTimeLineLayerSelected(m_layerId) ? QColor("#3E4044") : QColor("#303236"));
	p.fillRect(rc, clr);

	QRect rcLine(0, height() - nSizeDefine_1, drawWidth, nSizeDefine_1);
	p.fillRect(rcLine, Qt::black);

	auto pLayer = g_pTimeLineModel->GetLayer(m_layerId).lock();
	if (pLayer)
	{
		if (pLayer->IsLock())
			p.drawTiledPixmap(rc, m_pixmapLock);
		else if (pLayer->IsHide())
			p.fillRect(rc, QColor(48, 50, 54, 127));
	}
}

void CLayerWidget::wheelEvent(QWheelEvent* event)
{
	if (m_pLayersWidget)
		QApplication::sendEvent(m_pLayersWidget, event);
	else
		QWidget::wheelEvent(event);
}

bool CLayerWidget::ShouldForwardMouseToLayers(const QPoint& pos) const
{
	QWidget* pChild = childAt(pos);
	if (pChild == nullptr)
		return true;

	if (auto* pTransFx = qobject_cast<CClipTransFxWidget*>(pChild))
	{
		// childAt 可能命中转场，但点击在图层空白时应走图层背景逻辑
		if (!HitTestTransFxWidgetAt(pos))
			return true;
		return false;
	}

	auto* pObjWidget = qobject_cast<CObjectWidget*>(pChild);
	if (!pObjWidget)
		return false;

	if (m_layerId.isNull())
		return true;

	auto pLayer = g_pTimeLineModel->GetLayer(m_layerId).lock();
	if (!pLayer)
		return true;

	auto pObject = pLayer->GetObject(pObjWidget->GetObjectId()).lock();
	if (!pObject)
		return true;

	const int XOffset = g_pContentViewData->GetXOffset();
	const int modelX = g_pContentViewData->TimeToPosition(pObject->GetStartTime()) + XOffset;
	const int modelW = g_pContentViewData->TimeToPosition(pObject->GetDuration());
	if (pos.x() < modelX || pos.x() >= modelX + modelW)
		return true;

	return false;
}

void CLayerWidget::ForwardMouseEventToLayers(QMouseEvent* event)
{
	if (!m_pLayersWidget)
		return;
	QMouseEvent mappedEvent(
		event->type(),
		m_pLayersWidget->mapFromGlobal(event->globalPos()),
		event->globalPos(),
		event->button(),
		event->buttons(),
		event->modifiers());
	QApplication::sendEvent(m_pLayersWidget, &mappedEvent);
}

void CLayerWidget::HandleLayerBackgroundMousePress(QMouseEvent* event)
{
	if (event->button() != Qt::LeftButton || m_layerId.isNull())
		return;

	auto pLayer = g_pTimeLineModel->GetLayer(m_layerId).lock();
	if (!pLayer || pLayer->IsLock())
		return;

	CClipTransFxWidget::ClearRecentPressState();
	g_pSelectController->ClearTransFxSelect();
	g_pSelectController->AddTimeLineLayerSelect(m_layerId, true);
	g_pSelectController->ClearTimeLineObjectSelect();

	QUuid layerId = m_layerId;
	emit g_pSignalCenter->signalLayerSelectChanged(layerId);
	emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_LayersView | WndType::WndType_TimeLineTool);
	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_LayerSelect);
}

QUuid CLayerWidget::FindObjectIdAtLayerPos(const QPoint& posInLayer) const
{
	auto pLayer = g_pTimeLineModel->GetLayer(m_layerId).lock();
	if (!pLayer)
		return QUuid();
	const int xOffset = g_pContentViewData->GetXOffset();
	for (const auto& objectId : pLayer->GetAllObjectIDs())
	{
		auto pObject = pLayer->GetObject(objectId).lock();
		if (!pObject)
			continue;
		const int modelX = g_pContentViewData->TimeToPosition(pObject->GetStartTime()) + xOffset;
		const int modelW = g_pContentViewData->TimeToPosition(pObject->GetDuration());
		if (posInLayer.x() >= modelX && posInLayer.x() < modelX + modelW)
			return objectId;
	}
	return QUuid();
}

void CLayerWidget::mousePressEvent(QMouseEvent* event)
{
	if (ShouldForwardMouseToLayers(event->pos()))
	{
		HandleLayerBackgroundMousePress(event);
		ForwardMouseEventToLayers(event);
		return;
	}
	QWidget::mousePressEvent(event);
}

void CLayerWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (ShouldForwardMouseToLayers(event->pos()))
	{
		ForwardMouseEventToLayers(event);
		return;
	}
	QWidget::mouseMoveEvent(event);
}

void CLayerWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (HitTestTransFxWidgetAt(event->pos()))
	{
		QWidget::mouseReleaseEvent(event);
		return;
	}
	if (ShouldForwardMouseToLayers(event->pos()))
	{
		ForwardMouseEventToLayers(event);
		return;
	}
	QWidget::mouseReleaseEvent(event);
}

bool CLayerWidget::TryCommitObjectDrop(const QMimeData* pMime, const QPoint& posInLayer)
{
	if (!pMime || !m_pLayersWidget)
		return false;
	auto payload = CObjectDragMime::ParseObjectDragMime(pMime);
	if (!payload.has_value())
		return false;
	if (!TimeLineOverlapHelper::CanCommitObjectDrag(*payload, m_layerIndex, posInLayer, m_pLayersWidget))
		return false;

	const bool bPathClipSkipped = TimeLineOverlapHelper::IsObjectDragBlockedByPathClipOverlap(
		*payload, m_layerIndex, posInLayer, m_pLayersWidget);
	const bool bOk = CObjectDragMime::CommitObjectDrag(*payload, m_layerIndex, posInLayer, m_pLayersWidget);
	// 路径片段因同相机重叠保持原位时提示；媒体仍可能已成功移动
	if (bPathClipSkipped)
		CShowMessageWidget::ShowMessage(3000, CTimeLineWndTranslate::PathClipOverlapTip(), nullptr, ESMT_Warning);
	return bOk;
}

void CLayerWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(g_TimelineObjectDragFormat))
	{
		if (m_pLayersWidget)
			m_pLayersWidget->PreviewTimelineObjectDrag(event->mimeData(), event->pos());
		event->setDropAction(Qt::MoveAction);
		event->accept();
		return;
	}
	auto pLayer = g_pTimeLineModel->GetLayer(m_layerId).lock();
	if (pLayer && pLayer->IsLock())
		return;
	if (event->mimeData()->hasFormat("ResourceItem") || event->mimeData()->hasFormat(ModelObjectIDMimeType))
	{
		event->setDropAction(Qt::MoveAction);
		event->accept();
	}
}

void CLayerWidget::dragMoveEvent(QDragMoveEvent* event)
{
	if (event->mimeData()->hasFormat(g_TimelineObjectDragFormat))
	{
		if (m_pLayersWidget)
			m_pLayersWidget->PreviewTimelineObjectDrag(event->mimeData(), event->pos());
		event->acceptProposedAction();
		return;
	}
	HandleResourceDragMove(event);
}

void CLayerWidget::dragLeaveEvent(QDragLeaveEvent * event)
{
	g_pContentViewData->ClearObjectDragPreview();
	emit signalUpdateView(WndType::WndType_LayersControl | WndType::WndType_TimeLineTool);
}

void CLayerWidget::dropEvent(QDropEvent* event)
{
	HandleDropEvent(event);
}

void CLayerWidget::contextMenuEvent(QContextMenuEvent * event)
{
	// 刚发生过右键拖动平移：吞掉一次Layer菜单事件
	if (m_pLayersWidget && m_pLayersWidget->BlockLayerContextMenu())
	{
		event->accept();
		return;
	}

	if (m_layerId.isNull())
	{
		QWidget::contextMenuEvent(event);
		return;
	}

	auto pLayer = g_pTimeLineModel->GetLayer(m_layerId).lock();
	if (!pLayer || pLayer->IsLock())
	{
		QWidget::contextMenuEvent(event);
		return;
	}

	// 点在对象上，不弹Layer菜单，避免与ObjectWidget菜单冲突
	const QUuid hitObjId = FindObjectIdAtLayerPos(event->pos());
	if (!hitObjId.isNull())
	{
		event->ignore();
		return;
	}

	CMenuEx* pRBtnMenu = new CMenuEx(this);
	pRBtnMenu->setFixedWidth(nSizeDefine_148);

	auto pCopy = pRBtnMenu->addAction(CCommonTranslate::Copy());
	auto pPaste = pRBtnMenu->addAction(CCommonTranslate::Paste());
	pPaste->setEnabled(g_pContentViewData->GetTimelineCopyData().HasData());

	const auto pSelect = pRBtnMenu->exec(event->globalPos());
	if (pSelect == pCopy)
	{
		// 优先复制当前已有多选；若无多选则尝试按右键位置命中对象后复制
		if (!g_pSelectController->HasTimeLineObjectSelected())
		{
			const QUuid hitObjIdCopy = FindObjectIdAtLayerPos(event->pos());
			if (!hitObjIdCopy.isNull())
			{
				g_pSelectController->ClearTimeLineLayerSelect();
				g_pSelectController->ClearTimeLineObjectSelect();
				g_pSelectController->AddTimeLineLayerSelect(m_layerId, true);
				g_pSelectController->AddTimeLineObjectSelect(hitObjIdCopy, true);
			}
		}
		g_pContentViewData->CopySelectedObjects();
	}
	if (pSelect == pPaste)
	{
		if (!TimeLineOverlapHelper::CanPasteObjectsToLayer(event->pos(), m_layerIndex))
		{
			if (TimeLineOverlapHelper::IsPasteBlockedByPathClipOverlap(event->pos(), m_layerIndex))
			{
				CShowMessageWidget::ShowMessage(3000, CTimeLineWndTranslate::PathClipOverlapTip(), nullptr, ESMT_Warning);
			}
		}
		else
		{
			g_pContentViewData->PasteObjectsToLayer(event->pos(), m_layerIndex);
			if (m_pLayersWidget)
			{
				m_pLayersWidget->UpdateView();
				m_pLayersWidget->NotifyAfterObjectDrag();
			}
		}
	}

	pRBtnMenu->deleteLater();
	event->accept();
}

void CLayerWidget::HandleDropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat(g_TimelineObjectDragFormat))
	{
		if (TryCommitObjectDrop(event->mimeData(), event->pos()))
			event->acceptProposedAction();
		return;
	}
	HandleResourceDrop(event, event->pos());
}

void CLayerWidget::HandleResourceDropAt(QDropEvent* event, const QPoint& posInLayer)
{
	HandleResourceDrop(event, posInLayer);
}

void CLayerWidget::HandleResourceDragMove(QDragMoveEvent* event)
{
	auto pLayer = g_pTimeLineModel->GetLayer(m_layerId).lock();
	if (!pLayer || pLayer->IsLock())
	{
		g_pContentViewData->ClearObjectDragPreview();
		event->ignore();
		return;
	}

	const QPoint posInLayer = event->pos();
	const int posLayersX = posInLayer.x();
	const auto starttime = g_pContentViewData->PositionToTime(posLayersX - g_pContentViewData->GetXOffset());

	if (event->mimeData()->hasFormat("ResourceItem"))
	{
		g_pSelectController->ClearTimeLineObjectSelect();
		g_pSelectController->ClearTimeLineLayerSelect();

		const QUuid hitObjectId = FindObjectIdAtLayerPos(posInLayer);
		event->accept();
		g_pSelectController->AddTimeLineLayerSelect(m_layerId, true);
		if (!hitObjectId.isNull())
			g_pSelectController->AddTimeLineObjectSelect(hitObjectId, true);

		QByteArray itData = event->mimeData()->data("ResourceItem");
		QDataStream dataStream(&itData, QIODevice::ReadOnly);
		QList<QUuid> listItemId;
		QList<QString> listNames;
		dataStream >> listItemId >> listNames;
		if (listItemId.isEmpty())
		{
			event->ignore();
			return;
		}
		const QUuid resourceId = listItemId.first();
		(void)resourceId;

		auto resourceNode = g_pResourceModel->GetResourceNode(listItemId.first());
		if (resourceNode)
		{
			const auto duration = resourceNode->GetDuration();
			if (auto pHitObject = pLayer->GetObject(hitObjectId).lock())
			{
				if (pHitObject->GetType() != ITimeLineObject::ObjectType_Media)
					event->ignore();
			}
			else
			{
				for (const auto& objectId : pLayer->GetAllObjectIDs())
				{
					auto pObjectOther = pLayer->GetObject(objectId).lock();
					if (pObjectOther &&
						pObjectOther->GetStartTime() <= starttime + duration &&
						pObjectOther->GetStartTime() >= starttime)
					{
						if ((pObjectOther->GetStartTime() - starttime).count() < 1000)
							event->ignore();
						break;
					}
					else if (pObjectOther && pObjectOther->GetStartTime() >= starttime)
						break;
				}
			}
		}

		g_pContentViewData->SetSnap(false);
		const int snapPos = g_pContentViewData->TimeToPosition(g_pPlayerController->GetCurTime()) + g_pContentViewData->GetXOffset();
		if (abs(snapPos - posLayersX) < nSizeDefine_10)
			g_pContentViewData->SetSnap(true, snapPos);

		CObjectDragMime::PreviewResourceDragSnap(starttime, posLayersX, event->keyboardModifiers());
		g_pContentViewData->SetObjectDragPreviewTime(starttime);

		emit signalUpdateView(WndType::WndType_LayersView | WndType::WndType_LayerLabels |
			WndType::WndType_NoniusView | WndType::WndType_TimeLineTool | WndType::WndType_LayersControl );
		emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	}
	else if (event->mimeData()->hasFormat(ModelObjectIDMimeType))
	{
		// 路径相机片段拖入：校验相机类型、同层间隙、同相机路径片段不重叠
		g_pSelectController->ClearTimeLineObjectSelect();
		g_pSelectController->ClearTimeLineLayerSelect();

		const QUuid hitObjectId = FindObjectIdAtLayerPos(posInLayer);
		g_pSelectController->AddTimeLineLayerSelect(m_layerId, true);
		if (!hitObjectId.isNull())
			g_pSelectController->AddTimeLineObjectSelect(hitObjectId, true);

		QByteArray itData = event->mimeData()->data(ModelObjectIDMimeType);
		QDataStream dataStream(&itData, QIODevice::ReadOnly);
		QList<QUuid> listItemId;
		QList<QString> listNames;
		dataStream >> listItemId >> listNames;
		if (listItemId.isEmpty())
		{
			event->ignore();
			return;
		}

		const QUuid modelId = listItemId.first();
		auto pDragModel = g_pModelDataModel->GetModel(modelId).lock();
		auto pDragCamera = std::dynamic_pointer_cast<IModelCamera>(pDragModel);
		const auto duration = std::chrono::milliseconds(g_pProjectSetting->LoadDefaultCameraAnimDuration());
		bool bAccept = true;
		if (!pDragModel || !pDragCamera || pDragModel->GetModelType() != EMTNT_Camera)
		{
			bAccept = false;
		}
		else if (auto pHitObject = pLayer->GetObject(hitObjectId).lock())
		{
			bAccept = false;
		}
		else
		{
			for (const auto& objectId : pLayer->GetAllObjectIDs())
			{
				auto pObjectOther = pLayer->GetObject(objectId).lock();
				if (pObjectOther &&
					pObjectOther->GetStartTime() <= starttime + duration &&
					pObjectOther->GetStartTime() >= starttime)
				{
					if ((pObjectOther->GetStartTime() - starttime).count() < 1000)
						bAccept = false;
					break;
				}
				else if (pObjectOther && pObjectOther->GetStartTime() >= starttime)
					break;
			}
			if (bAccept && PathCameraClipHelper::IsPathClipRangeOverlap(starttime, duration, QUuid(), modelId))
				bAccept = false;
		}

		if (!bAccept)
		{
			event->ignore();
			return;
		}

		event->accept();
		g_pContentViewData->SetSnap(false);
		const int snapPos = g_pContentViewData->TimeToPosition(g_pPlayerController->GetCurTime()) + g_pContentViewData->GetXOffset();
		if (abs(snapPos - posLayersX) < nSizeDefine_10)
			g_pContentViewData->SetSnap(true, snapPos);

		CObjectDragMime::PreviewResourceDragSnap(starttime, posLayersX, event->keyboardModifiers());
		g_pContentViewData->SetObjectDragPreviewTime(starttime);

		emit signalUpdateView(WndType::WndType_LayersView | WndType::WndType_LayerLabels |
			WndType::WndType_NoniusView | WndType::WndType_TimeLineTool | WndType::WndType_LayersControl );
	}
}

void CLayerWidget::HandleResourceDrop(QDropEvent* event, const QPoint& posInLayer)
{
	auto pLayer = g_pTimeLineModel->GetLayer(m_layerId).lock();
	if (!pLayer || pLayer->IsLock())
	{
		g_pContentViewData->ClearObjectDragPreview();
		event->ignore();
		return;
	}

	const int posLayersX = posInLayer.x();
	auto starttime = g_pContentViewData->PositionToTime(posLayersX - g_pContentViewData->GetXOffset());
	const QUuid hitObjectId = FindObjectIdAtLayerPos(posInLayer);

	if (event->mimeData()->hasFormat(ResMimeType))
	{
		g_pContentViewData->SetSnap(false);
		emit signalUpdateView(WndType::WndType_NoniusView);

		QByteArray itData = event->mimeData()->data(ResMimeType);
		QDataStream dataStream(&itData, QIODevice::ReadOnly);
		QList<QUuid> listItemId;
		QList<QString> listNames;
		dataStream >> listItemId >> listNames;
		if (listItemId.isEmpty())
		{
			event->ignore();
			return;
		}
		const QUuid resourceId = listItemId.first();
		const QString resourceName = listNames.isEmpty() ? QString() : listNames.first();

		if (auto pOldbject = pLayer->GetObject(hitObjectId).lock())
		{
			auto pMedia = g_pMediaModel->GetMediaNode(pOldbject->GetId()).lock();
			if (pMedia)
			{
				pMedia->SetResourceId(resourceId);
				pMedia->SetMediaName(resourceName);
				if (auto pResource = g_pResourceModel->GetResourceNode(resourceId))
				{
					pMedia->SetClipEndTime(std::chrono::milliseconds(pResource->GetDuration()));
					if (pMedia->GetClipStartTime().count() + 1000 > pMedia->GetClipEndTime().count())
						pMedia->SetClipStartTime(pMedia->GetClipEndTime() - std::chrono::milliseconds(1000));
					pMedia->SetUsedChannelMap(false);
					pMedia->SetAudioChannelMap(SChannelMap::Default(pResource->GetAudioChannels()));
				}
				g_pPlayerController->ChangePlayer(pOldbject->GetId(), resourceId);
				if (g_pPlayerController->IsPlaying())
					g_pPlayerController->Pause();
				g_pPlayerController->SeekObject(pOldbject->GetId());
				g_pSelectController->AddTimeLineLayerSelect(m_layerId, true);
				g_pSelectController->AddTimeLineObjectSelect(pOldbject->GetId(), true);
				emit signalUpdateView(WndType::WndType_TimeLineTool );
				emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
				emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
				HistoryStackSave();
				if (m_pLayersWidget)
					m_pLayersWidget->UpdateView();
				event->accept();
				LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewContent(),
					CCommonTranslate::Replace(), QString(pMedia->GetMediaName() + "->" + resourceName));
			}
		}
		else
		{
			QUuid id = QUuid::createUuid();
			auto pObject = g_pTimeLineModel->CreateObject(id, ITimeLineObject::ObjectType_Media);
			starttime = CObjectDragMime::CalcResourceDropStartTime(starttime, posLayersX, event->keyboardModifiers());
			pObject->SetStartTime(starttime);

			QRect rc;
			if (auto resourceNode = g_pResourceModel->GetResourceNode(resourceId))
			{
				int nDuration = 0;
				const int nResType = resourceNode->GetType();
				if (nResType == ERT_Picture)
					nDuration = g_pProjectSetting->LoadDefaultPicDuration();
				else if (IsToolResource(nResType))
					nDuration = g_pProjectSetting->LoadDefaultToolDuration();
				else if (IsStreamResource(nResType))
					nDuration = g_pProjectSetting->LoadDefaultStreamDuration();
				else
					nDuration = resourceNode->GetDuration();
				resourceNode->SetDuration(nDuration);
				pObject->SetDuration(std::chrono::milliseconds(nDuration));
				rc = QRect(QPoint(0, 0), resourceNode->GetResolution());
			}

			for (const auto& objectId : pLayer->GetAllObjectIDs())
			{
				auto pObjectOther = pLayer->GetObject(objectId).lock();
				if (pObjectOther &&
					pObjectOther->GetStartTime() <= pObject->GetStartTime() + pObject->GetDuration() &&
					pObjectOther->GetStartTime() >= pObject->GetStartTime())
				{
					pObject->SetDuration(pObjectOther->GetStartTime() - pObject->GetStartTime());
				}
			}

			QRect rcScreen;
			if (ScreenModel::GetFirstSreenRect(true, rcScreen))
				rc = rcScreen;

			if (pObject->GetDuration().count() >= 1000)
			{
				// 先 AddMedia 再 AddObject，保证播放端收到 AddLayerObj 时 MediaNode 已存在
				MediaModel::AddMedia(id, resourceId, rc);
				pLayer->AddObject(g_pTimeLineModel->GetId(), pObject);
				if (g_pTimeLineModel->GetMaxUsefulDuration() > g_pTimeLineModel->GetDuration())
					g_pTimeLineModel->SetMaxUsefulDuration();
				if (g_pPlayerController->IsPlaying())
					g_pPlayerController->Pause();
				g_pPlayerController->SeekObject(id);
				g_pSelectController->AddTimeLineLayerSelect(m_layerId, true);
				g_pSelectController->AddTimeLineObjectSelect(id, true);
				HistoryStackSave();
				if (m_pLayersWidget)
					m_pLayersWidget->UpdateView();
				event->accept();
				if (auto pMedia = g_pMediaModel->GetMediaNode(pObject->GetId()).lock())
				{
					LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewContent(),
						CCommonTranslate::Add(), pMedia->GetMediaName());
				}
			}
		}
		if (m_pLayersWidget)
			m_pLayersWidget->setFocus();
	}
	else if (event->mimeData()->hasFormat(ModelObjectIDMimeType))
	{
		// Timeline：不支持路径相机拖入，忽略模型对象 MIME
		event->ignore();
		return;
	}

	g_pContentViewData->ClearObjectDragPreview();
	emit signalUpdateView(WndType::WndType_TimeLineTool | WndType::WndType_LayersControl );
}

// 清理图层上已无对应素材引用的孤儿转场控件（父控件为 CLayerWidget 的直接子控件）
void CLayerWidget::CleanupOrphanTransFxWidgets()
{
	std::set<CClipTransFxWidget*> activeWidgets;
	for (auto* pObj : m_vecObjects)
	{
		if (!pObj)
			continue;
		for (auto* pTransFx : pObj->GetClipTransFxWidgets())
		{
			if (pTransFx)
				activeWidgets.insert(pTransFx);
		}
	}

	const auto children = findChildren<CClipTransFxWidget*>(QString(), Qt::FindDirectChildrenOnly);
	for (auto* pTransFx : children)
	{
		if (!pTransFx || activeWidgets.count(pTransFx) > 0)
			continue;
		const QRect rc = pTransFx->geometry();
		pTransFx->hide();
		pTransFx->deleteLater();
		update(rc);
	}
}

// 在图层坐标下命中测试转场控件（优先检测 Z 序较高的直接子控件）
CClipTransFxWidget* CLayerWidget::HitTestTransFxWidgetAt(const QPoint& posInLayer) const
{
	const QObjectList layerChildren = children();
	for (int i = layerChildren.size() - 1; i >= 0; --i)
	{
		auto* pTransFx = qobject_cast<CClipTransFxWidget*>(layerChildren[i]);
		if (!pTransFx || !pTransFx->isVisible())
			continue;
		const QPoint posInTransFx = pTransFx->mapFrom(this, posInLayer);
		if (pTransFx->rect().contains(posInTransFx))
			return pTransFx;
	}

	for (auto* pObj : m_vecObjects)
	{
		if (!pObj)
			continue;
		for (auto* pTransFx : pObj->GetClipTransFxWidgets())
		{
			if (!pTransFx || !pTransFx->isVisible())
				continue;
			const QPoint posInTransFx = pTransFx->mapFrom(this, posInLayer);
			if (pTransFx->rect().contains(posInTransFx))
				return pTransFx;
		}
	}
	return nullptr;
}

// 将所有可见转场控件置于素材之上，避免被 CObjectWidget 遮挡
void CLayerWidget::RaiseTransFxWidgets()
{
	for (auto* pObj : m_vecObjects)
	{
		if (!pObj)
			continue;
		for (auto* pTransFx : pObj->GetClipTransFxWidgets())
		{
			if (pTransFx && pTransFx->isVisible())
				pTransFx->raise();
		}
	}
}

// 仅重新布局转场（不移动素材本体几何），用于拉伸预览等场景
void CLayerWidget::LayoutTransFxOnly()
{
	for (auto* pObj : m_vecObjects)
	{
		if (pObj)
			pObj->LayoutChildWidgets();
	}

	CleanupOrphanTransFxWidgets();
	RaiseTransFxWidgets();
	update();
}

// 读取指定转场 ID 当前左右半场的像素宽度（来自已布局的 CClipTransFxWidget）
std::pair<int, int> CLayerWidget::GetTransFxHalfWidthsPx(const QUuid& transFxId) const
{
	int leftHalfPx = 0;
	int rightHalfPx = 0;
	if (transFxId.isNull())
		return { leftHalfPx, rightHalfPx };

	for (auto* pObj : m_vecObjects)
	{
		if (!pObj)
			continue;

		for (auto* pWnd : pObj->GetClipTransFxWidgets())
		{
			if (!pWnd || pWnd->GetTransFxId() != transFxId)
				continue;
			auto pTransFx = g_pTimeLineModel->GetTransFx(transFxId).lock();
			if (!pTransFx)
				continue;
			if (pObj->GetObjectId() == pTransFx->GetLeftObjectId())
				leftHalfPx = pWnd->width();
			else if (pObj->GetObjectId() == pTransFx->GetRightObjectId())
				rightHalfPx = pWnd->width();
		}
	}
	return { leftHalfPx, rightHalfPx };
}

// 拉伸拖拽过程中的预览布局：显式指定左右半场宽度与交界点图层 X 坐标
void CLayerWidget::PreviewTransFxHalfWidths(const QUuid& transFxId, int leftHalfPx, int rightHalfPx,
	int previewJunctionLayerPx)
{
	if (transFxId.isNull() || leftHalfPx < 0 || rightHalfPx < 0)
		return;

	for (auto* pObj : m_vecObjects)
	{
		if (!pObj)
			continue;
		const auto transFxs = g_pTimeLineModel->GetTransFxsByObject(pObj->GetObjectId());
		bool bInvolved = false;
		for (const auto& pTransFx : transFxs)
		{
			if (pTransFx && pTransFx->GetId() == transFxId)
			{
				bInvolved = true;
				break;
			}
		}
		if (!bInvolved)
			continue;
		pObj->LayoutChildWidgets(leftHalfPx, rightHalfPx, previewJunctionLayerPx, transFxId);
	}
	RaiseTransFxWidgets();
}

// 根据图层内素材重叠关系同步转场模型，并刷新 UI 布局
void CLayerWidget::RefreshTransFxForLayer()
{
	if (m_layerId.isNull())
		return;
	// 1. 检测当前图层所有有效重叠对（左素材 start <= 右素材 start）
	const auto pairs = TimeLineOverlapHelper::DetectLayerOverlaps(m_layerId);
	std::set<std::pair<QUuid, QUuid>> validPairs;
	for (const auto& pair : pairs)
		validPairs.insert({ pair.leftObjectId, pair.rightObjectId });
	// 2. 删除已无重叠的转场数据
	const auto& allTransFxs = g_pTimeLineModel->GetAllTransFxs();
	std::vector<QUuid> toDelete;
	for (const auto& pTransFx : allTransFxs)
	{
		if (!pTransFx || pTransFx->GetLayerId() != m_layerId)
			continue;
		if (validPairs.find({ pTransFx->GetLeftObjectId(), pTransFx->GetRightObjectId() }) == validPairs.end())
			toDelete.push_back(pTransFx->GetId());
	}
	for (const auto& id : toDelete)
		g_pTimeLineModel->DeleteTransFx(id);
	// 3. 为每个重叠对创建或更新转场；Duration 表示完整重叠时长（Wl + Wr）
	for (const auto& pair : pairs)
	{
		auto pTransFx = g_pTimeLineModel->FindTransFxByObjects(pair.leftObjectId, pair.rightObjectId).lock();
		const auto maxDuration = pair.overlapDuration;
		if (pTransFx)
		{
			pTransFx->SetLayerId(m_layerId);
			pTransFx->SetLeftObjectId(pair.leftObjectId);
			pTransFx->SetRightObjectId(pair.rightObjectId);
			// 重叠区变化时同步完整重叠时长
			pTransFx->SetDuration(maxDuration);
		}
		else
		{
			pTransFx = g_pTimeLineModel->CreateTransFx();
			pTransFx->SetLayerId(m_layerId);
			pTransFx->SetLeftObjectId(pair.leftObjectId);
			pTransFx->SetRightObjectId(pair.rightObjectId);
			pTransFx->SetDuration(maxDuration);
			pTransFx->SetTransType(ETFT_Fade);
			g_pTimeLineModel->AddTransFx(pTransFx);
		}
	}
	LayoutTransFxWidgets();
}

// 完整刷新：先布局素材，再布局转场
void CLayerWidget::LayoutTransFxWidgets()
{
	LayoutObjects();
	LayoutTransFxOnly();
}
