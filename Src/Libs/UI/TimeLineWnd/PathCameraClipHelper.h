#pragma once
/**
 * Timeline：不迁移路径相机。空实现，全部返回 false / 空。
 */
#include <QUuid>
#include <QPoint>
#include <chrono>
#include <memory>
#include <set>
#include <vector>

#include "Common/Common/ITimeLineInterface.h"

/** 仅满足调用方对相机指针的方法名；永不返回非空实例 */
class IPathCameraStub
{
public:
	virtual ~IPathCameraStub() = default;
	virtual void AddPathCameraClipId(const QUuid&, std::chrono::milliseconds) {}
	virtual void SetPathCamera(bool) {}
	virtual void IncrementPathCameraClipCount() {}
};

namespace PathCameraClipHelper
{
	inline bool IsPathCameraClip(const std::shared_ptr<ITimeLineObject>&) { return false; }
	inline bool IsPathCameraClip(const QUuid&) { return false; }
	inline bool CanEditPathCameraKeyframes() { return false; }

	inline bool HitTestPathCameraKeyframe(
		const QUuid&,
		const QPoint&,
		int, int, int,
		std::chrono::milliseconds&)
	{
		return false;
	}

	inline bool IsPathCameraKeyframeEndpoint(const QUuid&, std::chrono::milliseconds) { return false; }

	inline bool MovePathCameraKeyframesAtTime(
		const QUuid&, std::chrono::milliseconds, std::chrono::milliseconds)
	{
		return false;
	}

	inline bool DeletePathCameraKeyframesAtTime(const QUuid&, std::chrono::milliseconds) { return false; }

	inline bool TryGetPlayheadSelectedPathKeyframe(const QUuid&, std::chrono::milliseconds&) { return false; }
	inline bool TryGetPlayheadSelectedPathKeyframe(
		const QUuid&, std::chrono::milliseconds, std::chrono::milliseconds&)
	{
		return false;
	}

	inline bool IsPathClipRangeOverlap(
		std::chrono::milliseconds, std::chrono::milliseconds,
		const QUuid& = QUuid(), const QUuid& = QUuid(),
		const std::set<QUuid>* = nullptr)
	{
		return false;
	}

	inline void SyncClipStartTimeToOwnerCamera(const QUuid&, std::chrono::milliseconds) {}

	inline void AdjustPathCameraKeyframesForRangeChange(
		const QUuid&,
		std::chrono::milliseconds, std::chrono::milliseconds,
		std::chrono::milliseconds, std::chrono::milliseconds)
	{}

	inline void RemoveClipFromOwnerCamera(const QUuid&) {}
	inline void RemoveAllPathClipsOwnedByCamera(const QUuid&) {}

	inline std::shared_ptr<IPathCameraStub> FindCameraByPathClipId(const QUuid&, QUuid* outCamId = nullptr)
	{
		if (outCamId) *outCamId = QUuid();
		return nullptr;
	}

	inline std::vector<std::chrono::milliseconds> CollectPathCameraKeyframeLocalTimes(const QUuid&)
	{
		return {};
	}
}
