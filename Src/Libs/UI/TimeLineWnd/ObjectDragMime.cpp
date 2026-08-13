#include "ObjectDragMime.h"
#include "ObjectWidget.h"
#include "LayersWidget.h"
#include "ContentViewData.h"
#include "TimeLineGlobalDefine.h"
#include "PathCameraClipHelper.h"
#include "TimeLineOverlapHelper.h"

#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Common/SignalCenter/GSNetSignalCenter.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"
#include "Controll/PlayCommandCenter/PlayCommandCenterExport.h"

#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "UICommon/UITranslate.h"

#include <QApplication>
#include <QDataStream>
#include <QPainter>
#include <algorithm>
#include <set>
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
	CObjectWidget* FindObjectWidget(CLayersWidget* pLayers, const QUuid& id)
	{
		if (!pLayers)
			return nullptr;
		for (auto* pObj : pLayers->GetObjects())
		{
			if (pObj && pObj->GetObjectId() == id)
				return pObj;
		}
		return nullptr;
	}

	struct SSnapCandidate
	{
		bool bFound = false;
		std::chrono::milliseconds snapTime{};
		std::chrono::milliseconds distance{};
	};

	SSnapCandidate FindNearestSnap(std::chrono::milliseconds edgeTime,
		const std::set<std::chrono::milliseconds>& snapTimes,
		std::chrono::milliseconds threshold)
	{
		SSnapCandidate best;
		for (const auto& t : snapTimes)
		{
			const std::chrono::milliseconds diff = (t > edgeTime) ? (t - edgeTime) : (edgeTime - t);
			if (diff < threshold && (!best.bFound || diff < best.distance))
			{
				best.bFound = true;
				best.snapTime = t;
				best.distance = diff;
			}
		}
		return best;
	}

	std::chrono::milliseconds GetObjectDuration(const QUuid& objId)
	{
		auto pLayer = g_pTimeLineModel->GetLayer(objId).lock();
		if (!pLayer)
			return std::chrono::milliseconds(0);
		auto pObj = pLayer->GetObject(objId).lock();
		return pObj ? pObj->GetDuration() : std::chrono::milliseconds(0);
	}

	int TimeToSnapLinePos(std::chrono::milliseconds time)
	{
		return static_cast<int>(g_pContentViewData->TimeToPosition(time) + g_pContentViewData->GetXOffset());
	}
}

QMimeData* CObjectDragMime::BuildObjectDragMime(CObjectWidget* pAnchor, CLayersWidget* pLayers, const QPoint& pressHotSpot)
{
	if (!pAnchor || !pLayers)
		return nullptr;

	STimelineObjectDragPayload payload;
	payload.anchorId = pAnchor->GetObjectId();
	payload.hotSpot = pressHotSpot;

	const bool bMulti = g_pSelectController->IsTimeLineObjectSelected(payload.anchorId)
		&& g_pSelectController->GetTimeLineObjectSelectedCount() > 1;

	if (bMulti)
	{
		for (const auto& id : g_pSelectController->GetTLObjectSelectIds())
		{
			auto pLayer = g_pTimeLineModel->GetLayer(id).lock();
			if (!pLayer)
				continue;
			auto pObj = pLayer->GetObject(id).lock();
			if (!pObj)
				continue;
			// 媒体与路径相机片段均可参与同批拖拽（路径片段为 ObjectType_3DModel）
			const bool bMedia = pObj->GetType() == ITimeLineObject::ObjectType_Media;
			const bool bPathClip = PathCameraClipHelper::IsPathCameraClip(id);
			if (!bMedia && !bPathClip)
				continue;
			payload.objectIds.push_back(id);
		}
	}
	else
	{
		payload.objectIds.push_back(payload.anchorId);
	}

	if (payload.objectIds.isEmpty())
		return nullptr;

	auto pAnchorLayer = g_pTimeLineModel->GetLayer(payload.anchorId).lock();
	auto pAnchorObj = pAnchorLayer ? pAnchorLayer->GetObject(payload.anchorId).lock() : nullptr;
	if (!pAnchorLayer || !pAnchorObj)
		return nullptr;

	const int anchorLayerIndex = g_pTimeLineModel->GetLayerIndex(pAnchorLayer->GetId());
	const auto anchorStartTime = pAnchorObj->GetStartTime();

	payload.timeOffsetMs.resize(payload.objectIds.size());
	payload.layerIndexOffset.resize(payload.objectIds.size());
	for (int i = 0; i < payload.objectIds.size(); ++i)
	{
		const QUuid& id = payload.objectIds[i];
		auto pLayer = g_pTimeLineModel->GetLayer(id).lock();
		auto pObj = pLayer ? pLayer->GetObject(id).lock() : nullptr;
		if (!pLayer || !pObj)
		{
			payload.timeOffsetMs[i] = 0;
			payload.layerIndexOffset[i] = 0;
			continue;
		}
		payload.timeOffsetMs[i] = pObj->GetStartTime().count() - anchorStartTime.count();
		payload.layerIndexOffset[i] = g_pTimeLineModel->GetLayerIndex(pLayer->GetId()) - anchorLayerIndex;
	}

	QByteArray bytes;
	QDataStream stream(&bytes, QIODevice::WriteOnly);
	stream << payload.anchorId << payload.hotSpot << payload.objectIds
		<< payload.timeOffsetMs << payload.layerIndexOffset;

	auto* pMime = new QMimeData();
	pMime->setData(g_TimelineObjectDragFormat, bytes);
	return pMime;
}

std::optional<STimelineObjectDragPayload> CObjectDragMime::ParseObjectDragMime(const QMimeData* pMime)
{
	if (!pMime || !pMime->hasFormat(g_TimelineObjectDragFormat))
		return std::nullopt;

	QByteArray bytes = pMime->data(g_TimelineObjectDragFormat);
	QDataStream stream(bytes);
	STimelineObjectDragPayload payload;
	stream >> payload.anchorId >> payload.hotSpot >> payload.objectIds
		>> payload.timeOffsetMs >> payload.layerIndexOffset;

	if (payload.objectIds.isEmpty())
		return std::nullopt;
	if (payload.timeOffsetMs.size() != payload.objectIds.size()
		|| payload.layerIndexOffset.size() != payload.objectIds.size())
		return std::nullopt;

	return payload;
}

QPixmap CObjectDragMime::CreateObjectDragPixmap(CObjectWidget* pAnchor, CLayersWidget* pLayers, const STimelineObjectDragPayload& payload, QPoint& outHotSpot)
{
	if (!pAnchor || !pLayers)
		return QPixmap();

	if (payload.objectIds.size() <= 1)
	{
		outHotSpot = payload.hotSpot;
		return pAnchor->grab();
	}

	struct SItem
	{
		CObjectWidget* pWidget = nullptr;
		int dx = 0;
		int dy = 0;
		int w = 0;
		int h = 0;
	};

	std::vector<SItem> items;
	int minX = 0, minY = 0, maxX = 0, maxY = 0;
	bool bFirst = true;

	for (int i = 0; i < payload.objectIds.size(); ++i)
	{
		auto* pWidget = FindObjectWidget(pLayers, payload.objectIds[i]);
		if (!pWidget)
			continue;

		const int dx = g_pContentViewData->TimeToPosition(std::chrono::milliseconds(payload.timeOffsetMs[i]));
		const int w = pWidget->width();
		// 优先用 CalcTotalHeight，避免几何未同步时 height() 偏小导致幽灵图对齐错误
		const int h = pWidget->CalcTotalHeight();
		// ClipFx 在素材条上方展开：按素材条底边对齐，而不是按控件顶对齐
		const int fxAboveMedia = h - SingleLayerHeight;
		const int dy = payload.layerIndexOffset[i] * SingleLayerHeight - fxAboveMedia;

		items.push_back({ pWidget, dx, dy, w, h });

		if (bFirst)
		{
			minX = dx;
			minY = dy;
			maxX = dx + w;
			maxY = dy + h;
			bFirst = false;
		}
		else
		{
			minX = std::min(minX, dx);
			minY = std::min(minY, dy);
			maxX = std::max(maxX, dx + w);
			maxY = std::max(maxY, dy + h);
		}
	}

	if (items.empty())
	{
		outHotSpot = payload.hotSpot;
		return pAnchor->grab();
	}

	const int pad = nSizeDefine_4;
	QPixmap composite(maxX - minX + pad * 2, maxY - minY + pad * 2);
	composite.fill(Qt::transparent);

	QPainter painter(&composite);
	for (const auto& item : items)
	{
		const QPixmap pix = item.pWidget->grab();
		const int x = item.dx - minX + pad;
		const int y = item.dy - minY + pad;
		const bool bAnchor = item.pWidget->GetObjectId() == payload.anchorId;
		if (!bAnchor)
			painter.setOpacity(0.9f);
		painter.drawPixmap(x, y, pix);
		if (!bAnchor)
			painter.setOpacity(1.0f);
	}
	painter.end();

	int anchorDx = 0, anchorDy = 0;
	for (const auto& item : items)
	{
		if (item.pWidget->GetObjectId() == payload.anchorId)
		{
			anchorDx = item.dx;
			anchorDy = item.dy;
			break;
		}
	}
	outHotSpot = QPoint(anchorDx - minX + pad + payload.hotSpot.x(), anchorDy - minY + pad + payload.hotSpot.y());
	return composite;
}

SObjectDragSnapResult CObjectDragMime::CalcObjectDragSnap(const STimelineObjectDragPayload& payload, const QPoint& posInLayer, CLayersWidget* pLayers)
{
	SObjectDragSnapResult result;
	if (!pLayers || payload.objectIds.isEmpty())
		return result;

	const int anchorLeft = posInLayer.x() - payload.hotSpot.x();
	std::chrono::milliseconds anchorTime = g_pContentViewData->PositionToTime(anchorLeft - g_pContentViewData->GetXOffset());
	if (anchorTime < std::chrono::milliseconds(0))
		anchorTime = std::chrono::milliseconds(0);

	const bool bMulti = payload.objectIds.size() > 1;

	const std::chrono::milliseconds threshold = g_pContentViewData->PositionToTime(nSizeDefine_10);
	const std::set<std::chrono::milliseconds> snapTimes = pLayers->CollectSnapTimes(payload.objectIds);

	std::chrono::milliseconds snapLineTime;

	if (!bMulti)
	{
		// 单选：检测当前素材左缘与右缘
		const std::chrono::milliseconds duration = GetObjectDuration(payload.anchorId);
		const auto leftTime = anchorTime;
		const auto rightTime = anchorTime + duration;

		const SSnapCandidate leftSnap = FindNearestSnap(leftTime, snapTimes, threshold);
		const SSnapCandidate rightSnap = FindNearestSnap(rightTime, snapTimes, threshold);

		if (leftSnap.bFound && rightSnap.bFound)
		{
			if (leftSnap.distance <= rightSnap.distance)
			{
				result.anchorTime = leftSnap.snapTime;
				snapLineTime = leftSnap.snapTime;
			}
			else
			{
				result.anchorTime = rightSnap.snapTime - duration;
				snapLineTime = rightSnap.snapTime;
			}
			result.bSnapped = true;
		}
		else if (leftSnap.bFound)
		{
			result.anchorTime = leftSnap.snapTime;
			snapLineTime = leftSnap.snapTime;
			result.bSnapped = true;
		}
		else if (rightSnap.bFound)
		{
			result.anchorTime = rightSnap.snapTime - duration;
			snapLineTime = rightSnap.snapTime;
			result.bSnapped = true;
		}
		else
		{
			result.anchorTime = anchorTime;
		}
	}
	else
	{
		// 多选：最左侧素材左缘与最右侧素材右缘
		qint64 minOffset = payload.timeOffsetMs[0];
		qint64 maxRightOffset = payload.timeOffsetMs[0] + GetObjectDuration(payload.objectIds[0]).count();
		for (int i = 1; i < payload.objectIds.size(); ++i)
		{
			minOffset = std::min(minOffset, payload.timeOffsetMs[i]);
			const qint64 rightOffset = payload.timeOffsetMs[i] + GetObjectDuration(payload.objectIds[i]).count();
			maxRightOffset = std::max(maxRightOffset, rightOffset);
		}

		const auto groupLeft = anchorTime + std::chrono::milliseconds(minOffset);
		const auto groupRight = anchorTime + std::chrono::milliseconds(maxRightOffset);

		const SSnapCandidate leftSnap = FindNearestSnap(groupLeft, snapTimes, threshold);
		const SSnapCandidate rightSnap = FindNearestSnap(groupRight, snapTimes, threshold);

		if (leftSnap.bFound && rightSnap.bFound)
		{
			if (leftSnap.distance <= rightSnap.distance)
			{
				result.anchorTime = leftSnap.snapTime - std::chrono::milliseconds(minOffset);
				snapLineTime = leftSnap.snapTime;
			}
			else
			{
				result.anchorTime = rightSnap.snapTime - std::chrono::milliseconds(maxRightOffset);
				snapLineTime = rightSnap.snapTime;
			}
			result.bSnapped = true;
		}
		else if (leftSnap.bFound)
		{
			result.anchorTime = leftSnap.snapTime - std::chrono::milliseconds(minOffset);
			snapLineTime = leftSnap.snapTime;
			result.bSnapped = true;
		}
		else if (rightSnap.bFound)
		{
			result.anchorTime = rightSnap.snapTime - std::chrono::milliseconds(maxRightOffset);
			snapLineTime = rightSnap.snapTime;
			result.bSnapped = true;
		}
		else
		{
			result.anchorTime = anchorTime;
		}
	}

	if (result.anchorTime < std::chrono::milliseconds(0))
		result.anchorTime = std::chrono::milliseconds(0);

	if (result.bSnapped)
		result.snapLinePos = TimeToSnapLinePos(snapLineTime);
	else
		result.anchorTime = anchorTime;

	return result;
}

void CObjectDragMime::ApplyObjectDragSnapPreview(const SObjectDragSnapResult& snap)
{
	if (snap.bSnapped && snap.snapLinePos >= 0)
		g_pContentViewData->SetSnap(true, snap.snapLinePos);
	else
		g_pContentViewData->SetSnap(false);
}

void CObjectDragMime::PreviewResourceDragSnap(const std::chrono::milliseconds& mouseTime, int posLayersX, Qt::KeyboardModifiers modifiers)
{
	g_pContentViewData->SetSnap(false);
	Q_UNUSED(modifiers);

	const int snapPos = static_cast<int>(g_pContentViewData->TimeToPosition(g_pPlayerController->GetCurTime()) + g_pContentViewData->GetXOffset());
	if (abs(snapPos - posLayersX) < nSizeDefine_10)
		g_pContentViewData->SetSnap(true, snapPos);
	g_pContentViewData->SetObjectDragPreviewTime(mouseTime);
}

std::chrono::milliseconds CObjectDragMime::CalcResourceDropStartTime(const std::chrono::milliseconds& mouseTime, int posLayersX, Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(modifiers);
	const int snapPos = static_cast<int>(g_pContentViewData->TimeToPosition(g_pPlayerController->GetCurTime()) + g_pContentViewData->GetXOffset());
	if (abs(snapPos - posLayersX) < nSizeDefine_10)
		return g_pPlayerController->GetCurTime();
	return mouseTime;
}

void CObjectDragMime::PreviewObjectDragSnap(const QMimeData* pMime, const QPoint& posInLayer, CLayersWidget* pLayers)
{
	auto payload = ParseObjectDragMime(pMime);
	if (!payload.has_value() || !pLayers)
	{
		g_pContentViewData->SetSnap(false);
		g_pContentViewData->ClearObjectDragPreview();
		return;
	}

	const SObjectDragSnapResult snap = CalcObjectDragSnap(*payload, posInLayer, pLayers);
	ApplyObjectDragSnapPreview(snap);
	// 仅单选时间线素材移动时显示 Alt 新建图层提示
	g_pContentViewData->SetObjectDragPreviewTime(snap.anchorTime, payload->objectIds.size() == 1);
}

bool CObjectDragMime::CommitObjectDrag(const STimelineObjectDragPayload& payload, int dropLayerIndex, const QPoint& posInLayer, CLayersWidget* pLayers)
{
	if (!pLayers || payload.objectIds.isEmpty())
		return false;

	std::vector<QUuid> layerIds = g_pTimeLineModel->GetAllLayerIDs();
	if (layerIds.empty())
		return false;

	const SObjectDragSnapResult snap = CalcObjectDragSnap(payload, posInLayer, pLayers);
	const std::chrono::milliseconds anchorTime = snap.anchorTime;
	const std::set<QUuid> skippedPathClips = TimeLineOverlapHelper::CollectObjectDragPathClipsSkippedDueToOverlap(
		payload, dropLayerIndex, posInLayer, pLayers);

	const Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
	// 仅单选素材移动时 Alt 才可新建图层；多选无效
	const bool bAltPressed = (modifiers & Qt::AltModifier) != 0 && payload.objectIds.size() == 1;

	auto pAnchorLayer = g_pTimeLineModel->GetLayer(payload.anchorId).lock();
	const int anchorLayerIndex = pAnchorLayer
		? g_pTimeLineModel->GetLayerIndex(pAnchorLayer->GetId()) : -1;
	const bool bCrossedLayers = anchorLayerIndex >= 0 && dropLayerIndex != anchorLayerIndex;
	const bool bCreateNewLayer = bAltPressed && bCrossedLayers;
	int newLayerIndex = dropLayerIndex;

	if (bCreateNewLayer)
	{
		// Alt + 跨越图层：向上跨时在目标图层下方插入，向下跨时在目标图层上方插入
		if (anchorLayerIndex > dropLayerIndex)
			newLayerIndex = dropLayerIndex + 1;
		else
			newLayerIndex = dropLayerIndex;

		auto pNewLayer = g_pTimeLineModel->CreateLayer();
		pNewLayer->SetName(CTimeLineWndTranslate::Layer() + QString::number(g_pTimeLineModel->GetNewLayerIndex()));
		g_pTimeLineModel->AddLayer(pNewLayer, newLayerIndex);
		layerIds = g_pTimeLineModel->GetAllLayerIDs();
	}
	else
	{
		for (int i = 0; i < payload.objectIds.size(); ++i)
		{
			if (skippedPathClips.count(payload.objectIds[i]))
				continue;
			int targetIndex = dropLayerIndex + payload.layerIndexOffset[i];
			if (targetIndex < 0 || targetIndex >= static_cast<int>(layerIds.size()))
				return false;
			auto pTargetLayer = g_pTimeLineModel->GetLayer(layerIds[targetIndex]).lock();
			if (!pTargetLayer || pTargetLayer->IsLock())
				return false;
		}
	}

	bool bChanged = false;
	for (int i = 0; i < payload.objectIds.size(); ++i)
	{
		const QUuid& objId = payload.objectIds[i];
		// 同相机重叠：路径片段保持原位，媒体照常移动
		if (skippedPathClips.count(objId))
			continue;

		int targetIndex = bCreateNewLayer
			? newLayerIndex
			: dropLayerIndex + payload.layerIndexOffset[i];
		targetIndex = std::max(0, std::min(targetIndex, static_cast<int>(layerIds.size()) - 1));
		const QUuid targetLayerId = layerIds[targetIndex];

		auto pOriginalLayer = g_pTimeLineModel->GetLayer(objId).lock();
		if (!pOriginalLayer)
			continue;
		auto pObject = pOriginalLayer->GetObject(objId).lock();
		if (!pObject)
			continue;

		std::chrono::milliseconds newTime = anchorTime + std::chrono::milliseconds(payload.timeOffsetMs[i]);
		if (newTime < std::chrono::milliseconds(0))
			newTime = std::chrono::milliseconds(0);

		const QUuid originalLayerId = pOriginalLayer->GetId();
		const auto oldStart = pObject->GetStartTime();
		const auto duration = pObject->GetDuration();
		if (newTime != oldStart || targetLayerId != originalLayerId)
			bChanged = true;

		pObject->SetStartTime(newTime);
		if (PathCameraClipHelper::IsPathCameraClip(objId))
		{
			PathCameraClipHelper::SyncClipStartTimeToOwnerCamera(objId, newTime);
			// 整体平移：关键帧跟随片段相对位置，仅帧对齐写回
			if (newTime != oldStart)
			{
				PathCameraClipHelper::AdjustPathCameraKeyframesForRangeChange(
					objId, oldStart, newTime, duration, duration);
			}
		}
		if (targetLayerId != originalLayerId)
		{
			auto pTargetLayer = g_pTimeLineModel->GetLayer(targetLayerId).lock();
			std::shared_ptr<ITimeLineObject> movedObject;
			if (pTargetLayer && pOriginalLayer->ExtractObject(objId, movedObject) && pTargetLayer->InsertObject(movedObject))
			{
				emit g_pGSNetSignal->signalMoveCrossLayerTimelineLayerObject(
					g_pTimeLineModel->GetId(), originalLayerId, targetLayerId, objId);
			}
			else
			{
				pOriginalLayer->DeleteObject(g_pTimeLineModel->GetId(), objId);
				if (pTargetLayer)
					pTargetLayer->AddObject(g_pTimeLineModel->GetId(), pObject);
			}
		}
		else
		{
			pOriginalLayer->MoveObject(g_pTimeLineModel->GetId(), objId);
		}
	}

	if (!bChanged)
		return false;

	if (g_pTimeLineModel->GetMaxUsefulDuration() > g_pTimeLineModel->GetDuration())
		g_pTimeLineModel->SetMaxUsefulDuration();

	HistoryStackSave();
	g_pContentViewData->SetSnap(false);
	pLayers->OnObjectDragCommitted();
	g_pContentViewData->ClearObjectDragPreview();
	if (!g_pContentViewData->IsTimelineRecordLocked())
		PlayCommandCenter::OnControllerSeekToPauseAfterTimelineEdit(g_pTimeLineModel->GetId());
	return true;
}
