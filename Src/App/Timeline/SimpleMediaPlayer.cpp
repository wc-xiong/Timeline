#include "SimpleMediaPlayer.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include <SDL3/SDL.h>
#include <Windows.h>

#ifdef _MSC_VER
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "SDL3.lib")
#endif

static void SDLCALL AudioCallback(void* userdata, SDL_AudioStream* /*stream*/, int additional_amount, int /*total_amount*/)
{
	auto* self = static_cast<CSimpleMediaPlayer*>(userdata);
	if (!self || additional_amount <= 0)
		return;
	std::vector<uint8_t> tmp(static_cast<size_t>(additional_amount), 0);
	self->FillAudio(tmp.data(), additional_amount);
	// PutAudioStreamData is called from FillAudio via queued data — callback mode uses stream write from outside
}

struct CSimpleMediaPlayer::Impl
{
	AVFormatContext* fmt = nullptr;
	AVCodecContext* vctx = nullptr;
	AVCodecContext* actx = nullptr;
	SwsContext* sws = nullptr;
	SwrContext* swr = nullptr;
	AVFrame* frame = nullptr;
	AVFrame* frameBgra = nullptr;
	AVFrame* aframe = nullptr;
	AVPacket* pkt = nullptr;
	int videoStream = -1;
	int audioStream = -1;
	SDL_AudioStream* sdlAudio = nullptr;
	SDL_AudioDeviceID audioDev = 0;
	int outSampleRate = 48000;
	int outChannels = 2;
};

CSimpleMediaPlayer::CSimpleMediaPlayer()
	: m_pImpl(new Impl)
{
	SDL_Init(SDL_INIT_AUDIO);
}

CSimpleMediaPlayer::~CSimpleMediaPlayer()
{
	Close();
	delete m_pImpl;
	m_pImpl = nullptr;
}

bool CSimpleMediaPlayer::OpenAudioDevice()
{
	CloseAudioDevice();

	SDL_AudioSpec spec = {};
	spec.freq = m_pImpl->outSampleRate;
	spec.format = SDL_AUDIO_S16;
	spec.channels = static_cast<Uint8>(m_pImpl->outChannels);

	m_pImpl->sdlAudio = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
	if (!m_pImpl->sdlAudio)
	{
		const char* err = SDL_GetError();
		OutputDebugStringA("[SimpleMediaPlayer] SDL_OpenAudioDeviceStream failed: ");
		OutputDebugStringA(err ? err : "(null)");
		OutputDebugStringA("\n");
		return false;
	}
	m_pImpl->audioDev = SDL_GetAudioStreamDevice(m_pImpl->sdlAudio);
	SDL_ResumeAudioDevice(m_pImpl->audioDev);
	return true;
}

void CSimpleMediaPlayer::CloseAudioDevice()
{
	if (m_pImpl->sdlAudio)
	{
		SDL_DestroyAudioStream(m_pImpl->sdlAudio);
		m_pImpl->sdlAudio = nullptr;
		m_pImpl->audioDev = 0;
	}
}

bool CSimpleMediaPlayer::Open(const QString& path, bool bNeedVideo)
{
	Close();

	const QByteArray utf8 = path.toUtf8();
	if (avformat_open_input(&m_pImpl->fmt, utf8.constData(), nullptr, nullptr) < 0)
		return false;
	if (avformat_find_stream_info(m_pImpl->fmt, nullptr) < 0)
	{
		Close();
		return false;
	}

	m_pImpl->videoStream = bNeedVideo
		? av_find_best_stream(m_pImpl->fmt, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0)
		: -1;
	m_pImpl->audioStream = av_find_best_stream(m_pImpl->fmt, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);

	if (m_pImpl->videoStream >= 0)
	{
		AVStream* st = m_pImpl->fmt->streams[m_pImpl->videoStream];
		const AVCodec* codec = avcodec_find_decoder(st->codecpar->codec_id);
		if (!codec)
		{
			Close();
			return false;
		}
		m_pImpl->vctx = avcodec_alloc_context3(codec);
		avcodec_parameters_to_context(m_pImpl->vctx, st->codecpar);
		if (avcodec_open2(m_pImpl->vctx, codec, nullptr) < 0)
		{
			Close();
			return false;
		}
		m_nWidth = m_pImpl->vctx->width;
		m_nHeight = m_pImpl->vctx->height;

		m_pImpl->frame = av_frame_alloc();
		m_pImpl->frameBgra = av_frame_alloc();
		const int bufSize = av_image_get_buffer_size(AV_PIX_FMT_BGRA, m_nWidth, m_nHeight, 1);
		uint8_t* buf = (uint8_t*)av_malloc(bufSize);
		av_image_fill_arrays(m_pImpl->frameBgra->data, m_pImpl->frameBgra->linesize, buf,
			AV_PIX_FMT_BGRA, m_nWidth, m_nHeight, 1);
		m_pImpl->sws = sws_getContext(
			m_nWidth, m_nHeight, m_pImpl->vctx->pix_fmt,
			m_nWidth, m_nHeight, AV_PIX_FMT_BGRA,
			SWS_BILINEAR, nullptr, nullptr, nullptr);
	}

	if (m_pImpl->audioStream >= 0)
	{
		AVStream* st = m_pImpl->fmt->streams[m_pImpl->audioStream];
		const AVCodec* codec = avcodec_find_decoder(st->codecpar->codec_id);
		if (codec)
		{
			m_pImpl->actx = avcodec_alloc_context3(codec);
			avcodec_parameters_to_context(m_pImpl->actx, st->codecpar);
			if (avcodec_open2(m_pImpl->actx, codec, nullptr) >= 0)
			{
				m_pImpl->aframe = av_frame_alloc();
				AVChannelLayout outCh;
				av_channel_layout_default(&outCh, m_pImpl->outChannels);
				swr_alloc_set_opts2(&m_pImpl->swr,
					&outCh, AV_SAMPLE_FMT_S16, m_pImpl->outSampleRate,
					&m_pImpl->actx->ch_layout, m_pImpl->actx->sample_fmt, m_pImpl->actx->sample_rate,
					0, nullptr);
				if (m_pImpl->swr)
					swr_init(m_pImpl->swr);
				if (!OpenAudioDevice())
				{
					// 仅音频模式：无播放设备则整次 Open 失败，便于 sidecar 感知
					if (!bNeedVideo)
					{
						av_channel_layout_uninit(&outCh);
						Close();
						return false;
					}
					OutputDebugStringA("[SimpleMediaPlayer] audio device unavailable; video continues muted\n");
				}
				av_channel_layout_uninit(&outCh);
			}
		}
	}
	else if (!bNeedVideo)
	{
		Close();
		return false;
	}

	if (m_pImpl->fmt->duration > 0)
		m_durationSec = m_pImpl->fmt->duration / (double)AV_TIME_BASE;

	m_pImpl->pkt = av_packet_alloc();
	m_bHasVideo = (m_pImpl->videoStream >= 0);
	m_bOpen = m_bHasVideo || (m_pImpl->actx != nullptr && m_pImpl->sdlAudio != nullptr);
	if (!bNeedVideo && !m_bOpen)
	{
		Close();
		return false;
	}
	m_path = path;
	m_currentMs = 0;
	return m_bOpen;
}

void CSimpleMediaPlayer::Close()
{
	SetPlaying(false);
	CloseAudioDevice();
	{
		std::lock_guard<std::mutex> lock(m_audioMutex);
		m_audioQueue.clear();
	}

	if (!m_pImpl)
		return;

	if (m_pImpl->sws)
	{
		sws_freeContext(m_pImpl->sws);
		m_pImpl->sws = nullptr;
	}
	if (m_pImpl->swr)
	{
		swr_free(&m_pImpl->swr);
	}
	if (m_pImpl->frameBgra)
	{
		if (m_pImpl->frameBgra->data[0])
			av_free(m_pImpl->frameBgra->data[0]);
		av_frame_free(&m_pImpl->frameBgra);
	}
	if (m_pImpl->frame)
		av_frame_free(&m_pImpl->frame);
	if (m_pImpl->aframe)
		av_frame_free(&m_pImpl->aframe);
	if (m_pImpl->pkt)
		av_packet_free(&m_pImpl->pkt);
	if (m_pImpl->vctx)
		avcodec_free_context(&m_pImpl->vctx);
	if (m_pImpl->actx)
		avcodec_free_context(&m_pImpl->actx);
	if (m_pImpl->fmt)
	{
		avformat_close_input(&m_pImpl->fmt);
		m_pImpl->fmt = nullptr;
	}
	m_pImpl->videoStream = m_pImpl->audioStream = -1;
	m_bOpen = false;
	m_bHasVideo = false;
	m_nWidth = m_nHeight = 0;
	m_durationSec = 0.0;
	m_currentMs = 0;
	m_path.clear();
}

bool CSimpleMediaPlayer::SeekMs(qint64 ms)
{
	if (!m_bOpen || !m_pImpl || !m_pImpl->fmt)
		return false;
	if (ms < 0)
		ms = 0;

	const int streamIndex = m_pImpl->videoStream >= 0 ? m_pImpl->videoStream : m_pImpl->audioStream;
	if (streamIndex < 0)
		return false;

	AVStream* st = m_pImpl->fmt->streams[streamIndex];
	const int64_t ts = av_rescale_q(ms, av_make_q(1, 1000), st->time_base);
	if (av_seek_frame(m_pImpl->fmt, streamIndex, ts, AVSEEK_FLAG_BACKWARD) < 0)
	{
		// 回退到全局时间基
		const int64_t tsGlobal = av_rescale_q(ms, av_make_q(1, 1000), av_make_q(1, AV_TIME_BASE));
		if (av_seek_frame(m_pImpl->fmt, -1, tsGlobal, AVSEEK_FLAG_BACKWARD) < 0)
			return false;
	}

	if (m_pImpl->vctx)
		avcodec_flush_buffers(m_pImpl->vctx);
	if (m_pImpl->actx)
		avcodec_flush_buffers(m_pImpl->actx);

	{
		std::lock_guard<std::mutex> lock(m_audioMutex);
		m_audioQueue.clear();
	}
	if (m_pImpl->sdlAudio)
		SDL_ClearAudioStream(m_pImpl->sdlAudio);

	m_currentMs = ms;
	return true;
}

void CSimpleMediaPlayer::SetPlaying(bool playing)
{
	m_bPlaying = playing;
	if (m_pImpl->audioDev)
	{
		if (playing)
			SDL_ResumeAudioDevice(m_pImpl->audioDev);
		else
			SDL_PauseAudioDevice(m_pImpl->audioDev);
	}
}

void CSimpleMediaPlayer::SetVolume(double volume)
{
	m_volume = volume;
}

void CSimpleMediaPlayer::FillAudio(uint8_t* stream, int len)
{
	std::lock_guard<std::mutex> lock(m_audioMutex);
	for (int i = 0; i < len; ++i)
	{
		if (m_audioQueue.empty() || !m_bPlaying)
		{
			stream[i] = 0;
			continue;
		}
		stream[i] = m_audioQueue.front();
		m_audioQueue.pop_front();
	}
}

void CSimpleMediaPlayer::PumpAudioPackets()
{
	// 有视频时音频在 ReadVideoFrame demux 旁路；仅音频模式用 PumpAudioOnly
}

void CSimpleMediaPlayer::PumpAudioOnly()
{
	if (!m_bOpen || m_bHasVideo || !m_pImpl || !m_pImpl->fmt || !m_pImpl->actx || !m_pImpl->swr || !m_pImpl->pkt)
		return;
	if (!m_pImpl->sdlAudio)
		return;

	// ~0.25s@48k stereo S16：队列过满则本轮少喂，避免无界堆积
	constexpr int kMaxQueuedBytes = 48000 * 2 * 2 / 4;
	if (SDL_GetAudioStreamQueued(m_pImpl->sdlAudio) >= kMaxQueuedBytes)
		return;

	for (int nTry = 0; nTry < 32; ++nTry)
	{
		if (SDL_GetAudioStreamQueued(m_pImpl->sdlAudio) >= kMaxQueuedBytes)
			break;
		if (av_read_frame(m_pImpl->fmt, m_pImpl->pkt) < 0)
			break;
		if (m_pImpl->pkt->stream_index != m_pImpl->audioStream)
		{
			av_packet_unref(m_pImpl->pkt);
			continue;
		}
		if (avcodec_send_packet(m_pImpl->actx, m_pImpl->pkt) >= 0)
		{
			while (avcodec_receive_frame(m_pImpl->actx, m_pImpl->aframe) >= 0)
			{
				uint8_t* outBuf = nullptr;
				const int outSamples = swr_get_out_samples(m_pImpl->swr, m_pImpl->aframe->nb_samples) + 256;
				av_samples_alloc(&outBuf, nullptr, m_pImpl->outChannels, outSamples, AV_SAMPLE_FMT_S16, 0);
				const int converted = swr_convert(m_pImpl->swr, &outBuf, outSamples,
					(const uint8_t**)m_pImpl->aframe->data, m_pImpl->aframe->nb_samples);
				if (converted > 0 && outBuf)
				{
					const int bytes = converted * m_pImpl->outChannels * 2;
					int16_t* samples = reinterpret_cast<int16_t*>(outBuf);
					const int n = bytes / 2;
					for (int i = 0; i < n; ++i)
					{
						int v = static_cast<int>(samples[i] * m_volume);
						if (v > 32767) v = 32767;
						if (v < -32768) v = -32768;
						samples[i] = static_cast<int16_t>(v);
					}
					SDL_PutAudioStreamData(m_pImpl->sdlAudio, outBuf, bytes);
				}
				if (outBuf)
					av_freep(&outBuf);
			}
		}
		av_packet_unref(m_pImpl->pkt);
	}
}

bool CSimpleMediaPlayer::ReadVideoFrame(std::vector<uint8_t>& outBgra, int& outW, int& outH)
{
	if (!m_bOpen)
		return false;

	if (m_pImpl->videoStream < 0)
		return false;

	// 限制单次 demux 循环，避免 EOF/坏流时长时间占住 UI 线程
	for (int nTry = 0; nTry < 64; ++nTry)
	{
		if (av_read_frame(m_pImpl->fmt, m_pImpl->pkt) < 0)
			break;
		if (m_pImpl->pkt->stream_index == m_pImpl->audioStream && m_pImpl->actx && m_pImpl->swr)
		{
			if (avcodec_send_packet(m_pImpl->actx, m_pImpl->pkt) >= 0)
			{
				while (avcodec_receive_frame(m_pImpl->actx, m_pImpl->aframe) >= 0)
				{
					uint8_t* outBuf = nullptr;
					const int outSamples = swr_get_out_samples(m_pImpl->swr, m_pImpl->aframe->nb_samples) + 256;
					av_samples_alloc(&outBuf, nullptr, m_pImpl->outChannels, outSamples, AV_SAMPLE_FMT_S16, 0);
					const int converted = swr_convert(m_pImpl->swr, &outBuf, outSamples,
						(const uint8_t**)m_pImpl->aframe->data, m_pImpl->aframe->nb_samples);
					if (converted > 0 && outBuf)
					{
						const int bytes = converted * m_pImpl->outChannels * 2;
						int16_t* samples = reinterpret_cast<int16_t*>(outBuf);
						const int n = bytes / 2;
						for (int i = 0; i < n; ++i)
						{
							int v = static_cast<int>(samples[i] * m_volume);
							if (v > 32767) v = 32767;
							if (v < -32768) v = -32768;
							samples[i] = static_cast<int16_t>(v);
						}
						if (m_pImpl->sdlAudio)
							SDL_PutAudioStreamData(m_pImpl->sdlAudio, outBuf, bytes);
					}
					if (outBuf)
						av_freep(&outBuf);
				}
			}
			av_packet_unref(m_pImpl->pkt);
			continue;
		}

		if (m_pImpl->pkt->stream_index != m_pImpl->videoStream)
		{
			av_packet_unref(m_pImpl->pkt);
			continue;
		}

		if (avcodec_send_packet(m_pImpl->vctx, m_pImpl->pkt) < 0)
		{
			av_packet_unref(m_pImpl->pkt);
			continue;
		}
		av_packet_unref(m_pImpl->pkt);

		const int ret = avcodec_receive_frame(m_pImpl->vctx, m_pImpl->frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			continue;
		if (ret < 0)
			return false;

		sws_scale(m_pImpl->sws,
			m_pImpl->frame->data, m_pImpl->frame->linesize, 0, m_nHeight,
			m_pImpl->frameBgra->data, m_pImpl->frameBgra->linesize);

		// 用帧 PTS 推进当前时间，供外部对齐（勿依赖墙钟 Seek）
		const int64_t pts = m_pImpl->frame->best_effort_timestamp;
		if (pts != AV_NOPTS_VALUE && m_pImpl->fmt->streams[m_pImpl->videoStream])
		{
			const AVRational tb = m_pImpl->fmt->streams[m_pImpl->videoStream]->time_base;
			m_currentMs = av_rescale_q(pts, tb, AVRational{ 1, 1000 });
		}

		outW = m_nWidth;
		outH = m_nHeight;
		const int size = m_nWidth * m_nHeight * 4;
		outBgra.resize(size);
		for (int y = 0; y < m_nHeight; ++y)
		{
			memcpy(outBgra.data() + y * m_nWidth * 4,
				m_pImpl->frameBgra->data[0] + y * m_pImpl->frameBgra->linesize[0],
				m_nWidth * 4);
		}
		return true;
	}

	// 本 tick 无新帧（EOF/暂无包）：保留上一帧，由时间线切换片段
	return false;
}
