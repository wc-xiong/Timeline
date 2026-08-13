#pragma once
#define PLAYERCONTROLLER_STATIC_LIB
#include "PlayerControllerGlobal.h"
#include <QObject>
#include <QUuid>
#include <chrono>

enum ETimelinePlayRangeMode
{
	TimelinePlayRangeMode_Normal = 0,
	TimelinePlayRangeMode_SectionEndPause,
	TimelinePlayRangeMode_SectionLoop,
};

class QTimer;

// Timeline 本地播控（不分离控制端/播放端）：时间推进 + 状态通知
class PLAYERCONTROLLER_EXPORT CPlayerController : public QObject
{
	Q_OBJECT
public:
	CPlayerController();
	~CPlayerController() override;

	void Play();
	void Pause();
	void Stop();
	void Seek(const std::chrono::milliseconds& ms);
	void SeekObject(const QUuid& objectId);
	void SeekMedia(const QUuid& resId, long long clipPlayTime);

	std::chrono::milliseconds GetCurTime();
	bool IsPlaying();
	bool IsStop();
	bool IsViewNeedUpdate();

	void SetPlayRangeMode(ETimelinePlayRangeMode mode);
	ETimelinePlayRangeMode GetPlayRangeMode() const;

	void DelMediaAndFreePlayer(const QUuid& mediaId);
	void CreatePlayer(const QUuid& id, const QUuid& Resid);
	void ChangePlayer(const QUuid& id, const QUuid& Resid);
	void DeletePlayer(const QUuid& id, const QUuid& Resid);
	void ClearPlayers();
	void UpdatePlayerClipStartEndTime(const QUuid& id);
	void ResetPlayersByHistoryStack();
	void NotifyAddMediaRender(const QUuid& mediaId);
	void NotifyRemoveRenderItem(const QUuid& mediaId);
	void NotifyClearAllRenderItem();
	void ReplaceRes(const QUuid& resId);
	void UpdatePlayerPty(const QUuid& resId);
	void SetDirty(const QUuid& mediaId);

	// 供 PlayCommandCenter 调用：标记视图需要刷新
	void MarkViewNeedUpdate();
	void NotifyPlayStateChanged();

signals:
	void signalPlayStateChanged();

private slots:
	void OnPlayTick();

private:
	bool m_bViewNeedUpdate = false;
	std::chrono::milliseconds m_lastUiAlignedMs{ -1 }; // 播放中 UI 帧对齐去抖
	ETimelinePlayRangeMode m_playRangeMode = TimelinePlayRangeMode_Normal;
	QTimer* m_pTickTimer = nullptr;
};

PLAYERCONTROLLER_EXPORT void InitPlayerController();
PLAYERCONTROLLER_EXPORT CPlayerController* GetPlayerController();
PLAYERCONTROLLER_EXPORT void UninitPlayerController();
#define g_pPlayerController GetPlayerController()
