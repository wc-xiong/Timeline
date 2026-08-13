#define _USE_MATH_DEFINES		// fix qmath.h vs math.h conflict
#include "ContentWidget.h"
#include "GSMedia/ThreadManager/ThreadManagerExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Controll/PlayCommandCenter/PlayCommandCenterExport.h"
#include "Controll/PlayStateCenter/PlayStateCenterExport.h"
#include "GSMedia/TimeServer/TimeServerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Common/SignalCenter/GSNetSignalCenter.h"
#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"

#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Model/MediaModel/MediaModelExport.h"

#include "UICommon/menuex.h"
#include "UICommon/Messagebox.h"
#include "UICommon/Colordialog.h"
#include "UICommon/UITranslate.h"
#include "UICommon/RenameDialog.h"
#include "UICommon/ShowMessageWidget.h"

#include "TopWidget.h"
#include "BarWidget.h"
#include "LayersViewWidget.h"
#include "LayersWidget.h"
#include "LayerWidget.h"
#include "NoniusViewWidget.h"
#include "PathCameraClipHelper.h"
#include "TimeLineOverlapHelper.h"
#include "Model/TimeLineModel/ConvertHelper.h"

#include <vector>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QPainter>
#include <QShortcut>
#include <QCursor>
#include <QKeySequence>
#include <QResizeEvent>
#include <QLabel>
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
	constexpr uint32_t kSeekRefreshViewMask = WndType::WndType_TimeLineTool
		| WndType::WndType_NoniusView | WndType::WndType_TimeLineScale
		| WndType::WndType_LayersView;

	constexpr uint32_t kLocateRefreshViewMask = WndType::WndType_HorScroll
		| WndType::WndType_TimeLineScale | WndType::WndType_LayersView
		| WndType::WndType_NoniusView;

	void RefreshViewAfterSeek(CContentWidget* pWidget)
	{
		pWidget->UpdateView(kSeekRefreshViewMask);
		emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_None, SignalCenter::EDDT_SectionSelect);
	}

	std::chrono::milliseconds SeekTimeByFrameOffset(int frameDelta)
	{
		const auto curTime = g_pPlayerController->GetCurTime();
		int frames = g_pConvertHelper->timeToFrames(curTime);
		frames += frameDelta;
		if (frames < 0)
			frames = 0;

		auto newTime = g_pConvertHelper->frameToTimes(frames);
		newTime = g_pConvertHelper->formatTimeWithFrame(newTime);
		const auto duration = g_pTimeLineModel->GetDuration();
		if (newTime > duration)
			newTime = duration;
		if (newTime < std::chrono::milliseconds(0))
			newTime = std::chrono::milliseconds(0);
		return newTime;
	}

	// Timeline record: controller Seek+Pause immediately; notify Displayer (no PLAY_SYNC_DELAY)
	void PauseTimelineRecordImmediate(const std::chrono::milliseconds& seekMs)
	{
		std::chrono::microseconds now;
		TimeServer_GetCurTime(now);
		SCellPlayState state;
		state.idCell = g_pContentViewData->GetTimeLineId();
		state.seek = seekMs.count();
		const auto mgr = PlayStateCenter::GetStateManager();
		mgr->FinishSeekTimeline(now, state);
		mgr->PauseTimeline(now, state);
		g_pContentViewData->SetNoniusTime(seekMs);
		PlayCommandCenter::OnControllerSeekToPauseImmediate(state);
		// SetNoniusTime 静默；录制停播等同 Seek，需通知属性/预览
		if (g_pSignalCenter)
			emit g_pSignalCenter->signalNoniusTimeChanged(g_pContentViewData->GetNoniusTime().count());
	}

	bool CalcSelectedObjectHeadTail(std::chrono::milliseconds& outHead, std::chrono::milliseconds& outTail)
	{
		const std::set<QUuid> selIds = g_pSelectController->GetTLObjectSelectIds();
		if (selIds.empty())
			return false;

		bool bFound = false;
		outHead = std::chrono::milliseconds::max();
		outTail = std::chrono::milliseconds(0);
		for (const QUuid& id : selIds)
		{
			auto pLayer = g_pTimeLineModel->GetLayer(id).lock();
			if (!pLayer)
				continue;
			auto pObject = pLayer->GetObject(id).lock();
			if (!pObject)
				continue;

			const auto startTime = pObject->GetStartTime();
			const auto endTime = startTime + pObject->GetDuration();
			if (!bFound || startTime < outHead)
				outHead = startTime;
			if (!bFound || endTime > outTail)
				outTail = endTime;
			bFound = true;
		}
		return bFound;
	}

	void ShowTimelineRecordBlockedTip() 
	{
		CShowMessageWidget::ShowMessage(3000, CCameraRecordTranslate::TimelineRecordLockedTip(), nullptr, ESMT_Warning);
	}

	bool SeekTimeLineAt(const std::chrono::milliseconds& msRaw)
	{
		bool res = g_pContentViewData->SeekTimelineAt(msRaw);
		if (!res)
			CShowMessageWidget::ShowMessage(3000, CCameraRecordTranslate::TimelineRecordLockedTip(), nullptr, ESMT_Warning);

		return res;
	}
}

CContentWidget::CContentWidget(QWidget* pParent /*= 0*/) :
	CBaseFrame(pParent)
{
	InitWidget();
	CreateConnects();
	UpdateTranslateText();

	m_ptimer = new CTimer(std::bind(&CContentWidget::OnProc, this), 1000.f / 60.f);
	m_ptimer->Start();
}

CContentWidget::~CContentWidget()
{
	if (m_ptimer)
	{
		m_ptimer->Stop();
		delete m_ptimer;
		m_ptimer = nullptr;
	}
}

void CContentWidget::UpdateView(uint32_t type)
{
	if (m_pTop && (type & WndType::WndType_TimeLineTool))
		m_pTop->UpdateView(type);
	else if (m_pTop && (type & WndType::WndType_PlayControl))
		m_pTop->UpdateView(type);
	if (m_pBar && (type & WndType::WndType_Bar))
		m_pBar->UpdateView(type);
	if (m_pLayersView && (type & WndType::WndType_View))
		m_pLayersView->UpdateView(type);
	if (m_pTop && m_pBar && m_pNoniusView && (type & WndType_NoniusView))
	{
		// Keep UI nonius while sync cmd pending; avoid PLAYING/old progress flicker
		if (g_pPlayerController->IsPlaying() && !PlayCommandCenter::IsWaitingSyncCmd())
			g_pContentViewData->SetNoniusTime(g_pPlayerController->GetCurTime());
		m_pTop->UpdateTime();
		m_pNoniusView->UpdateView();
	}
}

void CContentWidget::resizeEvent(QResizeEvent* event)
{
	CBaseFrame::resizeEvent(event);
	// 拉伸后强制重排/重绘，避免 Nonius 透明叠层与控件残影
	UpdateView(WndType::WndType_All);
	update();
	if (m_pTop) m_pTop->update();
	if (m_pBar) m_pBar->update();
	if (m_pLayersView) m_pLayersView->update();
	if (m_pNoniusView) m_pNoniusView->update();
}

void CContentWidget::InitWidget()
{
	auto CreateLine = [&](int w, int h) {
		QLabel* pLine = new QLabel(this);
		if (w != -1)
			pLine->setFixedWidth(w);
		if (h != -1)
			pLine->setFixedHeight(h);
		pLine->setStyleSheet("background-color: black;");
		return pLine;
	};

	this->setProperty("ResourceWnd", true);

	const int HEIGHT = Common_Height;
	const int SCROLLBARHEIGHT = SysDpiZoom(12);

	QVBoxLayout* pTopLayout = new QVBoxLayout;
	m_pTop = new CTopWidget(this);
	m_pTop->setFixedHeight(nSizeDefine_32);
	pTopLayout->setMargin(0);
	pTopLayout->setSpacing(0);
	pTopLayout->setContentsMargins(nSizeDefine_8, nSizeDefine_1, SCROLLBARHEIGHT + nSizeDefine_1, 0);
	pTopLayout->addWidget(m_pTop);

	QVBoxLayout* pVLayout = new QVBoxLayout;
	pVLayout->setMargin(0);
	pVLayout->setSpacing(0);
	pVLayout->setContentsMargins(0, 0, 0, 0);
	pVLayout->setAlignment(Qt::AlignTop);

	QVBoxLayout* pVLayoutBar = new QVBoxLayout;
	pVLayoutBar->setMargin(0);
	pVLayoutBar->setSpacing(0);
	pVLayoutBar->setContentsMargins(nSizeDefine_8, nSizeDefine_1, SCROLLBARHEIGHT + nSizeDefine_1, 0);

	m_pBar = new CBarWidget(this);
	pVLayoutBar->addWidget(CreateLine(-1, nSizeDefine_1));
	pVLayoutBar->addWidget(m_pBar);
	pVLayoutBar->addWidget(CreateLine(-1, nSizeDefine_1));

	QHBoxLayout* pHLayoutLayerView = new QHBoxLayout;
	pHLayoutLayerView->setMargin(0);
	pHLayoutLayerView->setSpacing(0);
	pHLayoutLayerView->setContentsMargins(nSizeDefine_8, nSizeDefine_1, nSizeDefine_1, nSizeDefine_1);

	m_pLayersView = new CLayersViewWidget(this);
	pHLayoutLayerView->addWidget(m_pLayersView);

	pVLayout->addLayout(pVLayoutBar);
	pVLayout->addLayout(pHLayoutLayerView);

	m_pNoniusView = new CNoniusViewWidget(this);
	QHBoxLayout* pViewCustomLayout = new QHBoxLayout;
	pViewCustomLayout->setMargin(0);
	pViewCustomLayout->setSpacing(0);
	pViewCustomLayout->setContentsMargins(nSizeDefine_8, nSizeDefine_1, SCROLLBARHEIGHT + nSizeDefine_1, HEIGHT + nSizeDefine_6);
	pViewCustomLayout->addWidget(m_pNoniusView);

	QGridLayout* pGridLayout = new QGridLayout();
	pGridLayout->setContentsMargins(0, 0, 0, 0);
	pGridLayout->setSpacing(0);
	pGridLayout->addLayout(pVLayout, 0, 0);
	pGridLayout->addLayout(pViewCustomLayout, 0, 0);
	
	QVBoxLayout* pMainLayout = new QVBoxLayout(this);
	pMainLayout->setMargin(0);
	pMainLayout->setSpacing(0);
	pMainLayout->addLayout(pTopLayout);
	pMainLayout->addLayout(pGridLayout);

	UpdateView();
}

void CContentWidget::CreateConnects()
{
	connect(this, &CContentWidget::signalUpdate, this, [&]() {UpdateView(WndType_NoniusView); }, Qt::QueuedConnection);
	connect(m_pTop, &CTopWidget::signalUpdateView, this, [&](uint32_t type) {UpdateView(type); });
	connect(m_pBar, &CBarWidget::signalUpdateView, this, [&](uint32_t type) {UpdateView(type); });
	connect(m_pLayersView, &CLayersViewWidget::signalUpdateView, this, [&](uint32_t type) {UpdateView(type); });
	connect(m_pLayersView, &CLayersViewWidget::signalUpdateDragLine, this, [&](int index) {
		g_pContentViewData->SetLayerDragLineIndex(index);
		UpdateView(WndType_NoniusView);
	});
	connect(m_pNoniusView, &CNoniusViewWidget::signalPlayWheelCycle, this, [&]() {
		m_pLayersView->SetHorScrollbarVal(0);
	});
	connect(m_pNoniusView, &CNoniusViewWidget::signalPlayWheel, this, [&]() {
		int offset = g_pContentViewData->TimeToPosition(g_pContentViewData->GetNoniusTime());
		m_pLayersView->SetHorScrollbarVal(offset);
	});
	connect(m_pLayersView, &CLayersViewWidget::signalHorScrollValueChanged, this, [&](int v) {
		g_pContentViewData->SetXOffset(-v);
		// 播放中自动翻页：只重定位素材几何，禁止 RefreshTransFx / CheckObject 全量重建
		if (g_pPlayerController && g_pPlayerController->IsPlaying())
		{
			if (auto* pLayers = m_pLayersView->GetLayersWidget())
				pLayers->RelayoutLayersGeometry();
			UpdateView(WndType::WndType_TimeLineScale | WndType::WndType_NoniusView);
		}
		else
		{
			UpdateView(WndType::WndType_TimeLineScale | WndType::WndType_LayersView |
				WndType::WndType_NoniusView);
		}
	});
	connect(g_pSignalCenter, &CSignalCenter::signalSeekTimelineAt, this, [](qint64 timelineMs) {
		g_pContentViewData->SeekTimelineAt(std::chrono::milliseconds(timelineMs));
	});
	connect(g_pSignalCenter, &CSignalCenter::signalLocateNonius, this, []() {
		g_pContentViewData->ScrollToKeepNoniusAtThird();
	});
	connect(g_pSignalCenter, &CSignalCenter::signalPlayPause, this, [&]() {
		if (g_pContentViewData->IsTimelineRecordLocked())
		{
			ShowTimelineRecordBlockedTip();
			return;
		}
		SCellPlayState state;
		state.idCell = g_pContentViewData->GetTimeLineId();
		state.seek = g_pContentViewData->GetNoniusTime().count();
		if (!g_pPlayerController->IsPlaying())
			PlayCommandCenter::OnControllerPlay(state);
		else
			PlayCommandCenter::OnControllerPause(state);
		UpdateView(WndType::WndType_LayersTool | WndType::WndType_PlayControl | WndType::WndType_NoniusView);
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
	});
	connect(g_pSignalCenter, &CSignalCenter::signalAddNewMediaByView, this, &CContentWidget::slotAddNewMediaByView);
	connect(g_pSignalCenter, &CSignalCenter::signalClearTimeLienWidgets, this, &CContentWidget::slotClearTimeLineWidgets);

	connect(g_pSignalCenter, &CSignalCenter::signalCameraTimelineRecordPrepare, this,[](qint64 startMs) {
			const auto alignedStart = g_pConvertHelper->formatTimeWithFrame(std::chrono::milliseconds(startMs));
			g_pContentViewData->SeekTimelineAt(alignedStart, true);
			g_pContentViewData->SetTimelineRecordLock(true);
			emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, WndType::WndType_NoniusView);
		});

	connect(g_pSignalCenter, &CSignalCenter::signalCameraTimelineRecordPlay, this, []() {
		SCellPlayState state;
		state.idCell = g_pContentViewData->GetTimeLineId();
		state.seek = g_pContentViewData->GetNoniusTime().count();
		PlayCommandCenter::OnControllerPlay(state);
	});

	// Timeline record stop: manual Stop / record panel Worker -> immediate pause
	connect(g_pSignalCenter, &CSignalCenter::signalCameraRecordStop, this, []() {
		if (!g_pContentViewData->IsTimelineRecordLocked())
			return;
		if (!g_pPlayerController->IsPlaying())
			return;
		const auto seekMs = g_pConvertHelper->formatTimeWithFrame(g_pPlayerController->GetCurTime());
		PauseTimelineRecordImmediate(seekMs);
	});

	connect(g_pSignalCenter, &CSignalCenter::signalCameraTimelineRecordRelease, this, []() {
		// Fallback: Pause once more before unlock if still playing
		if (g_pPlayerController->IsPlaying())
		{
			const auto seekMs = g_pConvertHelper->formatTimeWithFrame(g_pPlayerController->GetCurTime());
			PauseTimelineRecordImmediate(seekMs);
		}
		g_pContentViewData->SetTimelineRecordLock(false);
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, WndType::WndType_NoniusView);
	});

	QShortcut* pDelSelectShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
	// 仅时间线（ContentWidget 及其子控件）有焦点时响应，避免 3D/其它面板按 Delete 误删片段
	pDelSelectShortcut->setContext(Qt::WidgetWithChildrenShortcut);
	connect(pDelSelectShortcut, &QShortcut::activated, this, &CContentWidget::slotDeleteSelect);

	QShortcut* pPrevFrameShortcut = new QShortcut(QKeySequence(Qt::Key_Left), this);
	connect(pPrevFrameShortcut, &QShortcut::activated, this, &CContentWidget::slotSeekTimelinePrevFrame);

	QShortcut* pNextFrameShortcut = new QShortcut(QKeySequence(Qt::Key_Right), this);
	connect(pNextFrameShortcut, &QShortcut::activated, this, &CContentWidget::slotSeekTimelineNextFrame);

	QShortcut* pLocateNoniusShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_B), this);
	connect(pLocateNoniusShortcut, &QShortcut::activated, this, &CContentWidget::slotLocateNoniusAtThird);

	QShortcut* pSeekObjectHeadShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_H), this);
	connect(pSeekObjectHeadShortcut, &QShortcut::activated, this, &CContentWidget::slotSeekToSelectedObjectHead);

	QShortcut* pSeekObjectTailShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_L), this);
	connect(pSeekObjectTailShortcut, &QShortcut::activated, this, &CContentWidget::slotSeekToSelectedObjectTail);

	QShortcut* pCopyShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_C), this);
	connect(pCopyShortcut, &QShortcut::activated, this, &CContentWidget::slotCopySelectedObjects);

	QShortcut* pPasteShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_V), this);
	connect(pPasteShortcut, &QShortcut::activated, this, &CContentWidget::slotPasteObjectsAtPlayhead);
}

void CContentWidget::UpdateTranslateText()
{
	m_pTop->UpdateTranslateText();
	m_pBar->UpdateTranslateText();
	m_pLayersView->UpdateTranslateText();
}

void CContentWidget::OnProc()
{
	if(g_pPlayerController->IsViewNeedUpdate())
		emit signalUpdate();
}

void CContentWidget::slotClearTimeLineWidgets()
{
	m_pBar->Clear();
	UpdateView(WndType::WndType_All);
}

void CContentWidget::slotAddNewMediaByView(QUuid resourceId, QRect rcScreen)
{
	// Create a new timeline layer
	QDateTime currentDateTime = QDateTime::currentDateTime(); 
	QString timeStr = currentDateTime.toString("yyyyMMdd_HHmmsszzz");
	auto pLayer = g_pTimeLineModel->CreateLayer();
	pLayer->SetName(CTimeLineWndTranslate::Layer() + QString(u8"_") + timeStr);
	g_pTimeLineModel->AddLayer(pLayer, 0);
	g_pSelectController->ClearTimeLineObjectSelect();
	g_pSelectController->AddTimeLineLayerSelect(pLayer->GetId(), true);

	// Create media
	QUuid id = QUuid::createUuid();
	auto pObject = g_pTimeLineModel->CreateObject(id, ITimeLineObject::ObjectType_Media);
	pObject->SetStartTime(std::chrono::milliseconds(0));
	pObject->SetDuration(g_pTimeLineModel->GetDuration());

	QRect rc = rcScreen;
	QString name;
	auto resourceNode =g_pResourceModel->GetResourceNode(resourceId);
	if (resourceNode)
	{
		name = resourceNode->GetName();
		if (rc.width() < 1)
		{
			rc.setSize(resourceNode->GetResolution());
		}
	}

	if (pObject->GetDuration().count() >= 1000)
	{
		// AddMedia before AddObject so MediaNode exists when AddLayerObj arrives
		MediaModel::AddMedia(id, resourceId, rc);
		pLayer->AddObject(g_pTimeLineModel->GetId(), pObject);
		if (g_pTimeLineModel->GetMaxUsefulDuration() > g_pTimeLineModel->GetDuration())
			g_pTimeLineModel->SetMaxUsefulDuration();
		g_pSelectController->AddTimeLineObjectSelect(id, true);
	}
	UpdateView(WndType::WndType_LayerLabels | WndType::WndType_VerScroll |
		WndType::WndType_LayersView | WndType::WndType_TimeLineTool);

	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);

	SCellPlayState state;
	state.idCell	= g_pContentViewData->GetTimeLineId();
	state.seek		= g_pContentViewData->GetNoniusTime().count();
	PlayCommandCenter::OnControllerPlay(state);

	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
	emit g_pSignalCenter->signalUpdatePropertyShowType();
	HistoryStackSave();
}
void CContentWidget::slotDeleteSelect()
{
	bool bDeleted = false;
	const std::set<QUuid> setObjects = g_pSelectController->GetTLObjectSelectIds();
	if (!setObjects.empty())
	{
		bool bDeletedKeyframe = false;
		std::vector<QUuid> objectsToDeleteClip;
		objectsToDeleteClip.reserve(setObjects.size());

		for (const auto& id : setObjects)
		{
			// 路径片段：游标与关键帧重合时优先删关键帧，不删片段
			if (PathCameraClipHelper::IsPathCameraClip(id)
				&& PathCameraClipHelper::CanEditPathCameraKeyframes())
			{
				std::chrono::milliseconds kfLocal{ 0 };
				if (PathCameraClipHelper::TryGetPlayheadSelectedPathKeyframe(id, kfLocal))
				{
					if (PathCameraClipHelper::DeletePathCameraKeyframesAtTime(id, kfLocal))
					{
						bDeletedKeyframe = true;
						QUuid clipId = id;
						emit g_pSignalCenter->signalPathCameraClipSelectChanged(clipId);
						continue;
					}
					// 首/尾等删除失败：不回退为删片段，避免误删整段
					continue;
				}
			}
			objectsToDeleteClip.push_back(id);
		}

		if (bDeletedKeyframe)
		{
			emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
			emit g_pSignalCenter->signalRefreshWindow(
				SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
			UpdateView(WndType::WndType_LayerLabels | WndType::WndType_VerScroll |
				WndType::WndType_LayersView | WndType::WndType_TimeLineTool);
			bDeleted = true;
		}

		for (const auto& id : objectsToDeleteClip)
		{
			auto pLayer = g_pTimeLineModel->GetLayer(id).lock();
			if (!pLayer)
				continue;
			auto pObject = pLayer->GetObject(id).lock();
			if (pObject && pObject->GetType() == ITimeLineObject::ObjectType_Media)
			{
				g_pPlayerController->DelMediaAndFreePlayer(id);
				g_pMediaModel->DeleteMediaNode(id);
			}
			PathCameraClipHelper::RemoveClipFromOwnerCamera(id);
			pLayer->DeleteObject(g_pTimeLineModel->GetId(), id);
		}
		if (!objectsToDeleteClip.empty())
		{
			g_pSelectController->ClearTimeLineLayerSelect();
			g_pSelectController->ClearTimeLineObjectSelect();
			g_pSelectController->ClearTransFxSelect();
			emit g_pSignalCenter->signalLocateResNode(QUuid());
			emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
			emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
			emit g_pSignalCenter->signalUpdatePropertyShowType();
			if (g_pPlayerController->IsPlaying())
				g_pPlayerController->Pause();
			UpdateView(WndType::WndType_LayerLabels | WndType::WndType_VerScroll |
				WndType::WndType_LayersView | WndType::WndType_TimeLineTool);
			bDeleted = true;
		}
	}
	else
	{
		const std::set<QUuid> setLayers = g_pSelectController->GetTLLayerSelectIds();
		if (!setLayers.empty())
		{
			auto allLayerIds = g_pTimeLineModel->GetAllLayerIDs();
			int maxindex = INT_MIN;
			int minindex = INT_MAX;
			bool bLayerDeleted = false;
			for (const auto& id : setLayers)
			{
				int index = g_pTimeLineModel->GetLayerIndex(id);
				if (index > maxindex) maxindex = index;
				if (index < minindex) minindex = index;
				auto pLayer = g_pTimeLineModel->GetLayer(id).lock();
				if (!pLayer)
					continue;

				if (0 != pLayer->GetObjectCount())
				{
					auto res = UICommon::CMessageBox::warning(this, "GrandMapping",
						CTimeLineWndTranslate::DeleteTip(),
						QMessageBox::Ok | QMessageBox::Cancel);
					if (QMessageBox::Ok != res)
						continue;
					auto objectIds = pLayer->GetAllObjectIDs();
					for (const auto& objectId : objectIds)
					{
						auto pObject = pLayer->GetObject(objectId).lock();
						if (pObject && pObject->GetType() == ITimeLineObject::ObjectType_Media)
						{
							g_pPlayerController->DelMediaAndFreePlayer(objectId);
							g_pMediaModel->DeleteMediaNode(objectId);
						}
						PathCameraClipHelper::RemoveClipFromOwnerCamera(objectId);
					}
					emit g_pSignalCenter->signalLocateResNode(QUuid());
					emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
					emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
					if (g_pPlayerController->IsPlaying())
						g_pPlayerController->Pause();
				}
				g_pTimeLineModel->DeleteLayer(id);
				g_pSelectController->DeleteTimeLineLayerSelect(id);
				bLayerDeleted = true;
			}
			if (bLayerDeleted)
			{
				// Select neighboring layer after delete
				maxindex = maxindex + 1;
				minindex = minindex - 1;
				if (maxindex >= 0 && maxindex < (int)allLayerIds.size())
					g_pSelectController->AddTimeLineLayerSelect(allLayerIds[maxindex]);
				else if (minindex >= 0 && minindex < (int)allLayerIds.size())
					g_pSelectController->AddTimeLineLayerSelect(allLayerIds[minindex]);
				UpdateView(WndType::WndType_LayerLabels | WndType::WndType_VerScroll |
					WndType::WndType_LayersView | WndType::WndType_TimeLineTool);
				bDeleted = true;
			}
		}
	}

	if (bDeleted)
		HistoryStackSave();
}

void CContentWidget::slotSeekTimelinePrevFrame()
{
	if (g_pContentViewData->IsTimelineRecordLocked())
	{
		ShowTimelineRecordBlockedTip();
		return;
	}
	SeekTimeLineAt(SeekTimeByFrameOffset(-1));
	RefreshViewAfterSeek(this);
}

void CContentWidget::slotSeekTimelineNextFrame()
{
	if (g_pContentViewData->IsTimelineRecordLocked())
	{
		ShowTimelineRecordBlockedTip();
		return;
	}
	SeekTimeLineAt(SeekTimeByFrameOffset(1));
	RefreshViewAfterSeek(this);
}

void CContentWidget::slotLocateNoniusAtThird()
{
	g_pContentViewData->ScrollToKeepNoniusAtThird();
	UpdateView(kLocateRefreshViewMask);
}

void CContentWidget::slotSeekToSelectedObjectHead()
{
	if (g_pContentViewData->IsTimelineRecordLocked())
	{
		ShowTimelineRecordBlockedTip();
		return;
	}
	std::chrono::milliseconds headTime;
	std::chrono::milliseconds tailTime;
	if (!CalcSelectedObjectHeadTail(headTime, tailTime))
		return;

	headTime = g_pConvertHelper->formatTimeWithFrame(headTime);
	SeekTimeLineAt(headTime);
	RefreshViewAfterSeek(this);
}

void CContentWidget::slotSeekToSelectedObjectTail()
{
	if (g_pContentViewData->IsTimelineRecordLocked())
	{
		ShowTimelineRecordBlockedTip();
		return;
	}
	std::chrono::milliseconds headTime;
	std::chrono::milliseconds tailTime;
	if (!CalcSelectedObjectHeadTail(headTime, tailTime))
		return;

	tailTime = g_pConvertHelper->formatTimeWithFrame(tailTime);
	SeekTimeLineAt(tailTime);
	RefreshViewAfterSeek(this);
}

void CContentWidget::slotCopySelectedObjects()
{
	if (g_pContentViewData->IsTimelineRecordLocked())
	{
		ShowTimelineRecordBlockedTip();
		return;
	}
	// Copy selected media (Media only; no-op if empty)
	g_pContentViewData->CopySelectedObjects();
}

void CContentWidget::slotPasteObjectsAtPlayhead()
{
	if (g_pContentViewData->IsTimelineRecordLocked())
	{
		ShowTimelineRecordBlockedTip();
		return;
	}
	if (!g_pContentViewData->GetTimelineCopyData().HasData())
		return;

	int targetLayerIndex = -1;
	QPoint posInLayer;
	bool bFromMouse = false;

	// 鼠标在图层视图上时：粘贴到鼠标位置（与右键粘贴一致）
	CLayersWidget* pLayers = (m_pLayersView ? m_pLayersView->GetLayersWidget() : nullptr);
	if (pLayers)
	{
		const QPoint globalPos = QCursor::pos();
		const QPoint posInLayers = pLayers->mapFromGlobal(globalPos);
		if (pLayers->rect().contains(posInLayers))
		{
			const int layerIndex = pLayers->GetLayerIndexAtY(posInLayers.y());
			if (CLayerWidget* pLayer = pLayers->GetLayerWidget(layerIndex))
			{
				targetLayerIndex = layerIndex;
				posInLayer = pLayer->mapFromGlobal(globalPos);
				bFromMouse = true;
			}
		}
	}

	if (!bFromMouse)
	{
		// 解析目标图层：优先选中图层最小索引，其次选中对象所在图层最小索引，否则第 0 层
		const std::set<QUuid> setLayers = g_pSelectController->GetTLLayerSelectIds();
		if (!setLayers.empty())
		{
			int minIndex = INT_MAX;
			for (const auto& id : setLayers)
			{
				const int index = g_pTimeLineModel->GetLayerIndex(id);
				if (index >= 0 && index < minIndex)
					minIndex = index;
			}
			if (minIndex != INT_MAX)
				targetLayerIndex = minIndex;
		}

		if (targetLayerIndex < 0)
		{
			const std::set<QUuid> setObjects = g_pSelectController->GetTLObjectSelectIds();
			if (!setObjects.empty())
			{
				int minIndex = INT_MAX;
				for (const auto& objectId : setObjects)
				{
					auto pLayer = g_pTimeLineModel->GetLayer(objectId).lock();
					if (!pLayer)
						continue;
					const int index = g_pTimeLineModel->GetLayerIndex(pLayer->GetId());
					if (index >= 0 && index < minIndex)
						minIndex = index;
				}
				if (minIndex != INT_MAX)
					targetLayerIndex = minIndex;
			}
		}

		if (targetLayerIndex < 0)
		{
			if (g_pTimeLineModel->GetLayerCount() > 0)
				targetLayerIndex = 0;
			else
				return;
		}

		// 回退：粘贴位置使 PositionToTime(x - GetXOffset()) 等于游标时间
		const int x = static_cast<int>(g_pContentViewData->TimeToPosition(g_pContentViewData->GetNoniusTime()))
			+ g_pContentViewData->GetXOffset();
		posInLayer = QPoint(x, 0);
	}

	if (targetLayerIndex < 0)
		return;
	if (!TimeLineOverlapHelper::CanPasteObjectsToLayer(posInLayer, targetLayerIndex))
	{
		if (TimeLineOverlapHelper::IsPasteBlockedByPathClipOverlap(posInLayer, targetLayerIndex))
		{
			CShowMessageWidget::ShowMessage(3000, CTimeLineWndTranslate::PathClipOverlapTip(), nullptr, ESMT_Warning);
		}
		return;
	}
	g_pContentViewData->PasteObjectsToLayer(posInLayer, targetLayerIndex);
	UpdateView(WndType::WndType_LayersView | WndType::WndType_LayerLabels
		| WndType::WndType_NoniusView | WndType::WndType_TimeLineTool
		| WndType::WndType_HorScroll);
}
