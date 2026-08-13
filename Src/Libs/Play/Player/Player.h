#pragma once

#include "PlayerGlobal.h"
#include "Common/TexFrameCommon.h"
#include "Common/VideoDataCommon.h"
#include "GSMedia/RenderDevice/DeviceResource.h"
#include "Common/Common/IPlayerInterface.h"

#include <set>
#include <mutex>

class IResourceBase;

class PLAYER_EXPORT CBasePlayer : public IBasePlayer, public CD3DResourceChild
{
public:
	CBasePlayer(std::shared_ptr<IResourceBase> pResBase,
		const BasePlayerContext& ctx = BasePlayerContext());
	virtual ~CBasePlayer();

public:
	//IPlayer
	QUuid GetMediaId() override { return m_ctx.mediaId; };
	const BasePlayerContext& GetBasePlayerContext() const override { return m_ctx; };
	QUuid GetResId() override { return m_ctx.resId; };
	int GetPlayerType() override;

	virtual bool	SetPlayState(int type) { m_playState = type; return true; }
	virtual int		GetPlayState() { return m_playState; }

	virtual bool	IsStreamMedia() { return m_ctx.bStreamMedia; }

	virtual bool	Prepare() { m_playState = EDPS_Prepare;	return true; }
	virtual bool	Start(const microseconds& curTime);
	virtual bool	Pause(const microseconds& curTime) { m_playState = EDPS_Pause;		return true; }
	virtual	bool	Seek(LONGLONG dwTime) { m_playState = EDPS_Seek;		return true; }
	virtual bool	Stop();
	virtual bool	Freeze() { m_playState = EDPS_Frezze;	return true; }
	virtual bool	Restart(const microseconds& curTime) { m_playState = EDPS_Play;	return true; }
	virtual bool    SetPlayTime(const milliseconds& starttime, const milliseconds& playtime) { return true; }

	virtual bool	GetTextView(const microseconds& curTime, SDrawTextureData& videoData) override;	// 获取纹理数据
	virtual bool	GetAudioPty(SAudioPlayerProp& pty) override;										// 获取音频属性
	virtual bool	GetAudioData(const microseconds& curTs, BYTE* pBYTE, int nLen) override;			// 获取音频数据

	virtual bool	HasAudio() { return false; }
	virtual bool	IsSupportAudio() override { return false; }
	virtual bool	IsReady() {	return m_bReady;}
	virtual bool    IsEOF() { return false; }
	virtual bool	IsSeekReady() { return true; }
	virtual void	SeekToReadyPosition() {}
	virtual void	SeekToReady() {}

	void SetDirty(bool bDirty) override { 
		m_bDirty.store(bDirty); 
	}

	void SetDelayedRelease(bool bIsDelayRelease) override
	{
		m_bDelayRelease = bIsDelayRelease;
	}

	virtual void BeforePlayerFree(bool bNormalScene) {}

	QString GetResPath() override;

protected:
	// 将纹理送至videoData
	virtual void UpdateTexToData();

	// 将照片纹理送至videoData
	virtual bool UpdateImageToData(QImage& img);

protected:

	std::shared_ptr<IResourceBase> m_pResBase = nullptr;

	BasePlayerContext					m_ctx;				// 播放器上下文
	int									m_playState;		// 当前播放状态

	std::mutex							m_mtxVideoData;
	SDrawTextureData					m_videoData;		// 通用纹理（图片，视频，PPT，采集卡，NDI等）

	bool								m_bReady;

	int m_width = 0;
	int m_height = 0;
	bool m_bFrameChange = true;

	STexDatas m_sTexs;

	std::atomic_bool m_bDirty = true;	// 当前Player需要更新标志

	//player延迟释放
	bool m_bDelayRelease = false;	

	QString m_strDecodePath;
};
