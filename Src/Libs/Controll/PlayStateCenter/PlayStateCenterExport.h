#pragma once
#define PLAYSTATECENTER_STATIC_LIB
#include "PlayStateCenterGlobal.h"
#include "Common/Common/CommonDefine.h"
#include "Common/Common/PlayCommand.h"
#include <QUuid>
#include <chrono>

struct STimelinePlayInfo
{
	std::chrono::microseconds progress{ 0 };
};

class IStateManager
{
public:
	virtual ~IStateManager() = default;
	virtual EnPlayState GetTimelineState() const = 0;
	virtual STimelinePlayInfo GetTimelinePlayInfo(
		std::chrono::microseconds ts, const QUuid& timelineId) const = 0;
	virtual void FinishSeekTimeline(std::chrono::microseconds ts, const SCellPlayState& state) = 0;
	virtual void PauseTimeline(std::chrono::microseconds ts, const SCellPlayState& state) = 0;
};

namespace PlayStateCenter
{
	inline const QUuid kDefaultTimelineId = QUuid();

	PLAYSTATECENTER_EXPORT IStateManager* GetStateManager();
	PLAYSTATECENTER_EXPORT void InitPlayStateCenter();
	PLAYSTATECENTER_EXPORT void UninitPlayStateCenter();
}
