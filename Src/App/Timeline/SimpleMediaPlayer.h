#pragma once

#include <QString>
#include <cstdint>
#include <vector>
#include <mutex>
#include <deque>

// 精简 FFmpeg 音视频：视频供 DX11 预览；音频经 SDL3 播放
class CSimpleMediaPlayer
{
public:
	CSimpleMediaPlayer();
	~CSimpleMediaPlayer();

	/** bNeedVideo=false：仅打开音频（HW 视频旁路 SDL） */
	bool Open(const QString& path, bool bNeedVideo = true);
	void Close();
	bool IsOpen() const { return m_bOpen; }
	bool HasVideo() const { return m_bHasVideo; }

	bool ReadVideoFrame(std::vector<uint8_t>& outBgra, int& outW, int& outH);
	/** 无视频时 demux 喂音频队列 */
	void PumpAudioOnly();
	// 按毫秒定位（时间线驱动预览）
	bool SeekMs(qint64 ms);
	qint64 CurrentMs() const { return m_currentMs; }

	void SetPlaying(bool playing);
	void SetVolume(double volume); // 0~2

	int Width() const { return m_nWidth; }
	int Height() const { return m_nHeight; }
	double DurationSec() const { return m_durationSec; }
	QString Path() const { return m_path; }

	// SDL 回调拉取 PCM
	void FillAudio(uint8_t* stream, int len);

private:
	bool OpenAudioDevice();
	void CloseAudioDevice();
	void PumpAudioPackets();

	struct Impl;
	Impl* m_pImpl = nullptr;
	bool m_bOpen = false;
	bool m_bHasVideo = false;
	bool m_bPlaying = false;
	int m_nWidth = 0;
	int m_nHeight = 0;
	double m_durationSec = 0.0;
	double m_volume = 1.0;
	qint64 m_currentMs = 0;
	QString m_path;
	std::mutex m_audioMutex;
	std::deque<uint8_t> m_audioQueue;
};
