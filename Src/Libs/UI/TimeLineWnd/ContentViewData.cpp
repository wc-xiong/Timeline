#include "ContentViewData.h"
#include "PathCameraClipHelper.h"
#include "TimeLineOverlapHelper.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/TimeLineModel/TimeLineObject.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Controll/PlayCommandCenter/PlayCommandCenterExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Model/ModelModel/ModelModelExport.h"
#include "UICommon/UITranslate.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Common/CommonFunctionApi.h"
#include <QElapsedTimer>
#include <QtGlobal>
#include <rapidjson/document.h>
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
static const int KWheelZoomThreshold = 150;
void SContentViewData::SetTimelineRecordLock(bool bLocked)
{
	m_bTimelineRecordLocked = bLocked;
}

void SContentViewData::ClampXOffset()
{
	if (m_nXOffset > 0)
		m_nXOffset = 0;

	const int timeLength = static_cast<int>(TimeToPosition(g_pTimeLineModel->GetDuration()));
	const int viewWidth = GetTimelineViewWidth();
	const int maxScroll = std::max(timeLength - viewWidth, 0);
	if (-m_nXOffset > maxScroll)
		m_nXOffset = -maxScroll;
	if (timeLength + m_nXOffset < viewWidth)
		m_nXOffset = std::min(0, viewWidth - timeLength);
}

void SContentViewData::SetZoomRatioLevelIndex(int index, const std::chrono::milliseconds& hitTime)
{
	if (index < 0 || index >= GetRatioLevelSize() || index == m_nRatioIndex)
		return;

	// 缩放前记录锚点在屏幕上的位置，缩放后保持该屏幕坐标不变
	const int64_t curHitX = TimeToPosition(hitTime);
	const int oldOffsetX = m_nXOffset;
	const int64_t screenX = curHitX + oldOffsetX;
	m_nRatioIndex = index;
	const int64_t nHitPos = TimeToPosition(hitTime);
	m_nXOffset = static_cast<int>(screenX - nHitPos);
	ClampXOffset();
}

bool SContentViewData::DoInterfaceZoom(bool bIncrease, const std::chrono::milliseconds& hitTime)
{
	const int nextIndex = m_nRatioIndex + (bIncrease ? 1 : -1);
	if (nextIndex < 0 || nextIndex >= GetRatioLevelSize())
		return false;

	const float oldRatio = GetRatioLevel();
	SetZoomRatioLevelIndex(nextIndex, hitTime);
	return !FloatIsEqual(oldRatio, GetRatioLevel());
}

bool SContentViewData::HandleTimelineCtrlWheelZoom(int wheelDeltaY, int mouseXInWidget, int& outHorScrollDelta, bool& outZoomed)
{
	outHorScrollDelta = 0;
	outZoomed = false;
	if (wheelDeltaY == 0)
		return false;

	m_nWheelZoomAccum += wheelDeltaY;
	if (qAbs(m_nWheelZoomAccum) < KWheelZoomThreshold)
		return true;

	const bool bIncrease = m_nWheelZoomAccum > 0;
	m_nWheelZoomAccum = 0;

	const int oldOffset = m_nXOffset;
	const auto hitTime = PositionToTime(mouseXInWidget - oldOffset);
	if (DoInterfaceZoom(bIncrease, hitTime))
	{
		outZoomed = true;
		outHorScrollDelta = oldOffset - GetXOffset();
	}
	return true;
}


bool SContentViewData::HandleTimelineShiftWheelScroll(int wheelDeltaY)
{
	if (wheelDeltaY == 0)
		return false;
	// 与 CLayersWidget 原 Shift 路径一致：horDelta = -delta/12，再 XOffset -= horDelta
	const int horDelta = -wheelDeltaY / 12;
	if (horDelta == 0)
		return false;
	const int oldOffset = m_nXOffset;
	m_nXOffset -= horDelta;
	ClampXOffset();
	return m_nXOffset != oldOffset;
}

// 根据当前缩放比计算对应的帧编辑模式
EEditMode SContentViewData::CalcFrameEditMode(float zoomRatio)
{
	int index = m_nRatioIndex;
	// 非帧模式缩放比索引范围：0~20（Min_ZoomRatio ~ 300.0f）
	if (index < 21)
		return EEDitMode_Time;

	// 帧模式索引范围：21~35（300.0f ~ 3000.0f），分别对应 Level1~Level12
	int frameIndex = index - 21;
	if (frameIndex >= 0 && frameIndex <= 11)
		return static_cast<EEditMode>(EEDitMode_Frame_Level1 + frameIndex);
	else
		return EEDitMode_Frame_Level12;
}

// 获取当前的帧编辑模式
EEditMode SContentViewData::GetFrameEditMode()
{
	float zoomRatio = GetRatioLevel();
	return CalcFrameEditMode(zoomRatio);
}

// 获取帧模式下的一组参数：<一秒内需要绘制的刻度数, 刻度显示帧号的间隔>
std::pair<int, int> SContentViewData::GetFrameScaleInfo()
{
	int fps = static_cast<int>(std::ceil(g_pConvertHelper->getFPS()));
	EEditMode editMode = GetFrameEditMode();

	// 查表：优先精确匹配当前帧率
	auto itFps = m_FpsFrames.find(fps);
	if (itFps != m_FpsFrames.end())
	{
		auto itMode = itFps->second.find(editMode);
		if (itMode != itFps->second.end())
			return itMode->second;
	}

	// 降级：尝试用 30fps 的表
	auto itFallback = m_FpsFrames.find(30);
	if (itFallback != m_FpsFrames.end())
	{
		auto itMode = itFallback->second.find(editMode);
		if (itMode != itFallback->second.end())
			return itMode->second;
	}

	// 最终兜底
	return { 10, 5 };
}

std::pair<std::chrono::milliseconds, int> SContentViewData::GetScaleIncre(float zoomRatio)
{
	int playFps = std::ceil(g_pConvertHelper->getFPS());
	EEditMode editMode = GetFrameEditMode();
	if (zoomRatio >= 300)
	{
		if (editMode > EEDitMode_Frame_Level12)
			return { std::chrono::milliseconds(1000), m_FpsFrames[playFps][EEDitMode_Frame_Level12].first };
		return { std::chrono::milliseconds(1000), m_FpsFrames[playFps][editMode].first };
	}
	else if (zoomRatio >= 60.0f)
		return { std::chrono::milliseconds(1000),5 };
	else if (zoomRatio >= 24.0f)
		return { std::chrono::milliseconds(5000),5 };
	else if (zoomRatio >= 8.0f)
		return { std::chrono::milliseconds(15000),3 };
	else if (zoomRatio >= 2.0f)
		return { std::chrono::milliseconds(60000),4 };
	else if (zoomRatio >= 0.4f)
		return { std::chrono::milliseconds(300000),5 };
	else
		return { std::chrono::milliseconds(900000),3 };
}

QString SContentViewData::GetFrameNumber(STimeCode time, int index)
{
	QString str;
	EEditMode editMode = GetFrameEditMode();
	if (editMode == EEDitMode_Time)
		return str;
	int frameNumber = -1;

	int interval = 0;
	// 前12个缩放级别区分不同显示，后3个只做放大作用，后续有需要可以放开
	if (editMode > EEDitMode_Frame_Level12)
		interval = m_FpsFrames[time.GetFps()][EEDitMode_Frame_Level12].second;
	else
		interval = m_FpsFrames[time.GetFps()][editMode].second;

	if (index % interval != 0) return str;

	if (g_pProjectSetting->LoadTimeLineTimeCodeStyle())
	{
		frameNumber = time.frame;
		str = QString("%1f").arg(frameNumber);
	}
	else
	{
		frameNumber = g_pConvertHelper->timecodeToFrames(time);
		str = QString("%1").arg(frameNumber, 6, 10, QChar('0'));
	}
	return str;
}

void SContentViewData::SetNoniusTime(const std::chrono::milliseconds& ms)
{
	auto playTime = ms;
	if (playTime.count() < 0)
		playTime = std::chrono::milliseconds(0);
	else if (playTime > g_pTimeLineModel->GetDuration())
		playTime = g_pTimeLineModel->GetDuration();

	const auto newTime = g_pConvertHelper->formatTimeWithFrame(playTime);
	if (newTime == m_noniusTime)
		return;

	// 仅写视觉游标，不发信号（播放每帧 SetNoniusTime 不得扇出）
	m_noniusTime = newTime;
	g_pTimeLineModel->SetNoniusPos(m_noniusTime);
}

QUuid SContentViewData::GetTimeLineId()
{
	return g_pTimeLineModel->GetId();
}

bool SContentViewData::SeekTimelineAt(const std::chrono::milliseconds& msRaw, bool bForce)
{
	if (!bForce && m_bTimelineRecordLocked)
	{
		return false;
	}

	SetNoniusTime(msRaw);
	// 本地一体：游标拖拽走 SmartSeek（播放中保持播，暂停中保持停）
	SCellPlayState state;
	state.idCell = GetTimeLineId();
	state.seek = GetNoniusTime().count();
	PlayCommandCenter::OnControllerSmartSeek(state);
	emit g_pSignalCenter->signalNoniusTimeChanged(GetNoniusTime().count());
	return true;
}

void SContentViewData::ScrollToKeepNoniusAtThird()
{
	auto msPos = GetNoniusTime();
	const auto duration = g_pTimeLineModel->GetDuration();
	if (msPos > duration)
		msPos = duration;
	const int viewWidth = GetTimelineViewWidth();
	const auto offsetTime = PositionToTime(viewWidth / 3);
	int offset = static_cast<int>(TimeToPosition(msPos - offsetTime));
	if (duration - msPos < offsetTime * 2)
		offset = static_cast<int>(TimeToPosition(duration - offsetTime * 3));
	// 游标在 1/3 线左侧时，不能再向左滚
	if (offset < 0)
		offset = 0;
	int newXOffset = -offset;
	// 右边界：与 HorScroll 一致，避免滚过头
	const int timeLength = static_cast<int>(TimeToPosition(duration));
	const int maxScroll = std::max(timeLength - viewWidth, 0);
	if (-newXOffset > maxScroll)
		newXOffset = -maxScroll;
	// 已在目标位置则不刷新（可选）
	if (newXOffset == GetXOffset())
		return;
	SetXOffset(newXOffset);
}


bool SContentViewData::EnsureNoniusVisibleForDrag(int mouseXInView, int viewWidth)
{
	if (viewWidth <= 0)
		return false;
	// 仍在可视区内：不滚动
	if (mouseXInView >= 0 && mouseXInView < viewWidth)
		return false;

	const int absPos = static_cast<int>(TimeToPosition(GetNoniusTime()));
	int newOffset = m_nXOffset;
	if (mouseXInView < 0)
	{
		// 鼠标在左缘外：将游标贴到可视区左边界
		newOffset = -absPos;
	}
	else
	{
		// 鼠标在右缘外：将游标贴到可视区右边界
		newOffset = viewWidth - absPos;
	}

	const int oldOffset = m_nXOffset;
	m_nXOffset = newOffset;
	ClampXOffset(); // 限制在合法横向偏移范围内
	return m_nXOffset != oldOffset;
}

void SContentViewData::SetObjectDragPreviewTime(const std::chrono::milliseconds& t, bool bShowAltNewLayerTip)
{
	m_bObjectDragPreview = true;
	m_bObjectDragAltNewLayerTip = bShowAltNewLayerTip;
	m_objectDragPreviewTime = t;
}

void SContentViewData::ClearObjectDragPreview()
{
	m_bObjectDragPreview = false;
	m_bObjectDragAltNewLayerTip = false;
	m_objectDragPreviewTime = 0ms;
}

//void CloneObjectFxData(const std::shared_ptr<ITimeLineObject>& pSrcObj, const std::shared_ptr<ITimeLineObject>& pDstObj)
//{
//	if (!pSrcObj || !pDstObj)
//		return;
//	const int fxTypes[] = {
//		ITimeLineObject::FxType_PosX,
//		ITimeLineObject::FxType_PosY,
//		ITimeLineObject::FxType_PosZ,
//		ITimeLineObject::FxType_RotX,
//		ITimeLineObject::FxType_RotY,
//		ITimeLineObject::FxType_RotZ,
//		ITimeLineObject::FxType_Scale,
//		ITimeLineObject::FxType_Size
//	};
//	for (const int fxType : fxTypes)
//	{
//		auto pSrcFx = pSrcObj->GetFx(fxType).lock();
//		if (!pSrcFx)
//			continue;
//		pDstObj->AddFx(fxType);
//		auto pDstFx = pDstObj->GetFx(fxType).lock();
//		if (!pDstFx)
//			continue;
//		rapidjson::Document doc;
//		doc.SetObject();
//		auto& allocator = doc.GetAllocator();
//		pSrcFx->SaveDataToRapidJson(doc, allocator);
//		pDstFx->LoadDataFromRapidJson(doc);
//	}
//}

void SContentViewData::CopySelectedObjects()
{
	m_timelineCopyData.ResetCopyData();

	const auto setSelected = g_pSelectController->GetTLObjectSelectIds();
	if (setSelected.empty())
		return;

	for (const auto& objectId : setSelected)
	{
		auto pLayer = g_pTimeLineModel->GetLayer(objectId).lock();
		if (!pLayer)
			continue;

		auto pObj = pLayer->GetObject(objectId).lock();
		if (!pObj)
			continue;

		// 支持 Media，以及路径相机动画片段
		const bool bMedia = (pObj->GetType() == ITimeLineObject::ObjectType_Media);
		const bool bPathClip = PathCameraClipHelper::IsPathCameraClip(objectId);
		if (!bMedia && !bPathClip)
			continue;

		const int layerIndex = g_pTimeLineModel->GetLayerIndex(pLayer->GetId());
		if (layerIndex < 0)
			continue;

		m_timelineCopyData.AddCopyObject(
			objectId,
			layerIndex,
			pObj->GetStartTime(),
			pObj->GetDuration());
	}
}

void SContentViewData::PasteObjectsToLayer(const QPoint& posInLayer, int targetLayerIndex)
{
	if (!m_timelineCopyData.HasData())
		return;

	if (targetLayerIndex < 0)
		return;

	// 鼠标点击处时间
	auto msHit = PositionToTime(posInLayer.x() - GetXOffset());
	if (msHit < 0ms)
		msHit = 0ms;

	const int layerSpan = static_cast<int>(m_timelineCopyData.nMaxLayerIndex - m_timelineCopyData.nMinLayerIndex);
	const int requiredMaxLayerIndex = targetLayerIndex + layerSpan;

	// 图层不足时自动补齐（插到末尾）
	while (g_pTimeLineModel->GetLayerCount() <= requiredMaxLayerIndex)
	{
		auto pNewLayer = g_pTimeLineModel->CreateLayer();
		pNewLayer->SetName(CTimeLineWndTranslate::Layer() + QString::number(g_pTimeLineModel->GetNewLayerIndex()));
		g_pTimeLineModel->AddLayer(pNewLayer);
	}

	const auto layerIds = g_pTimeLineModel->GetAllLayerIDs();
	if (layerIds.empty())
		return;

	std::vector<QUuid> vecNewIds;
	QUuid firstTargetLayerId;
	QUuid lastPathClipId; // 粘贴的最后一个路径片段，用于属性联动

	for (const auto& copyInfo : m_timelineCopyData.vecCopyObjects)
	{
		auto pSrcLayer = g_pTimeLineModel->GetLayer(copyInfo.sourceObjectId).lock();
		if (!pSrcLayer)
			continue;

		auto pSrcObj = pSrcLayer->GetObject(copyInfo.sourceObjectId).lock();
		if (!pSrcObj)
			continue;

		const bool bPathClip = PathCameraClipHelper::IsPathCameraClip(copyInfo.sourceObjectId);
		const bool bMedia = (pSrcObj->GetType() == ITimeLineObject::ObjectType_Media);
		if (!bMedia && !bPathClip)
			continue;

		const int destLayerIndex = targetLayerIndex + static_cast<int>(copyInfo.nLayerIndex - m_timelineCopyData.nMinLayerIndex);
		if (destLayerIndex < 0 || destLayerIndex >= static_cast<int>(layerIds.size()))
			continue;

		auto pDestLayer = g_pTimeLineModel->GetLayer(layerIds[destLayerIndex]).lock();
		if (!pDestLayer || pDestLayer->IsLock())
			continue;

		if (firstTargetLayerId.isNull())
			firstTargetLayerId = pDestLayer->GetId();

		auto newStart = msHit + (copyInfo.sourceStartTime - m_timelineCopyData.msMinPos);
		if (newStart < 0ms)
			newStart = 0ms;

		if (bPathClip)
		{
			QUuid ownerCamId;
			auto pCam = PathCameraClipHelper::FindCameraByPathClipId(copyInfo.sourceObjectId, &ownerCamId);
			if (!pCam || ownerCamId.isNull())
				continue;

			// 与同一相机下其它路径片段时间重叠则跳过该项
			if (PathCameraClipHelper::IsPathClipRangeOverlap(newStart, copyInfo.sourceDuration, QUuid(), ownerCamId))
				continue;

			const QUuid newId = QUuid::createUuid();
			auto pNewObj = g_pTimeLineModel->CreateObject(newId, ITimeLineObject::ObjectType_3DModel);
			if (!pNewObj)
				continue;

			// RapidJson 深拷贝 Fx / ShowPath / 显示名等，再覆盖 id 与起始时间
			rapidjson::Document doc;
			doc.SetObject();
			auto& allocator = doc.GetAllocator();
			pSrcObj->SaveDataToRapidJson(doc, allocator);
			const QByteArray idUtf8 = newId.toString().toUtf8();
			if (doc.HasMember("TimeLineObject_id") && doc["TimeLineObject_id"].IsString())
			{
				doc["TimeLineObject_id"].SetString(idUtf8.constData(),
					static_cast<rapidjson::SizeType>(idUtf8.size()), allocator);
			}
			else
			{
				rapidjson::Value idKey("TimeLineObject_id", allocator);
				rapidjson::Value idVal(idUtf8.constData(),
					static_cast<rapidjson::SizeType>(idUtf8.size()), allocator);
				doc.AddMember(idKey, idVal, allocator);
			}
			pNewObj->LoadDataFromRapidJson(doc);
			pNewObj->SetStartTime(newStart);
			pNewObj->SetDuration(copyInfo.sourceDuration);

			pDestLayer->AddObject(g_pTimeLineModel->GetId(), pNewObj);
			pCam->AddPathCameraClipId(newId, newStart);
			pCam->SetPathCamera(true);
			pCam->IncrementPathCameraClipCount();

			// 与拖拽一致：同步相机容器顺序，并对关键帧做帧对齐写回
			PathCameraClipHelper::SyncClipStartTimeToOwnerCamera(newId, newStart);
			if (newStart != copyInfo.sourceStartTime)
			{
				PathCameraClipHelper::AdjustPathCameraKeyframesForRangeChange(
					newId, copyInfo.sourceStartTime, newStart,
					copyInfo.sourceDuration, copyInfo.sourceDuration);
			}

			vecNewIds.emplace_back(newId);
			lastPathClipId = newId;
			continue;
		}

		auto pSrcMedia = g_pMediaModel->GetMediaNode(copyInfo.sourceObjectId).lock();
		if (!pSrcMedia)
			continue;

		const QUuid newId = QUuid::createUuid();
		auto pNewObj = g_pTimeLineModel->CreateObject(newId, pSrcObj->GetType());
		if (!pNewObj)
			continue;

		pNewObj->SetStartTime(newStart);
		pNewObj->SetDuration(copyInfo.sourceDuration);

		pDestLayer->AddObject(g_pTimeLineModel->GetId(), pNewObj);

		// 先复制媒体节点再上时间线，保证播放端 AddLayerObj 时 MediaNode 已存在
		// 复制媒体节点（深拷贝）
		auto pMediaClone = std::shared_ptr<IMediaNode>(dynamic_cast<IMediaNode*>(pSrcMedia->Copy()));
		if (pMediaClone)
		{
			pMediaClone->SetMediaId(newId);
			g_pMediaModel->AddMedia(pMediaClone);

			// 资源引用计数 +1，保持与 AddMedia(resourceId) 路径一致
			auto pRes = g_pResourceModel->GetResourceNode(pMediaClone->GetResourceId());
			if (pRes)
				pRes->SetResourceUseCount(pRes->GetResourceUseCount() + 1);

			g_pPlayerController->CreatePlayer(newId, pMediaClone->GetResourceId());
		}

		pDestLayer->AddObject(g_pTimeLineModel->GetId(), pNewObj);

		vecNewIds.emplace_back(newId);
	}

	if (vecNewIds.empty())
		return;

	if (g_pTimeLineModel->GetMaxUsefulDuration() > g_pTimeLineModel->GetDuration())
		g_pTimeLineModel->SetMaxUsefulDuration();

	// 粘贴后选中新对象
	g_pSelectController->ClearTimeLineLayerSelect();
	g_pSelectController->ClearTimeLineObjectSelect();

	if (!firstTargetLayerId.isNull())
		g_pSelectController->AddTimeLineLayerSelect(firstTargetLayerId, true);

	for (const auto& id : vecNewIds)
		g_pSelectController->AddTimeLineObjectSelect(id);

	if (!lastPathClipId.isNull() && g_pSignalCenter)
		emit g_pSignalCenter->signalPathCameraClipSelectChanged(lastPathClipId);

	HistoryStackSave();

	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
}
