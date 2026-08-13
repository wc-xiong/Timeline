#pragma once

#include "SoftShareSession.h"

/*
* 上层音频播放器注册资格策略（组合进 CVAPlayer，避免 Soft/Hard 继承拆分）
*
* - Independent：沿用独立路径亲和预填 bShouldDecodeAudio（min-Adapter 等）；Shared* 忽略该字段
* - SharedCreator：先到创建者可注册
* - SharedBind：后到挂接者不可注册
*
* 素材是否含音频由 CVAPlayer::IsSupportAudio 在委托前另行校验。
*/

struct IAudioRegisterPolicy
{
public:
	virtual ~IAudioRegisterPolicy() = default;
	virtual bool CanRegisterAudio() const = 0;
};

class SessionAudioRegisterPolicy : public IAudioRegisterPolicy
{
public:
	// bShouldDecodeAudio：仅 Independent 用于注册资格；SharedCreator/SharedBind 忽略
	SessionAudioRegisterPolicy(EDecoderSession session, bool bShouldDecodeAudio)
		: m_session(session)
		, m_bShouldDecodeAudio(bShouldDecodeAudio)
	{
	}

	bool CanRegisterAudio() const override
	{
		switch (m_session)
		{
		case EDecoderSession::SharedCreator:
			return true;
		case EDecoderSession::SharedBind:
			return false;
		case EDecoderSession::Independent:
		default:
			return m_bShouldDecodeAudio;
		}
	}

private:
	EDecoderSession m_session = EDecoderSession::Independent;
	bool m_bShouldDecodeAudio = true;
};
