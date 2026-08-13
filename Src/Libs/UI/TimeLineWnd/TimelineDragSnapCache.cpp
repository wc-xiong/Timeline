#include "TimelineDragSnapCache.h"
#include "LayersWidget.h"
#include "TimelineWinMacros.h"

namespace
{
	std::set<std::chrono::milliseconds> g_snapTimes;
	bool g_bActive = false;
}

namespace TimelineDragSnapCache
{
	void Begin(CLayersWidget* pLayers, const QVector<QUuid>& excludeIds)
	{
		g_bActive = false;
		g_snapTimes.clear();
		if (!pLayers)
			return;
		g_snapTimes = pLayers->CollectSnapTimes(excludeIds);
		g_bActive = true;
	}

	void Clear()
	{
		g_bActive = false;
		g_snapTimes.clear();
	}

	bool IsActive()
	{
		return g_bActive;
	}

	const std::set<std::chrono::milliseconds>& Get()
	{
		return g_snapTimes;
	}
}
