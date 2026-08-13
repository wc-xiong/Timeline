#include "TimeLineModelExport.h"
#include "TimeLineModel.h"

#include <QDebug>
#include <algorithm>
#include "Common/Common/QUuidEx.h"

#include <chrono>
#include <memory>
#include <unordered_set>
#include <vector>

using namespace std::chrono;

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CTimeLineModel* gTimeLineModel = nullptr;
TIMELINEMODEL_EXPORT void InitTimeLineModel()
{
	gTimeLineModel = new CTimeLineModel();
}

TIMELINEMODEL_EXPORT ITimeLineModel* GetTimeLineModel()
{
	return gTimeLineModel;
}

TIMELINEMODEL_EXPORT void UninitTimeLineModel()
{
	delete gTimeLineModel;
}

namespace TimelineModel
{
	bool IsMsInClip(const std::shared_ptr<ITimeLineObject>& pObj, milliseconds ms)
	{
		if (!pObj)
			return false;

		const milliseconds start = pObj->GetStartTime();
		const milliseconds end = start + pObj->GetDuration();
		return ms >= start && ms < end;
	}

	std::vector<QUuid> GetLayerPlayingClipIds(
		const std::shared_ptr<ITimeLineLayer>& pLayer,
		milliseconds ms)
	{
		std::vector<QUuid> playingIds;
		if (!pLayer)
			return playingIds;
		const auto& vecMediaIds = pLayer->GetAllObjectIDs();
		for (const auto& id : vecMediaIds)
		{
			const auto pObj = pLayer->GetObject(id).lock();
			if (IsMsInClip(pObj, ms))
				playingIds.push_back(id);
		}

		return playingIds;
	}

	std::vector<QUuid> GetAllPlayingClipIds(std::chrono::milliseconds ms)
	{
		const auto vecLayerIds = g_pTimeLineModel->GetAllLayerIDs();
		std::vector<QUuid> mediaIds;
		for (int i = vecLayerIds.size() - 1; i >= 0; i--)
		{
			auto pLayer = g_pTimeLineModel->GetLayer(vecLayerIds[i]).lock();
			if (!pLayer || pLayer->IsHide())
				continue;

			const auto &overlapping = GetLayerPlayingClipIds(pLayer, ms);
			for (const auto& clipId : overlapping)
				mediaIds.push_back(clipId);
		}

		return mediaIds;
	}

	void GetLayerPlayingAndNotPlayingClipIds(
		const std::shared_ptr<ITimeLineLayer>& pLayer,
		milliseconds ms,
		std::vector<QUuid>& playingIds,
		std::vector<QUuid>& notPlayingIds)
	{
		playingIds.clear();
		notPlayingIds.clear();
		if (!pLayer)
			return;

		playingIds = GetLayerPlayingClipIds(pLayer, ms);
		const std::unordered_set<QUuid, SUuidHashFunc> playingSet(playingIds.begin(), playingIds.end());

		for (const auto& id : pLayer->GetAllObjectIDs())
		{
			if (playingSet.find(id) == playingSet.end())
				notPlayingIds.push_back(id);
		}
	}
}
