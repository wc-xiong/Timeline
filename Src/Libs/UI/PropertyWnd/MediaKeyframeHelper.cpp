#include "MediaKeyframeHelper.h"
#include "MediaPropertyFrame.h"

#include "Model/MediaModel/MediaModelExport.h"
#include "Model/MediaModel/MediaNode.h"
#include "Model/MediaModel/FxProperty.h"
#include "Model/MediaModel/FxFactor.h"
#include "Model/MediaModel/EffectProperty.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/TimeLineModel//ConvertHelper.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Common/SignalCenter/GSNetSignalCenter.h"
#include "Controll/PlayStateCenter/TimelinePropertyKeyframeHelper.h"
#include "Common/UICommon/PropertyKeyFrameBar.h"
#include "Common/UICommon/UITranslate.h"
#include "Common/Common/XEffectCommon.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Model/ResourceModel/ResourceModelExport.h"

#include <QTimer>
#include <algorithm>
#include <variant>

namespace
{
	int ToPropId(EMediaPropertyId propId)
	{
		return static_cast<int>(propId);
	}

	// ClipFx Sync 传入 clip 本地时间后，短暂覆盖游标，避免 PlayerController Seek 滞后导致 Toggle 错态；
	// 属性面板 Toggle 仍走 PlayerController，不依赖可能过期的 ContentViewData。
	std::optional<std::chrono::milliseconds> g_playheadOverrideMs;

	std::chrono::milliseconds ResolvePlayheadTime(std::optional<std::chrono::milliseconds> playheadMs)
	{
		if (playheadMs)
			return *playheadMs;
		if (g_playheadOverrideMs)
			return *g_playheadOverrideMs;
		if (g_pPlayerController)
			return g_pPlayerController->GetCurTime();
		return std::chrono::milliseconds(0);
	}

	void ScheduleClearPlayheadOverride(CMediaPropertyFrame* pFrame)
	{
		if (!pFrame)
		{
			g_playheadOverrideMs.reset();
			return;
		}
		QTimer::singleShot(0, pFrame, []() { g_playheadOverrideMs.reset(); });
	}

	CMediaPropertyFrame* g_pRegisteredMediaPropertyFrame = nullptr;
}

CTimeLineObject* CMediaKeyframeHelper::GetTimelineObject(const QUuid& mediaId)
{
	if (!g_pTimeLineModel || mediaId.isNull())
		return nullptr;
	auto pObj = g_pTimeLineModel->GetTimeLineObject(mediaId).lock();
	return dynamic_cast<CTimeLineObject*>(pObj.get());
}

std::shared_ptr<CMediaNode> CMediaKeyframeHelper::GetMediaNode(const QUuid& mediaId)
{
	if (!g_pMediaModel || mediaId.isNull())
		return nullptr;
	return std::dynamic_pointer_cast<CMediaNode>(g_pMediaModel->GetMediaNode(mediaId).lock());
}

namespace
{
	const EMediaPropertyGroupId kAllGroups[] = {
		EMediaPropertyGroupId::Volume,
		EMediaPropertyGroupId::Position,
		EMediaPropertyGroupId::Scale,
		EMediaPropertyGroupId::Rotation,
		EMediaPropertyGroupId::Crop,
		EMediaPropertyGroupId::Feather,
		EMediaPropertyGroupId::Keying,
		EMediaPropertyGroupId::Effect,
	};

	/**
	 * @brief 读取指定媒体属性的「静态默认值」
	 *
	 * 优先级：CMediaNode 当前值 -> CFxProperty/CEffectProperty 因子 -> XEffectCommon 结构体默认。
	 * 用于切换素材时初始化 UI，以及无关键帧轨道时的插值 fallback。
	 */
	float GetPropertyDefaultValue(const QUuid& mediaId, EMediaPropertyId propId)
	{
		const SCropFactor kDefaultCrop;
		const SFeatherFactor kDefaultFeather;
		const SKeyingFactor kDefaultKeying;

		auto pMedia = CMediaKeyframeHelper::GetMediaNode(mediaId);
		if (!pMedia)
		{
			switch (propId)
			{
			case EMediaPropertyId::FeatherRadius: return kDefaultFeather.size;
			case EMediaPropertyId::KeyingRed: return kDefaultKeying.r;
			case EMediaPropertyId::KeyingGreen: return kDefaultKeying.g;
			case EMediaPropertyId::KeyingBlue: return kDefaultKeying.b;
			case EMediaPropertyId::KeyingTolerance: return kDefaultKeying.threshold;
			case EMediaPropertyId::KeyingDegree: return kDefaultKeying.weight;
			case EMediaPropertyId::EffectRed:
			case EMediaPropertyId::EffectGreen:
			case EMediaPropertyId::EffectBlue:
			case EMediaPropertyId::EffectAlpha:
				return 1.f;
			default:
				return 0.f;
			}
		}

		auto pEffect = pMedia->GetEffectProperty();
		auto pFxPty = std::dynamic_pointer_cast<CFxProperty>(pMedia->GetFxProperty());

		switch (propId)
		{
		case EMediaPropertyId::VolumeIntensity:
			return pMedia->GetVolumn();
		case EMediaPropertyId::PosX:
			return (float)pMedia->GetMediaPos().x;
		case EMediaPropertyId::PosY:
			return (float)pMedia->GetMediaPos().y;
		case EMediaPropertyId::PosZ:
			return 0.f;
		case EMediaPropertyId::ScaleWidth:
			return (float)pMedia->GetMediaWidth();
		case EMediaPropertyId::ScaleHeight:
			return (float)pMedia->GetMediaHeight();
		case EMediaPropertyId::RotX:
		case EMediaPropertyId::RotY:
			return 0.f;
		case EMediaPropertyId::RotZ:
			return pMedia->GetMediaRotateAngle();
		case EMediaPropertyId::CropFill:
			return 0.f;
		case EMediaPropertyId::CropLeft:
			if (pFxPty)
				if (auto pFactor = pFxPty->GetFxFactor(EEffectType::Cut))
					if (auto pData = std::dynamic_pointer_cast<SCropFactor>(pFactor->GetData()))
						return pData->l;
			return kDefaultCrop.l;
		case EMediaPropertyId::CropRight:
			if (pFxPty)
				if (auto pFactor = pFxPty->GetFxFactor(EEffectType::Cut))
					if (auto pData = std::dynamic_pointer_cast<SCropFactor>(pFactor->GetData()))
						return pData->r;
			return kDefaultCrop.r;
		case EMediaPropertyId::CropTop:
			if (pFxPty)
				if (auto pFactor = pFxPty->GetFxFactor(EEffectType::Cut))
					if (auto pData = std::dynamic_pointer_cast<SCropFactor>(pFactor->GetData()))
						return pData->t;
			return kDefaultCrop.t;
		case EMediaPropertyId::CropBottom:
			if (pFxPty)
				if (auto pFactor = pFxPty->GetFxFactor(EEffectType::Cut))
					if (auto pData = std::dynamic_pointer_cast<SCropFactor>(pFactor->GetData()))
						return pData->b;
			return kDefaultCrop.b;
		case EMediaPropertyId::FeatherRadius:
			if (pFxPty)
				if (auto pFactor = pFxPty->GetFxFactor(EEffectType::Feather))
					if (auto pData = std::dynamic_pointer_cast<SFeatherFactor>(pFactor->GetData()))
						return pData->size;
			return kDefaultFeather.size;
		case EMediaPropertyId::KeyingRed:
			if (pFxPty)
				if (auto pFactor = pFxPty->GetFxFactor(EEffectType::Keying))
					if (auto pData = std::dynamic_pointer_cast<SKeyingFactor>(pFactor->GetData()))
						return pData->r;
			return kDefaultKeying.r;
		case EMediaPropertyId::KeyingGreen:
			if (pFxPty)
				if (auto pFactor = pFxPty->GetFxFactor(EEffectType::Keying))
					if (auto pData = std::dynamic_pointer_cast<SKeyingFactor>(pFactor->GetData()))
						return pData->g;
			return kDefaultKeying.g;
		case EMediaPropertyId::KeyingBlue:
			if (pFxPty)
				if (auto pFactor = pFxPty->GetFxFactor(EEffectType::Keying))
					if (auto pData = std::dynamic_pointer_cast<SKeyingFactor>(pFactor->GetData()))
						return pData->b;
			return kDefaultKeying.b;
		case EMediaPropertyId::KeyingTolerance:
			if (pFxPty)
				if (auto pFactor = pFxPty->GetFxFactor(EEffectType::Keying))
					if (auto pData = std::dynamic_pointer_cast<SKeyingFactor>(pFactor->GetData()))
						return pData->threshold;
			return kDefaultKeying.threshold;
		case EMediaPropertyId::KeyingDegree:
			if (pFxPty)
				if (auto pFactor = pFxPty->GetFxFactor(EEffectType::Keying))
					if (auto pData = std::dynamic_pointer_cast<SKeyingFactor>(pFactor->GetData()))
						return pData->weight;
			return kDefaultKeying.weight;
		case EMediaPropertyId::EffectRed:
			return pEffect ? pEffect->GetRed() : 1.f;
		case EMediaPropertyId::EffectGreen:
			return pEffect ? pEffect->GetGreen() : 1.f;
		case EMediaPropertyId::EffectBlue:
			return pEffect ? pEffect->GetBlue() : 1.f;
		case EMediaPropertyId::EffectAlpha:
			return pEffect ? pEffect->GetAlpha() : 1.f;
		case EMediaPropertyId::EffectLuminance:
			return pEffect ? pEffect->GetLuminance() : 0.f;
		case EMediaPropertyId::EffectContrast:
			return pEffect ? pEffect->GetContrast() : 0.f;
		case EMediaPropertyId::EffectHue:
			return pEffect ? pEffect->GetHue() : 0.f;
		case EMediaPropertyId::EffectSaturation:
			return pEffect ? pEffect->GetSaturability() : 0.f;
		default:
			return 0.f;
		}
	}

	/**
	 * @brief 属性出厂默认值（与节点当前值无关）
	 * @details ResetPropertyGroup 使用本函数恢复 UI / 模型，而非 GetPropertyDefaultValue（后者优先读节点当前值）
	 */
	float GetPropertyFactoryDefaultValue(const QUuid& mediaId, EMediaPropertyId propId)
	{
		const SCropFactor kDefaultCrop;
		const SFeatherFactor kDefaultFeather;
		const SKeyingFactor kDefaultKeying;
		auto pMedia = CMediaKeyframeHelper::GetMediaNode(mediaId);

		switch (propId)
		{
		case EMediaPropertyId::VolumeIntensity:
			return g_pProjectSetting ? (float)g_pProjectSetting->LoadDefaultVolume() : 1.f;
		case EMediaPropertyId::PosX:
		case EMediaPropertyId::PosY:
		case EMediaPropertyId::PosZ:
			return 0.f;
		case EMediaPropertyId::ScaleWidth:
			if (pMedia)
			{
				if (g_pResourceModel)
				{
					if (auto pRes = g_pResourceModel->GetResourceNode(pMedia->GetResourceId()))
					{
						const QSize res = pRes->GetResolution();
						if (res.width() > 0)
							return (float)res.width();
					}
				}
				return pMedia->GetMediaWidth();
			}
			return 1.f;
		case EMediaPropertyId::ScaleHeight:
			if (pMedia)
			{
				if (g_pResourceModel)
				{
					if (auto pRes = g_pResourceModel->GetResourceNode(pMedia->GetResourceId()))
					{
						const QSize res = pRes->GetResolution();
						if (res.height() > 0)
							return (float)res.height();
					}
				}
				return pMedia->GetMediaHeight();
			}
			return 1.f;
		case EMediaPropertyId::RotX:
		case EMediaPropertyId::RotY:
		case EMediaPropertyId::RotZ:
			return 0.f;
		case EMediaPropertyId::CropFill:
			return 0.f;
		case EMediaPropertyId::CropLeft:
			return kDefaultCrop.l;
		case EMediaPropertyId::CropRight:
			return kDefaultCrop.r;
		case EMediaPropertyId::CropTop:
			return kDefaultCrop.t;
		case EMediaPropertyId::CropBottom:
			return kDefaultCrop.b;
		case EMediaPropertyId::FeatherRadius:
			return kDefaultFeather.size;
		case EMediaPropertyId::KeyingRed:
			return kDefaultKeying.r;
		case EMediaPropertyId::KeyingGreen:
			return kDefaultKeying.g;
		case EMediaPropertyId::KeyingBlue:
			return kDefaultKeying.b;
		case EMediaPropertyId::KeyingTolerance:
			return kDefaultKeying.threshold;
		case EMediaPropertyId::KeyingDegree:
			return kDefaultKeying.weight;
		case EMediaPropertyId::EffectRed:
		case EMediaPropertyId::EffectGreen:
		case EMediaPropertyId::EffectBlue:
		case EMediaPropertyId::EffectAlpha:
			return 1.f;
		case EMediaPropertyId::EffectLuminance:
		case EMediaPropertyId::EffectContrast:
		case EMediaPropertyId::EffectHue:
		case EMediaPropertyId::EffectSaturation:
			return 0.f;
		default:
			return 0.f;
		}
	}

	void ClearAllKeyframesForProperty(CTimeLineObject* pObject, int nPropId)
	{
		if (!pObject)
			return;
		// 先拷贝时间列表 — DeletePropertyKeyframe 可能擦除轨道导致迭代器失效
		auto times = pObject->GetPropertyKeyframeTimes(nPropId);
		for (const auto& t : times)
			pObject->DeletePropertyKeyframe(nPropId, t);
	}

	bool IsEffectPropertyId(EMediaPropertyId propId)
	{
		switch (propId)
		{
		case EMediaPropertyId::EffectRed:
		case EMediaPropertyId::EffectGreen:
		case EMediaPropertyId::EffectBlue:
		case EMediaPropertyId::EffectAlpha:
		case EMediaPropertyId::EffectLuminance:
		case EMediaPropertyId::EffectContrast:
		case EMediaPropertyId::EffectHue:
		case EMediaPropertyId::EffectSaturation:
			return true;
		default:
			return false;
		}
	}

	void WritePropertyBaseValue(const QUuid& mediaId, EMediaPropertyId propId, float value,
		CMediaPropertyFrame* pFrame)
	{
		auto pMedia = CMediaKeyframeHelper::GetMediaNode(mediaId);
		if (!pMedia)
			return;
		auto pEffect = pMedia->GetEffectProperty();
		switch (propId)
		{
		case EMediaPropertyId::VolumeIntensity:
			pMedia->SetVolumn(value);
			break;
		case EMediaPropertyId::PosX:
			pMedia->SetMediaPos(value, pMedia->GetMediaPos().y);
			break;
		case EMediaPropertyId::PosY:
			pMedia->SetMediaPos(pMedia->GetMediaPos().x, value);
			break;
		case EMediaPropertyId::PosZ:
			break; // UI-only cache updated via SetPropertyDisplayValue
		case EMediaPropertyId::ScaleWidth:
			pMedia->SetMediaWidth(value);
			break;
		case EMediaPropertyId::ScaleHeight:
			pMedia->SetMediaHeight(value);
			break;
		case EMediaPropertyId::RotX:
		case EMediaPropertyId::RotY:
			break;
		case EMediaPropertyId::RotZ:
			pMedia->SetMediaRotateAngle(value);
			break;
		case EMediaPropertyId::EffectRed:
			if (pEffect) pEffect->SetRed(value);
			break;
		case EMediaPropertyId::EffectGreen:
			if (pEffect) pEffect->SetGreen(value);
			break;
		case EMediaPropertyId::EffectBlue:
			if (pEffect) pEffect->SetBlue(value);
			break;
		case EMediaPropertyId::EffectAlpha:
			if (pEffect) pEffect->SetAlpha(value);
			break;
		case EMediaPropertyId::EffectLuminance:
			if (pEffect) pEffect->SetLuminance(value);
			break;
		case EMediaPropertyId::EffectContrast:
			if (pEffect) pEffect->SetContrast(value);
			break;
		case EMediaPropertyId::EffectHue:
			if (pEffect) pEffect->SetHue(value);
			break;
		case EMediaPropertyId::EffectSaturation:
			if (pEffect) pEffect->SetSaturability(value);
			break;
		default:
			break;
		}
		CMediaKeyframeHelper::EnsureFxFactorForProperty(mediaId, propId, value, pFrame);
	}
}

bool CMediaKeyframeHelper::IsPlayheadOnObject(const QUuid& mediaId,
	std::optional<std::chrono::milliseconds> playheadMs)
{
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
		return false;

	const auto t = ResolvePlayheadTime(playheadMs);
	return t >= pObject->GetStartTime() && t < pObject->GetStartTime() + pObject->GetDuration();
}

std::chrono::milliseconds CMediaKeyframeHelper::GetAlignedClipLocalMs(const QUuid& mediaId,
	std::optional<std::chrono::milliseconds> playheadMs)
{
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
		return std::chrono::milliseconds(0);

	const auto t = ResolvePlayheadTime(playheadMs);
	const auto localMs = t - pObject->GetStartTime();
	if (g_pConvertHelper)
		return g_pConvertHelper->formatTimeWithFrame(localMs);
	return localMs;
}

std::vector<EMediaPropertyId> CMediaKeyframeHelper::GetGroupPropertyIds(EMediaPropertyGroupId groupId)
{
	switch (groupId)
	{
	case EMediaPropertyGroupId::Volume:
		return { EMediaPropertyId::VolumeIntensity };
	case EMediaPropertyGroupId::Position:
		return { EMediaPropertyId::PosX, EMediaPropertyId::PosY, EMediaPropertyId::PosZ };
	case EMediaPropertyGroupId::Scale:
		return { EMediaPropertyId::ScaleWidth, EMediaPropertyId::ScaleHeight };
	case EMediaPropertyGroupId::Rotation:
		return { EMediaPropertyId::RotX, EMediaPropertyId::RotY, EMediaPropertyId::RotZ };
	case EMediaPropertyGroupId::Crop:
		return { EMediaPropertyId::CropFill, EMediaPropertyId::CropLeft, EMediaPropertyId::CropRight,
			EMediaPropertyId::CropTop, EMediaPropertyId::CropBottom };
	case EMediaPropertyGroupId::Feather:
		return { EMediaPropertyId::FeatherRadius };
	case EMediaPropertyGroupId::Keying:
		return { EMediaPropertyId::KeyingRed, EMediaPropertyId::KeyingGreen, EMediaPropertyId::KeyingBlue,
			EMediaPropertyId::KeyingTolerance, EMediaPropertyId::KeyingDegree };
	case EMediaPropertyGroupId::Effect:
		return { EMediaPropertyId::EffectRed, EMediaPropertyId::EffectGreen, EMediaPropertyId::EffectBlue,
			EMediaPropertyId::EffectAlpha, EMediaPropertyId::EffectLuminance, EMediaPropertyId::EffectContrast,
			EMediaPropertyId::EffectHue, EMediaPropertyId::EffectSaturation };
	default:
		return {};
	}
}

bool CMediaKeyframeHelper::HasGroupKeyframes(const CTimeLineObject* pObject, EMediaPropertyGroupId groupId)
{
	if (!pObject)
		return false;
	for (auto propId : GetGroupPropertyIds(groupId))
	{
		if (!pObject->IsPropertyKeyframeTrackEmpty(ToPropId(propId)))
			return true;
	}
	return false;
}

QString CMediaKeyframeHelper::GetPropertyDisplayName(EMediaPropertyId propId)
{
	switch (propId)
	{
	case EMediaPropertyId::VolumeIntensity: return CPropertyTranslate::Intensity();
	case EMediaPropertyId::PosX: return QStringLiteral("X");
	case EMediaPropertyId::PosY: return QStringLiteral("Y");
	case EMediaPropertyId::PosZ: return QStringLiteral("Z");
	case EMediaPropertyId::ScaleWidth: return CCommonTranslate::W();
	case EMediaPropertyId::ScaleHeight: return CCommonTranslate::H();
	case EMediaPropertyId::RotX: return QStringLiteral("RotX");
	case EMediaPropertyId::RotY: return QStringLiteral("RotY");
	case EMediaPropertyId::RotZ: return QStringLiteral("RotZ");
	case EMediaPropertyId::CropFill: return CFxPropertyTranslate::Complement();
	case EMediaPropertyId::CropLeft: return CFxPropertyTranslate::Left();
	case EMediaPropertyId::CropRight: return CFxPropertyTranslate::Right();
	case EMediaPropertyId::CropTop: return CFxPropertyTranslate::Top();
	case EMediaPropertyId::CropBottom: return CFxPropertyTranslate::Bottom();
	case EMediaPropertyId::FeatherRadius: return CFxPropertyTranslate::FeatherRadius();
	case EMediaPropertyId::KeyingRed: return CCommonTranslate::Red();
	case EMediaPropertyId::KeyingGreen: return CCommonTranslate::Green();
	case EMediaPropertyId::KeyingBlue: return CCommonTranslate::Blue();
	case EMediaPropertyId::KeyingTolerance: return CFxPropertyTranslate::Threshold();
	case EMediaPropertyId::KeyingDegree: return CFxPropertyTranslate::Degree();
	case EMediaPropertyId::EffectRed: return CCommonTranslate::Red();
	case EMediaPropertyId::EffectGreen: return CCommonTranslate::Green();
	case EMediaPropertyId::EffectBlue: return CCommonTranslate::Blue();
	case EMediaPropertyId::EffectAlpha: return CCommonTranslate::Transparent();
	case EMediaPropertyId::EffectLuminance: return CCommonTranslate::Brightness();
	case EMediaPropertyId::EffectContrast: return CCommonTranslate::Contrast();
	case EMediaPropertyId::EffectHue: return CCommonTranslate::Hue();
	case EMediaPropertyId::EffectSaturation: return CCommonTranslate::Saturation();
	default: return QString();
	}
}

QString CMediaKeyframeHelper::GetGroupDisplayName(EMediaPropertyGroupId groupId)
{
	switch (groupId)
	{
	case EMediaPropertyGroupId::Volume: return CPropertyTranslate::Volumn();
	case EMediaPropertyGroupId::Position: return CCommonTranslate::Position();
	case EMediaPropertyGroupId::Scale: return CSliceViewTranslate::Zoom();
	case EMediaPropertyGroupId::Rotation: return CSliceViewTranslate::Rotate();
	case EMediaPropertyGroupId::Crop: return CFxPropertyTranslate::Cut();
	case EMediaPropertyGroupId::Feather: return CFxPropertyTranslate::Feather();
	case EMediaPropertyGroupId::Keying: return CFxPropertyTranslate::Keying();
	case EMediaPropertyGroupId::Effect: return CFxPropertyTranslate::Effect();
	default: return QString();
	}
}

void CMediaKeyframeHelper::GetPropertyValueRange(EMediaPropertyId propId, float& outMin, float& outMax)
{
	outMin = 0.f;
	outMax = 1.f;
	switch (propId)
	{
	case EMediaPropertyId::PosX:
	case EMediaPropertyId::PosY:
	case EMediaPropertyId::PosZ:
		outMin = -10000.f;
		outMax = 10000.f;
		break;
	case EMediaPropertyId::ScaleWidth:
	case EMediaPropertyId::ScaleHeight:
		outMin = 0.f;
		outMax = 100.f;
		break;
	case EMediaPropertyId::RotX:
	case EMediaPropertyId::RotY:
	case EMediaPropertyId::RotZ:
		outMin = -360.f;
		outMax = 360.f;
		break;
	case EMediaPropertyId::EffectLuminance:
	case EMediaPropertyId::EffectContrast:
	case EMediaPropertyId::EffectHue:
	case EMediaPropertyId::EffectSaturation:
		// Match CMediaPropertyFrame effect sliders: internal [-1,1] => UI -100%~100%
		outMin = -1.f;
		outMax = 1.f;
		break;
	default:
		break;
	}
}

float CMediaKeyframeHelper::ReadPropertyValue(CMediaPropertyFrame* pFrame, EMediaPropertyId propId)
{
	if (!pFrame)
		return 0.f;
	return pFrame->GetPropertyKeyframeValue(propId);
}

void CMediaKeyframeHelper::EnsureFxFactorForProperty(const QUuid& mediaId, EMediaPropertyId propId, float value,
	CMediaPropertyFrame* pFrame)
{
	Q_UNUSED(value);
	auto pMedia = GetMediaNode(mediaId);
	if (!pMedia)
		return;
	auto pFxPty = std::dynamic_pointer_cast<CFxProperty>(pMedia->GetFxProperty());
	if (!pFxPty)
		return;

	auto ensureFactor = [&](EEffectType type) {
		if (!pFxPty->GetFxFactor(type))
			pFxPty->AddFxFaxtor(std::make_shared<CFxFactor>(type));
	};

	switch (propId)
	{
	case EMediaPropertyId::CropFill:
	case EMediaPropertyId::CropLeft:
	case EMediaPropertyId::CropRight:
	case EMediaPropertyId::CropTop:
	case EMediaPropertyId::CropBottom:
	{
		ensureFactor(EEffectType::Cut);
		if (auto pFactor = pFxPty->GetFxFactor(EEffectType::Cut))
		{
			if (auto pData = std::dynamic_pointer_cast<SCropFactor>(pFactor->GetData()))
			{
				if (pFrame)
				{
					pData->l = pFrame->GetPropertyKeyframeValue(EMediaPropertyId::CropLeft);
					pData->r = pFrame->GetPropertyKeyframeValue(EMediaPropertyId::CropRight);
					pData->t = pFrame->GetPropertyKeyframeValue(EMediaPropertyId::CropTop);
					pData->b = pFrame->GetPropertyKeyframeValue(EMediaPropertyId::CropBottom);
				}
			}
		}
		break;
	}
	case EMediaPropertyId::FeatherRadius:
	{
		ensureFactor(EEffectType::Feather);
		if (auto pFactor = pFxPty->GetFxFactor(EEffectType::Feather))
		{
			if (auto pData = std::dynamic_pointer_cast<SFeatherFactor>(pFactor->GetData()))
				pData->size = value;
		}
		break;
	}
	case EMediaPropertyId::KeyingRed:
	case EMediaPropertyId::KeyingGreen:
	case EMediaPropertyId::KeyingBlue:
	case EMediaPropertyId::KeyingTolerance:
	case EMediaPropertyId::KeyingDegree:
	{
		ensureFactor(EEffectType::Keying);
		if (auto pFactor = pFxPty->GetFxFactor(EEffectType::Keying))
		{
			if (auto pData = std::dynamic_pointer_cast<SKeyingFactor>(pFactor->GetData()))
			{
				// 五元组一次写齐；无面板时用出厂默认（绿幕），禁止残留 0 键把画面抠空
				const SKeyingFactor kDefaultKeying;
				if (pFrame)
				{
					pData->r = pFrame->GetPropertyKeyframeValue(EMediaPropertyId::KeyingRed);
					pData->g = pFrame->GetPropertyKeyframeValue(EMediaPropertyId::KeyingGreen);
					pData->b = pFrame->GetPropertyKeyframeValue(EMediaPropertyId::KeyingBlue);
					pData->threshold = pFrame->GetPropertyKeyframeValue(EMediaPropertyId::KeyingTolerance);
					pData->weight = pFrame->GetPropertyKeyframeValue(EMediaPropertyId::KeyingDegree);
				}
				else
				{
					pData->r = (propId == EMediaPropertyId::KeyingRed) ? value : kDefaultKeying.r;
					pData->g = (propId == EMediaPropertyId::KeyingGreen) ? value : kDefaultKeying.g;
					pData->b = (propId == EMediaPropertyId::KeyingBlue) ? value : kDefaultKeying.b;
					pData->threshold = (propId == EMediaPropertyId::KeyingTolerance) ? value : kDefaultKeying.threshold;
					pData->weight = (propId == EMediaPropertyId::KeyingDegree) ? value : kDefaultKeying.weight;
				}
				pData->visiable = true;
			}
		}
		break;
	}
	default:
		break;
	}
}

bool CMediaKeyframeHelper::ApplyPropertyValueAtPlayhead(const QUuid& mediaId, EMediaPropertyId propId, float value,
	CMediaPropertyFrame* pFrame, std::optional<std::chrono::milliseconds> playheadMs)
{
	if (!IsPlayheadOnObject(mediaId, playheadMs))
		return false;

	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
		return false;

	const auto clipLocalMs = GetAlignedClipLocalMs(mediaId, playheadMs);
	const int nPropId = ToPropId(propId);
	// 游标在 clip 内：已有关键帧则更新，否则在游标处新增（滑动条可直接打点）
	if (pObject->HasPropertyKeyframeAt(nPropId, clipLocalMs))
		pObject->SetPropertyKeyframeValue(nPropId, clipLocalMs, value);
	else
		pObject->AddPropertyKeyframe(nPropId, clipLocalMs, value);

	EnsureFxFactorForProperty(mediaId, propId, value, pFrame);
	ApplyEvaluatedPropertiesToMediaNode(mediaId, pObject->GetStartTime() + clipLocalMs, pFrame);

	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_MediaNodeDataChange);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_SliceWnd, SignalCenter::EDDT_MediaNodeDataChange);
	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	emit g_pSignalCenter->signalUpdateMediaProperty(mediaId);
	NotifyClipFxPropertyKeyframeChanged(mediaId, propId, clipLocalMs);
	return true;
}

void CMediaKeyframeHelper::NotifyClipFxPropertyKeyframeChanged(const QUuid& mediaId, EMediaPropertyId propId,
	const std::chrono::milliseconds& clipLocalMs)
{
	emit g_pSignalCenter->signalClipFxPropertyKeyframeChanged(mediaId, static_cast<int>(propId), clipLocalMs.count());
	// 同步属性关键帧到播放端（对齐音频声道映射通知）
	if (g_pGSNetSignal)
		emit g_pGSNetSignal->signalModifyMediaPropertyKeyframes(mediaId);
}

// 切换选中素材后，将所有可关键帧属性 UI 重置为模型默认值（不依赖游标位置）
void CMediaKeyframeHelper::LoadFxPropertyDisplayDefaults(CMediaPropertyFrame* pFrame, const QUuid& mediaId)
{
	if (!pFrame)
		return;

	for (auto groupId : kAllGroups)
	{
		for (auto propId : GetGroupPropertyIds(groupId))
			SetPropertyFrameDisplayValue(pFrame, propId, GetPropertyDefaultValue(mediaId, propId));
	}
}

bool CMediaKeyframeHelper::TogglePropertyKeyframe(const QUuid& mediaId, EMediaPropertyId propId, float value,
	CMediaPropertyFrame* pFrame)
{
	// 属性面板增删关键帧必须以 Player 游标为准，清掉 ClipFx Sync 留下的短时覆盖
	g_playheadOverrideMs.reset();
	if (!IsPlayheadOnObject(mediaId))
		return false;
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
		return false;

	const auto clipLocalMs = GetAlignedClipLocalMs(mediaId);
	const int nPropId = ToPropId(propId);
	if (pObject->HasPropertyKeyframeAt(nPropId, clipLocalMs))
	{
		// 取消子属性：只删当前属性在游标处的关键帧
		pObject->DeletePropertyKeyframe(nPropId, clipLocalMs);
		const auto timelineMs = pObject->GetStartTime() + clipLocalMs;
		ApplyEvaluatedPropertiesToMediaNode(mediaId, timelineMs, pFrame);
		if (IsEffectPropertyId(propId))
		{
			// 立刻按「无轨出厂 / 有轨插值」写回该通道，避免画面停在旧关键帧数值
			CTimelinePropertyKeyframeHelper::ApplyEffectChannelAtTime(mediaId, nPropId, timelineMs);
			CTimelinePropertyKeyframeHelper::ResetEffectWhenNoKeyframeTracks(mediaId);
		}
		if (pFrame)
			SyncPropertyFrameFromTimeline(pFrame, mediaId, propId, clipLocalMs);
	}
	else
	{
		pObject->AddPropertyKeyframe(nPropId, clipLocalMs, value);
		EnsureFxFactorForProperty(mediaId, propId, value, pFrame);
		ApplyEvaluatedPropertiesToMediaNode(mediaId, pObject->GetStartTime() + clipLocalMs, pFrame);
		if (pFrame)
			SyncPropertyFrameFromTimeline(pFrame, mediaId, propId, clipLocalMs);
	}

	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_MediaNodeDataChange);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_SliceWnd, SignalCenter::EDDT_MediaNodeDataChange);
	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	NotifyClipFxPropertyKeyframeChanged(mediaId, propId, clipLocalMs);
	// 等比例锁定时同步配对宽/高关键帧
	SyncPairedScaleKeyframeAfterToggle(mediaId, propId, pFrame);
	return true;
}

bool CMediaKeyframeHelper::IsScaleSizePropertyId(EMediaPropertyId propId)
{
	return propId == EMediaPropertyId::ScaleWidth || propId == EMediaPropertyId::ScaleHeight;
}

EMediaPropertyId CMediaKeyframeHelper::GetPairedScalePropertyId(EMediaPropertyId propId)
{
	if (propId == EMediaPropertyId::ScaleWidth)
		return EMediaPropertyId::ScaleHeight;
	if (propId == EMediaPropertyId::ScaleHeight)
		return EMediaPropertyId::ScaleWidth;
	return propId;
}

bool CMediaKeyframeHelper::IsScaleAspectLocked(const QUuid& mediaId)
{
	auto pMedia = GetMediaNode(mediaId);
	if (!pMedia || !pMedia->GetBaseProperty())
		return false;
	return pMedia->GetBaseProperty()->GetKeepAspectRatio();
}

float CMediaKeyframeHelper::ComputePairedScaleValue(const QUuid& mediaId, EMediaPropertyId editedProp, float editedValue)
{
	auto pMedia = GetMediaNode(mediaId);
	if (!pMedia || !pMedia->GetBaseProperty())
		return editedValue;

	QSizeF base = pMedia->GetBaseProperty()->GetSizeKeepAspectRatio();
	float baseW = static_cast<float>(base.width());
	float baseH = static_cast<float>(base.height());
	if (baseW <= 0.f || baseH <= 0.f)
	{
		baseW = pMedia->GetMediaWidth();
		baseH = pMedia->GetMediaHeight();
	}
	if (baseW <= 0.f || baseH <= 0.f)
		return editedValue;

	const float ratio = baseW / baseH;
	if (editedProp == EMediaPropertyId::ScaleWidth)
		return editedValue / ratio;
	if (editedProp == EMediaPropertyId::ScaleHeight)
		return editedValue * ratio;
	return editedValue;
}

void CMediaKeyframeHelper::SyncPairedScaleKeyframeAfterToggle(const QUuid& mediaId, EMediaPropertyId propId,
	CMediaPropertyFrame* pFrame)
{
	if (!IsScaleSizePropertyId(propId) || !IsScaleAspectLocked(mediaId))
		return;

	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
		return;

	const EMediaPropertyId paired = GetPairedScalePropertyId(propId);
	const auto clipLocalMs = GetAlignedClipLocalMs(mediaId);
	const int nProp = ToPropId(propId);
	const int nPaired = ToPropId(paired);
	const bool hasPrimary = pObject->HasPropertyKeyframeAt(nProp, clipLocalMs);
	const bool hasPaired = pObject->HasPropertyKeyframeAt(nPaired, clipLocalMs);

	if (hasPrimary != hasPaired)
	{
		if (hasPrimary && !hasPaired)
		{
			SPropertyKeyframeNode primaryNode;
			float primaryVal = ReadPropertyValueAtPlayhead(mediaId, propId);
			if (pObject->GetPropertyKeyframeNode(nProp, clipLocalMs, primaryNode))
				primaryVal = primaryNode.value;
			const float pairedVal = ComputePairedScaleValue(mediaId, propId, primaryVal);
			pObject->AddPropertyKeyframe(nPaired, clipLocalMs, pairedVal);
			if (auto pMedia = GetMediaNode(mediaId))
			{
				if (paired == EMediaPropertyId::ScaleWidth)
					pMedia->SetMediaWidth(pairedVal);
				else
					pMedia->SetMediaHeight(pairedVal);
			}
			EnsureFxFactorForProperty(mediaId, paired, pairedVal, pFrame);
		}
		else if (!hasPrimary && hasPaired)
		{
			pObject->DeletePropertyKeyframe(nPaired, clipLocalMs);
		}

		ApplyEvaluatedPropertiesToMediaNode(mediaId, pObject->GetStartTime() + clipLocalMs, pFrame);
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_MediaNodeDataChange);
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_SliceWnd, SignalCenter::EDDT_MediaNodeDataChange);
		emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	}

	if (pFrame)
		SyncPropertyFrameFromTimeline(pFrame, mediaId, paired, clipLocalMs);
	NotifyClipFxPropertyKeyframeChanged(mediaId, paired, clipLocalMs);
}

void CMediaKeyframeHelper::SyncPairedScaleKeyframeAfterDrag(const QUuid& mediaId, EMediaPropertyId propId,
	const std::chrono::milliseconds& originalKeyTime,
	const std::chrono::milliseconds& newKeyTime,
	float editedValue)
{
	if (!IsScaleSizePropertyId(propId) || !IsScaleAspectLocked(mediaId))
		return;

	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
		return;

	const EMediaPropertyId paired = GetPairedScalePropertyId(propId);
	const int nPaired = ToPropId(paired);
	const float pairedVal = ComputePairedScaleValue(mediaId, propId, editedValue);

	if (originalKeyTime != newKeyTime
		&& pObject->HasPropertyKeyframeAt(nPaired, originalKeyTime))
	{
		pObject->MovePropertyKeyframe(nPaired, originalKeyTime, newKeyTime);
	}

	if (pObject->HasPropertyKeyframeAt(nPaired, newKeyTime))
		pObject->SetPropertyKeyframeValue(nPaired, newKeyTime, pairedVal);
	else
		pObject->AddPropertyKeyframe(nPaired, newKeyTime, pairedVal);

	if (auto pMedia = GetMediaNode(mediaId))
	{
		if (propId == EMediaPropertyId::ScaleWidth)
		{
			pMedia->SetMediaWidth(editedValue);
			pMedia->SetMediaHeight(pairedVal);
		}
		else
		{
			pMedia->SetMediaHeight(editedValue);
			pMedia->SetMediaWidth(pairedVal);
		}
	}

	NotifyClipFxPropertyKeyframeChanged(mediaId, paired, newKeyTime);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_MediaNodeDataChange);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_SliceWnd, SignalCenter::EDDT_MediaNodeDataChange);
	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
}

bool CMediaKeyframeHelper::ToggleGroupKeyframes(const QUuid& mediaId, EMediaPropertyGroupId groupId,
	CMediaPropertyFrame* pFrame)
{
	g_playheadOverrideMs.reset();
	if (!IsPlayheadOnObject(mediaId))
		return false;
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
		return false;

	const auto props = GetGroupPropertyIds(groupId);
	if (props.empty())
		return false;

	const auto clipLocalMs = GetAlignedClipLocalMs(mediaId);
	bool bAllHave = true;
	for (auto propId : props)
	{
		if (!pObject->HasPropertyKeyframeAt(ToPropId(propId), clipLocalMs))
		{
			bAllHave = false;
			break;
		}
	}

	if (bAllHave)
	{
		// 分组取消：删除组内全部属性在游标处的关键帧，并立即清掉残留特效
		for (auto propId : props)
			pObject->DeletePropertyKeyframe(ToPropId(propId), clipLocalMs);
		ApplyEvaluatedPropertiesToMediaNode(mediaId, pObject->GetStartTime() + clipLocalMs, pFrame);
		if (groupId == EMediaPropertyGroupId::Effect)
			CTimelinePropertyKeyframeHelper::ResetEffectWhenNoKeyframeTracks(mediaId);
	}
	else
	{
		// 分组添加：为尚无关键帧的属性在游标处补齐关键帧
		for (auto propId : props)
		{
			const int nPropId = ToPropId(propId);
			if (pObject->HasPropertyKeyframeAt(nPropId, clipLocalMs))
				continue;
			const float value = pFrame ? ReadPropertyValue(pFrame, propId)
				: ReadPropertyValueAtPlayhead(mediaId, propId);
			pObject->AddPropertyKeyframe(nPropId, clipLocalMs, value);
			EnsureFxFactorForProperty(mediaId, propId, value, pFrame);
		}
		ApplyEvaluatedPropertiesToMediaNode(mediaId, pObject->GetStartTime() + clipLocalMs, pFrame);
	}

	if (g_pSignalCenter)
	{
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_MediaNodeDataChange);
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_SliceWnd, SignalCenter::EDDT_MediaNodeDataChange);
		emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	}
	if (!props.empty())
		NotifyClipFxPropertyKeyframeChanged(mediaId, props.front(), clipLocalMs);
	return true;
}

bool CMediaKeyframeHelper::ResetPropertyGroup(const QUuid& mediaId, EMediaPropertyGroupId groupId,
	CMediaPropertyFrame* pFrame)
{
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
		return false;

	const auto props = GetGroupPropertyIds(groupId);
	if (props.empty())
		return false;

	// 1) 清空组内每个属性的全部关键帧
	for (auto propId : props)
		ClearAllKeyframesForProperty(pObject, ToPropId(propId));

	// 2) 先刷新全部 UI
	for (auto propId : props)
	{
		const float value = GetPropertyFactoryDefaultValue(mediaId, propId);
		SetPropertyFrameDisplayValue(pFrame, propId, value);
	}

	// 3) Keying/Feather/Crop：删除 FX 因子（禁止 WritePropertyBaseValue→Ensure 重建默认绿幕等）
	//    Effect 等其它组：写回出厂默认到媒体属性
	if (groupId == EMediaPropertyGroupId::Keying
		|| groupId == EMediaPropertyGroupId::Feather
		|| groupId == EMediaPropertyGroupId::Crop)
	{
		if (auto pMedia = GetMediaNode(mediaId))
		{
			if (auto pFxPty = std::dynamic_pointer_cast<CFxProperty>(pMedia->GetFxProperty()))
			{
				if (groupId == EMediaPropertyGroupId::Keying)
					pFxPty->DeleteFxFactor(EEffectType::Keying);
				else if (groupId == EMediaPropertyGroupId::Feather)
					pFxPty->DeleteFxFactor(EEffectType::Feather);
				else
					pFxPty->DeleteFxFactor(EEffectType::Cut);
			}
		}
	}
	else
	{
		for (auto propId : props)
		{
			const float value = GetPropertyFactoryDefaultValue(mediaId, propId);
			WritePropertyBaseValue(mediaId, propId, value, pFrame);
		}
	}

	// 4) 再走一遍评估：无轨道时 Timeline helper 也会清残留 Effect / FX
	if (g_pPlayerController)
		ApplyEvaluatedPropertiesToMediaNode(mediaId, g_pPlayerController->GetCurTime(), pFrame);

	// 5) 刷新时间线 / 切片 / 2D 视图，并通知 ClipFx 重建
	if (g_pSignalCenter)
	{
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_MediaNodeDataChange);
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_SliceWnd, SignalCenter::EDDT_MediaNodeDataChange);
		emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
		for (auto propId : props)
			NotifyClipFxPropertyKeyframeChanged(mediaId, propId, std::chrono::milliseconds(-1));
	}
	return true;
}



bool CMediaKeyframeHelper::NavigatePropertyKeyframe(const QUuid& mediaId, EMediaPropertyId propId, EKeyFrameOp op,
	std::optional<std::chrono::milliseconds>* outTargetClipLocalMs)
{
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject || !g_pSignalCenter)
		return false;

	const auto times = pObject->GetPropertyKeyframeTimes(ToPropId(propId));
	if (times.empty())
		return false;

	const auto curLocal = GetAlignedClipLocalMs(mediaId);
	std::chrono::milliseconds target = curLocal;
	if (op == EKeyFrameOp::Pre)
	{
		auto it = std::lower_bound(times.begin(), times.end(), curLocal);
		if (it == times.begin())
			return false;
		target = *(--it);
	}
	else if (op == EKeyFrameOp::Next)
	{
		auto it = std::upper_bound(times.begin(), times.end(), curLocal);
		if (it == times.end())
			return false;
		target = *it;
	}
	else
	{
		return false;
	}

	const auto absTime = pObject->GetStartTime() + target;
	emit g_pSignalCenter->signalSeekTimelineAt(absTime.count());
	// 同步刷新时间线主游标（与键盘 Seek 的 RefreshViewAfterSeek 一致）
	constexpr int kSeekRefreshViewMask = (1 << 1) | (1 << 7) | (1 << 17); // TimeLineTool | TimeLineScale | NoniusView
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, kSeekRefreshViewMask);
	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	if (outTargetClipLocalMs)
		*outTargetClipLocalMs = target;
	return true;
}
void CMediaKeyframeHelper::RefreshKeyframeToggleState(CMediaPropertyFrame* pFrame, EMediaPropertyId propId,
	CPropertyKeyFrameBar* pBar, std::optional<std::chrono::milliseconds> playheadMs)
{
	if (!pBar)
		return;
	if (!pFrame || pFrame->GetSelectedMediaIds().size() != 1)
	{
		pBar->SetEnabled(false);
		pBar->SetToggleChecked(false);
		return;
	}

	const QUuid mediaId = *pFrame->GetSelectedMediaIds().begin();
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject || !IsPlayheadOnObject(mediaId, playheadMs))
	{
		pBar->SetEnabled(false);
		pBar->SetToggleChecked(false);
		return;
	}

	const int nPropId = ToPropId(propId);
	const auto clipLocalMs = GetAlignedClipLocalMs(mediaId, playheadMs);
	pBar->SetEnabled(true);
	pBar->SetToggleChecked(pObject->HasPropertyKeyframeAt(nPropId, clipLocalMs));
	pBar->SetNavEnabled(pObject->HasPrePropertyKeyframe(nPropId, clipLocalMs),
		pObject->HasNextPropertyKeyframe(nPropId, clipLocalMs));
}

void CMediaKeyframeHelper::RefreshGroupKeyframeToggleState(CMediaPropertyFrame* pFrame, EMediaPropertyGroupId groupId,
	CPropertyKeyFrameBar* pBar, std::optional<std::chrono::milliseconds> playheadMs)
{
	if (!pBar)
		return;
	if (!pFrame || pFrame->GetSelectedMediaIds().size() != 1)
	{
		pBar->SetEnabled(false);
		pBar->SetToggleChecked(false);
		return;
	}

	const QUuid mediaId = *pFrame->GetSelectedMediaIds().begin();
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject || !IsPlayheadOnObject(mediaId, playheadMs))
	{
		pBar->SetEnabled(false);
		pBar->SetToggleChecked(false);
		return;
	}

	const auto clipLocalMs = GetAlignedClipLocalMs(mediaId, playheadMs);
	const auto props = GetGroupPropertyIds(groupId);
	// 分组 Toggle 勾选：组内全部属性在当前游标处均有关键帧（缺一则取消勾选）
	bool bAllHaveKeyframeAt = !props.empty();
	bool bHasPre = false;
	bool bHasNext = false;
	for (auto propId : props)
	{
		const int nPropId = ToPropId(propId);
		if (!pObject->HasPropertyKeyframeAt(nPropId, clipLocalMs))
			bAllHaveKeyframeAt = false;
		if (pObject->HasPrePropertyKeyframe(nPropId, clipLocalMs))
			bHasPre = true;
		if (pObject->HasNextPropertyKeyframe(nPropId, clipLocalMs))
			bHasNext = true;
	}

	pBar->SetEnabled(true);
	pBar->SetToggleChecked(bAllHaveKeyframeAt);
	pBar->SetNavEnabled(bHasPre, bHasNext);
}

void CMediaKeyframeHelper::RefreshAllKeyframeToggleStates(CMediaPropertyFrame* pFrame)
{
	if (!pFrame)
		return;
	pFrame->RefreshPropertyKeyframeToggleStates();
}

float CMediaKeyframeHelper::EvaluatePropertyFloat(const CTimeLineObject* pObject, EMediaPropertyId propId,
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

void CMediaKeyframeHelper::ApplyEvaluatedPropertiesToMediaNode(const QUuid& mediaId,
	const std::chrono::milliseconds& timelineMs, CMediaPropertyFrame* pFrame)
{
	auto pObject = GetTimelineObject(mediaId);
	auto pMedia = GetMediaNode(mediaId);
	if (!pObject || !pMedia)
		return;
	if (timelineMs < pObject->GetStartTime() || timelineMs >= pObject->GetStartTime() + pObject->GetDuration())
	{
		// 游标不在 clip 内时仍清无轨 FX（重置等场景）
		CTimelinePropertyKeyframeHelper::ClearFxWhenNoKeyframeTracks(mediaId);
		return;
	}

	const auto clipLocalMs = timelineMs - pObject->GetStartTime();

	// Effect / Keying：播放端只写有轨的值；UI 再清无轨残留 FX
	CTimelinePropertyKeyframeHelper::ApplyPropertyKeyframesToMedia(mediaId, timelineMs);
	CTimelinePropertyKeyframeHelper::ClearFxWhenNoKeyframeTracks(mediaId);

	auto applyFloat = [&](EMediaPropertyId propId, float fallback, auto setter) {
		setter(EvaluatePropertyFloat(pObject, propId, clipLocalMs, fallback));
	};

	applyFloat(EMediaPropertyId::VolumeIntensity, pMedia->GetVolumn(), [&](float v) { pMedia->SetVolumn(v); });

	// 仅当存在 Crop 关键帧轨道时才 Ensure Cut，避免仅加抠像关键帧时误创建裁剪因子
	if (pFrame)
	{
		static const EMediaPropertyId kCropProps[] = {
			EMediaPropertyId::CropFill, EMediaPropertyId::CropLeft, EMediaPropertyId::CropRight,
			EMediaPropertyId::CropTop, EMediaPropertyId::CropBottom
		};
		bool bHasCropTrack = false;
		for (auto cropId : kCropProps)
		{
			if (!pObject->IsPropertyKeyframeTrackEmpty(static_cast<int>(cropId)))
			{
				bHasCropTrack = true;
				break;
			}
		}
		if (bHasCropTrack)
		{
			EnsureFxFactorForProperty(mediaId, EMediaPropertyId::CropLeft,
				EvaluatePropertyFloat(pObject, EMediaPropertyId::CropLeft, clipLocalMs,
					pFrame->GetPropertyKeyframeValue(EMediaPropertyId::CropLeft)),
				pFrame);
		}
	}
}

bool CMediaKeyframeHelper::IsPropertyVerticalDragAllowed(EMediaPropertyId propId)
{
	switch (propId)
	{
	case EMediaPropertyId::PosX:
	case EMediaPropertyId::PosY:
	case EMediaPropertyId::PosZ:
		return false;
	default:
		return true;
	}
}

float CMediaKeyframeHelper::ReadPropertyValueAtPlayhead(const QUuid& mediaId, EMediaPropertyId propId, float fallback)
{
	// 无轨道时用属性默认值（抠像绿=1、容差=0.5 等），禁止默认 fallback=0 写成黑键导致画面被抠空
	Q_UNUSED(fallback);
	const float defaultValue = GetPropertyDefaultValue(mediaId, propId);
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject || !IsPlayheadOnObject(mediaId))
		return defaultValue;
	return EvaluatePropertyFloat(pObject, propId, GetAlignedClipLocalMs(mediaId), defaultValue);
}

bool CMediaKeyframeHelper::TogglePropertyKeyframeForClipFx(const QUuid& mediaId, EMediaPropertyId propId,
	CMediaPropertyFrame* pFrame)
{
	const float value = pFrame
		? ReadPropertyValue(pFrame, propId)
		: ReadPropertyValueAtPlayhead(mediaId, propId);
	return TogglePropertyKeyframe(mediaId, propId, value, pFrame);
}

void CMediaKeyframeHelper::RefreshClipFxKeyframeBar(const QUuid& mediaId, EMediaPropertyId propId,
	CPropertyKeyFrameBar* pBar, std::optional<std::chrono::milliseconds> refKeyTime)
{
	if (!pBar)
		return;
	const bool bEnabled = IsPlayheadOnObject(mediaId);
	if (!bEnabled)
	{
		pBar->SetEnabled(false);
		pBar->SetToggleChecked(false);
		return;
	}

	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
	{
		pBar->SetEnabled(false);
		pBar->SetToggleChecked(false);
		return;
	}

	const int nPropId = ToPropId(propId);
	const auto refTime = refKeyTime.value_or(GetAlignedClipLocalMs(mediaId));

	pBar->SetEnabled(true);
	pBar->SetToggleChecked(pObject->HasPropertyKeyframeAt(nPropId, refTime));
	pBar->SetNavEnabled(pObject->HasPrePropertyKeyframe(nPropId, refTime),
		pObject->HasNextPropertyKeyframe(nPropId, refTime));
}

bool CMediaKeyframeHelper::ApplyPropertyKeyframeCurveMode(const QUuid& mediaId, EMediaPropertyId propId,
	const std::chrono::milliseconds& keyTime, EPropertyKeyframeCurveMode mode)
{
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
		return false;
	if (IsPropertyKeyframeCurveModeLockedToLine(propId)
		&& mode != EPropertyKeyframeCurveMode::Line)
		return false;
	if (mode == EPropertyKeyframeCurveMode::AutoBezier
		&& !CanUseAutoBezierCurve(pObject, propId, keyTime))
		return false;
	if (mode == EPropertyKeyframeCurveMode::FreeBezier
		&& !CanUseFreeBezierCurve(pObject, propId, keyTime))
		return false;
	pObject->SetPropertyKeyframeCurveMode(ToPropId(propId), keyTime, mode);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_MediaNodeDataChange);
	return true;
}

bool CMediaKeyframeHelper::CanUseAutoBezierCurve(const CTimeLineObject* pObject, EMediaPropertyId propId,
	const std::chrono::milliseconds& keyTime)
{
	if (IsPropertyKeyframeCurveModeLockedToLine(propId))
		return false;
	if (!pObject)
		return false;
	const int nPropId = ToPropId(propId);
	return pObject->HasPrePropertyKeyframe(nPropId, keyTime)
		|| pObject->HasNextPropertyKeyframe(nPropId, keyTime);
}

bool CMediaKeyframeHelper::CanUseFreeBezierCurve(const CTimeLineObject* pObject, EMediaPropertyId propId,
	const std::chrono::milliseconds& keyTime)
{
	if (IsPropertyKeyframeCurveModeLockedToLine(propId))
		return false;
	if (!pObject)
		return false;
	const int nPropId = ToPropId(propId);
	return pObject->HasPrePropertyKeyframe(nPropId, keyTime)
		&& pObject->HasNextPropertyKeyframe(nPropId, keyTime);
}

void CMediaKeyframeHelper::SeekToClipLocalTime(const QUuid& mediaId, const std::chrono::milliseconds& clipLocalMs)
{
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject || !g_pSignalCenter)
		return;
	const auto absTime = pObject->GetStartTime() + clipLocalMs;
	emit g_pSignalCenter->signalSeekTimelineAt(absTime.count());
	constexpr int kSeekRefreshViewMask = (1 << 1) | (1 << 7) | (1 << 17);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, kSeekRefreshViewMask);
	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
}

void CMediaKeyframeHelper::RegisterMediaPropertyFrame(CMediaPropertyFrame* pFrame)
{
	g_pRegisteredMediaPropertyFrame = pFrame;
}

CMediaPropertyFrame* CMediaKeyframeHelper::GetRegisteredMediaPropertyFrame()
{
	return g_pRegisteredMediaPropertyFrame;
}

void CMediaKeyframeHelper::SetPropertyFrameDisplayValue(CMediaPropertyFrame* pFrame, EMediaPropertyId propId, float value)
{
	if (!pFrame)
		return;
	pFrame->SetPropertyDisplayValue(propId, value);
}

void CMediaKeyframeHelper::SyncPropertyFrameFromTimeline(CMediaPropertyFrame* pFrame, const QUuid& mediaId,
	EMediaPropertyId propId, std::optional<std::chrono::milliseconds> clipLocalMs)
{
	if (!pFrame)
		return;
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
		return;

	const auto evalTime = clipLocalMs.value_or(GetAlignedClipLocalMs(mediaId));
	// 无轨道时用出厂默认，避免取消关键帧后仍读到节点上残留的上次插值
	const float fallback = pObject->IsPropertyKeyframeTrackEmpty(ToPropId(propId))
		? GetPropertyFactoryDefaultValue(mediaId, propId)
		: GetPropertyDefaultValue(mediaId, propId);
	const float value = EvaluatePropertyFloat(pObject, propId, evalTime, fallback);
	SetPropertyFrameDisplayValue(pFrame, propId, value);

	// ClipFx Sync 带本地时间时，覆盖随后 RefreshPropertyKeyframeToggleStates() 所用游标
	if (clipLocalMs)
	{
		g_playheadOverrideMs = pObject->GetStartTime() + *clipLocalMs;
		ScheduleClearPlayheadOverride(pFrame);
	}
}

// 游标在素材内时，按 clip 本地时间插值全部属性并刷新面板显示；
// 无轨道时用出厂默认（避免取消关键帧后 UI 仍显示上次插值，看起来像效果未恢复）
void CMediaKeyframeHelper::SyncKeyframeEvaluatedValuesAtPlayhead(CMediaPropertyFrame* pFrame, const QUuid& mediaId,
	std::optional<std::chrono::milliseconds> playheadMs)
{
	if (!pFrame)
		return;

	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
		return;

	// 游标不在 clip 范围内时不覆盖 UI（由 SetPropertyActionBarsEnabled 禁用编辑）
	if (!IsPlayheadOnObject(mediaId, playheadMs))
		return;

	const auto clipLocalMs = GetAlignedClipLocalMs(mediaId, playheadMs);
	for (auto groupId : kAllGroups)
	{
		for (auto propId : GetGroupPropertyIds(groupId))
		{
			const float fallback = pObject->IsPropertyKeyframeTrackEmpty(ToPropId(propId))
				? GetPropertyFactoryDefaultValue(mediaId, propId)
				: GetPropertyDefaultValue(mediaId, propId);
			const float value = EvaluatePropertyFloat(pObject, propId, clipLocalMs, fallback);
			SetPropertyFrameDisplayValue(pFrame, propId, value);
		}
	}
}

void CMediaKeyframeHelper::SyncActivePropertyFrame(const QUuid& mediaId, EMediaPropertyId propId,
	std::optional<std::chrono::milliseconds> clipLocalMs)
{
	auto* pFrame = GetRegisteredMediaPropertyFrame();
	if (!pFrame || pFrame->GetSelectedMediaIds().size() != 1)
		return;
	if (*pFrame->GetSelectedMediaIds().begin() != mediaId)
		return;
	SyncPropertyFrameFromTimeline(pFrame, mediaId, propId, clipLocalMs);
}
