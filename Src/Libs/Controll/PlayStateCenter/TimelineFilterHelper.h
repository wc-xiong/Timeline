#pragma once
#include "PlayStateCenterGlobal.h"

#include "Common/Common/MediaCommon.h"
#include "Common/Common/QUuidEx.h"

#include <chrono>
#include <memory>

class ITimeLineLayer;
class ITimeLineTransFx;
class PLAYSTATECENTER_EXPORT CTimelineFilterHelper
{
public:
	// 根据时间线进度为单个 clip 计算转场阶段与进度；无转场时返回 S_FALSE
	HRESULT CalcPlayStageAndProcess(
		const QUuid& clipId,
		std::chrono::milliseconds tlProgress,
		STransInfo& transInfo,
		bool bOnlyStage = false);

	// 从已填充的 STransInfo 推导有效 alpha
	static float GetEffectiveAlpha(const STransInfo& transInfo);

private:
	struct SOverlapContext
	{
		std::shared_ptr<ITimeLineLayer> pLayer;
		QUuid leftId;
		QUuid rightId;
		std::chrono::milliseconds leftEnd{ 0 };
		std::chrono::milliseconds overlapStart{ 0 };
		std::chrono::milliseconds overlapEnd{ 0 };
		std::chrono::milliseconds transDuration{ 0 };
		std::shared_ptr<ITimeLineTransFx> pTransFx;
		int nTransType = -1;
	};

	bool BuildOverlapContext(
		const QUuid& clipId,
		std::chrono::milliseconds tlProgress,
		SOverlapContext& ctx);

	HRESULT FillTransInfo(
		const QUuid& clipId,
		std::chrono::milliseconds tlProgress,
		const SOverlapContext& ctx,
		STransInfo& transInfo);

	HRESULT FillTransInfo_Fade(
		const QUuid& clipId,
		std::chrono::milliseconds tlProgress,
		const SOverlapContext& ctx,
		STransInfo& transInfo);

	HRESULT FillTransInfo_Vertex(
		const QUuid& clipId,
		std::chrono::milliseconds tlProgress,
		const SOverlapContext& ctx,
		STransInfo& transInfo);

	HRESULT FillTransInfo_Texture(
		const QUuid& clipId,
		std::chrono::milliseconds tlProgress,
		const SOverlapContext& ctx,
		STransInfo& transInfo);

	static float ClampProgress(float value);
	static bool IsVertexTransType(int nTransType);
};