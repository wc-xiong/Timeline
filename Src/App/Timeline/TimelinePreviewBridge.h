#pragma once

#include <QObject>
#include <QString>
#include <QUuid>
#include <QTimer>
#include <chrono>
#include <vector>
#include "Common/Common/MediaCommon.h"
#include "PreviewLayerTypes.h"

class CMediaPreviewWnd;

// 将时间线播控驱动到预览：Seek 全量；播放跟时钟；多层可见片段合成
class CTimelinePreviewBridge : public QObject
{
	Q_OBJECT
public:
	explicit CTimelinePreviewBridge(CMediaPreviewWnd* preview, QObject* parent = nullptr);

public slots:
	void OnPlayStateChanged();
	void OnNoniusTimeChanged(qint64 timelineMs);
	void SyncNow();
	void OnPlayTick();
	void OnMediaPropertyChanged(const QUuid& mediaId);

private:
	bool ResolveVisibleLayers(std::chrono::milliseconds timelineMs,
		std::vector<SPreviewLayerInfo>& outLayers, QUuid& outAudioFocusId) const;
	bool ResolveActiveClip(std::chrono::milliseconds timelineMs,
		QString& outPath, qint64& outClipLocalMs, QUuid& outClipId,
		std::chrono::milliseconds* outClipStart = nullptr,
		std::chrono::milliseconds* outClipEnd = nullptr,
		std::chrono::milliseconds* outMediaClipStart = nullptr) const;
	void FillLayerEffects(SPreviewLayerInfo& layer, std::chrono::milliseconds timelineMs) const;
	void SyncTimelineAt(std::chrono::milliseconds timelineMs, bool bForceSeek);
	void FlushPendingNoniusSeek();
	void OnFinishSeekTimeline(qint64 timelineMs);

	CMediaPreviewWnd* m_pPreview = nullptr;
	std::vector<QUuid> m_lastLayerIds;
	QUuid m_lastAudioFocus;
	bool m_lastPlaying = false;
	qint64 m_lastFxSampleTimelineMs = -1;
	float m_lastVol = -1.f;
	QTimer m_noniusDebounce;
	qint64 m_pendingNoniusMs = -1;
};
