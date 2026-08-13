#pragma once

#include "Player.h"
#include "../PlayCore/VAPlayer.h"

#include "PlayerGlobal.h"

#include <atomic>
#include <QObject>

class IVideoResource;
class IAudioResource;

class PLAYER_EXPORT CVideoPlayer : public QObject, public CBasePlayer
	, public CVAPlayer
	, public IVADecodeObject
	, public IVideoDecodeInfo
	, public IVideoPlayer
{
	Q_OBJECT

public:
	CVideoPlayer(std::shared_ptr<IResourceBase> pResBase, 
		const BasePlayerContext& ctx, const milliseconds& starttime, 
		const milliseconds& cropPlaytime, 
		const milliseconds& mediaPlayTime);
	virtual ~CVideoPlayer();

public:
	//IPlayer
	bool			Start(const microseconds& curTime) override;
	bool			Prepare() override;								// 渲染准备
	bool			Seek(LONGLONG dwTime) override;					// 跳转
	bool			Stop() override;
	bool			Restart(const microseconds& curTime) override;

	bool			SetPlayTime(const milliseconds& starttime, const milliseconds& playtime) override;
	bool			IsSeekReady() override;
	void			SeekToReadyPosition() override;
	void			SeekToReady() override;

	//IVideoRenderObject
	EDecoderType	GetDecodeType();
	bool			IsMediaEnough();
	microseconds	GetFirstPts() { return m_firstPts; }
	void			SetFirstPts(const microseconds& firstPts) { m_firstPts = firstPts; }

	//ITextureDataSrc
	bool			GetTextView(const microseconds& curTime, SDrawTextureData& videoData) override;
	bool			GetAudioData(const microseconds& curTime, BYTE* pBYTE, int nLen) override;
	bool			GetAudioPty(SAudioPlayerProp& pty) override;

	// CVAPlayer
	bool			PrepareReady();
	bool			IsEOF() override;

	//IVAPresent
	bool			PresentTexture(STextureFrame* pVideoData);
	void			VideoComplete();

	bool            HasAudio();
	bool            IsSupportAudio() override;

	// IVideoDecodeInfo
	std::pair<QUuid, SVideoDecodeData> GetVideoDecodeData() override;

	QString GetVideoFrameFormat() override;

protected:
	bool			PrepareVideo();

	QString			GetVideoFrameType(STextureFrame* texFrame);

	void			MediaLoopPlayback();

signals:
	void signal_ReadyFinish(const QUuid&);

protected:
	// 渲染线程视频上次请求的时间戳
	microseconds		m_videoPtsOfLastQuery{ 0 };
	// 音频播放线程上次请求的时间戳
	microseconds		m_audioPtsOfLastQuery{ 0 };

	milliseconds		m_clipStartTime;					// 素材截取起始时间( GetClipStartTime )
	milliseconds		m_clipCropPlayTime;					// 素材截取后播放时长( GetClipEndTime() - GetClipStartTime() )
	milliseconds		m_mediaPlayTime;					// 媒体素材播放时长
	microseconds		m_firstPts;							// 首帧

	//记录每秒解码多少帧
	std::atomic_int			m_nLastVideoDecodePerSecond;

	// 每秒绘制多少帧
	std::atomic_int			m_nLastVideoDrawPerSecond;

	__int64 m_nLastDrawTime = -1;
	int						m_nDecodeType;							// 解码类型
	QString m_strVideoFrameType;	// 解码纹理类型

	std::string		m_filePath;
	bool			m_bPrepared;

	bool m_bCirclePlay = false;

	std::shared_ptr<IVideoResource> m_pVideoRes = nullptr;
	std::shared_ptr<IAudioResource> m_pAudioRes = nullptr;
};