#pragma once

#include <QString>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

class CSimpleMediaPlayer;

/**
 * @brief 预览软解帧队列的最小子集：
 * worker 解码入环，UI 按 PTS（clockMs）取最近帧，不在 UI 线程 demux。
 */
class CPreviewDecodeSession
{
public:
	CPreviewDecodeSession();
	~CPreviewDecodeSession();

	bool Open(const QString& path);
	/** HW 视频旁路：仅 SDL 音频 */
	bool OpenAudioOnly(const QString& path);
	void Close();
	bool IsOpen() const;
	bool IsAudioOnly() const;
	bool SeekMs(qint64 ms);
	/** Seek 后若仍 seeking（软解路径 SeekMs 内已 prime，通常已 Finish） */
	void FinishSeek();
	bool IsSeeking() const;
	void SetPlaying(bool playing);
	void SetVolume(double volume);

	QString Path() const;
	int Width() const;
	int Height() const;

	/**
	 * 取 pts<=clockMs 的最新帧。
	 * lastPtsMs：UI 已持有的 PTS；相同时不拷贝并返回 false（仍会丢弃过期帧）。
	 */
	bool TryGetFrame(qint64 clockMs, qint64 lastPtsMs,
		std::vector<uint8_t>& outBgra, int& outW, int& outH, qint64& outPtsMs);

private:
	void EnsureWorker();
	void StopWorker();
	void WorkerMain();
	void ClearRing_NoLock();

	struct FrameSlot
	{
		qint64 ptsMs = -1;
		int w = 0;
		int h = 0;
		std::vector<uint8_t> bgra;
	};

	static constexpr size_t kRingMax = 4;

	mutable std::mutex m_mtx;
	std::condition_variable m_cv;
	std::thread m_thread;
	std::atomic<bool> m_bStop{ false };

	enum class ECmd : int
	{
		None = 0,
		Open,
		OpenAudioOnly,
		Close,
		Seek,
	};
	ECmd m_cmd = ECmd::None;
	QString m_cmdPath;
	qint64 m_cmdSeekMs = 0;
	bool m_bCmdDone = true;
	bool m_bCmdOk = false;

	bool m_bAudioOnly = false;
	bool m_bPlaying = false;
	double m_volume = 1.0;
	bool m_bVolumeDirty = false;
	bool m_bNeedPrimeFrame = false; // Seek/Open 后解到目标附近再入环
	/** Seek 已下发、目标帧尚未入环 */
	bool m_bSeeking = false;
	qint64 m_primeSeekTargetMs = -1;

	std::vector<FrameSlot> m_ring;
	size_t m_ringCount = 0;

	CSimpleMediaPlayer* m_pPlayer = nullptr; // 仅 worker 访问
	QString m_path;
	bool m_bOpen = false;
	int m_width = 0;
	int m_height = 0;
};
