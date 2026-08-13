#include "TimeLineGlobalDefine.h"
#include "ClipFxKeyframeHelper.h"

#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/TimeLineModel//ConvertHelper.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "ContentViewData.h"
#include "UICommon/PropertyKeyFrameBar.h"
#include "Common/Common/XEffectCommon.h"

#include <algorithm>
#include <optional>
#include <variant>
#include <vector>
#include "TimelineWinMacros.h"

namespace
{
	int ToPropId(EMediaPropertyId propId)
	{
		return static_cast<int>(propId);
	}

	std::shared_ptr<IMediaNode> GetMediaNode(const QUuid& mediaId)
	{
		// Timeline 首版 MediaModel 桩：无媒体节点时关键帧属性同步跳过
		if (!g_pMediaModel || mediaId.isNull())
			return nullptr;
		return g_pMediaModel->GetMediaNode(mediaId).lock();
	}

	CTimeLineObject* GetTimelineObject(const QUuid& mediaId)
	{
		if (!g_pTimeLineModel || mediaId.isNull())
			return nullptr;
		auto pObj = g_pTimeLineModel->GetTimeLineObject(mediaId).lock();
		return dynamic_cast<CTimeLineObject*>(pObj.get());
	}

	/** 刷新时间线主游标/刻度（与 ContentWidget::RefreshViewAfterSeek 一致） */
	void RefreshTimelineNoniusViews()
	{
		if (!g_pSignalCenter)
			return;
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd,
			WndType::WndType_TimeLineTool | WndType::WndType_NoniusView
			| WndType::WndType_TimeLineScale | WndType::WndType_LayersView);
		emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	}

	/**
	 * @brief Seek 并保证主游标与 ClipFx Plot 同步
	 * 先 SetNoniusTime 并刷新主游标，再通知 Plot（signalNoniusTimeChanged）
	 */
	bool SeekTimelineAtWithSyncNonius(const std::chrono::milliseconds& absTime, bool bForce = false)
	{
		if (!g_pContentViewData || !g_pSignalCenter || !g_pPlayerController)
			return false;
		if (!bForce && g_pContentViewData->IsTimelineRecordLocked())
			return false;

		g_pContentViewData->SetNoniusTime(absTime);
		RefreshTimelineNoniusViews();
		g_pPlayerController->Seek(g_pContentViewData->GetNoniusTime());
		emit g_pSignalCenter->signalNoniusTimeChanged(g_pContentViewData->GetNoniusTime().count());
		return true;
	}
}

bool CClipFxKeyframeHelper::IsPlayheadOnObject(const QUuid& mediaId)
{
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
		return false;

	std::chrono::milliseconds t{ 0 };
	if (g_pContentViewData)
		t = g_pContentViewData->GetNoniusTime();
	else if (g_pPlayerController)
		t = g_pPlayerController->GetCurTime();
	else
		return false;

	return t >= pObject->GetStartTime() && t < pObject->GetStartTime() + pObject->GetDuration();
}

std::chrono::milliseconds CClipFxKeyframeHelper::GetAlignedClipLocalMs(const QUuid& mediaId)
{
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
		return std::chrono::milliseconds(0);

	std::chrono::milliseconds t{ 0 };
	if (g_pContentViewData)
		t = g_pContentViewData->GetNoniusTime();
	else if (g_pPlayerController)
		t = g_pPlayerController->GetCurTime();
	else
		return std::chrono::milliseconds(0);

	const auto localMs = t - pObject->GetStartTime();
	if (g_pConvertHelper)
		return g_pConvertHelper->formatTimeWithFrame(localMs);
	return localMs;
}

void CClipFxKeyframeHelper::GetPropertyValueRange(EMediaPropertyId propId, float& outMin, float& outMax)
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

bool CClipFxKeyframeHelper::IsPropertyVerticalDragAllowed(EMediaPropertyId propId)
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


static float GetPropertyFactoryDefaultValue(EMediaPropertyId propId)
{
	const SKeyingFactor kDefaultKeying;
	switch (propId)
	{
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
	case EMediaPropertyId::EffectLuminance:
	case EMediaPropertyId::EffectContrast:
	case EMediaPropertyId::EffectHue:
	case EMediaPropertyId::EffectSaturation:
		return 0.f;
	default:
		return 0.f;
	}
}


float CClipFxKeyframeHelper::EvaluatePropertyFloat(const CTimeLineObject* pObject, EMediaPropertyId propId,
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

float CClipFxKeyframeHelper::ReadPropertyValueAtPlayhead(const QUuid& mediaId, EMediaPropertyId propId, float fallback)
{
	Q_UNUSED(fallback);
	const float defaultValue = GetPropertyFactoryDefaultValue(propId);
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject || !IsPlayheadOnObject(mediaId))
		return defaultValue;
	return EvaluatePropertyFloat(pObject, propId, GetAlignedClipLocalMs(mediaId), defaultValue);
}

bool CClipFxKeyframeHelper::TogglePropertyKeyframe(const QUuid& mediaId, EMediaPropertyId propId)
{
	if (!IsPlayheadOnObject(mediaId))
		return false;
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject)
		return false;

	const auto clipLocalMs = GetAlignedClipLocalMs(mediaId);
	const int nPropId = ToPropId(propId);
	const float value = ReadPropertyValueAtPlayhead(mediaId, propId);

	if (pObject->HasPropertyKeyframeAt(nPropId, clipLocalMs))
	{
		// 取消子属性：只删当前属性关键帧，不影响同组其它属性
		pObject->DeletePropertyKeyframe(nPropId, clipLocalMs);
	}
	else
	{
		pObject->AddPropertyKeyframe(nPropId, clipLocalMs, value);
	}

	// 等比例锁定时同步配对宽/高关键帧
	SyncPairedScaleKeyframeAfterToggle(mediaId, propId);
	return true;
}

bool CClipFxKeyframeHelper::IsScaleSizePropertyId(EMediaPropertyId propId)
{
	return propId == EMediaPropertyId::ScaleWidth || propId == EMediaPropertyId::ScaleHeight;
}

EMediaPropertyId CClipFxKeyframeHelper::GetPairedScalePropertyId(EMediaPropertyId propId)
{
	if (propId == EMediaPropertyId::ScaleWidth)
		return EMediaPropertyId::ScaleHeight;
	if (propId == EMediaPropertyId::ScaleHeight)
		return EMediaPropertyId::ScaleWidth;
	return propId;
}

bool CClipFxKeyframeHelper::IsScaleAspectLocked(const QUuid& mediaId)
{
	auto pMedia = GetMediaNode(mediaId);
	if (!pMedia || !pMedia->GetBaseProperty())
		return false;
	return pMedia->GetBaseProperty()->GetKeepAspectRatio();
}

float CClipFxKeyframeHelper::ComputePairedScaleValue(const QUuid& mediaId, EMediaPropertyId editedProp, float editedValue)
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

void CClipFxKeyframeHelper::SyncPairedScaleKeyframeAfterToggle(const QUuid& mediaId, EMediaPropertyId propId)
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
		}
		else if (!hasPrimary && hasPaired)
		{
			pObject->DeletePropertyKeyframe(nPaired, clipLocalMs);
		}
	}

	SyncActivePropertyFrame(mediaId, paired, clipLocalMs);
}

void CClipFxKeyframeHelper::SyncPairedScaleKeyframeAfterDrag(const QUuid& mediaId, EMediaPropertyId propId,
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

	SyncActivePropertyFrame(mediaId, paired, newKeyTime);
}

void CClipFxKeyframeHelper::RefreshClipFxKeyframeBar(const QUuid& mediaId, EMediaPropertyId propId,
	CPropertyKeyFrameBar* pBar)
{
	if (!pBar)
		return;
	if (!IsPlayheadOnObject(mediaId))
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
	// 始终以当前游标对齐后的 clip 本地时间刷新 toggle / 前后导航可用态
	const auto playheadMs = GetAlignedClipLocalMs(mediaId);

	pBar->SetEnabled(true);
	pBar->SetToggleChecked(pObject->HasPropertyKeyframeAt(nPropId, playheadMs));
	pBar->SetNavEnabled(pObject->HasPrePropertyKeyframe(nPropId, playheadMs),
		pObject->HasNextPropertyKeyframe(nPropId, playheadMs));
}

bool CClipFxKeyframeHelper::ApplyPropertyKeyframeCurveMode(const QUuid& mediaId, EMediaPropertyId propId,
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
	if (g_pSignalCenter)
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd, SignalCenter::EDDT_MediaNodeDataChange);
	return true;
}

bool CClipFxKeyframeHelper::CanUseAutoBezierCurve(const CTimeLineObject* pObject, EMediaPropertyId propId,
	const std::chrono::milliseconds& keyTime)
{
	if (IsPropertyKeyframeCurveModeLockedToLine(propId))
		return false;
	if (!pObject)
		return false;
	const int nPropId = ToPropId(propId);
	// 至少一侧有同属性相邻关键帧才可选自动贝塞尔
	return pObject->HasPrePropertyKeyframe(nPropId, keyTime)
		|| pObject->HasNextPropertyKeyframe(nPropId, keyTime);
}

bool CClipFxKeyframeHelper::CanUseFreeBezierCurve(const CTimeLineObject* pObject, EMediaPropertyId propId,
	const std::chrono::milliseconds& keyTime)
{
	if (IsPropertyKeyframeCurveModeLockedToLine(propId))
		return false;
	if (!pObject)
		return false;
	const int nPropId = ToPropId(propId);
	// 必须夹在两个同属性关键帧中间才可选自由曲线
	return pObject->HasPrePropertyKeyframe(nPropId, keyTime)
		&& pObject->HasNextPropertyKeyframe(nPropId, keyTime);
}

void CClipFxKeyframeHelper::SeekToClipLocalTime(const QUuid& mediaId, const std::chrono::milliseconds& clipLocalMs)
{
	auto pObject = GetTimelineObject(mediaId);
	if (!pObject || !g_pContentViewData)
		return;

	const auto absTime = pObject->GetStartTime() + clipLocalMs;
	SeekTimelineAtWithSyncNonius(absTime);
}

void CClipFxKeyframeHelper::SyncActivePropertyFrame(const QUuid& mediaId, EMediaPropertyId propId,
	std::optional<std::chrono::milliseconds> clipLocalMs)
{
	if (!g_pSignalCenter)
		return;
	const qint64 ms = clipLocalMs.has_value()
		? static_cast<qint64>(clipLocalMs->count()) : -1;
	emit g_pSignalCenter->signalSyncClipFxActiveProperty(mediaId, static_cast<int>(propId), ms);
}
