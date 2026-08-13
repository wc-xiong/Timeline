#pragma once
#include "TimeLineModelGlobal.h"
#include "Common/Common/ITimeLineInterface.h"


TIMELINEMODEL_EXPORT void InitTimeLineModel();
TIMELINEMODEL_EXPORT ITimeLineModel* GetTimeLineModel();
TIMELINEMODEL_EXPORT void UninitTimeLineModel();

#define g_pTimeLineModel GetTimeLineModel()


namespace TimelineModel
{
	TIMELINEMODEL_EXPORT bool IsMsInClip(const std::shared_ptr<ITimeLineObject>& pObj, std::chrono::milliseconds ms);

	// hide media 不在返回容器内
	TIMELINEMODEL_EXPORT std::vector<QUuid> GetLayerPlayingClipIds(
		const std::shared_ptr<ITimeLineLayer>& pLayer,
		std::chrono::milliseconds ms);

	// hide media 不在返回容器内
	TIMELINEMODEL_EXPORT std::vector<QUuid> GetAllPlayingClipIds(
		std::chrono::milliseconds ms);

	TIMELINEMODEL_EXPORT void GetLayerPlayingAndNotPlayingClipIds(
		const std::shared_ptr<ITimeLineLayer>& pLayer,
		std::chrono::milliseconds ms,
		std::vector<QUuid>& playingIds,
		std::vector<QUuid>& notPlayingIds);
}
