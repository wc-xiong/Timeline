#pragma once
#include <chrono>
#include <set>
#include <QVector>
#include <QUuid>

class CLayersWidget;
// 素材拖拽期间缓存吸附时间点，避免 dragMove 每帧全时间线扫描
namespace TimelineDragSnapCache
{
	void Begin(CLayersWidget* pLayers, const QVector<QUuid>& excludeIds);
	void Clear();
	bool IsActive();
	const std::set<std::chrono::milliseconds>& Get();
}
