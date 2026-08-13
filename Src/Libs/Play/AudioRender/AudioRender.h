#pragma once
#pragma warning(disable : 4819)

#include <map>
#include <string>
#include <vector>
#include <QObject>

#include <SDL3/SDL.h>
#pragma comment(lib, "SDL3.lib")

#include "Common/AudioCommon.h"

class CAudioCtrl;

class CAudioRender : public QObject
{
	Q_OBJECT
public:
	CAudioRender(const SAudioPlayerProp& audioPty, CAudioCtrl* pAudioCtrl);
	~CAudioRender();

public:
	// 初始化
	SDL_AudioDeviceID Init(const std::string& speakerDeviceName);
	// 启动
	void Start();
	// 关闭
	void Close();

	SAudioPlayerProp GetPlayPty() { return m_playerPty; }

	bool IsStopped();

	// 是否播放完 (针对延时输出)
	bool IsFinished();

signals:
	void signalAudioPlayerFinish();

protected:
	friend void SDLCALL sdl_audio_stream_callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);
	void OnSdlCallBack(Uint8 *stream, int len);
	void DoClose();

private:
	CAudioCtrl* m_pAudioCtrl = nullptr;

	SAudioPlayerProp m_playerPty;		// 播放属性
	SDL_AudioDeviceID m_audioDeviceID = 0;
	SDL_AudioStream* m_audioStream = nullptr;

	std::atomic_bool m_closed = false;
	std::atomic_bool m_bSdlCallbacking = false;
	unsigned int m_delay = 0;
	std::vector<unsigned char> m_cache;

	bool m_fallbackOnLostDevice = false;
	bool m_bFinish = false;
};
