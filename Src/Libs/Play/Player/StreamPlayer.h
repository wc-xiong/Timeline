#pragma once
#include <QObject>
#include "Player.h"
#include "PlayerGlobal.h"

#include <QString>
#include "Common/StreamMediaCommon.h"
#include <shared_mutex>
#include <set>

#include "boost/thread/condition_variable.hpp"
#include "boost/thread/mutex.hpp"

class IMediaStream;

class PLAYER_EXPORT CStreamPlayer 
	: public CBasePlayer
	, public IStreamMediaPlayer
{
public:
	CStreamPlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx);
	virtual ~CStreamPlayer();

public:
	bool	IsStreamMedia() override;

	bool	Start(const microseconds& curTime) override;
	bool	Prepare() override;								// 渲染准备
	bool	Stop() override;
	bool    Pause(const microseconds& curTime) override;
	bool	Restart(const microseconds& curTime) override;
	void BeforePlayerFree(bool bNormalScene) override;

	bool	GetTextView(const microseconds& curTime, SDrawTextureData& videoData) override;
	bool	GetAudioPty(SAudioPlayerProp& pty) override;									//获取音频属性
	bool	GetAudioData(const microseconds& curTs, BYTE* pBYTE, int nLen) override;		//获取音频数据

	// IStreamMediaPlayer
	void InitAudioRenderCallback(AddResAudioRenderCallBack task) override;

protected:
	bool PreperStream(bool bReStart);

protected:

	std::shared_timed_mutex m_mtxStream;
	std::shared_ptr<IMediaStream> m_pStream = nullptr;				// 流数据

	bool m_bHasVideo;						// 使用视频
	bool m_bHasAudio;						// 使用音频
	SAudioStreamFormat m_audioFormat;
	bool m_bAudioInitFinish = true;	// 音频数据是否加载完成

	AddResAudioRenderCallBack m_addAudioRenderCallback;	// 添加AudioRender回调函数

	QUuid m_streamPlayerId;
	QString m_strStreamKey;
};