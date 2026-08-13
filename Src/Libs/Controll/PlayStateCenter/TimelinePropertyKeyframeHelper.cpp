#include "TimelinePropertyKeyframeHelper.h"

#include "Common/Common/MediaPropertyKeyframeIds.h"
#include "Common/Common/XEffectCommon.h"
#include "Common/Common/IMediaPropertyInterface.h"
#include "Model/MediaModel/FxFactor.h"
#include "Model/MediaModel/FxProperty.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "Model/MediaModel/MediaNode.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/TimeLineModel/TimeLineObject.h"

#include <variant>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

namespace
{
	int ToPropId(EMediaPropertyId propId)
	{
		return static_cast<int>(propId);
	}

	float EvaluatePropertyFloat(const CTimeLineObject* pObject, EMediaPropertyId propId,
		const std::chrono::milliseconds& clipLocalMs, float fallback)
	{
		if (!pObject)
			return fallback;
		auto pTrack = pObject->GetPropertyKeyframeTrack(ToPropId(propId)).lock();
		if (!pTrack || pTrack->IsEmpty())
			return fallback;
		auto v = pTrack->GetValue(clipLocalMs);
		if (std::holds_alternative<float>(v))
			return std::get<float>(v);
		return fallback;
	}

	bool HasAnyTrack(const CTimeLineObject* pObject, const EMediaPropertyId* props, size_t count)
	{
		if (!pObject)
			return false;
		for (size_t i = 0; i < count; ++i)
		{
			if (!pObject->IsPropertyKeyframeTrackEmpty(ToPropId(props[i])))
				return true;
		}
		return false;
	}

	void ResetEffectToFactory(const std::shared_ptr<IEffectProperty>& pEffect)
	{
		if (!pEffect)
			return;
		pEffect->SetRed(1.f);
		pEffect->SetGreen(1.f);
		pEffect->SetBlue(1.f);
		pEffect->SetAlpha(1.f);
		pEffect->SetLuminance(0.f);
		pEffect->SetContrast(0.f);
		pEffect->SetHue(0.f);
		pEffect->SetSaturability(0.f);
	}

	void ApplyEffectKeyframes(CTimeLineObject* pObject, const std::shared_ptr<CMediaNode>& pMedia,
		const std::chrono::milliseconds& clipLocalMs)
	{
		static const EMediaPropertyId kEffectProps[] = {
			EMediaPropertyId::EffectRed, EMediaPropertyId::EffectGreen, EMediaPropertyId::EffectBlue,
			EMediaPropertyId::EffectAlpha, EMediaPropertyId::EffectLuminance, EMediaPropertyId::EffectContrast,
			EMediaPropertyId::EffectHue, EMediaPropertyId::EffectSaturation
		};
		auto pEffect = pMedia->GetEffectProperty();
		if (!pEffect)
			return;
		if (!HasAnyTrack(pObject, kEffectProps, sizeof(kEffectProps) / sizeof(kEffectProps[0])))
		{
			ResetEffectToFactory(pEffect);
			return;
		}

		auto evalOrFactory = [&](EMediaPropertyId id, float factory) -> float {
			if (pObject->IsPropertyKeyframeTrackEmpty(ToPropId(id)))
				return factory;
			return EvaluatePropertyFloat(pObject, id, clipLocalMs, factory);
		};

		pEffect->SetRed(evalOrFactory(EMediaPropertyId::EffectRed, 1.f));
		pEffect->SetGreen(evalOrFactory(EMediaPropertyId::EffectGreen, 1.f));
		pEffect->SetBlue(evalOrFactory(EMediaPropertyId::EffectBlue, 1.f));
		pEffect->SetAlpha(evalOrFactory(EMediaPropertyId::EffectAlpha, 1.f));
		pEffect->SetLuminance(evalOrFactory(EMediaPropertyId::EffectLuminance, 0.f));
		pEffect->SetContrast(evalOrFactory(EMediaPropertyId::EffectContrast, 0.f));
		pEffect->SetHue(evalOrFactory(EMediaPropertyId::EffectHue, 0.f));
		pEffect->SetSaturability(evalOrFactory(EMediaPropertyId::EffectSaturation, 0.f));
	}

	void ApplyKeyingKeyframes(CTimeLineObject* pObject, const std::shared_ptr<CMediaNode>& pMedia,
		const std::chrono::milliseconds& clipLocalMs)
	{
		static const EMediaPropertyId kKeyingProps[] = {
			EMediaPropertyId::KeyingRed, EMediaPropertyId::KeyingGreen, EMediaPropertyId::KeyingBlue,
			EMediaPropertyId::KeyingTolerance, EMediaPropertyId::KeyingDegree
		};
		if (!HasAnyTrack(pObject, kKeyingProps, sizeof(kKeyingProps) / sizeof(kKeyingProps[0])))
			return;

		auto pFxPty = std::dynamic_pointer_cast<CFxProperty>(pMedia->GetFxProperty());
		if (!pFxPty)
			return;

		if (!pFxPty->GetFxFactor(EEffectType::Keying))
			pFxPty->AddFxFaxtor(std::make_shared<CFxFactor>(EEffectType::Keying));

		auto pFactor = pFxPty->GetFxFactor(EEffectType::Keying);
		if (!pFactor)
			return;
		auto pData = std::dynamic_pointer_cast<SKeyingFactor>(pFactor->GetData());
		if (!pData)
			return;

		const SKeyingFactor kDefaultKeying;
		pData->r = EvaluatePropertyFloat(pObject, EMediaPropertyId::KeyingRed, clipLocalMs, kDefaultKeying.r);
		pData->g = EvaluatePropertyFloat(pObject, EMediaPropertyId::KeyingGreen, clipLocalMs, kDefaultKeying.g);
		pData->b = EvaluatePropertyFloat(pObject, EMediaPropertyId::KeyingBlue, clipLocalMs, kDefaultKeying.b);
		pData->threshold = EvaluatePropertyFloat(pObject, EMediaPropertyId::KeyingTolerance, clipLocalMs, kDefaultKeying.threshold);
		pData->weight = EvaluatePropertyFloat(pObject, EMediaPropertyId::KeyingDegree, clipLocalMs, kDefaultKeying.weight);
		pData->visiable = true;
	}

	void ApplyFeatherKeyframes(CTimeLineObject* pObject, const std::shared_ptr<CMediaNode>& pMedia,
		const std::chrono::milliseconds& clipLocalMs)
	{
		static const EMediaPropertyId kFeatherProps[] = { EMediaPropertyId::FeatherRadius };
		if (!HasAnyTrack(pObject, kFeatherProps, sizeof(kFeatherProps) / sizeof(kFeatherProps[0])))
			return;

		auto pFxPty = std::dynamic_pointer_cast<CFxProperty>(pMedia->GetFxProperty());
		if (!pFxPty)
			return;

		if (!pFxPty->GetFxFactor(EEffectType::Feather))
			pFxPty->AddFxFaxtor(std::make_shared<CFxFactor>(EEffectType::Feather));

		auto pFactor = pFxPty->GetFxFactor(EEffectType::Feather);
		if (!pFactor)
			return;
		auto pData = std::dynamic_pointer_cast<SFeatherFactor>(pFactor->GetData());
		if (!pData)
			return;

		const SFeatherFactor kDefaultFeather;
		pData->size = EvaluatePropertyFloat(pObject, EMediaPropertyId::FeatherRadius, clipLocalMs, kDefaultFeather.size);
		pData->visiable = true;
	}
}

bool CTimelinePropertyKeyframeHelper::ApplyPropertyKeyframesToMedia(const QUuid& mediaId,
	const std::chrono::milliseconds& timelineMs)
{
	if (mediaId.isNull() || !g_pTimeLineModel || !g_pMediaModel)
		return false;

	auto pObj = g_pTimeLineModel->GetTimeLineObject(mediaId).lock();
	auto pTlObj = dynamic_cast<CTimeLineObject*>(pObj.get());
	if (!pTlObj)
		return false;

	if (timelineMs < pTlObj->GetStartTime()
		|| timelineMs >= pTlObj->GetStartTime() + pTlObj->GetDuration())
		return false;

	auto pMedia = std::dynamic_pointer_cast<CMediaNode>(g_pMediaModel->GetMediaNode(mediaId).lock());
	if (!pMedia)
		return false;

	const auto clipLocalMs = timelineMs - pTlObj->GetStartTime();

	ApplyEffectKeyframes(pTlObj, pMedia, clipLocalMs);
	ApplyKeyingKeyframes(pTlObj, pMedia, clipLocalMs);
	ApplyFeatherKeyframes(pTlObj, pMedia, clipLocalMs);
	ClearFxWhenNoKeyframeTracks(mediaId);
	return true;
}

void CTimelinePropertyKeyframeHelper::ClearFxWhenNoKeyframeTracks(const QUuid& mediaId)
{
	if (mediaId.isNull() || !g_pTimeLineModel || !g_pMediaModel)
		return;

	auto pObj = g_pTimeLineModel->GetTimeLineObject(mediaId).lock();
	auto pTlObj = dynamic_cast<CTimeLineObject*>(pObj.get());
	auto pMedia = std::dynamic_pointer_cast<CMediaNode>(g_pMediaModel->GetMediaNode(mediaId).lock());
	if (!pTlObj || !pMedia)
		return;

	static const EMediaPropertyId kKeyingProps[] = {
		EMediaPropertyId::KeyingRed, EMediaPropertyId::KeyingGreen, EMediaPropertyId::KeyingBlue,
		EMediaPropertyId::KeyingTolerance, EMediaPropertyId::KeyingDegree
	};
	static const EMediaPropertyId kFeatherProps[] = { EMediaPropertyId::FeatherRadius };
	static const EMediaPropertyId kCropProps[] = {
		EMediaPropertyId::CropFill, EMediaPropertyId::CropLeft, EMediaPropertyId::CropRight,
		EMediaPropertyId::CropTop, EMediaPropertyId::CropBottom
	};

	auto pFxPty = std::dynamic_pointer_cast<CFxProperty>(pMedia->GetFxProperty());
	if (!pFxPty)
		return;

	if (!HasAnyTrack(pTlObj, kKeyingProps, sizeof(kKeyingProps) / sizeof(kKeyingProps[0])))
	{
		if (pFxPty->GetFxFactor(EEffectType::Keying))
			pFxPty->DeleteFxFactor(EEffectType::Keying);
	}
	if (!HasAnyTrack(pTlObj, kFeatherProps, sizeof(kFeatherProps) / sizeof(kFeatherProps[0])))
	{
		if (pFxPty->GetFxFactor(EEffectType::Feather))
			pFxPty->DeleteFxFactor(EEffectType::Feather);
	}
	if (!HasAnyTrack(pTlObj, kCropProps, sizeof(kCropProps) / sizeof(kCropProps[0])))
	{
		if (pFxPty->GetFxFactor(EEffectType::Cut))
			pFxPty->DeleteFxFactor(EEffectType::Cut);
	}
}

void CTimelinePropertyKeyframeHelper::ResetEffectWhenNoKeyframeTracks(const QUuid& mediaId)
{
	if (mediaId.isNull() || !g_pTimeLineModel || !g_pMediaModel)
		return;

	auto pObj = g_pTimeLineModel->GetTimeLineObject(mediaId).lock();
	auto pTlObj = dynamic_cast<CTimeLineObject*>(pObj.get());
	auto pMedia = std::dynamic_pointer_cast<CMediaNode>(g_pMediaModel->GetMediaNode(mediaId).lock());
	if (!pTlObj || !pMedia)
		return;

	static const EMediaPropertyId kEffectProps[] = {
		EMediaPropertyId::EffectRed, EMediaPropertyId::EffectGreen, EMediaPropertyId::EffectBlue,
		EMediaPropertyId::EffectAlpha, EMediaPropertyId::EffectLuminance, EMediaPropertyId::EffectContrast,
		EMediaPropertyId::EffectHue, EMediaPropertyId::EffectSaturation
	};
	if (!HasAnyTrack(pTlObj, kEffectProps, sizeof(kEffectProps) / sizeof(kEffectProps[0])))
		ResetEffectToFactory(pMedia->GetEffectProperty());
}

void CTimelinePropertyKeyframeHelper::ApplyEffectChannelAtTime(const QUuid& mediaId, int nPropId,
	const std::chrono::milliseconds& timelineMs)
{
	if (mediaId.isNull() || !g_pTimeLineModel || !g_pMediaModel)
		return;

	auto pObj = g_pTimeLineModel->GetTimeLineObject(mediaId).lock();
	auto pTlObj = dynamic_cast<CTimeLineObject*>(pObj.get());
	auto pMedia = std::dynamic_pointer_cast<CMediaNode>(g_pMediaModel->GetMediaNode(mediaId).lock());
	if (!pTlObj || !pMedia)
		return;
	if (timelineMs < pTlObj->GetStartTime()
		|| timelineMs >= pTlObj->GetStartTime() + pTlObj->GetDuration())
		return;

	auto pEffect = pMedia->GetEffectProperty();
	if (!pEffect)
		return;

	const auto clipLocalMs = timelineMs - pTlObj->GetStartTime();
	const auto propId = static_cast<EMediaPropertyId>(nPropId);

	auto writeChannel = [&](float factory, auto setter) {
		if (pTlObj->IsPropertyKeyframeTrackEmpty(nPropId))
			setter(factory);
		else
			setter(EvaluatePropertyFloat(pTlObj, propId, clipLocalMs, factory));
	};

	switch (propId)
	{
	case EMediaPropertyId::EffectRed:
		writeChannel(1.f, [&](float v) { pEffect->SetRed(v); });
		break;
	case EMediaPropertyId::EffectGreen:
		writeChannel(1.f, [&](float v) { pEffect->SetGreen(v); });
		break;
	case EMediaPropertyId::EffectBlue:
		writeChannel(1.f, [&](float v) { pEffect->SetBlue(v); });
		break;
	case EMediaPropertyId::EffectAlpha:
		writeChannel(1.f, [&](float v) { pEffect->SetAlpha(v); });
		break;
	case EMediaPropertyId::EffectLuminance:
		writeChannel(0.f, [&](float v) { pEffect->SetLuminance(v); });
		break;
	case EMediaPropertyId::EffectContrast:
		writeChannel(0.f, [&](float v) { pEffect->SetContrast(v); });
		break;
	case EMediaPropertyId::EffectHue:
		writeChannel(0.f, [&](float v) { pEffect->SetHue(v); });
		break;
	case EMediaPropertyId::EffectSaturation:
		writeChannel(0.f, [&](float v) { pEffect->SetSaturability(v); });
		break;
	default:
		break;
	}
}