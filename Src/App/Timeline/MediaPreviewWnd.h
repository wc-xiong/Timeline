#pragma once

#include <QWidget>
#include <QTimer>
#include <QString>
#include <QUuid>
#include <vector>
#include <cstdint>
#include "Common/Common/MediaCommon.h"
#include "PreviewLayerTypes.h"

struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

class CPreviewDecodeSession;
class CTimelinePreviewBridge;

// 预览窗：硬解多层 GPU 合成（阶段5）；软解为回落；SDL 音频旁路
class CMediaPreviewWnd : public QWidget
{
	Q_OBJECT
public:
	explicit CMediaPreviewWnd(QWidget* parent = nullptr);
	~CMediaPreviewWnd() override;

	QPaintEngine* paintEngine() const override { return nullptr; }

	void SetPreviewBridge(CTimelinePreviewBridge* bridge);

public slots:
	void OpenFile(const QString& path);
	void SyncTimelineMedia(const QString& path, qint64 clipLocalMs, bool bPlaying);
	/** 阶段5：多层可见片段（底→顶） */
	void SyncTimelineLayers(const std::vector<SPreviewLayerInfo>& layers, bool bPlaying,
		const QUuid& audioFocusId, bool bForceSeek = false);
	void SetPreviewClockMs(qint64 clipLocalMs);
	void Play();
	void Pause();
	void Stop();
	void SetVolume(double volume);
	void SetPreviewColor(const SColorPty& color);
	void SetPreviewFx(float alpha, float rMul, float gMul, float bMul);
	void InvalidatePreviewFx();
	void RefreshSurface();
	/** 若仍 seeking 则阻塞 prime 首帧后再 Present */
	void FinishSeekPreview();

protected:
	void showEvent(QShowEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private slots:
	void OnTick();

private:
	bool EnsureDx();
	bool EnsurePipeline();
	void PresentBlack();
	void PresentLastFrame();
	void ClearFrameCache();
	void DrawFrameToBackBuffer(bool bUploadPixels);
	bool PullFrameAtClock(qint64 clockMs);
	bool PullLayersAtClock();
	/** 暂停 seek 后异步再拉一帧（不阻塞 UI） */
	void SchedulePausedFrameRetry();
	/** Seek 后有限次阻塞出帧并 FinishSeek（硬解） */
	void PrimeAndFinishSeekFrames();
	void EnsureAudioSidecar(const QString& path, qint64 seekMs, bool bPlaying);

	struct DxState;
	struct SLayerGpuFrame
	{
		QUuid clipId;
		ID3D11ShaderResourceView* srv = nullptr;
		ID3D11Texture2D* tex = nullptr;
		int w = 0;
		int h = 0;
		qint64 pts = -1;
		SColorPty color;
		STransInfo trans;
	};

	DxState* m_pDx = nullptr;
	CPreviewDecodeSession* m_pSession = nullptr;
	CTimelinePreviewBridge* m_pBridge = nullptr;
	QTimer m_timer;
	QTimer m_primeRetryTimer;
	int m_primeRetryLeft = 0;
	/** 每次暂停 Seek 递增；retry 只接受当前代数，避免 Present 上一 Seek 画面 */
	uint64_t m_seekEpoch = 0;
	uint64_t m_retryEpoch = 0;
	bool m_bPlaying = false;
	QString m_openedPath;
	QString m_audioPath;
	qint64 m_lastSeekMs = -1;
	/** 音频 sidecar 上次对齐的时间线时钟（播放中跟踪用，避免每帧 Seek 清 SDL） */
	qint64 m_lastAudioSeekMs = -1;
	qint64 m_previewClockMs = 0;
	qint64 m_lastPresentedPts = -1;
	float m_fZoom = 1.0f;
	std::vector<uint8_t> m_lastBgra;
	int m_lastFw = 0;
	int m_lastFh = 0;
	SColorPty m_previewColor;
	bool m_bFrameDirty = false;
	bool m_bFxDirty = false;
	bool m_bGpuHasFrame = false;
	bool m_bUseHwPipeline = false;
	bool m_bMultiLayer = false;
	std::vector<SPreviewLayerInfo> m_layerInfos;
	std::vector<SLayerGpuFrame> m_layerFrames;
	QUuid m_audioFocusId;
};
