#include "TimelineFilterHelper.h"


#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Common/Common/TransationTypeDefine.h"
#include "Common/Common/ITimelineInterface.h"

#include <algorithm>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

float CTimelineFilterHelper::ClampProgress(float value)
{
	return std::max(0.0f, std::min(1.0f, value));
}

bool CTimelineFilterHelper::IsVertexTransType(int nTransType)
{
	return nTransType >= ETFT_LeftMove && nTransType <= ETFT_PerspectiveRotationBottom;
}

float CTimelineFilterHelper::GetEffectiveAlpha(const STransInfo& transInfo)
{
	if (!transInfo.bTransition || transInfo.stage == ETFS_Normal)
		return 1.0f;

	if (transInfo.nTransType != ETFT_Fade)
		return 1.0f;

	if (transInfo.stage == ETFS_In)
		return ClampProgress(transInfo.fProgress);

	if (transInfo.stage == ETFS_Out)
		return ClampProgress(1.0f - transInfo.fProgress);

	return 1.0f;
}

bool CTimelineFilterHelper::BuildOverlapContext(
	const QUuid& clipId,
	std::chrono::milliseconds tlProgress,
	SOverlapContext& ctx)
{
	if (!g_pTimeLineModel)
		return false;

	// clipId 为素材 ObjectId，需反查所在图层（不可直接 GetLayer(clipId)）
	for (const auto& layerId : g_pTimeLineModel->GetAllLayerIDs())
	{
		auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
		if (pLayer && pLayer->GetObject(clipId).lock())
		{
			ctx.pLayer = pLayer;
			break;
		}
	}
	if (!ctx.pLayer)
		return false;

	const auto& overlapping = TimelineModel::GetLayerPlayingClipIds(ctx.pLayer, tlProgress);
	if (overlapping.size() != 2)
		return false;

	const bool bClipInOverlap = (overlapping[0] == clipId || overlapping[1] == clipId);
	if (!bClipInOverlap)
		return false;

	// CollectOverlappingClips 已按 start 升序：左素材(先出)在前，右素材(后入)在后
	ctx.leftId = overlapping[0];
	ctx.rightId = overlapping[1];

	auto pLeftObj = ctx.pLayer->GetObject(ctx.leftId).lock();
	auto pRightObj = ctx.pLayer->GetObject(ctx.rightId).lock();
	if (!pLeftObj || !pRightObj)
		return false;

	const auto leftStart = pLeftObj->GetStartTime();
	ctx.leftEnd = leftStart + pLeftObj->GetDuration();
	const auto rightStart = pRightObj->GetStartTime();
	const auto rightEnd = rightStart + pRightObj->GetDuration();

	ctx.overlapStart = std::max(leftStart, rightStart);
	ctx.overlapEnd = std::min(ctx.leftEnd, rightEnd);
	if (ctx.overlapEnd <= ctx.overlapStart)
		return false;

	ctx.transDuration = ctx.overlapEnd - ctx.overlapStart;
	ctx.nTransType = ETFT_Fade;

	ctx.pTransFx = g_pTimeLineModel->FindTransFxByObjects(ctx.leftId, ctx.rightId).lock();
	if (ctx.pTransFx)
	{
		ctx.nTransType = ctx.pTransFx->GetTransType();
		const auto fxDuration = ctx.pTransFx->GetDuration();
		if (fxDuration.count() > 0)
			ctx.transDuration = fxDuration;
	}

	if (ctx.transDuration.count() <= 0)
		return false;

	return true;
}

HRESULT CTimelineFilterHelper::CalcPlayStageAndProcess(
	const QUuid& clipId,
	std::chrono::milliseconds tlProgress,
	STransInfo& transInfo,
	bool bOnlyStage)
{
	transInfo.Reset();

	SOverlapContext ctx;
	if (!BuildOverlapContext(clipId, tlProgress, ctx))
		return S_FALSE;

	const auto fadeEnd = ctx.overlapStart + ctx.transDuration;
	const bool bInTransWindow = (tlProgress >= ctx.overlapStart && tlProgress < fadeEnd);

	if (clipId == ctx.rightId)
	{
		if (!bInTransWindow)
			return S_FALSE;
		transInfo.stage = ETFS_In;
	}
	else
	{
		// 左素材 Out
		if (bInTransWindow)
		{
			transInfo.stage = ETFS_Out;
		}
		else if (tlProgress >= fadeEnd && tlProgress < ctx.leftEnd)
		{
			// 转场结束后左素材仍可能处于重叠区
			transInfo.stage = ETFS_Out;
		}
		else
		{
			return S_FALSE;
		}
	}

	if (bOnlyStage)
		return S_OK;

	return FillTransInfo(clipId, tlProgress, ctx, transInfo);
}

HRESULT CTimelineFilterHelper::FillTransInfo(
	const QUuid& clipId,
	std::chrono::milliseconds tlProgress,
	const SOverlapContext& ctx,
	STransInfo& transInfo)
{
	if (ctx.nTransType == ETFT_Fade || ctx.nTransType == ETFT_None || ctx.nTransType < 0)
		return FillTransInfo_Fade(clipId, tlProgress, ctx, transInfo);

	if (IsVertexTransType(ctx.nTransType))
		return FillTransInfo_Vertex(clipId, tlProgress, ctx, transInfo);

	return FillTransInfo_Texture(clipId, tlProgress, ctx, transInfo);
}

HRESULT CTimelineFilterHelper::FillTransInfo_Fade(
	const QUuid& clipId,
	std::chrono::milliseconds tlProgress,
	const SOverlapContext& ctx,
	STransInfo& transInfo)
{
	const auto fadeEnd = ctx.overlapStart + ctx.transDuration;
	const bool bInFadeWindow = (tlProgress >= ctx.overlapStart && tlProgress < fadeEnd);

	transInfo.bTransition = true;
	transInfo.nTransType = ETFT_Fade;

	if (clipId == ctx.rightId)
	{
		if (!bInFadeWindow)
			return S_FALSE;

		const float t = ClampProgress(
			static_cast<float>(tlProgress.count() - ctx.overlapStart.count())
			/ static_cast<float>(ctx.transDuration.count()));

		transInfo.stage = ETFS_In;
		transInfo.fProgress = t;
		return S_OK;
	}

	// 左素材 OutClip
	if (bInFadeWindow)
	{
		const float t = ClampProgress(
			static_cast<float>(tlProgress.count() - ctx.overlapStart.count())
			/ static_cast<float>(ctx.transDuration.count()));

		transInfo.stage = ETFS_Out;
		transInfo.fProgress = t;
		return S_OK;
	}

	// 转场结束后左素材仍可能处于重叠区，保持完全透明（fProgress=1 → alpha=0）
	if (tlProgress >= fadeEnd && tlProgress < ctx.leftEnd)
	{
		transInfo.stage = ETFS_Out;
		transInfo.fProgress = 1.0f;
		return S_OK;
	}

	return S_FALSE;
}

HRESULT CTimelineFilterHelper::FillTransInfo_Vertex(
	const QUuid& clipId,
	std::chrono::milliseconds tlProgress,
	const SOverlapContext& ctx,
	STransInfo& transInfo)
{
	const auto transEnd = ctx.overlapStart + ctx.transDuration;
	const bool bInTransWindow = (tlProgress >= ctx.overlapStart && tlProgress < transEnd);

	transInfo.bTransition = true;
	transInfo.nTransType = ctx.nTransType;

	if (clipId == ctx.rightId)
	{
		if (!bInTransWindow)
			return S_FALSE;

		transInfo.stage = ETFS_In;
		transInfo.fProgress = ClampProgress(
			static_cast<float>(tlProgress.count() - ctx.overlapStart.count())
			/ static_cast<float>(ctx.transDuration.count()));
		return S_OK;
	}

	if (bInTransWindow)
	{
		transInfo.stage = ETFS_Out;
		transInfo.fProgress = ClampProgress(
			static_cast<float>(tlProgress.count() - ctx.overlapStart.count())
			/ static_cast<float>(ctx.transDuration.count()));
		return S_OK;
	}

	if (tlProgress >= transEnd && tlProgress < ctx.leftEnd)
	{
		transInfo.stage = ETFS_Out;
		transInfo.fProgress = 1.0f;
		return S_OK;
	}

	return S_FALSE;
}

HRESULT CTimelineFilterHelper::FillTransInfo_Texture(
	const QUuid& clipId,
	std::chrono::milliseconds tlProgress,
	const SOverlapContext& ctx,
	STransInfo& transInfo)
{
	return FillTransInfo_Vertex(clipId, tlProgress, ctx, transInfo);
}
