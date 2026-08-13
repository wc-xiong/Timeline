#pragma once
#include "CommonDefine.h"

#include <QUuid>
#include <QRect>
#include <chrono>
#include <functional>
#include <memory>

#include <Windows.h>
//
//#include "Common/Common/AudioCommon.h"
#include "AVCommon.h"


using namespace std::chrono;
class IBasePlayer;

struct SDrawTextureData;
struct SMediaPlayData;
class IPlayerController
{
public:
	virtual ~IPlayerController() {}

	//获取/更新/查询更新Player
	virtual std::weak_ptr<IBasePlayer> GetBasePlayer(const QUuid &id) = 0;

	virtual void Play() = 0;
	virtual void Pause() = 0;
	virtual void Stop() = 0;
	virtual void Seek(const std::chrono::milliseconds& ms) = 0;
	virtual void SeekObject(const QUuid& objectId) = 0;
	virtual void SeekMedia(const QUuid& resId, long long clipPlayTime) = 0;

	virtual std::chrono::milliseconds GetCurTime() = 0;

	virtual bool IsPlaying() = 0;
	virtual bool IsStop() = 0;
	virtual bool IsViewNeedUpdate() = 0;

	// 判断当前Player是否已经创建
	virtual bool IsPlayerExist(const QUuid &uuid) = 0;

	//添加
	virtual void AddMedia(const QUuid mediaId, const QUuid &resourceId, QRect& Rect = QRect(), QPoint pos = QPoint(),float angle = 0) = 0;

	//创建/删除/清空Player
	virtual void DelMediaAndFreePlayer(const QUuid& resId) = 0;
	virtual	void DeletePlayer(const QUuid &id, const QUuid& Resid) = 0;
	virtual	void CreatePlayer(const QUuid &uuid, const QUuid& resid) = 0;
	virtual void ChangePlayer(const QUuid& id, const QUuid& Resid) = 0;
	virtual	void ClearPlayers() = 0;
	virtual void UpdatePlayerClipStartEndTime(const QUuid& id) = 0;

	virtual void ResetPlayersByHistoryStack() = 0;

	//渲染回调函数
	virtual bool GetTexture(const QUuid& playerId, const std::chrono::microseconds& ts, SDrawTextureData& data, bool bReady) = 0;

	virtual void ReplaceRes(const QUuid& resId) = 0;

	// 更新Player，素材更新时需要Player重绘等操作
	virtual void UpdatePlayerPty(const QUuid& resId) = 0;
	virtual void SetDirty(const QUuid& mediaId) = 0;

	virtual bool GetMediaPlayData(const QUuid& resId, SMediaPlayData &data) = 0;

	virtual void PictureTrans(const QUuid& resId) = 0;
};

using namespace std::chrono;
struct SDrawTextureData;
struct SAudioPlayerProp;

struct BasePlayerContext
{
	// 时间线QUuid
	QUuid tlId;
	// 图层QUuid
	QUuid layerId;
	// 时间线媒体素材QUuid
	QUuid mediaId;
	// 素材QUuid
	QUuid resId;
	// 是否是流媒体素材
	bool bStreamMedia = false;
	// 解码偏好（EDecoding）
	int decodingMode = 0;
	// 物理显卡下标（与 DecoderManager::SetDeviceIndex 一致）
	UINT gpuIndex = 0;
	// 本卡是否需要音频解码（多卡跨屏时仅 min(AdapterIndex) 为 true）
	bool bNeedAudio = true;
};

class IBasePlayer
{
public:
	virtual ~IBasePlayer() {}

	//媒体ID
	virtual QUuid GetMediaId() = 0;

	// 播放器上下文
	virtual const BasePlayerContext& GetBasePlayerContext() const = 0;

	//素材ID
	virtual QUuid GetResId() = 0;

	//获取类型
	virtual int	GetPlayerType() = 0;

	// 获取解码状态
	virtual int GetPlayState() = 0;
	virtual bool SetPlayState(int type) = 0;

	virtual bool IsStreamMedia() = 0;

	//准备
	virtual bool Prepare() = 0;

	//开始播放
	virtual	bool Start(const microseconds& curTime) = 0;

	// 暂停播放
	virtual bool Pause(const microseconds& curTime) = 0;

	//跳转
	virtual	bool Seek(LONGLONG dwTime) = 0;

	// 停止
	virtual bool Stop() = 0;

	// 冻结
	virtual bool Freeze() = 0;

	// 冻结
	virtual bool Restart(const microseconds& curTime) = 0;

	// 设置时间参数（起始时间和时长）
	virtual bool SetPlayTime(const milliseconds& starttime, const milliseconds& playtime) = 0;

	//获取指定时间纹理视图-从解码器到纹理中心
	virtual bool GetTextView(const microseconds& curTime, SDrawTextureData& videoData) = 0;

	//获取音频属性
	virtual bool GetAudioPty(SAudioPlayerProp& pty) = 0;

	//获取音频数据
	virtual bool GetAudioData(const microseconds& curTs, BYTE* pBYTE, int nLen) = 0;

	// 是否有音频
	virtual bool HasAudio() = 0;

	virtual bool IsReady() = 0;
	virtual bool IsEOF() = 0;

	virtual bool IsSeekReady() = 0;
	virtual void SeekToReadyPosition() = 0;
	virtual void SeekToReady() = 0;

	virtual void SetDirty(bool bDirty) = 0;

	//延迟释放
	virtual void SetDelayedRelease(bool bIsDelayRelease) = 0;

	// Player释放前会被调用
	// bNormalScene = true：表示当前在常规场景释放，反之则为淡出场景
	virtual void BeforePlayerFree(bool bNormalScene) = 0;

	virtual QString GetResPath() = 0;
};

class IVADecodeObject
{
public:
	virtual ~IVADecodeObject() {}
	//获取解码类型
	virtual EDecoderType	GetDecodeType() = 0;

	//是否有足够的纹理用于播放同步
	virtual bool			IsMediaEnough() = 0;

	// 首帧同步
	virtual microseconds	GetFirstPts() = 0;
	virtual void			SetFirstPts(const microseconds& firstPts) = 0;
};

struct SVideoDecodeData;

class IVideoDecodeInfo
{
public:
	virtual ~IVideoDecodeInfo() {}

	// 获取视频解码数据
	virtual std::pair<QUuid, SVideoDecodeData> GetVideoDecodeData() = 0;

	// 获取视频解码纹理类型
	virtual QString GetVideoFrameFormat() = 0;
};

typedef std::function<void(SAudioPlayerProp &pty, const BasePlayerContext &ctx)> AddAudioRenderCallBack;
typedef std::function<void(SAudioPlayerProp &pty, const BasePlayerContext &ctx)> AddResAudioRenderCallBack;
typedef std::function<void(const QUuid &resId, const BasePlayerContext &ctx)> DelAudioRenderCallBack;

class IStreamMediaPlayer
{
public:
	virtual ~IStreamMediaPlayer() {}

	virtual void InitAudioRenderCallback(AddResAudioRenderCallBack task) = 0;
};


// 或是支持翻页的媒体继承此类，可使用播放状态窗口翻页按钮
class IPageTurnPlayer
{
public:
	virtual ~IPageTurnPlayer() {}
	// 翻页操作
	virtual void TurnPageOp(int nType, const microseconds& curTime) = 0;

	// 获取当前播放页数
	virtual int GetCurPage() { return m_nCurPage; }

	virtual void SetCurPage(int nPage) { m_nCurPage = nPage; }

	virtual int GetPageInterval() = 0;
protected:
	int m_nCurPage = 0;
};

class IPicPlayer
{
public:
	virtual ~IPicPlayer() {}

	virtual void PictureTrans(const QString& path) = 0;
};

class IVideoPlayer
{
public:
	virtual ~IVideoPlayer() {}
};

class IToolPlayer
{
public:
	virtual ~IToolPlayer() {}
};