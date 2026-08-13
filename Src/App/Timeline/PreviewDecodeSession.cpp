#include "PreviewDecodeSession.h"
#include "SimpleMediaPlayer.h"

#include <algorithm>
#include <chrono>
#include <limits>

CPreviewDecodeSession::CPreviewDecodeSession()
{
	m_ring.resize(kRingMax);
	EnsureWorker();
}

CPreviewDecodeSession::~CPreviewDecodeSession()
{
	StopWorker();
}

void CPreviewDecodeSession::EnsureWorker()
{
	if (m_thread.joinable())
		return;
	m_bStop.store(false);
	m_thread = std::thread(&CPreviewDecodeSession::WorkerMain, this);
}

void CPreviewDecodeSession::StopWorker()
{
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		m_bStop.store(true);
		m_cmd = ECmd::Close;
		m_bCmdDone = false;
	}
	m_cv.notify_all();
	if (m_thread.joinable())
		m_thread.join();

	// m_pPlayer 由 WorkerMain 退出时释放
	m_pPlayer = nullptr;
	m_bOpen = false;
	m_path.clear();
	ClearRing_NoLock();
}

void CPreviewDecodeSession::ClearRing_NoLock()
{
	for (auto& s : m_ring)
	{
		s.ptsMs = -1;
		s.w = s.h = 0;
		s.bgra.clear();
	}
	m_ringCount = 0;
}

bool CPreviewDecodeSession::Open(const QString& path)
{
	EnsureWorker();
	std::unique_lock<std::mutex> lock(m_mtx);
	m_cmd = ECmd::Open;
	m_cmdPath = path;
	m_bCmdDone = false;
	m_bCmdOk = false;
	m_cv.notify_all();
	m_cv.wait(lock, [this] { return m_bCmdDone || m_bStop.load(); });
	return m_bCmdOk;
}

bool CPreviewDecodeSession::OpenAudioOnly(const QString& path)
{
	EnsureWorker();
	std::unique_lock<std::mutex> lock(m_mtx);
	m_cmd = ECmd::OpenAudioOnly;
	m_cmdPath = path;
	m_bCmdDone = false;
	m_bCmdOk = false;
	m_cv.notify_all();
	m_cv.wait(lock, [this] { return m_bCmdDone || m_bStop.load(); });
	return m_bCmdOk;
}

bool CPreviewDecodeSession::IsAudioOnly() const
{
	std::lock_guard<std::mutex> lock(m_mtx);
	return m_bOpen && m_bAudioOnly;
}

void CPreviewDecodeSession::Close()
{
	if (!m_thread.joinable())
		return;
	std::unique_lock<std::mutex> lock(m_mtx);
	m_cmd = ECmd::Close;
	m_bCmdDone = false;
	m_bCmdOk = false;
	m_cv.notify_all();
	m_cv.wait(lock, [this] { return m_bCmdDone || m_bStop.load(); });
}

bool CPreviewDecodeSession::IsOpen() const
{
	std::lock_guard<std::mutex> lock(m_mtx);
	return m_bOpen;
}

bool CPreviewDecodeSession::SeekMs(qint64 ms)
{
	EnsureWorker();
	std::unique_lock<std::mutex> lock(m_mtx);
	if (!m_bOpen)
		return false;
	m_bSeeking = true;
	m_cmd = ECmd::Seek;
	m_cmdSeekMs = ms < 0 ? 0 : ms;
	m_bCmdDone = false;
	m_bCmdOk = false;
	m_cv.notify_all();
	m_cv.wait(lock, [this] { return m_bCmdDone || m_bStop.load(); });
	return m_bCmdOk;
}

void CPreviewDecodeSession::FinishSeek()
{
	std::lock_guard<std::mutex> lock(m_mtx);
	m_bSeeking = false;
}

bool CPreviewDecodeSession::IsSeeking() const
{
	std::lock_guard<std::mutex> lock(m_mtx);
	return m_bSeeking;
}

void CPreviewDecodeSession::SetPlaying(bool playing)
{
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		m_bPlaying = playing;
	}
	m_cv.notify_all();
}

void CPreviewDecodeSession::SetVolume(double volume)
{
	{
		std::lock_guard<std::mutex> lock(m_mtx);
		m_volume = volume;
		m_bVolumeDirty = true;
	}
	m_cv.notify_all();
}

QString CPreviewDecodeSession::Path() const
{
	std::lock_guard<std::mutex> lock(m_mtx);
	return m_path;
}

int CPreviewDecodeSession::Width() const
{
	std::lock_guard<std::mutex> lock(m_mtx);
	return m_width;
}

int CPreviewDecodeSession::Height() const
{
	std::lock_guard<std::mutex> lock(m_mtx);
	return m_height;
}

bool CPreviewDecodeSession::TryGetFrame(qint64 clockMs, qint64 lastPtsMs,
	std::vector<uint8_t>& outBgra, int& outW, int& outH, qint64& outPtsMs)
{
	std::lock_guard<std::mutex> lock(m_mtx);
	if (m_ringCount == 0)
		return false;

	int best = -1;
	qint64 bestPts = -1;
	for (size_t i = 0; i < m_ringCount; ++i)
	{
		const auto& s = m_ring[i];
		if (s.ptsMs < 0 || s.bgra.empty())
			continue;
		if (s.ptsMs <= clockMs && s.ptsMs >= bestPts)
		{
			bestPts = s.ptsMs;
			best = static_cast<int>(i);
		}
	}
	// 尚无 <=clock 的帧：已有画面则等下一帧；Seek 后 lastPtsMs<0 取最接近 clock 的帧
	if (best < 0)
	{
		if (lastPtsMs >= 0)
		{
			m_cv.notify_all();
			return false;
		}
		best = -1;
		bestPts = -1;
		qint64 bestDist = std::numeric_limits<qint64>::max();
		for (size_t i = 0; i < m_ringCount; ++i)
		{
			if (m_ring[i].ptsMs < 0 || m_ring[i].bgra.empty())
				continue;
			const qint64 dist = std::llabs(m_ring[i].ptsMs - clockMs);
			if (dist < bestDist)
			{
				bestDist = dist;
				bestPts = m_ring[i].ptsMs;
				best = static_cast<int>(i);
			}
		}
		if (best < 0)
		{
			m_cv.notify_all();
			return false;
		}
	}

	outPtsMs = bestPts;

	// 丢弃严格过期帧（pts < best），减轻环压力
	size_t write = 0;
	for (size_t i = 0; i < m_ringCount; ++i)
	{
		if (m_ring[i].ptsMs >= bestPts)
		{
			if (write != i)
				m_ring[write] = std::move(m_ring[i]);
			++write;
		}
		else
		{
			m_ring[i].bgra.clear();
			m_ring[i].ptsMs = -1;
		}
	}
	m_ringCount = write;

	// 同 PTS：不拷贝，立刻返回（播放中绝大多数 tick）
	if (bestPts == lastPtsMs)
	{
		m_cv.notify_all();
		return false;
	}

	// 重新定位 best（prune 后一般在 0）
	best = 0;
	for (size_t i = 0; i < m_ringCount; ++i)
	{
		if (m_ring[i].ptsMs == bestPts)
		{
			best = static_cast<int>(i);
			break;
		}
	}
	if (m_ringCount == 0 || m_ring[static_cast<size_t>(best)].bgra.empty())
	{
		m_cv.notify_all();
		return false;
	}

	FrameSlot& slot = m_ring[static_cast<size_t>(best)];
	outBgra = std::move(slot.bgra);
	outW = slot.w;
	outH = slot.h;
	slot.ptsMs = -1;
	slot.w = slot.h = 0;

	// 移出已消费帧，保留更新的帧
	write = 0;
	for (size_t i = 0; i < m_ringCount; ++i)
	{
		if (m_ring[i].ptsMs >= 0 && !m_ring[i].bgra.empty())
		{
			if (write != i)
				m_ring[write] = std::move(m_ring[i]);
			++write;
		}
	}
	m_ringCount = write;
	m_cv.notify_all();
	return !outBgra.empty() && outW > 0 && outH > 0;
}

void CPreviewDecodeSession::WorkerMain()
{
	m_pPlayer = new CSimpleMediaPlayer();

	while (!m_bStop.load())
	{
		ECmd cmd = ECmd::None;
		QString openPath;
		qint64 seekMs = 0;
		bool bPlaying = false;
		bool bNeedPrime = false;
		bool bVolDirty = false;
		double vol = 1.0;
		size_t ringCount = 0;
		bool bAudioOnly = false;

		{
			std::unique_lock<std::mutex> lock(m_mtx);
			m_cv.wait_for(lock, std::chrono::milliseconds(8), [this] {
				return m_bStop.load()
					|| m_cmd != ECmd::None
					|| m_bVolumeDirty
					|| (m_bOpen && m_bAudioOnly && m_bPlaying)
					|| (m_bOpen && !m_bAudioOnly && (m_bPlaying || m_bNeedPrimeFrame) && m_ringCount < kRingMax);
			});
			if (m_bStop.load())
				break;

			cmd = m_cmd;
			openPath = m_cmdPath;
			seekMs = m_cmdSeekMs;
			bPlaying = m_bPlaying;
			bNeedPrime = m_bNeedPrimeFrame;
			bVolDirty = m_bVolumeDirty;
			vol = m_volume;
			ringCount = m_ringCount;
			bAudioOnly = m_bAudioOnly;

			if (cmd != ECmd::None)
				m_cmd = ECmd::None;
			if (bVolDirty)
				m_bVolumeDirty = false;
		}

		auto pushDecodedFrame = [this](std::vector<uint8_t>&& bgra, int fw, int fh, qint64 ptsMs) {
			std::lock_guard<std::mutex> lock(m_mtx);
			if (m_ringCount < kRingMax)
			{
				auto& slot = m_ring[m_ringCount++];
				slot.ptsMs = ptsMs;
				slot.w = fw;
				slot.h = fh;
				slot.bgra = std::move(bgra);
			}
			m_bNeedPrimeFrame = false;
			m_bSeeking = false; // 首帧入环即 FinishSeek
			m_width = fw;
			m_height = fh;
		};

		// Open/Seek：先解出首帧再回 CmdDone，避免 UI 立刻 TryGetFrame 空环
		auto primeOneFrame = [&]() -> bool {
			std::vector<uint8_t> bgra;
			int fw = 0, fh = 0;
			for (int i = 0; i < 96; ++i)
			{
				if (!m_pPlayer->ReadVideoFrame(bgra, fw, fh) || fw <= 0 || fh <= 0)
					continue;
				pushDecodedFrame(std::move(bgra), fw, fh, m_pPlayer->CurrentMs());
				return true;
			}
			return false;
		};

		if (cmd == ECmd::Open || cmd == ECmd::OpenAudioOnly)
		{
			const bool bAudio = (cmd == ECmd::OpenAudioOnly);
			bool ok = false;
			m_pPlayer->Close();
			ok = m_pPlayer->Open(openPath, !bAudio);
			if (ok)
			{
				// mp3 等无视频流：即使走 Open 也要进 PumpAudioOnly，否则播放无声
				const bool bOnlyAudio = bAudio || !m_pPlayer->HasVideo();
				m_pPlayer->SeekMs(0);
				{
					std::lock_guard<std::mutex> lock(m_mtx);
					ClearRing_NoLock();
					m_bOpen = true;
					m_bAudioOnly = bOnlyAudio;
					m_path = openPath;
					m_width = m_pPlayer->Width();
					m_height = m_pPlayer->Height();
				}
				if (!bOnlyAudio)
					primeOneFrame();
			}
			{
				std::lock_guard<std::mutex> lock(m_mtx);
				if (!ok)
				{
					ClearRing_NoLock();
					m_bOpen = false;
					m_bAudioOnly = false;
					m_path.clear();
					m_width = m_height = 0;
				}
				m_bNeedPrimeFrame = false;
				m_bCmdOk = ok;
				m_bCmdDone = true;
			}
			m_cv.notify_all();
			continue;
		}

		if (cmd == ECmd::Close)
		{
			m_pPlayer->Close();
			{
				std::lock_guard<std::mutex> lock(m_mtx);
				ClearRing_NoLock();
				m_bOpen = false;
				m_bAudioOnly = false;
				m_path.clear();
				m_width = m_height = 0;
				m_bNeedPrimeFrame = false;
				m_bPlaying = false;
				m_bCmdOk = true;
				m_bCmdDone = true;
			}
			m_cv.notify_all();
			continue;
		}

		if (cmd == ECmd::Seek)
		{
			// Seek 立即完成：不在此阻塞；worker 异步解到 seek 目标附近再入环
			bool ok = m_pPlayer->IsOpen() && m_pPlayer->SeekMs(seekMs);
			{
				std::lock_guard<std::mutex> lock(m_mtx);
				if (ok)
				{
					ClearRing_NoLock();
					m_bSeeking = !bAudioOnly;
					m_bNeedPrimeFrame = !bAudioOnly;
					m_primeSeekTargetMs = seekMs;
				}
				else
				{
					m_bSeeking = false;
					m_bNeedPrimeFrame = false;
					m_primeSeekTargetMs = -1;
				}
				m_bCmdOk = ok;
				m_bCmdDone = true;
			}
			m_cv.notify_all();
			continue;
		}

		if (bVolDirty && m_pPlayer->IsOpen())
			m_pPlayer->SetVolume(vol);

		m_pPlayer->SetPlaying(bPlaying);

		if (bAudioOnly)
		{
			if (m_pPlayer->IsOpen() && bPlaying)
				m_pPlayer->PumpAudioOnly();
			continue;
		}

		const bool bDecode = m_pPlayer->IsOpen() && (bPlaying || bNeedPrime) && ringCount < kRingMax;
		if (!bDecode)
			continue;

		// Seek 后：丢弃关键帧，解到目标时刻附近再入环（否则暂停画面停在错误关键帧）
		if (bNeedPrime)
		{
			constexpr qint64 kSeekSlackMs = 80;
			qint64 targetMs = 0;
			{
				std::lock_guard<std::mutex> lock(m_mtx);
				targetMs = m_primeSeekTargetMs >= 0 ? m_primeSeekTargetMs : 0;
			}
			bool bHit = false;
			bool bAborted = false;
			std::vector<uint8_t> bestBgra;
			int bestW = 0, bestH = 0;
			qint64 bestPts = -1;
			for (int i = 0; i < 192; ++i)
			{
				// 拖拽中新 Seek 已改目标：丢弃本轮，避免把上一 Seek 的帧推入环（慢一拍）
				{
					std::lock_guard<std::mutex> lock(m_mtx);
					if (!m_bNeedPrimeFrame || m_primeSeekTargetMs != targetMs)
					{
						bAborted = true;
						break;
					}
					targetMs = m_primeSeekTargetMs;
				}
				std::vector<uint8_t> bgra;
				int fw = 0, fh = 0;
				if (!m_pPlayer->ReadVideoFrame(bgra, fw, fh) || fw <= 0 || fh <= 0)
					continue;
				const qint64 pts = m_pPlayer->CurrentMs();
				bestBgra = std::move(bgra);
				bestW = fw;
				bestH = fh;
				bestPts = pts;
				if (pts + kSeekSlackMs >= targetMs)
				{
					bHit = true;
					break;
				}
			}
			if (bAborted)
				continue;
			// 推入前再次确认目标未变
			{
				std::lock_guard<std::mutex> lock(m_mtx);
				if (!m_bNeedPrimeFrame || (m_primeSeekTargetMs >= 0 && m_primeSeekTargetMs != targetMs))
					continue;
			}
			if (bestW > 0 && bestH > 0 && !bestBgra.empty())
			{
				pushDecodedFrame(std::move(bestBgra), bestW, bestH, bestPts >= 0 ? bestPts : targetMs);
				{
					std::lock_guard<std::mutex> lock(m_mtx);
					if (m_primeSeekTargetMs == targetMs)
						m_primeSeekTargetMs = -1;
				}
				m_cv.notify_all();
			}
			else
			{
				std::lock_guard<std::mutex> lock(m_mtx);
				if (m_primeSeekTargetMs == targetMs)
				{
					m_bNeedPrimeFrame = false;
					m_bSeeking = false;
					m_primeSeekTargetMs = -1;
				}
			}
			continue;
		}

		std::vector<uint8_t> bgra;
		int fw = 0, fh = 0;
		if (!m_pPlayer->ReadVideoFrame(bgra, fw, fh) || fw <= 0 || fh <= 0)
			continue;

		pushDecodedFrame(std::move(bgra), fw, fh, m_pPlayer->CurrentMs());
		m_cv.notify_all();
	}

	if (m_pPlayer)
	{
		m_pPlayer->Close();
		delete m_pPlayer;
		m_pPlayer = nullptr;
	}
}
