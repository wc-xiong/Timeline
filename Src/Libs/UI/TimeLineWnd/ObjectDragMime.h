#pragma once
#include <QMimeData>
#include <QPixmap>
#include <QPoint>
#include <QUuid>
#include <QVector>
#include <chrono>
#include <optional>

class CObjectWidget;
class CLayersWidget;

static const char* const g_TimelineObjectDragFormat = "TimelineObjectDrag";

struct STimelineObjectDragPayload
{
	QUuid anchorId;
	QPoint hotSpot;
	QVector<QUuid> objectIds;
	QVector<qint64> timeOffsetMs;
	QVector<int> layerIndexOffset;
};

// 素材拖拽吸附计算结果
struct SObjectDragSnapResult
{
	std::chrono::milliseconds anchorTime{ 0 };
	int snapLinePos = -1;
	bool bSnapped = false;
};

class CObjectDragMime
{
public:
	static QMimeData* BuildObjectDragMime(CObjectWidget* pAnchor, CLayersWidget* pLayers, const QPoint& pressHotSpot);
	static std::optional<STimelineObjectDragPayload> ParseObjectDragMime(const QMimeData* pMime);
	static QPixmap CreateObjectDragPixmap(CObjectWidget* pAnchor, CLayersWidget* pLayers, const STimelineObjectDragPayload& payload, QPoint& outHotSpot);
	static SObjectDragSnapResult CalcObjectDragSnap(const STimelineObjectDragPayload& payload, const QPoint& posInLayer, CLayersWidget* pLayers);
	static void ApplyObjectDragSnapPreview(const SObjectDragSnapResult& snap);
	static void PreviewObjectDragSnap(const QMimeData* pMime, const QPoint& posInLayer, CLayersWidget* pLayers);
	static bool CommitObjectDrag(const STimelineObjectDragPayload& payload, int dropLayerIndex, const QPoint& posInLayer, CLayersWidget* pLayers);
	static void PreviewResourceDragSnap(const std::chrono::milliseconds& mouseTime, int posLayersX, Qt::KeyboardModifiers modifiers);
	static std::chrono::milliseconds CalcResourceDropStartTime(const std::chrono::milliseconds& mouseTime, int posLayersX, Qt::KeyboardModifiers modifiers);
};
