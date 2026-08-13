#pragma once

#include <chrono>
#include <mutex>
using namespace std::chrono;

extern "C"
{
#include <libavcodec/avcodec.h>
}

struct SDL_AudioStream;

#include "Common/AudioCommon.h"
#include "PlayCoreGlobal.h"

struct SAudioCheckProperty
{
	AVChannelLayout		m_channelLayout;		// 声道布局
	int			m_sampleRate = 0;			// 采样率
	int			m_channels = 0;				// 声道数
	int			m_nSample = 0;				// audio buffer size in samples

	int				m_silenceLen = 0;		// 静音长度
	milliseconds	m_waitTime = milliseconds(0);			// 超时时间
};

#include <map>
class PLAYCORE_EXPORT CAudioChecker
{
public:
	static CAudioChecker* GetInstance();
	bool CheckAudioProperty(AVChannelLayout channel_layout, int sample_rate, SAudioCheckProperty& pro);

	void AddAudioDevice(int nAudioDeviceId);

	// stream 非空时通过 DestroyAudioStream 关闭（OpenAudioDeviceStream 路径）；否则 CloseAudioDevice
	void DelAudioDevice(int nAudioDeviceId, SDL_AudioStream* stream = nullptr);

private:
	CAudioChecker();
	CAudioChecker(const CAudioChecker&) = delete;
	CAudioChecker& operator=(const CAudioChecker&) = delete;

	std::mutex m_mutex;
	std::map<unsigned int, int> m_mapAudioDevIdCount;
};
