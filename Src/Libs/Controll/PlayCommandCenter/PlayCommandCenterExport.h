#pragma once
#define PLAYCOMMANDCENTER_STATIC_LIB
#include "PlayCommandCenterGlobal.h"
#include "Common/Common/PlayCommand.h"
#include <QUuid>
#include <string>
#include <memory>

class DisplayerCommander;
class IPlayCommandNetBridge;
class CPlayCommandCenter;

namespace PlayCommandCenter
{
	PLAYCOMMANDCENTER_EXPORT void InitPlayCommandController();
	PLAYCOMMANDCENTER_EXPORT void InitPlayCommandDisplayer();
	PLAYCOMMANDCENTER_EXPORT void Release();
	PLAYCOMMANDCENTER_EXPORT void UninitPlayCommand();
	PLAYCOMMANDCENTER_EXPORT CPlayCommandCenter* GetInstance();
	PLAYCOMMANDCENTER_EXPORT DisplayerCommander* GetDisplayerCommander();
	PLAYCOMMANDCENTER_EXPORT void SetNetBridge(IPlayCommandNetBridge* bridge);
	PLAYCOMMANDCENTER_EXPORT void SetFps(double fps);
	PLAYCOMMANDCENTER_EXPORT void SetIsDisplayer(bool isDisplayer);

	PLAYCOMMANDCENTER_EXPORT void OnControllerPlay(const SCellPlayState& cellState);
	PLAYCOMMANDCENTER_EXPORT void OnControllerPause(const SCellPlayState& cellState);
	PLAYCOMMANDCENTER_EXPORT void OnControllerSeek(const SCellPlayState& cellState);
	PLAYCOMMANDCENTER_EXPORT void OnControllerSeekToPlay(const SCellPlayState& cellState);
	PLAYCOMMANDCENTER_EXPORT void OnControllerSeekToPause(const SCellPlayState& cellState);
	PLAYCOMMANDCENTER_EXPORT void OnControllerSeekToPauseImmediate(const SCellPlayState& cellState);
	PLAYCOMMANDCENTER_EXPORT void OnControllerSeekToPauseAfterTimelineEdit(const QUuid& timelineId);
	PLAYCOMMANDCENTER_EXPORT void OnControllerSmartSeek(const SCellPlayState& cellState);
	PLAYCOMMANDCENTER_EXPORT void OnControllerSwitch(const SCellPlayState& cellState);
	PLAYCOMMANDCENTER_EXPORT void OnControllerStop();
	PLAYCOMMANDCENTER_EXPORT void OnSelfPlayStateReport(const SIpPlayState& state);
	PLAYCOMMANDCENTER_EXPORT void OnPlayStateReport(const std::string& ip, const SIpPlayState& state);
	PLAYCOMMANDCENTER_EXPORT void OnCellReady(std::string ip, SCellPlayState cellState);

	PLAYCOMMANDCENTER_EXPORT bool IsWaitingSyncCmd();
}
