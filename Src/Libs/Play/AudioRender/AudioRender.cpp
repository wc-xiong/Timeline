#include "AudioRender.h"
#include "AudioCtrl.h"
#include <mutex>
#include <unordered_set>
#include <algorithm>
#include <cstring>
#include <memory>

#include "GlobalModel/GlobalModel.h"
#include "GlobalModel/GlobalModelExport.h"
#include "GlobalModel/PrjSettingData.h"
#include <iostream>
#include "../PlayCore/AudioChecker.h"
#include <QDebug>

extern "C"
{
#include <libavutil/common.h>
#include <libavutil/avutil.h>
#include <libavutil/samplefmt.h>
}
#pragma comment(lib, "avutil.lib")

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

// SDL3 音频 API 已声明任意线程可调用；此处仅保留对象存活集，防止回调 UAF
class SDLWrapper
{
public:
	void addAudioPlayer(void* p)
	{
		std::lock_guard<std::mutex> locker(m_setMutex);
		m_setAudioPlayer.insert(p);
	}

	void removeAudioPlayer(void* p)
	{
		std::lock_guard<std::mutex> locker(m_setMutex);
		m_setAudioPlayer.erase(p);
	}

	bool IsAlive(void* p)
	{
		std::lock_guard<std::mutex> locker(m_setMutex);
		return m_setAudioPlayer.count(p) != 0;
	}

private:
	std::mutex m_setMutex;
	std::unordered_set<void*> m_setAudioPlayer;
};
static SDLWrapper g_sdlwrapper;

#define AUDIO_DELAY g_pProjectSetting->LoadAudioOutputDelay()

static SDL_AudioDeviceID FindPlaybackDeviceByName(const std::string& speakerDeviceName)
{
	if (speakerDeviceName.empty())
		return SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;

	int count = 0;
	SDL_AudioDeviceID* devices = SDL_GetAudioPlaybackDevices(&count);
	if (!devices || count <= 0)
	{
		if (devices)
			SDL_free(devices);
		return 0;
	}

	SDL_AudioDeviceID found = 0;
	for (int i = 0; i < count; ++i)
	{
		const char* name = SDL_GetAudioDeviceName(devices[i]);
		if (name && speakerDeviceName == name)
		{
			found = devices[i];
			break;
		}
	}
	SDL_free(devices);
	return found;
}

static void LogPlaybackDevices()
{
	int count = 0;
	SDL_AudioDeviceID* devices = SDL_GetAudioPlaybackDevices(&count);
	if (!devices)
		return;
	for (int i = 0; i < count; ++i)
	{
		const char* name = SDL_GetAudioDeviceName(devices[i]);
		qDebug() << "CAudioRender ---- currently audio device " << (name ? name : "(null)");
	}
	SDL_free(devices);
}

void SDLCALL sdl_audio_stream_callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
	Q_UNUSED(total_amount);
	if (additional_amount <= 0)
		return;

	if (!g_sdlwrapper.IsAlive(userdata))
		return;

	CAudioRender* pThis = (CAudioRender*)userdata;
	std::unique_ptr<Uint8[]> buffer(new Uint8[additional_amount]);
	SDL_memset(buffer.get(), 0, additional_amount);
	pThis->OnSdlCallBack(buffer.get(), additional_amount);
	SDL_PutAudioStreamData(stream, buffer.get(), additional_amount);
}

CAudioRender::CAudioRender(const SAudioPlayerProp& audioPty, CAudioCtrl* pAudioCtrl) :
	m_playerPty(audioPty),
	m_pAudioCtrl(pAudioCtrl),
	m_audioDeviceID(0),
	m_audioStream(nullptr)
{
	g_sdlwrapper.addAudioPlayer(this);
}

CAudioRender::~CAudioRender()
{
	m_closed.store(true);
	g_sdlwrapper.removeAudioPlayer(this);
	DoClose();
}

SDL_AudioDeviceID CAudioRender::Init(const std::string& speakerDeviceName)
{
	qDebug() << "--------AudioRender InitBegin :" << m_audioDeviceID << speakerDeviceName.c_str();
	DoClose();

	SDL_AudioSpec wanted_spec = {};
	wanted_spec.channels = m_playerPty.channels;
	wanted_spec.freq = (int)m_playerPty.sampleRate;
	switch (m_playerPty.bitsPerSample)
	{
	case 8:
		wanted_spec.format = SDL_AUDIO_S8;
		break;
	case 16:
	default:
		wanted_spec.format = SDL_AUDIO_S16;
		break;
	}

	SDL_AudioDeviceID physicalDev = FindPlaybackDeviceByName(speakerDeviceName);
	if (!speakerDeviceName.empty() && physicalDev == 0)
	{
		qDebug() << "CAudioRender ---- Failed to find device " << speakerDeviceName.c_str() << SDL_GetError();
		LogPlaybackDevices();
		Init("");
		m_fallbackOnLostDevice = true;
		return m_audioDeviceID;
	}
	if (physicalDev == 0)
		physicalDev = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;

	static const int next_sample_rates[] = { 0, 44100, 48000, 96000, 192000 };
	static const int next_nb_channels[] = { 0, 0, 1, 6, 2, 6, 4, 6 };
	int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;
	while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq)
		next_sample_rate_idx--;

	SDL_AudioStream* stream = nullptr;
	SDL_AudioSpec try_spec = wanted_spec;
	for (;;)
	{
		stream = SDL_OpenAudioDeviceStream(physicalDev, &try_spec, sdl_audio_stream_callback, this);
		if (stream)
			break;

		try_spec.channels = next_nb_channels[FFMIN(7, try_spec.channels)];
		if (!try_spec.channels)
		{
			try_spec.freq = next_sample_rates[next_sample_rate_idx--];
			try_spec.channels = wanted_spec.channels;
			if (!try_spec.freq)
				break;
		}
	}

	m_fallbackOnLostDevice = false;
	if (!stream)
	{
		qDebug() << "CAudioRender ---- Failed to open device " << speakerDeviceName.c_str() << SDL_GetError();
		LogPlaybackDevices();
		if (!speakerDeviceName.empty())
		{
			Init("");
			m_fallbackOnLostDevice = true;
			return m_audioDeviceID;
		}
		m_audioDeviceID = 0;
		m_audioStream = nullptr;
		return 0;
	}

	m_audioStream = stream;
	m_audioDeviceID = SDL_GetAudioStreamDevice(stream);

	SDL_AudioSpec src_spec = {};
	SDL_AudioSpec dst_spec = {};
	if (SDL_GetAudioStreamFormat(stream, &src_spec, &dst_spec))
	{
		if (m_playerPty.channels != src_spec.channels)
			printf("AudioRender Init diff! %d, %d.\n\n\n\n", src_spec.channels, m_playerPty.channels);
		if (src_spec.channels > 0)
			m_playerPty.channels = src_spec.channels;
	}

	CAudioChecker::GetInstance()->AddAudioDevice((int)m_audioDeviceID);
	return m_audioDeviceID;
}

void CAudioRender::Start()
{
	m_closed.store(false);
	if (!m_audioStream)
		return;

	if (SDL_AudioStreamDevicePaused(m_audioStream))
		SDL_ResumeAudioStreamDevice(m_audioStream);
}

void CAudioRender::Close()
{
	m_closed.store(true);
}

bool CAudioRender::IsStopped()
{
	if (!m_audioStream || m_audioDeviceID == 0)
		return true;
	return SDL_AudioStreamDevicePaused(m_audioStream);
}

bool CAudioRender::IsFinished()
{
	return m_closed.load() && !m_bSdlCallbacking.load();
}

void CAudioRender::OnSdlCallBack(Uint8 * pAudioData, int len)
{
	m_bSdlCallbacking.store(true);

	if (!m_pAudioCtrl)
	{
		m_bSdlCallbacking.store(false);
		return;
	}

	if (!m_closed.load())
	{
		SDL_memset(pAudioData, 0, len);
		auto pDst = pAudioData;
		auto pBuffer = std::make_unique<unsigned char[]>(len);

		m_pAudioCtrl->SdlCallback(m_playerPty, pBuffer.get(), len);

		int delay = m_closed ? 0 : AUDIO_DELAY;
		delay = std::min(2000, std::max(0, delay));

		if (delay != (int)m_delay)
		{
			m_delay = delay;
			auto size = m_playerPty.bitsPerSample / 8 * m_playerPty.channels * m_playerPty.sampleRate * delay / 1000;
			m_cache.clear();
			m_cache.resize(size, 0);
		}

		const auto POP = std::min((int)m_cache.size(), len);
		const auto LEFT = len - POP;

		memcpy(pDst, m_cache.data(), POP);
		m_cache.erase(m_cache.begin(), m_cache.begin() + POP);
		pDst += POP;

		auto pSrc = pBuffer.get();
		memcpy(pDst, pSrc, LEFT);
		pSrc += LEFT;
		pDst += LEFT;

		m_cache.insert(m_cache.end(), pSrc, pSrc + POP);
	}
	else
		SDL_memset(pAudioData, 0, len);

	if (m_fallbackOnLostDevice)
	{
		SDL_memset(pAudioData, 0, len);
	}

	m_bSdlCallbacking.store(false);
}

void CAudioRender::DoClose()
{
	if (m_audioStream)
	{
		CAudioChecker::GetInstance()->DelAudioDevice((int)m_audioDeviceID, m_audioStream);
		m_audioStream = nullptr;
		m_audioDeviceID = 0;
	}
	else if (m_audioDeviceID != 0)
	{
		CAudioChecker::GetInstance()->DelAudioDevice((int)m_audioDeviceID, nullptr);
		m_audioDeviceID = 0;
	}
}
