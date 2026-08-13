#include "TimeLineOverlapHelper.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "ContentViewData.h"
#include "ObjectDragMime.h"
#include "LayersWidget.h"
#include "PathCameraClipHelper.h"
#include <QApplication>
#include <algorithm>
#include <map>
#include <tuple>
#include <vector>
#include "TimelineWinMacros.h"
#ifdef GetObject
#undef GetObject
#endif
namespace
{
	bool IntervalCoversOther(const SPendingInterval& cover, const SPendingInterval& inner)
	{
		if (cover.layerIndex != inner.layerIndex)
			return false;
		return TimeLineOverlapHelper::FullyCovers(cover.start, cover.end, inner.start, inner.end);
	}

	bool DoesIntervalFullyCoverPending(
		const SPendingInterval& cover,
		const std::vector<SPendingInterval>& pending,
		size_t skipIndex)
	{
		for (size_t i = 0; i < pending.size(); ++i)
		{
			if (i == skipIndex)
				continue;
			if (IntervalCoversOther(cover, pending[i]))
				return true;
		}
		return false;
	}

	std::vector<SOverlapPair> DetectOverlapsFromObjectIntervals(
		const std::vector<SObjectInterval>& objects)
	{
		std::vector<SOverlapPair> result;
		for (size_t i = 0; i < objects.size(); ++i)
		{
			for (size_t j = i + 1; j < objects.size(); ++j)
			{
				const auto& a = objects[i];
				const auto& b = objects[j];
				const auto overlapDur = TimeLineOverlapHelper::CalcOverlapDuration(
					a.start, a.duration, b.start, b.duration);
				if (overlapDur.count() <= 0)
					continue;

				const bool aIsLeft = a.start <= b.start;
				const auto& left = aIsLeft ? a : b;
				const auto& right = aIsLeft ? b : a;
				const auto overlapStart = std::max(left.start, right.start);

				SOverlapPair pair;
				pair.leftObjectId = left.objectId;
				pair.rightObjectId = right.objectId;
				pair.overlapStart = overlapStart;
				pair.overlapDuration = overlapDur;
				result.push_back(pair);
			}
		}
		return result;
	}

	// 模拟 RefreshTransFxForLayer：每对重叠使用完整重叠时长，校验转场是否合法
	bool CanLayerSatisfyTransFxRulesAfterRefresh(const std::vector<SObjectInterval>& objects)
	{
		if (objects.empty())
			return true;

		const auto pairs = DetectOverlapsFromObjectIntervals(objects);

		for (const auto& obj : objects)
		{
			if (obj.duration.count() <= 0)
				continue;

			std::chrono::milliseconds overlapSum{ 0 };
			for (const auto& pair : pairs)
			{
				if (pair.leftObjectId == obj.objectId || pair.rightObjectId == obj.objectId)
					overlapSum += pair.overlapDuration;
			}
			if (overlapSum >= obj.duration)
				return false;
		}

		for (const auto& pair : pairs)
		{
			const SObjectInterval* pLeft = nullptr;
			for (const auto& obj : objects)
			{
				if (obj.objectId == pair.leftObjectId)
				{
					pLeft = &obj;
					break;
				}
			}
			if (!pLeft)
				continue;

			const auto leftEnd = pLeft->start + pLeft->duration;
			const auto overlapStart = pair.overlapStart;
			const auto overlapEnd = leftEnd;

			for (const auto& other : objects)
			{
				if (other.objectId == pair.leftObjectId || other.objectId == pair.rightObjectId)
					continue;
				const auto otherEnd = other.start + other.duration;
				if (TimeLineOverlapHelper::FullyCovers(overlapStart, overlapEnd, other.start, otherEnd))
					return false;
			}
		}

		return true;
	}

	bool BuildLayerIntervalsAfterObjectDrag(
		const STimelineObjectDragPayload& payload,
		const std::chrono::milliseconds& anchorTime,
		bool bCreateNewLayer,
		int dropLayerIndex,
		int newLayerIndex,
		const std::vector<QUuid>& layerIds,
		std::map<QUuid, std::vector<SObjectInterval>>& outLayerIntervals,
		std::vector<SObjectInterval>& outVirtualNewLayerIntervals)
	{
		outLayerIntervals.clear();
		outVirtualNewLayerIntervals.clear();
		std::set<QUuid> dragObjectIds;
		for (const QUuid& id : payload.objectIds)
			dragObjectIds.insert(id);

		struct SPendingMove
		{
			QUuid objectId;
			QUuid targetLayerId;
			std::chrono::milliseconds newStart{ 0 };
			std::chrono::milliseconds duration{ 0 };
		};
		std::vector<SPendingMove> pendingMoves;
		pendingMoves.reserve(payload.objectIds.size());

		for (int i = 0; i < payload.objectIds.size(); ++i)
		{
			const QUuid& objId = payload.objectIds[i];
			auto pOriginalLayer = g_pTimeLineModel->GetLayer(objId).lock();
			if (!pOriginalLayer)
				continue;
			auto pObject = pOriginalLayer->GetObject(objId).lock();
			if (!pObject)
				continue;

			std::chrono::milliseconds newTime = anchorTime + std::chrono::milliseconds(payload.timeOffsetMs[i]);
			if (newTime < std::chrono::milliseconds(0))
				newTime = std::chrono::milliseconds(0);

			SPendingMove move;
			move.objectId = objId;
			move.newStart = newTime;
			move.duration = pObject->GetDuration();

			if (bCreateNewLayer)
			{
				move.targetLayerId = QUuid();
			}
			else
			{
				int targetIndex = dropLayerIndex + payload.layerIndexOffset[i];
				targetIndex = std::max(0, std::min(targetIndex, static_cast<int>(layerIds.size()) - 1));
				move.targetLayerId = layerIds[targetIndex];
			}
			pendingMoves.push_back(move);
		}

		if (bCreateNewLayer)
		{
			for (const auto& move : pendingMoves)
				outVirtualNewLayerIntervals.push_back({ move.objectId, move.newStart, move.duration });

			std::set<QUuid> sourceLayerIds;
			for (const auto& move : pendingMoves)
			{
				auto pOriginalLayer = g_pTimeLineModel->GetLayer(move.objectId).lock();
				if (pOriginalLayer)
					sourceLayerIds.insert(pOriginalLayer->GetId());
			}
			for (const QUuid& layerId : sourceLayerIds)
			{
				auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
				if (!pLayer)
					continue;

				std::vector<SObjectInterval> intervals;
				for (const auto& objId : pLayer->GetAllObjectIDs())
				{
					if (dragObjectIds.count(objId) > 0)
						continue;
					auto pObject = pLayer->GetObject(objId).lock();
					if (!pObject)
						continue;
					intervals.push_back({ objId, pObject->GetStartTime(), pObject->GetDuration() });
				}
				outLayerIntervals[layerId] = std::move(intervals);
			}
			return !pendingMoves.empty();
		}

		std::set<QUuid> affectedLayerIds;
		for (const auto& move : pendingMoves)
		{
			affectedLayerIds.insert(move.targetLayerId);
			auto pOriginalLayer = g_pTimeLineModel->GetLayer(move.objectId).lock();
			if (pOriginalLayer)
				affectedLayerIds.insert(pOriginalLayer->GetId());
		}

		for (const QUuid& layerId : affectedLayerIds)
		{
			auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
			if (!pLayer)
				continue;

			std::vector<SObjectInterval> intervals;
			for (const auto& objId : pLayer->GetAllObjectIDs())
			{
				if (dragObjectIds.count(objId) > 0)
					continue;
				auto pObject = pLayer->GetObject(objId).lock();
				if (!pObject)
					continue;
				intervals.push_back({ objId, pObject->GetStartTime(), pObject->GetDuration() });
			}

			for (const auto& move : pendingMoves)
			{
				if (move.targetLayerId != layerId)
					continue;
				intervals.push_back({ move.objectId, move.newStart, move.duration });
			}

			outLayerIntervals[layerId] = std::move(intervals);
		}

		return !outLayerIntervals.empty();
	}

	bool CanSatisfyTransFxRulesAfterObjectDrag(
		const STimelineObjectDragPayload& payload,
		const std::chrono::milliseconds& anchorTime,
		bool bCreateNewLayer,
		int dropLayerIndex,
		int newLayerIndex,
		const std::vector<QUuid>& layerIds)
	{
		std::map<QUuid, std::vector<SObjectInterval>> layerIntervals;
		std::vector<SObjectInterval> virtualNewLayerIntervals;
		if (!BuildLayerIntervalsAfterObjectDrag(
			payload, anchorTime, bCreateNewLayer, dropLayerIndex, newLayerIndex, layerIds,
			layerIntervals, virtualNewLayerIntervals))
			return true;

		if (bCreateNewLayer)
		{
			if (!CanLayerSatisfyTransFxRulesAfterRefresh(virtualNewLayerIntervals))
				return false;
		}

		for (const auto& entry : layerIntervals)
		{
			if (!CanLayerSatisfyTransFxRulesAfterRefresh(entry.second))
				return false;
		}
		return true;
	}
}

namespace TimeLineOverlapHelper
{
	std::chrono::milliseconds CalcOverlapDuration(
		std::chrono::milliseconds startA,
		std::chrono::milliseconds durA,
		std::chrono::milliseconds startB,
		std::chrono::milliseconds durB)
	{
		const auto endA = startA + durA;
		const auto endB = startB + durB;
		const auto overlapStart = std::max(startA, startB);
		const auto overlapEnd = std::min(endA, endB);
		if (overlapEnd <= overlapStart)
			return std::chrono::milliseconds(0);
		return overlapEnd - overlapStart;
	}

	std::vector<SOverlapPair> DetectLayerOverlaps(const QUuid& layerId)
	{
		std::vector<SOverlapPair> result;
		auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
		if (!pLayer)
			return result;

		struct SObjectTime
		{
			QUuid id;
			std::chrono::milliseconds start;
			std::chrono::milliseconds duration;
		};

		std::vector<SObjectTime> objects;
		for (const auto& objId : pLayer->GetAllObjectIDs())
		{
			auto pObject = pLayer->GetObject(objId).lock();
			if (!pObject)
				continue;
			objects.push_back({ objId, pObject->GetStartTime(), pObject->GetDuration() });
		}

		for (size_t i = 0; i < objects.size(); ++i)
		{
			for (size_t j = i + 1; j < objects.size(); ++j)
			{
				const auto& a = objects[i];
				const auto& b = objects[j];
				const auto overlapDur = CalcOverlapDuration(a.start, a.duration, b.start, b.duration);
				if (overlapDur.count() <= 0)
					continue;
				// 路径相机片段不参与转场
				if (PathCameraClipHelper::IsPathCameraClip(a.id)
					|| PathCameraClipHelper::IsPathCameraClip(b.id))
					continue;

				const bool aIsLeft = a.start <= b.start;
				const auto& left = aIsLeft ? a : b;
				const auto& right = aIsLeft ? b : a;
				const auto overlapStart = std::max(left.start, right.start);

				SOverlapPair pair;
				pair.leftObjectId = left.id;
				pair.rightObjectId = right.id;
				pair.overlapStart = overlapStart;
				pair.overlapDuration = overlapDur;
				result.push_back(pair);
			}
		}

		return result;
	}

	bool GetTransFxObjectTimes(
		const QUuid& leftObjectId,
		const QUuid& rightObjectId,
		STransFxObjectTimes& outTimes)
	{
		auto pLeftObj = g_pTimeLineModel->GetTimeLineObject(leftObjectId).lock();
		auto pRightObj = g_pTimeLineModel->GetTimeLineObject(rightObjectId).lock();
		if (!pLeftObj || !pRightObj)
			return false;

		outTimes.leftStart = pLeftObj->GetStartTime();
		outTimes.leftEnd = outTimes.leftStart + pLeftObj->GetDuration();
		outTimes.rightStart = pRightObj->GetStartTime();
		outTimes.rightEnd = outTimes.rightStart + pRightObj->GetDuration();
		return true;
	}

	bool FullyCovers(
		std::chrono::milliseconds coverStart,
		std::chrono::milliseconds coverEnd,
		std::chrono::milliseconds innerStart,
		std::chrono::milliseconds innerEnd)
	{
		return coverStart <= innerStart && coverEnd >= innerEnd;
	}

	bool DoesIntervalFullyCoverOtherOnLayer(
		const QUuid& layerId,
		std::chrono::milliseconds coverStart,
		std::chrono::milliseconds coverEnd,
		const std::set<QUuid>& excludeIds)
	{
		auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
		if (!pLayer)
			return false;

		for (const auto& objId : pLayer->GetAllObjectIDs())
		{
			if (excludeIds.count(objId) > 0)
				continue;

			auto pOther = pLayer->GetObject(objId).lock();
			if (!pOther)
				continue;

			const auto otherStart = pOther->GetStartTime();
			const auto otherEnd = otherStart + pOther->GetDuration();
			if (FullyCovers(coverStart, coverEnd, otherStart, otherEnd))
				return true;
		}
		return false;
	}

	bool IsIntervalFullyCoveredByOtherOnLayer(
		const QUuid& layerId,
		std::chrono::milliseconds intervalStart,
		std::chrono::milliseconds intervalEnd,
		const std::set<QUuid>& excludeIds)
	{
		auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
		if (!pLayer)
			return false;

		for (const auto& objId : pLayer->GetAllObjectIDs())
		{
			if (excludeIds.count(objId) > 0)
				continue;

			auto pOther = pLayer->GetObject(objId).lock();
			if (!pOther)
				continue;

			const auto otherStart = pOther->GetStartTime();
			const auto otherEnd = otherStart + pOther->GetDuration();
			if (FullyCovers(otherStart, otherEnd, intervalStart, intervalEnd))
				return true;
		}
		return false;
	}

	bool IsObjectFullyCoveringOtherOnLayer(
		const QUuid& layerId,
		const QUuid& subjectId,
		std::chrono::milliseconds subjectStart,
		std::chrono::milliseconds subjectDuration,
		const std::set<QUuid>& excludeIds)
	{
		std::set<QUuid> mergedExclude = excludeIds;
		mergedExclude.insert(subjectId);
		const auto subjectEnd = subjectStart + subjectDuration;
		return DoesIntervalFullyCoverOtherOnLayer(layerId, subjectStart, subjectEnd, mergedExclude);
	}

	std::chrono::milliseconds SumTransFxOverlapOnObject(
		const QUuid& objectId,
		const QUuid& editingTransFxId,
		std::chrono::milliseconds proposedRightStart)
	{
		std::chrono::milliseconds sum{ 0 };
		if (objectId.isNull())
			return sum;

		for (const auto& pTransFx : g_pTimeLineModel->GetTransFxsByObject(objectId))
		{
			if (!pTransFx)
				continue;

			auto pLeftObject = g_pTimeLineModel->GetTimeLineObject(pTransFx->GetLeftObjectId()).lock();
			auto pRightObject = g_pTimeLineModel->GetTimeLineObject(pTransFx->GetRightObjectId()).lock();
			if (!pLeftObject || !pRightObject)
				continue;

			auto rightStart = pRightObject->GetStartTime();
			if (pTransFx->GetId() == editingTransFxId)
				rightStart = proposedRightStart;

			sum += CalcOverlapDuration(
				pLeftObject->GetStartTime(), pLeftObject->GetDuration(),
				rightStart, pRightObject->GetDuration());
		}
		return sum;
	}

	bool IsObjectTransFxOverlapSumWithinDuration(
		const QUuid& objectId,
		const QUuid& editingTransFxId,
		std::chrono::milliseconds proposedRightStart)
	{
		auto pObject = g_pTimeLineModel->GetTimeLineObject(objectId).lock();
		if (!pObject)
			return false;

		const auto objectDuration = pObject->GetDuration();
		if (objectDuration.count() <= 0)
			return true;

		const auto overlapSum = SumTransFxOverlapOnObject(objectId, editingTransFxId, proposedRightStart);
		return overlapSum < objectDuration;
	}

	bool CanPlaceObjectOnLayer(const QUuid& layerId, const QUuid& objectId,
		std::chrono::milliseconds newStart, std::chrono::milliseconds newDuration,
		const std::set<QUuid>& excludeIds)
	{
		if (layerId.isNull() || objectId.isNull() || newDuration.count() <= 0)
			return true;

		std::set<QUuid> mergedExclude = excludeIds;
		mergedExclude.insert(objectId);
		const auto newEnd = newStart + newDuration;
		if (TimeLineOverlapHelper::IsObjectFullyCoveringOtherOnLayer(
			layerId, objectId, newStart, newDuration, excludeIds))
			return false;
		if (TimeLineOverlapHelper::IsIntervalFullyCoveredByOtherOnLayer(
			layerId, newStart, newEnd, mergedExclude))
			return false;
		return true;
	}

	std::set<QUuid> CollectObjectDragPathClipsSkippedDueToOverlap(
		const STimelineObjectDragPayload& payload,
		int dropLayerIndex, const QPoint& posInLayer, CLayersWidget* pLayers)
	{
		Q_UNUSED(dropLayerIndex);
		std::set<QUuid> skipped;
		if (!pLayers || payload.objectIds.isEmpty())
			return skipped;

		const SObjectDragSnapResult snap = CObjectDragMime::CalcObjectDragSnap(payload, posInLayer, pLayers);
		const std::chrono::milliseconds anchorTime = snap.anchorTime;

		std::set<QUuid> pathDragIds;
		for (const QUuid& id : payload.objectIds)
		{
			if (PathCameraClipHelper::IsPathCameraClip(id))
				pathDragIds.insert(id);
		}
		if (pathDragIds.empty())
			return skipped;

		std::set<QUuid> blockedCameraIds;
		for (int i = 0; i < payload.objectIds.size(); ++i)
		{
			const QUuid& objId = payload.objectIds[i];
			if (!pathDragIds.count(objId))
				continue;

			auto pObject = g_pTimeLineModel->GetTimeLineObject(objId).lock();
			if (!pObject)
				continue;

			std::chrono::milliseconds newTime = anchorTime + std::chrono::milliseconds(payload.timeOffsetMs[i]);
			if (newTime < std::chrono::milliseconds(0))
				newTime = std::chrono::milliseconds(0);

			QUuid camId;
			PathCameraClipHelper::FindCameraByPathClipId(objId, &camId);
			if (camId.isNull())
				continue;

			// 同批路径片段一并排除：相对位置不变时彼此不误报；仅检测与静止片段冲突
			if (PathCameraClipHelper::IsPathClipRangeOverlap(
				newTime, pObject->GetDuration(), objId, camId, &pathDragIds))
				blockedCameraIds.insert(camId);
		}

		if (blockedCameraIds.empty())
			return skipped;

		for (const QUuid& objId : pathDragIds)
		{
			QUuid camId;
			PathCameraClipHelper::FindCameraByPathClipId(objId, &camId);
			if (!camId.isNull() && blockedCameraIds.count(camId))
				skipped.insert(objId);
		}
		return skipped;
	}

	bool CanCommitObjectDrag(const STimelineObjectDragPayload& payload,
		int dropLayerIndex, const QPoint& posInLayer, CLayersWidget* pLayers)
	{
		if (!pLayers || payload.objectIds.isEmpty())
			return false;

		std::vector<QUuid> layerIds = g_pTimeLineModel->GetAllLayerIDs();
		if (layerIds.empty())
			return false;

		const SObjectDragSnapResult snap = CObjectDragMime::CalcObjectDragSnap(payload, posInLayer, pLayers);
		const std::chrono::milliseconds anchorTime = snap.anchorTime;
		const std::set<QUuid> skippedPathClips = CollectObjectDragPathClipsSkippedDueToOverlap(
			payload, dropLayerIndex, posInLayer, pLayers);

		const Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
		// 仅单选素材移动时 Alt 才可新建图层；多选无效
		const bool bAltPressed = (modifiers & Qt::AltModifier) != 0 && payload.objectIds.size() == 1;

		auto pAnchorLayer = g_pTimeLineModel->GetLayer(payload.anchorId).lock();
		const int anchorLayerIndex = pAnchorLayer
			? g_pTimeLineModel->GetLayerIndex(pAnchorLayer->GetId()) : -1;
		const bool bCrossedLayers = anchorLayerIndex >= 0 && dropLayerIndex != anchorLayerIndex;
		const bool bCreateNewLayer = bAltPressed && bCrossedLayers;
		int newLayerIndex = dropLayerIndex;

		if (bCreateNewLayer)
		{
			if (anchorLayerIndex > dropLayerIndex)
				newLayerIndex = dropLayerIndex + 1;
			else
				newLayerIndex = dropLayerIndex;
		}
		else
		{
			for (int i = 0; i < payload.objectIds.size(); ++i)
			{
				const QUuid& objId = payload.objectIds[i];
				if (skippedPathClips.count(objId))
					continue;
				int targetIndex = dropLayerIndex + payload.layerIndexOffset[i];
				if (targetIndex < 0 || targetIndex >= static_cast<int>(layerIds.size()))
					return false;
				auto pTargetLayer = g_pTimeLineModel->GetLayer(layerIds[targetIndex]).lock();
				if (!pTargetLayer || pTargetLayer->IsLock())
					return false;
			}
		}

		// 实际会移动的对象（跳过的路径片段仍留在原位，不可从覆盖检测中排除）
		std::set<QUuid> movingObjectIds;
		for (const QUuid& id : payload.objectIds)
		{
			if (!skippedPathClips.count(id))
				movingObjectIds.insert(id);
		}

		for (int i = 0; i < payload.objectIds.size(); ++i)
		{
			const QUuid& objId = payload.objectIds[i];
			if (skippedPathClips.count(objId))
				continue;

			int targetIndex = bCreateNewLayer
				? newLayerIndex
				: dropLayerIndex + payload.layerIndexOffset[i];
			if (!bCreateNewLayer)
			{
				targetIndex = std::max(0, std::min(targetIndex, static_cast<int>(layerIds.size()) - 1));
			}
			const QUuid targetLayerId = bCreateNewLayer ? QUuid() : layerIds[targetIndex];

			auto pOriginalLayer = g_pTimeLineModel->GetLayer(objId).lock();
			if (!pOriginalLayer)
				continue;
			auto pObject = pOriginalLayer->GetObject(objId).lock();
			if (!pObject)
				continue;

			std::chrono::milliseconds newTime = anchorTime + std::chrono::milliseconds(payload.timeOffsetMs[i]);
			if (newTime < std::chrono::milliseconds(0))
				newTime = std::chrono::milliseconds(0);

			if (!bCreateNewLayer)
			{
				if (!CanPlaceObjectOnLayer(targetLayerId, objId, newTime, pObject->GetDuration(), movingObjectIds))
					return false;
			}
		}

		if (bCreateNewLayer)
		{
			for (size_t i = 0; i < payload.objectIds.size(); ++i)
			{
				const QUuid& objId = payload.objectIds[i];
				if (skippedPathClips.count(objId))
					continue;
				auto pOriginalLayer = g_pTimeLineModel->GetLayer(objId).lock();
				if (!pOriginalLayer)
					continue;
				auto pObject = pOriginalLayer->GetObject(objId).lock();
				if (!pObject)
					continue;

				std::chrono::milliseconds newTime = anchorTime + std::chrono::milliseconds(payload.timeOffsetMs[i]);
				if (newTime < std::chrono::milliseconds(0))
					newTime = std::chrono::milliseconds(0);

				for (size_t j = 0; j < payload.objectIds.size(); ++j)
				{
					if (i == j || skippedPathClips.count(payload.objectIds[j]))
						continue;
					const QUuid& otherId = payload.objectIds[j];
					auto pOtherLayer = g_pTimeLineModel->GetLayer(otherId).lock();
					if (!pOtherLayer)
						continue;
					auto pOtherObject = pOtherLayer->GetObject(otherId).lock();
					if (!pOtherObject)
						continue;

					std::chrono::milliseconds otherTime = anchorTime + std::chrono::milliseconds(payload.timeOffsetMs[j]);
					if (otherTime < std::chrono::milliseconds(0))
						otherTime = std::chrono::milliseconds(0);

					const auto subjectEnd = newTime + pObject->GetDuration();
					const auto otherEnd = otherTime + pOtherObject->GetDuration();
					if (TimeLineOverlapHelper::FullyCovers(newTime, subjectEnd, otherTime, otherEnd)
						|| TimeLineOverlapHelper::FullyCovers(otherTime, otherEnd, newTime, subjectEnd))
						return false;
				}
			}
		}

		if (movingObjectIds.empty())
			return true;

		// 转场规则仅按实际会移动的对象模拟
		STimelineObjectDragPayload movingPayload = payload;
		movingPayload.objectIds.clear();
		movingPayload.timeOffsetMs.clear();
		movingPayload.layerIndexOffset.clear();
		for (int i = 0; i < payload.objectIds.size(); ++i)
		{
			if (skippedPathClips.count(payload.objectIds[i]))
				continue;
			movingPayload.objectIds.push_back(payload.objectIds[i]);
			movingPayload.timeOffsetMs.push_back(payload.timeOffsetMs[i]);
			movingPayload.layerIndexOffset.push_back(payload.layerIndexOffset[i]);
		}
		if (!movingPayload.objectIds.contains(movingPayload.anchorId) && !movingPayload.objectIds.isEmpty())
			movingPayload.anchorId = movingPayload.objectIds.first();

		return CanSatisfyTransFxRulesAfterObjectDrag(
			movingPayload, anchorTime, bCreateNewLayer, dropLayerIndex, newLayerIndex, layerIds);
	}

	bool IsObjectDragBlockedByPathClipOverlap(const STimelineObjectDragPayload& payload,
		int dropLayerIndex, const QPoint& posInLayer, CLayersWidget* pLayers)
	{
		return !CollectObjectDragPathClipsSkippedDueToOverlap(
			payload, dropLayerIndex, posInLayer, pLayers).empty();
	}

	bool CanPasteObjectsToLayer(const QPoint& posInLayer, int targetLayerIndex)
	{
		const auto& copyData = g_pContentViewData->GetTimelineCopyData();
		if (!copyData.HasData() || targetLayerIndex < 0)
			return false;

		auto msHit = g_pContentViewData->PositionToTime(posInLayer.x() - g_pContentViewData->GetXOffset());
		if (msHit < 0ms)
			msHit = 0ms;

		const auto layerIds = g_pTimeLineModel->GetAllLayerIDs();
		if (layerIds.empty() && targetLayerIndex > 0)
			return false;

		std::vector<SPendingInterval> pending;
		pending.reserve(copyData.vecCopyObjects.size());
		// 待粘贴路径片段区间（同相机互相重叠检测）：{camId, start, end}
		std::vector<std::tuple<QUuid, std::chrono::milliseconds, std::chrono::milliseconds>> pathPending;

		for (const auto& copyInfo : copyData.vecCopyObjects)
		{
			auto pSrcLayer = g_pTimeLineModel->GetLayer(copyInfo.sourceObjectId).lock();
			if (!pSrcLayer)
				continue;
			auto pSrcObj = pSrcLayer->GetObject(copyInfo.sourceObjectId).lock();
			if (!pSrcObj)
				continue;

			const bool bPathClip = PathCameraClipHelper::IsPathCameraClip(copyInfo.sourceObjectId);
			const bool bMedia = (pSrcObj->GetType() == ITimeLineObject::ObjectType_Media);
			if (!bMedia && !bPathClip)
				continue;

			const int destLayerIndex = targetLayerIndex
				+ static_cast<int>(copyInfo.nLayerIndex - copyData.nMinLayerIndex);
			if (destLayerIndex < 0)
				continue;

			QUuid destLayerId;
			if (destLayerIndex < static_cast<int>(layerIds.size()))
			{
				auto pDestLayer = g_pTimeLineModel->GetLayer(layerIds[destLayerIndex]).lock();
				if (!pDestLayer || pDestLayer->IsLock())
					continue;
				destLayerId = pDestLayer->GetId();
			}

			auto newStart = msHit + (copyInfo.sourceStartTime - copyData.msMinPos);
			if (newStart < 0ms)
				newStart = 0ms;

			if (bPathClip)
			{
				QUuid ownerCamId;
				PathCameraClipHelper::FindCameraByPathClipId(copyInfo.sourceObjectId, &ownerCamId);
				if (ownerCamId.isNull())
					continue;
				if (PathCameraClipHelper::IsPathClipRangeOverlap(
					newStart, copyInfo.sourceDuration, QUuid(), ownerCamId))
					return false;
				for (const auto& pr : pathPending)
				{
					if (std::get<0>(pr) != ownerCamId)
						continue;
					const auto dur = std::get<2>(pr) - std::get<1>(pr);
					if (TimeLineOverlapHelper::CalcOverlapDuration(
						newStart, copyInfo.sourceDuration, std::get<1>(pr), dur).count() > 0)
						return false;
				}
				pathPending.emplace_back(ownerCamId, newStart, newStart + copyInfo.sourceDuration);
			}

			SPendingInterval interval;
			interval.layerIndex = destLayerIndex;
			interval.layerId = destLayerId;
			interval.start = newStart;
			interval.end = newStart + copyInfo.sourceDuration;
			pending.push_back(interval);
		}

		if (pending.empty())
			return false;

		for (size_t i = 0; i < pending.size(); ++i)
		{
			const auto& item = pending[i];
			if (!item.layerId.isNull())
			{
				if (TimeLineOverlapHelper::DoesIntervalFullyCoverOtherOnLayer(
					item.layerId, item.start, item.end, {}))
					return false;
				if (TimeLineOverlapHelper::IsIntervalFullyCoveredByOtherOnLayer(
					item.layerId, item.start, item.end, {}))
					return false;

			}

			if (DoesIntervalFullyCoverPending(item, pending, i))
				return false;
		}

		return true;
	}

	bool IsPasteBlockedByPathClipOverlap(const QPoint& posInLayer, int targetLayerIndex)
	{
		using namespace std::chrono_literals;
		const auto& copyData = g_pContentViewData->GetTimelineCopyData();
		if (!copyData.HasData() || targetLayerIndex < 0)
			return false;

		auto msHit = g_pContentViewData->PositionToTime(posInLayer.x() - g_pContentViewData->GetXOffset());
		if (msHit < 0ms)
			msHit = 0ms;

		// 待粘贴路径片段区间（同相机互相重叠检测）：{camId, start, end}
		std::vector<std::tuple<QUuid, std::chrono::milliseconds, std::chrono::milliseconds>> pathPending;

		for (const auto& copyInfo : copyData.vecCopyObjects)
		{
			if (!PathCameraClipHelper::IsPathCameraClip(copyInfo.sourceObjectId))
				continue;

			auto newStart = msHit + (copyInfo.sourceStartTime - copyData.msMinPos);
			if (newStart < 0ms)
				newStart = 0ms;

			QUuid ownerCamId;
			PathCameraClipHelper::FindCameraByPathClipId(copyInfo.sourceObjectId, &ownerCamId);
			if (ownerCamId.isNull())
				continue;

			if (PathCameraClipHelper::IsPathClipRangeOverlap(
				newStart, copyInfo.sourceDuration, QUuid(), ownerCamId))
				return true;

			for (const auto& pr : pathPending)
			{
				if (std::get<0>(pr) != ownerCamId)
					continue;
				const auto dur = std::get<2>(pr) - std::get<1>(pr);
				if (TimeLineOverlapHelper::CalcOverlapDuration(
					newStart, copyInfo.sourceDuration, std::get<1>(pr), dur).count() > 0)
					return true;
			}
			pathPending.emplace_back(ownerCamId, newStart, newStart + copyInfo.sourceDuration);
		}
		return false;
	}

	bool CanCommitTransFxResize(const QUuid& transFxId,
		std::chrono::milliseconds newRightStart, std::chrono::milliseconds totalOverlapDuration)
	{
		if (transFxId.isNull() || totalOverlapDuration.count() <= 0)
			return false;

		auto pTransFx = g_pTimeLineModel->GetTransFx(transFxId).lock();
		if (!pTransFx)
			return false;

		auto pLeftObject = g_pTimeLineModel->GetTimeLineObject(pTransFx->GetLeftObjectId()).lock();
		auto pRightObject = g_pTimeLineModel->GetTimeLineObject(pTransFx->GetRightObjectId()).lock();
		if (!pLeftObject || !pRightObject)
			return false;

		auto pLayer = g_pTimeLineModel->GetLayer(pTransFx->GetLeftObjectId()).lock();
		if (!pLayer)
			return false;

		// 与该转场关联的左右素材：各自身上全部转场重叠时长之和须 < 素材时长
		if (!TimeLineOverlapHelper::IsObjectTransFxOverlapSumWithinDuration(
			pTransFx->GetLeftObjectId(), transFxId, newRightStart))
			return false;
		if (!TimeLineOverlapHelper::IsObjectTransFxOverlapSumWithinDuration(
			pTransFx->GetRightObjectId(), transFxId, newRightStart))
			return false;

		const auto leftEnd = pLeftObject->GetStartTime() + pLeftObject->GetDuration();
		const auto overlapStart = newRightStart;
		const auto overlapEnd = leftEnd;

		std::set<QUuid> participantIds;
		participantIds.insert(pTransFx->GetLeftObjectId());
		participantIds.insert(pTransFx->GetRightObjectId());

		if (TimeLineOverlapHelper::DoesIntervalFullyCoverOtherOnLayer(
			pLayer->GetId(), overlapStart, overlapEnd, participantIds))
			return false;

		if (!CanPlaceObjectOnLayer(
			pLayer->GetId(), pTransFx->GetRightObjectId(), newRightStart, pRightObject->GetDuration()))
			return false;

		return true;
	}
}
