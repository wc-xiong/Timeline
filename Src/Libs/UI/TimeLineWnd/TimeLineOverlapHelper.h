#pragma once
#include <QUuid>
#include <chrono>
#include <set>
#include <vector>
#include <QPoint>
struct STimelineObjectDragPayload;
class CLayersWidget;
// 一对存在时间重叠的相邻素材（left.start <= right.start）
struct SOverlapPair
{
	QUuid leftObjectId;
	QUuid rightObjectId;
	std::chrono::milliseconds overlapStart;
	std::chrono::milliseconds overlapDuration;
};
/**
 * 待放置时间区间（粘贴校验用）。
 * 表示一次粘贴操作中，某个待粘贴素材在目标图层上的计划落位区间 [start, end)。
 * 用于 CanPasteObjectsToLayer：检测是否会完全覆盖图层上已有素材，或覆盖同批粘贴中的其它待放置区间。
 */
struct SPendingInterval
{
	int layerIndex = -1;
	QUuid layerId;
	std::chrono::milliseconds start{ 0 };
	std::chrono::milliseconds end{ 0 };
};
/**
 * 图层内素材时间区间快照（拖拽提交前模拟用）。
 * 从模型抽取或根据拖拽 payload 推算，不修改 TimeLineModel。
 * 用于 BuildLayerIntervalsAfterObjectDrag / CanLayerSatisfyTransFxRulesAfterRefresh：
 * 模拟 RefreshTransFxForLayer 后的重叠关系，校验转场重叠之和、转场区间是否完全覆盖其它素材等规则。
 */
struct SObjectInterval
{
	QUuid objectId;
	std::chrono::milliseconds start{ 0 };
	std::chrono::milliseconds duration{ 0 };
};
namespace TimeLineOverlapHelper
{
	// 计算两段时间区间的交集长度
	std::chrono::milliseconds CalcOverlapDuration(
		std::chrono::milliseconds startA,
		std::chrono::milliseconds durA,
		std::chrono::milliseconds startB,
		std::chrono::milliseconds durB);

	// 枚举图层内所有两两重叠，按 start 时间区分左右素材
	std::vector<SOverlapPair> DetectLayerOverlaps(const QUuid& layerId);

	// 转场关联的左右素材起止时间，用于计算重叠区与交界点 J
	struct STransFxObjectTimes
	{
		std::chrono::milliseconds leftStart{ 0 };
		std::chrono::milliseconds leftEnd{ 0 };
		std::chrono::milliseconds rightStart{ 0 };
		std::chrono::milliseconds rightEnd{ 0 };
	};

	// 读取转场左右素材的起止时间（布局与拉伸锚点计算依赖此数据）
	bool GetTransFxObjectTimes(
		const QUuid& leftObjectId,
		const QUuid& rightObjectId,
		STransFxObjectTimes& outTimes);

	// 区间 [coverStart, coverEnd) 是否完全覆盖 [innerStart, innerEnd)（含完全相同）
	bool FullyCovers(
		std::chrono::milliseconds coverStart,
		std::chrono::milliseconds coverEnd,
		std::chrono::milliseconds innerStart,
		std::chrono::milliseconds innerEnd);

	bool DoesIntervalFullyCoverOtherOnLayer(
		const QUuid& layerId,
		std::chrono::milliseconds coverStart,
		std::chrono::milliseconds coverEnd,
		const std::set<QUuid>& excludeIds = {});

	// 图层上是否存在素材完全覆盖区间 [intervalStart, intervalEnd)（含完全相同）
	bool IsIntervalFullyCoveredByOtherOnLayer(
		const QUuid& layerId,
		std::chrono::milliseconds intervalStart,
		std::chrono::milliseconds intervalEnd,
		const std::set<QUuid>& excludeIds = {});

	// 指定素材否完全覆盖任一其它素材
	bool IsObjectFullyCoveringOtherOnLayer(
		const QUuid& layerId,
		const QUuid& subjectId,
		std::chrono::milliseconds subjectStart,
		std::chrono::milliseconds subjectDuration,
		const std::set<QUuid>& excludeIds = {});

	// 素材上全部转场重叠时长之和（拟提交右起点仅作用于 editingTransFxId 对应转场）
	std::chrono::milliseconds SumTransFxOverlapOnObject(
		const QUuid& objectId,
		const QUuid& editingTransFxId,
		std::chrono::milliseconds proposedRightStart);

	// 素材上全部转场重叠时长之和是否严格小于素材时长（>= 则非法）
	bool IsObjectTransFxOverlapSumWithinDuration(
		const QUuid& objectId,
		const QUuid& editingTransFxId,
		std::chrono::milliseconds proposedRightStart);

	// 单素材放置：新区间不得完全覆盖同图层其它素材
	bool CanPlaceObjectOnLayer(const QUuid& layerId, const QUuid& objectId,
		std::chrono::milliseconds newStart, std::chrono::milliseconds newDuration,
		const std::set<QUuid>& excludeIds = {});

	// 拖拽提交前批量检测（路径片段重叠不整批拒绝：重叠的路径片段将跳过移动）
	bool CanCommitObjectDrag(const STimelineObjectDragPayload& payload,
		int dropLayerIndex, const QPoint& posInLayer, CLayersWidget* pLayers);

	/**
	 * @brief 多选拖拽时，因同相机时间重叠而应保持不动的路径相机片段
	 * @details 同一相机下，只要有一个被拖拽路径片段新区间与静止片段冲突，
	 *          则该相机本次拖拽中的全部路径片段均跳过，避免相对布局被拆散后产生新重叠。
	 *          媒体不受影响，仍可提交移动。
	 */
	std::set<QUuid> CollectObjectDragPathClipsSkippedDueToOverlap(
		const STimelineObjectDragPayload& payload,
		int dropLayerIndex, const QPoint& posInLayer, CLayersWidget* pLayers);

	// 拖拽落点是否会导致路径相机片段因重叠而被跳过
	bool IsObjectDragBlockedByPathClipOverlap(const STimelineObjectDragPayload& payload,
		int dropLayerIndex, const QPoint& posInLayer, CLayersWidget* pLayers);

	// 粘贴前检测（不创建对象）
	bool CanPasteObjectsToLayer(const QPoint& posInLayer, int targetLayerIndex);

	// 粘贴落点是否因路径相机片段时间重叠被拒绝
	bool IsPasteBlockedByPathClipOverlap(const QPoint& posInLayer, int targetLayerIndex);

	// 转场拉伸提交前检测
	bool CanCommitTransFxResize(const QUuid& transFxId,
		std::chrono::milliseconds newRightStart, std::chrono::milliseconds totalOverlapDuration);
}