#include "TimelinePreviewBridge.h"
#include "MediaPreviewWnd.h"
#include "PreviewFxPass.h"

#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/TimeLineModel/TimeLineObject.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "Model/MediaModel/MediaNode.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/PlayStateCenter/TimelineFilterHelper.h"
#include "Controll/PlayStateCenter/TimelinePropertyKeyframeHelper.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Common/Common/ResourceDefine.h"
#include "Common/Common/ITimelineInterface.h"
#include "Common/Common/MediaPropertyKeyframeIds.h"
#include "Common/Common/MediaCommon.h"

#include <set>
#include <cmath>
#include <variant>

using namespace std::chrono;

namespace
{
	constexpr qint64 kSeekJumpMs = 180;
	constexpr qint64 kFxSampleIntervalMs = 33;
	constexpr size_t kMaxPreviewLayers = 4;

	bool IsVisualResType(int t)
	{
		return t == ERT_Video || t == ERT_VideoCapture || t == ERT_Picture;
	}

	bool IsAudibleResType(int t)
	{
		return t == ERT_Audio || t == ERT_AudioCapture
			|| t == ERT_Video || t == ERT_VideoCapture;
	}
}

CTimelinePreviewBridge::CTimelinePreviewBridge(CMediaPreviewWnd* preview, QObject* parent)
	: QObject(parent)
	, m_pPreview(preview)
{
	if (m_pPreview)
		m_pPreview->SetPreviewBridge(this);

	m_noniusDebounce.setSingleShot(true);
	connect(&m_noniusDebounce, &QTimer::timeout, this, &CTimelinePreviewBridge::FlushPendingNoniusSeek);

	if (g_pSignalCenter)
	{
		connect(g_pSignalCenter, &CSignalCenter::signalUpdateMediaProperty,
			this, &CTimelinePreviewBridge::OnMediaPropertyChanged);
		connect(g_pSignalCenter, &CSignalCenter::signalFinishSeekTimeline,
			this, &CTimelinePreviewBridge::OnFinishSeekTimeline);
		connect(g_pSignalCenter, &CSignalCenter::signalRefreshWindow, this,
			[this](int, int nDataType) {
				if ((nDataType & SignalCenter::EDDT_MediaNodeDataChange) == 0)
					return;
				if (!m_lastLayerIds.empty())
					OnMediaPropertyChanged(m_lastLayerIds.back());
			});
	}
}

void CTimelinePreviewBridge::FlushPendingNoniusSeek()
{
	if (m_pendingNoniusMs < 0)
		return;
	const qint64 ms = m_pendingNoniusMs;
	m_pendingNoniusMs = -1;
	SyncTimelineAt(milliseconds(ms), true);
}

void CTimelinePreviewBridge::OnFinishSeekTimeline(qint64 timelineMs)
{
	// FinishSeek 之后 Present：强制 Seek + 出帧收尾
	m_pendingNoniusMs = -1;
	m_noniusDebounce.stop();
	const milliseconds ms = timelineMs >= 0
		? milliseconds(timelineMs)
		: (g_pPlayerController ? g_pPlayerController->GetCurTime() : milliseconds(0));
	SyncTimelineAt(ms, true);
	if (m_pPreview)
		m_pPreview->FinishSeekPreview();
}

void CTimelinePreviewBridge::OnPlayStateChanged()
{
	m_lastLayerIds.clear();
	m_lastAudioFocus = QUuid();
	m_lastPlaying = false;
	m_lastFxSampleTimelineMs = -1;
	m_lastVol = -1.f;
	m_pendingNoniusMs = -1;
	m_noniusDebounce.stop();
	SyncNow();
}

void CTimelinePreviewBridge::OnNoniusTimeChanged(qint64)
{
	if (!g_pPlayerController)
		return;
	const milliseconds timelineMs = g_pPlayerController->GetCurTime();
	// 播放中立即同步；暂停拖游标：合并 16ms 内事件，但拖拽中持续 Flush（勿每次重置定时器）
	if (g_pPlayerController->IsPlaying())
	{
		m_pendingNoniusMs = -1;
		m_noniusDebounce.stop();
		SyncTimelineAt(timelineMs, true);
		return;
	}
	m_pendingNoniusMs = timelineMs.count();
	if (!m_noniusDebounce.isActive())
		m_noniusDebounce.start(16);
}

void CTimelinePreviewBridge::SyncNow()
{
	SyncTimelineAt(g_pPlayerController ? g_pPlayerController->GetCurTime()
		: milliseconds(0), false);
}

void CTimelinePreviewBridge::OnMediaPropertyChanged(const QUuid& mediaId)
{
	if (!m_pPreview || mediaId.isNull())
		return;
	if (g_pPlayerController && g_pPlayerController->IsPlaying())
		return;

	bool bHit = m_lastLayerIds.empty();
	for (const auto& id : m_lastLayerIds)
	{
		if (id == mediaId)
		{
			bHit = true;
			break;
		}
	}
	if (!bHit)
		return;

	SyncTimelineAt(g_pPlayerController ? g_pPlayerController->GetCurTime()
		: milliseconds(0), false);
}

void CTimelinePreviewBridge::FillLayerEffects(SPreviewLayerInfo& layer, milliseconds timelineMs) const
{
	layer.color.SetDefault();
	layer.volume = 1.f;
	layer.trans.Reset();

	if (!g_pPlayerController || !g_pPlayerController->IsPlaying())
		CTimelinePropertyKeyframeHelper::ApplyPropertyKeyframesToMedia(layer.clipId, timelineMs);

	if (auto pMedia = std::dynamic_pointer_cast<CMediaNode>(g_pMediaModel->GetMediaNode(layer.clipId).lock()))
	{
		layer.volume = pMedia->GetVolumn();
		if (auto pFx = pMedia->GetEffectProperty())
			layer.color = pFx->GetColorProperty();

		if (auto pObj = dynamic_cast<CTimeLineObject*>(g_pTimeLineModel->GetTimeLineObject(layer.clipId).lock().get()))
		{
			const auto localMs = timelineMs - pObj->GetStartTime();
			if (localMs.count() >= 0)
			{
				auto eval = [&](EMediaPropertyId id, float fallback) {
					auto pTrack = pObj->GetPropertyKeyframeTrack(static_cast<int>(id)).lock();
					if (!pTrack || pTrack->IsEmpty())
						return fallback;
					auto v = pTrack->GetValue(localMs);
					if (std::holds_alternative<float>(v))
						return std::get<float>(v);
					return fallback;
				};
				layer.volume = eval(EMediaPropertyId::VolumeIntensity, layer.volume);
				layer.color.red = eval(EMediaPropertyId::EffectRed, layer.color.red);
				layer.color.green = eval(EMediaPropertyId::EffectGreen, layer.color.green);
				layer.color.blue = eval(EMediaPropertyId::EffectBlue, layer.color.blue);
				layer.color.alpha = eval(EMediaPropertyId::EffectAlpha, layer.color.alpha);
				layer.color.luminance = eval(EMediaPropertyId::EffectLuminance, layer.color.luminance);
				layer.color.contrast = eval(EMediaPropertyId::EffectContrast, layer.color.contrast);
				layer.color.hue = eval(EMediaPropertyId::EffectHue, layer.color.hue);
				layer.color.saturability = eval(EMediaPropertyId::EffectSaturation, layer.color.saturability);
			}
		}
	}

	CTimelineFilterHelper filter;
	if (SUCCEEDED(filter.CalcPlayStageAndProcess(layer.clipId, timelineMs, layer.trans)))
		layer.color.alpha *= CTimelineFilterHelper::GetEffectiveAlpha(layer.trans);
	else
		layer.trans.Reset();
}

bool CTimelinePreviewBridge::ResolveVisibleLayers(milliseconds timelineMs,
	std::vector<SPreviewLayerInfo>& outLayers, QUuid& outAudioFocusId) const
{
	outLayers.clear();
	outAudioFocusId = QUuid();
	if (!g_pTimeLineModel || !g_pMediaModel || !g_pResourceModel)
		return false;

	const auto playingIds = TimelineModel::GetAllPlayingClipIds(timelineMs);
	if (playingIds.empty())
		return false;

	auto buildLayer = [&](const QUuid& id, bool bVisual) -> bool {
		auto pMedia = g_pMediaModel->GetMediaNode(id).lock();
		if (!pMedia)
			return false;
		auto pRes = g_pResourceModel->GetResourceNode(pMedia->GetResourceId());
		if (!pRes)
			return false;
		const QString path = pRes->GetPath();
		if (path.isEmpty())
			return false;
		auto pObj = g_pTimeLineModel->GetTimeLineObject(id).lock();
		if (!pObj)
			return false;

		SPreviewLayerInfo L;
		L.clipId = id;
		L.path = path;
		L.bVisual = bVisual;
		const auto start = pObj->GetStartTime();
		const auto mediaStart = pMedia->GetClipStartTime();
		const qint64 localMs = (timelineMs - start + mediaStart).count();
		L.clipLocalMs = localMs < 0 ? 0 : localMs;
		outLayers.push_back(L);
		return true;
	};

	std::vector<QUuid> visualIds;
	std::vector<QUuid> audibleIds;
	std::vector<int> audibleTypes;
	for (const auto& id : playingIds)
	{
		auto pMedia = g_pMediaModel->GetMediaNode(id).lock();
		if (!pMedia)
			continue;
		const int t = pMedia->GetResourceType();
		if (IsVisualResType(t))
			visualIds.push_back(id);
		if (IsAudibleResType(t))
		{
			audibleIds.push_back(id);
			audibleTypes.push_back(t);
		}
	}

	if (visualIds.empty() && audibleIds.empty())
		return false;

	if (visualIds.size() > kMaxPreviewLayers)
		visualIds.erase(visualIds.begin(), visualIds.end() - static_cast<std::ptrdiff_t>(kMaxPreviewLayers));

	for (const auto& id : visualIds)
		buildLayer(id, true);

	for (auto& L : outLayers)
		FillLayerEffects(L, timelineMs);

	const auto selected = g_pSelectController ? g_pSelectController->GetTLObjectSelectIds() : std::set<QUuid>{};
	for (const auto& id : audibleIds)
	{
		if (selected.count(id))
		{
			outAudioFocusId = id;
			break;
		}
	}
	if (outAudioFocusId.isNull())
	{
		// 优先纯音频轨（顶层），否则取顶层可发声片段
		for (int i = static_cast<int>(audibleIds.size()) - 1; i >= 0; --i)
		{
			if (audibleTypes[static_cast<size_t>(i)] == ERT_Audio
				|| audibleTypes[static_cast<size_t>(i)] == ERT_AudioCapture)
			{
				outAudioFocusId = audibleIds[static_cast<size_t>(i)];
				break;
			}
		}
	}
	if (outAudioFocusId.isNull() && !audibleIds.empty())
		outAudioFocusId = audibleIds.back();

	if (!outAudioFocusId.isNull())
	{
		bool bInLayers = false;
		for (const auto& L : outLayers)
		{
			if (L.clipId == outAudioFocusId)
			{
				bInLayers = true;
				break;
			}
		}
		if (!bInLayers)
		{
			const size_t before = outLayers.size();
			if (buildLayer(outAudioFocusId, false))
				FillLayerEffects(outLayers.back(), timelineMs);
			else if (before == outLayers.size())
				outAudioFocusId = QUuid();
		}
	}

	return !outLayers.empty();
}

void CTimelinePreviewBridge::OnPlayTick()
{
	if (!m_pPreview || !g_pPlayerController || !g_pPlayerController->IsPlaying())
		return;

	const milliseconds timelineMs = g_pPlayerController->GetCurTime();
	std::vector<SPreviewLayerInfo> layers;
	QUuid audioFocus;
	if (!ResolveVisibleLayers(timelineMs, layers, audioFocus))
	{
		if (!m_lastLayerIds.empty())
		{
			m_lastLayerIds.clear();
			m_pPreview->SyncTimelineLayers({}, false, QUuid());
		}
		return;
	}

	std::vector<QUuid> ids;
	ids.reserve(layers.size());
	for (const auto& L : layers)
		ids.push_back(L.clipId);

	const bool bSameSet = (ids == m_lastLayerIds);
	const bool bNeedFx = (m_lastFxSampleTimelineMs < 0)
		|| (std::llabs(timelineMs.count() - m_lastFxSampleTimelineMs) >= kFxSampleIntervalMs);

	if (!bSameSet)
	{
		m_lastLayerIds = ids;
		m_lastAudioFocus = audioFocus;
		m_lastFxSampleTimelineMs = timelineMs.count();
		m_lastPlaying = true;
		m_pPreview->SyncTimelineLayers(layers, true, audioFocus);
		float vol = 1.f;
		for (const auto& L : layers)
		{
			if (L.clipId == audioFocus)
			{
				vol = L.volume;
				break;
			}
		}
		if (vol != m_lastVol)
		{
			m_lastVol = vol;
			m_pPreview->SetVolume(vol);
		}
		return;
	}

	// 同层集合：更新时钟（每层 localMs 已在 Resolve 中算好）
	m_lastPlaying = true;
	if (bNeedFx)
	{
		m_lastFxSampleTimelineMs = timelineMs.count();
		m_pPreview->SyncTimelineLayers(layers, true, audioFocus);
	}
	else
	{
		// 轻量：只推进顶层时钟，PullLayers 用 m_layerInfos 内各层 localMs
		// 刷新 layerInfos 时钟：再次 Sync 代价可接受（槽复用）
		m_pPreview->SyncTimelineLayers(layers, true, audioFocus);
	}

	float vol = 1.f;
	for (const auto& L : layers)
	{
		if (L.clipId == audioFocus)
		{
			vol = L.volume;
			break;
		}
	}
	if (vol != m_lastVol)
	{
		m_lastVol = vol;
		m_pPreview->SetVolume(vol);
	}
}

void CTimelinePreviewBridge::SyncTimelineAt(milliseconds timelineMs, bool bForceSeek)
{
	if (!m_pPreview || !g_pPlayerController)
		return;

	const bool bPlaying = g_pPlayerController->IsPlaying();
	std::vector<SPreviewLayerInfo> layers;
	QUuid audioFocus;
	if (!ResolveVisibleLayers(timelineMs, layers, audioFocus))
	{
		m_lastLayerIds.clear();
		m_lastPlaying = false;
		m_pPreview->SyncTimelineLayers({}, false, QUuid(), false);
		return;
	}

	m_lastLayerIds.clear();
	for (const auto& L : layers)
		m_lastLayerIds.push_back(L.clipId);
	m_lastAudioFocus = audioFocus;
	m_lastFxSampleTimelineMs = timelineMs.count();
	m_lastPlaying = bPlaying;

	m_pPreview->SyncTimelineLayers(layers, bPlaying, audioFocus, bForceSeek);

	float vol = 1.f;
	for (const auto& L : layers)
	{
		if (L.clipId == audioFocus)
		{
			vol = L.volume;
			break;
		}
	}
	if (vol != m_lastVol)
	{
		m_lastVol = vol;
		m_pPreview->SetVolume(vol);
	}

	// 顶层颜色给单层兼容路径
	if (!layers.empty())
		m_pPreview->SetPreviewColor(layers.back().color);
}

bool CTimelinePreviewBridge::ResolveActiveClip(milliseconds timelineMs,
	QString& outPath, qint64& outClipLocalMs, QUuid& outClipId,
	milliseconds* outClipStart, milliseconds* outClipEnd, milliseconds* outMediaClipStart) const
{
	std::vector<SPreviewLayerInfo> layers;
	QUuid audio;
	if (!ResolveVisibleLayers(timelineMs, layers, audio) || layers.empty())
		return false;

	QUuid chosen = audio;
	const auto selected = g_pSelectController ? g_pSelectController->GetTLObjectSelectIds() : std::set<QUuid>{};
	for (const auto& L : layers)
	{
		if (selected.count(L.clipId))
		{
			chosen = L.clipId;
			break;
		}
	}

	const SPreviewLayerInfo* pSel = &layers.back();
	for (const auto& L : layers)
	{
		if (L.clipId == chosen)
		{
			pSel = &L;
			break;
		}
	}

	outPath = pSel->path;
	outClipLocalMs = pSel->clipLocalMs;
	outClipId = pSel->clipId;
	if (auto pObj = g_pTimeLineModel->GetTimeLineObject(pSel->clipId).lock())
	{
		if (outClipStart) *outClipStart = pObj->GetStartTime();
		if (outClipEnd) *outClipEnd = pObj->GetStartTime() + pObj->GetDuration();
	}
	if (auto pMedia = g_pMediaModel->GetMediaNode(pSel->clipId).lock())
	{
		if (outMediaClipStart) *outMediaClipStart = pMedia->GetClipStartTime();
	}
	return true;
}
