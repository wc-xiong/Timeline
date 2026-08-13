#include "Controll/PlayerController/PlayerController.h"
#include "Controll/PlayCommandCenter/PlayCommandCenterExport.h"
#include "Controll/PlayStateCenter/PlayStateCenterExport.h"
#include "GSMedia/TimeServer/TimeServerExport.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Common/Common/CommonDefine.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "UI/TimeLineWnd/ContentViewData.h"
#include "UI/TimeLineWnd/TimeLineGlobalDefine.h"
#include "Model/TimeLineModel/ConvertHelper.h"
#include "Common/Common/ITimelineInterface.h"

#include <QTimer>
#include <algorithm>

using namespace std::chrono;

// ==================== TimeServer：本地单调时钟（无多机偏移） ====================
static microseconds s_tsOrigin{ 0 };
static bool s_tsInited = false;

static microseconds TimeServerNowUs()
{
	return duration_cast<microseconds>(steady_clock::now().time_since_epoch());
}

void TimeServer_Init()
{
	s_tsOrigin = TimeServerNowUs();
	s_tsInited = true;
}

void TimeServer_Final()
{
	s_tsInited = false;
}

void TimeServer_GetCurTime(seconds& curTime)
{
	curTime = duration_cast<seconds>(TimeServerNowUs() - s_tsOrigin);
}

void TimeServer_GetCurTime(milliseconds& curTime)
{
	curTime = duration_cast<milliseconds>(TimeServerNowUs() - s_tsOrigin);
}

void TimeServer_GetCurTime(microseconds& curTime)
{
	curTime = TimeServerNowUs() - s_tsOrigin;
}

namespace TimerSever
{
	void SetServerPresentOffset(microseconds) {}
	void GetServerPresentOffset(microseconds& o) { o = microseconds(0); }
	void SetCtrlPresentOffset(microseconds) {}
	void GetCtrlPresentOffset(microseconds& o) { o = microseconds(0); }
	void GetTimeOffset(microseconds& o) { o = microseconds(0); }
	void SetTimeOffset(microseconds) {}
}

// ==================== PlayStateCenter：锚点进度 ====================
namespace
{
	struct SLocalPlayState
	{
		EnPlayState state = EPS_PAUSE;
		milliseconds progressAtChange{ 0 };	// 状态切换时的进度
		microseconds timeStampAtChange{ 0 };	// TimeServer 时间戳
	};

	SLocalPlayState g_playState;

	milliseconds ClampTimelineMs(milliseconds ms)
	{
		if (ms.count() < 0)
			ms = milliseconds(0);
		if (g_pTimeLineModel)
		{
			const auto dur = g_pTimeLineModel->GetDuration();
			if (ms > dur)
				ms = dur;
		}
		if (g_pConvertHelper)
			ms = g_pConvertHelper->formatTimeWithFrame(ms);
		return ms;
	}

	milliseconds ComputeProgressNow()
	{
		if (g_playState.state != EPS_PLAYING)
			return g_playState.progressAtChange;

		microseconds nowUs;
		TimeServer_GetCurTime(nowUs);
		const auto delta = duration_cast<milliseconds>(nowUs - g_playState.timeStampAtChange);
		return ClampTimelineMs(g_playState.progressAtChange + delta);
	}

	void AnchorAt(milliseconds progress, EnPlayState state)
	{
		microseconds nowUs;
		TimeServer_GetCurTime(nowUs);
		g_playState.progressAtChange = ClampTimelineMs(progress);
		g_playState.timeStampAtChange = nowUs;
		g_playState.state = state;
	}

	void FreezeAt(milliseconds progress, EnPlayState state)
	{
		AnchorAt(progress, state);
	}
}

class CLocalStateManager : public IStateManager
{
public:
	EnPlayState GetTimelineState() const override { return g_playState.state; }

	STimelinePlayInfo GetTimelinePlayInfo(std::chrono::microseconds, const QUuid&) const override
	{
		STimelinePlayInfo info;
		info.progress = duration_cast<microseconds>(ComputeProgressNow());
		return info;
	}

	void FinishSeekTimeline(std::chrono::microseconds, const SCellPlayState& state) override
	{
		const auto ms = milliseconds(state.seek);
		if (g_playState.state == EPS_PLAYING)
			AnchorAt(ms, EPS_PLAYING);
		else
			FreezeAt(ms, g_playState.state == EPS_PAUSE_SECTION_END ? EPS_PAUSE : g_playState.state);
		// 时钟到位后通知预览完成 Seek→出帧
		if (g_pSignalCenter)
			emit g_pSignalCenter->signalFinishSeekTimeline(ms.count());
	}

	void PauseTimeline(std::chrono::microseconds, const SCellPlayState& state) override
	{
		FreezeAt(milliseconds(state.seek), EPS_PAUSE);
	}
};

static CLocalStateManager s_stateMgr;

namespace PlayStateCenter
{
	IStateManager* GetStateManager() { return &s_stateMgr; }
	void InitPlayStateCenter()
	{
		AnchorAt(milliseconds(0), EPS_PAUSE);
	}
	void UninitPlayStateCenter() {}
}

// ==================== 区段边界：到头暂停 / 循环（本地，无网络） ====================
static void HandleSectionBoundaryWhilePlaying(milliseconds curMs)
{
	if (!g_pTimeLineModel || g_playState.state != EPS_PLAYING)
		return;

	const int mode = g_pTimeLineModel->GetSectionPlayMode();
	auto pSection = g_pTimeLineModel->GetSection(curMs).lock();
	if (!pSection)
	{
		// 超出时长：停在末尾
		const auto dur = g_pTimeLineModel->GetDuration();
		if (curMs >= dur && dur.count() > 0)
		{
			FreezeAt(dur, EPS_PAUSE);
			if (GetPlayerController())
				GetPlayerController()->NotifyPlayStateChanged();
		}
		return;
	}

	const auto sectionEnd = pSection->GetSectionEndTime();
	// 到达或越过 section 末尾（留 1ms 防止卡在下一段）
	if (curMs + milliseconds(1) < sectionEnd)
		return;

	if (mode == ESectionPlayMode::ECPM_EndOfSection
		|| mode == ITimeLineModel::SectionPlayMode_EndOfSection)
	{
		auto endPos = sectionEnd;
		if (endPos.count() > 0)
			endPos -= milliseconds(1);
		FreezeAt(endPos, EPS_PAUSE_SECTION_END);
		if (GetPlayerController())
			GetPlayerController()->NotifyPlayStateChanged();
		return;
	}

	if (mode == ESectionPlayMode::ECPM_LoopSection
		|| mode == ITimeLineModel::SectionPlayMode_LoopSection)
	{
		AnchorAt(pSection->GetSectionStartTime(), EPS_PLAYING);
		if (GetPlayerController())
			GetPlayerController()->NotifyPlayStateChanged();
		return;
	}

	// 正常播放：越过最后一段末尾则暂停
	const auto dur = g_pTimeLineModel->GetDuration();
	if (curMs >= dur && dur.count() > 0)
	{
		FreezeAt(dur, EPS_PAUSE);
		if (GetPlayerController())
			GetPlayerController()->NotifyPlayStateChanged();
	}
}

// ==================== PlayerController ====================
static CPlayerController* s_player = nullptr;

CPlayerController::CPlayerController()
{
	m_pTickTimer = new QTimer(this);
	m_pTickTimer->setInterval(16); // ~60Hz，驱动游标与区段检测
	connect(m_pTickTimer, &QTimer::timeout, this, &CPlayerController::OnPlayTick);
	m_pTickTimer->start();
}

CPlayerController::~CPlayerController() = default;

void CPlayerController::OnPlayTick()
{
	if (g_playState.state == EPS_PLAYING)
	{
		const auto cur = ComputeProgressNow();
		HandleSectionBoundaryWhilePlaying(cur);
		// 仅帧对齐时间变化时刷 UI，避免 16ms 无条件脏标拖垮时间线
		const auto aligned = g_pConvertHelper
			? g_pConvertHelper->formatTimeWithFrame(cur) : cur;
		if (aligned != m_lastUiAlignedMs)
		{
			m_lastUiAlignedMs = aligned;
			m_bViewNeedUpdate = true;
		}
	}
}

void CPlayerController::Play()
{
	const auto cur = ComputeProgressNow();
	AnchorAt(cur, EPS_PLAYING);
	m_bViewNeedUpdate = true;
	emit signalPlayStateChanged();
}

void CPlayerController::Pause()
{
	const auto cur = ComputeProgressNow();
	FreezeAt(cur, EPS_PAUSE);
	m_bViewNeedUpdate = true;
	emit signalPlayStateChanged();
}

void CPlayerController::Stop()
{
	FreezeAt(milliseconds(0), EPS_PAUSE);
	m_bViewNeedUpdate = true;
	emit signalPlayStateChanged();
}

void CPlayerController::Seek(const milliseconds& ms)
{
	const auto aligned = ClampTimelineMs(ms);
	if (g_playState.state == EPS_PLAYING)
		AnchorAt(aligned, EPS_PLAYING);
	else
		FreezeAt(aligned, g_playState.state == EPS_PAUSE_SECTION_END ? EPS_PAUSE : g_playState.state);
	m_bViewNeedUpdate = true;
}

void CPlayerController::SeekObject(const QUuid&) {}
void CPlayerController::SeekMedia(const QUuid&, long long) {}

milliseconds CPlayerController::GetCurTime()
{
	return ComputeProgressNow();
}

bool CPlayerController::IsPlaying()
{
	return g_playState.state == EPS_PLAYING;
}

bool CPlayerController::IsStop()
{
	return g_playState.state != EPS_PLAYING && ComputeProgressNow().count() == 0;
}

bool CPlayerController::IsViewNeedUpdate()
{
	if (m_bViewNeedUpdate)
	{
		m_bViewNeedUpdate = false;
		return true;
	}
	return false;
}

void CPlayerController::SetPlayRangeMode(ETimelinePlayRangeMode mode) { m_playRangeMode = mode; }
ETimelinePlayRangeMode CPlayerController::GetPlayRangeMode() const { return m_playRangeMode; }

void CPlayerController::DelMediaAndFreePlayer(const QUuid&) {}
void CPlayerController::CreatePlayer(const QUuid&, const QUuid&) {}
void CPlayerController::ChangePlayer(const QUuid&, const QUuid&) {}
void CPlayerController::DeletePlayer(const QUuid&, const QUuid&) {}
void CPlayerController::ClearPlayers() {}
void CPlayerController::UpdatePlayerClipStartEndTime(const QUuid&) {}
void CPlayerController::ResetPlayersByHistoryStack() {}
void CPlayerController::NotifyAddMediaRender(const QUuid&) {}
void CPlayerController::NotifyRemoveRenderItem(const QUuid&) {}
void CPlayerController::NotifyClearAllRenderItem() {}
void CPlayerController::ReplaceRes(const QUuid&) {}
void CPlayerController::UpdatePlayerPty(const QUuid&) {}
void CPlayerController::SetDirty(const QUuid&) {}

void CPlayerController::MarkViewNeedUpdate() { m_bViewNeedUpdate = true; }
void CPlayerController::NotifyPlayStateChanged()
{
	m_bViewNeedUpdate = true;
	emit signalPlayStateChanged();
}

void InitPlayerController()
{
	if (!s_player)
		s_player = new CPlayerController;
}

CPlayerController* GetPlayerController()
{
	if (!s_player)
		InitPlayerController();
	return s_player;
}

void UninitPlayerController()
{
	delete s_player;
	s_player = nullptr;
}

// ==================== PlayCommandCenter：本地一体，无网络/无播放端分离 ====================
namespace PlayCommandCenter
{
	static void RefreshTimelineUi()
	{
		emit g_pSignalCenter->signalRefreshWindow(
			SignalCenter::EWT_TimeLineWnd,
			static_cast<int>(WndType::WndType_NoniusView | WndType::WndType_PlayControl
				| WndType::WndType_TimeLineTool | WndType::WndType_LayersTool));
	}

	void InitPlayCommandController() {}
	void InitPlayCommandDisplayer() {}
	void Release() {}
	void UninitPlayCommand() {}
	CPlayCommandCenter* GetInstance() { return nullptr; }
	DisplayerCommander* GetDisplayerCommander() { return nullptr; }
	void SetNetBridge(IPlayCommandNetBridge*) {}
	void SetFps(double) {}
	void SetIsDisplayer(bool) {}

	void OnControllerPlay(const SCellPlayState& s)
	{
		auto* p = GetPlayerController();
		if (!p)
			return;
		p->Seek(milliseconds(s.seek));
		p->Play();
		if (g_pContentViewData)
			g_pContentViewData->SetNoniusTime(p->GetCurTime());
		RefreshTimelineUi();
	}

	void OnControllerPause(const SCellPlayState& s)
	{
		auto* p = GetPlayerController();
		if (!p)
			return;
		if (s.seek >= 0)
			p->Seek(milliseconds(s.seek));
		p->Pause();
		if (g_pContentViewData)
			g_pContentViewData->SetNoniusTime(p->GetCurTime());
		RefreshTimelineUi();
	}

	void OnControllerSeek(const SCellPlayState& s)
	{
		auto* p = GetPlayerController();
		if (!p)
			return;
		p->Seek(milliseconds(s.seek));
		if (g_pContentViewData)
			g_pContentViewData->SetNoniusTime(p->GetCurTime());
		RefreshTimelineUi();
	}

	void OnControllerSeekToPlay(const SCellPlayState& s)
	{
		OnControllerSeek(s);
		OnControllerPlay(s);
	}

	void OnControllerSeekToPause(const SCellPlayState& s)
	{
		OnControllerSeek(s);
		OnControllerPause(s);
	}

	void OnControllerSeekToPauseImmediate(const SCellPlayState& s)
	{
		OnControllerSeekToPause(s);
	}

	void OnControllerSeekToPauseAfterTimelineEdit(const QUuid&)
	{
		auto* p = GetPlayerController();
		if (!p)
			return;
		SCellPlayState s;
		s.seek = p->GetCurTime().count();
		OnControllerSeekToPause(s);
	}

	void OnControllerSmartSeek(const SCellPlayState& s)
	{
		auto* p = GetPlayerController();
		if (!p)
			return;
		if (p->IsPlaying())
			OnControllerSeekToPlay(s);
		else
			OnControllerSeekToPause(s);
		// 预览 Seek→FinishSeek 由 SeekTimelineAt 的 signalNoniusTimeChanged 驱动（暂停有防抖）
	}

	void OnControllerSwitch(const SCellPlayState& s)
	{
		// 区段循环跳转：本地直接 seek 继续播
		OnControllerSeekToPlay(s);
	}

	void OnControllerStop()
	{
		auto* p = GetPlayerController();
		if (!p)
			return;
		p->Stop();
		if (g_pContentViewData)
			g_pContentViewData->SetNoniusTime(milliseconds(0));
		RefreshTimelineUi();
	}

	void OnSelfPlayStateReport(const SIpPlayState&) {}
	void OnPlayStateReport(const std::string&, const SIpPlayState&) {}
	void OnCellReady(std::string, SCellPlayState) {}
	bool IsWaitingSyncCmd() { return false; }
}
