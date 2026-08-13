#include "AudioChecker.h"

#include <cmath>
#include <iostream>
#include <SDL3/SDL.h>

extern "C"
{
#include <libavutil/avutil.h>
#include <libavutil/samplefmt.h>
}

#pragma comment(lib, "SDL3.lib")
#pragma comment(lib, "avutil.lib")

#define SDL_AUDIO_MIN_BUFFER_SIZE 512				// SDL回调最小Buffer大小
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30			// 每秒SDL回调次数：确定SDL回调buffer大小

CAudioChecker* CAudioChecker::GetInstance()
{
	static CAudioChecker instance;
	return &instance;
}

bool CAudioChecker::CheckAudioProperty(AVChannelLayout channel_layout, int sample_rate, SAudioCheckProperty& pro)
{
	if (channel_layout.nb_channels <= 0 || sample_rate <= 0)
		return false;

	SDL_AudioSpec wanted_spec = {};
	wanted_spec.channels = channel_layout.nb_channels;
	wanted_spec.freq = sample_rate;
	wanted_spec.format = SDL_AUDIO_S16;

	const int requested_samples = (int)FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE,
		(int)ceill((double)wanted_spec.freq / (double)SDL_AUDIO_MAX_CALLBACKS_PER_SEC));

	SDL_AudioSpec actual_spec = wanted_spec;
	int sample_frames = requested_samples;

	SDL_AudioDeviceID audio_dev = 0;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		audio_dev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &wanted_spec);
		if (audio_dev != 0)
		{
			m_mapAudioDevIdCount[audio_dev]++;
			SDL_GetAudioDeviceFormat(audio_dev, &actual_spec, &sample_frames);
			if (sample_frames <= 0)
				sample_frames = requested_samples;
		}
	}

#ifdef _DEBUG
	if (wanted_spec.channels != actual_spec.channels)
	{
	 	std::cout<<("音频设备使用了不同的通道数\n");
	}
	if (wanted_spec.freq != actual_spec.freq)
	{
	 	std::cout << ("音频设备使用了不同的频率\n");
	}
#endif

	pro.m_channelLayout = channel_layout;
	pro.m_sampleRate = actual_spec.freq > 0 ? actual_spec.freq : sample_rate;
	pro.m_channels = channel_layout.nb_channels;
	pro.m_nSample = sample_frames;

	const int use_channels = actual_spec.channels > 0 ? actual_spec.channels : channel_layout.nb_channels;
	const int use_freq = pro.m_sampleRate;
	int m_bytesPerSecond = av_samples_get_buffer_size(nullptr, use_channels, use_freq, AV_SAMPLE_FMT_S16, 1);
	int m_bytesPerSample = av_samples_get_buffer_size(nullptr, use_channels, 1, AV_SAMPLE_FMT_S16, 1);

	pro.m_silenceLen = SDL_AUDIO_MIN_BUFFER_SIZE;
	const int buffer_bytes = sample_frames * (m_bytesPerSample > 0 ? m_bytesPerSample : (use_channels * 2));
	if (m_bytesPerSecond > 0)
		pro.m_waitTime = milliseconds(1000LL * buffer_bytes / m_bytesPerSecond / 2);
	else
		pro.m_waitTime = milliseconds(0);

	DelAudioDevice((int)audio_dev, nullptr);

	if (m_bytesPerSample <= 0 || m_bytesPerSecond <= 0)
	{
		std::cout << "av_samples_get_buffer_size failed\n";
		return false;
	}
	return true;
}

void CAudioChecker::AddAudioDevice(int nAudioDeviceId)
{
	if (nAudioDeviceId == 0)
		return;
	std::lock_guard<std::mutex> lock(m_mutex);
	m_mapAudioDevIdCount[(unsigned int)nAudioDeviceId] += 1;
}

void CAudioChecker::DelAudioDevice(int nAudioDeviceId, SDL_AudioStream* stream)
{
	if (nAudioDeviceId == 0 && !stream)
		return;
	std::lock_guard<std::mutex> lock(m_mutex);

	if (nAudioDeviceId != 0)
	{
		auto item = m_mapAudioDevIdCount.find((unsigned int)nAudioDeviceId);
		if (item != m_mapAudioDevIdCount.end())
		{
			item->second--;
			if (item->second > 0)
				return;
			m_mapAudioDevIdCount.erase(item);
		}
	}

	if (stream)
	{
		SDL_DestroyAudioStream(stream);
	}
	else if (nAudioDeviceId != 0)
	{
		SDL_PauseAudioDevice((SDL_AudioDeviceID)nAudioDeviceId);
		SDL_CloseAudioDevice((SDL_AudioDeviceID)nAudioDeviceId);
	}
}

CAudioChecker::CAudioChecker()
{
}
