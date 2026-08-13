#pragma once

#include "PlayCoreGlobal.h"
#include "Common/TexFrameCommon.h"
#include "Common/AudioCommon.h"
#include "Common/AVCommon.h"
#include <deque>
#include <queue>
#include <memory>
#include <mutex>
#include <atomic>
#include <string>
#include <vector>
#include <QReadWriteLock>
#include "Common/VideoDataCommon.h"
#include "VABaseDecoder.h"
#include "SoftShareSession.h"
#include "AudioRegisterPolicy.h"

class CVABaseDecoder;
class PLAYCORE_EXPORT CVAPlayer
	: public IVACallBacker
	, public IVAPresent
	, public IDECCallBacker
{
public:
	CVAPlayer();
	~CVAPlayer();

protected:
	//IVACallBacker
	virtual bool		IsVideoEnough(bool bCicleDecode);	// 视频纹理足够
	virtual bool		IsAudioEnough(bool bCicleDecode);	// 音频数据足够

	bool IsVideoEmpty();

	virtual void		ClearVideoData();					// 清空视频纹理数据
	virtual void		ClearAudioData();					// 清空音频数据
	virtual	void		VideoComplete();					// 视频解码结束
	virtual	void		AudioComplete();					// 音频解码结束
	virtual void		Complete();
	virtual bool        IsComplete();

	virtual bool        HasVideo();
	virtual bool        HasAudio();

	//IVAPresent
	virtual bool		PresentTexture(STextureFrame* pVideoData);
	virtual bool		PresentAudioData(AUDIO_DATA* pAudioData);
	virtual STextureFrame*	FetchVideoData();

	//IDECCallbacker
	virtual microseconds GetRenderVideoPts() { return m_lastVideoPts; }

	bool	Stop();

	// 
protected:
	/*
	* @brief 初始化音视频底层解码器
	* @paramer1: pr, D3D显卡资源及回调抽象接口；其 bMultiGpusMode/shareKey 携带软解复用
	*            状态，由 SoftShareSession::TryAttachIfEligible 判定共享/独立分流
	* @paramer2: strFilePath, 媒体素材文件路径
	* @paramer3: nDecodeType, 解码类型: 硬解、软解, 自动模式
	* @paramer4: bNeedVideo, 是否需要解码视频
	* @paramer5: bShouldDecodeAudio, 独立路径音频亲和：本卡是否应打开音频流并具备注册资格
	*            （多卡非归属卡传 false；共享路径策略忽略该预填，由 CreateOrBind 角色裁决）
	* @paramer6: bLocate, 是否需要从指定位置开始解码
	* @paramer7: position, 指定解码位置
	* @note 不按 Soft/Hard 拆子类。门控命中后共享失败 → InitPlayer 失败（打日志），不做独立软解降级。
	*/
	bool InitPlayer(
		SPlayerResource pr,
		std::string& strFilePath,
		EDecoderType nDecodeType,
		bool bNeedVideo,
		bool bShouldDecodeAudio = true,
		bool bLocate = false,
		std::chrono::milliseconds position = std::chrono::milliseconds(0)
	);

	// 共享释放唯一收口：先摘本卡 PR（断回调）再计数释放；幂等
	void ReleaseSharedDecoder();

	// Seek/音频属性：PR 表键由内部处理（单一语义 = gpuIndex），调用方无需感知
	bool SeekDecoder(milliseconds position, bool bExact, bool bReset = false);
	bool GetAudioPty(SAudioPlayerProp& pty);

	/*
	* @brief 本实例是否允许创建上层音频播放器（Prepare/InitPlayer 之后有效）
	* @note 上层唯一查询入口；委托 IAudioRegisterPolicy：
	*       - SharedCreator：先到创建者且素材含音频
	*       - SharedBind：不可注册
	*       - Independent：建播 bShouldDecodeAudio（min-Adapter 等）且素材含音频
	*/
	bool IsSupportAudio();

	// 必须在析构前调用
	bool FinalizePlayer();

	void				ClearData();
	bool				GetVideoData(const microseconds curPts, STextureFrame& videoData);
	bool				GetAudioData(microseconds& curPts, BYTE* pByte, int nLen);
	bool				GetAudioPty(int gpuIndex, SAudioPlayerProp& pty);

protected:
	virtual bool		PrepareReady() PURE;

	void BindAudioPolicy(EDecoderSession session);

protected:
	void ReleaseVideoData(STextureFrame* data, bool bReSave = true);
	void AppendVideoFetchLog(const microseconds& curPts, const STextureFrame& matched);

protected:
	QReadWriteLock			m_videoMutex;
	QReadWriteLock			m_audioMutex;

	bool					m_bVideoDecComplete;	// 视频解码结束,并不代表Present结束
	bool					m_bAudioDecComplete;	// 音频解码结束,并不代表Present结束

	__int64					m_minAudioPts;			// 最小帧音频
	bool					m_bSyncAudio;			// 同步音频

	std::shared_ptr<CVABaseDecoder>	m_pVAPlayer = nullptr;			// 视频播放器（独立路径独占；共享路径多持有者）

	// 软解复用：会话组合 + 音频注册策略（见 SoftShareSession / AudioRegisterPolicy）
	SoftShareSession		m_softShare;
	std::unique_ptr<IAudioRegisterPolicy> m_audioPolicy;
	bool					m_bShouldDecodeAudio = true;	// 独立路径音频亲和预填（策略 Independent 用；独立 Init 作 needAudio）
	UINT					m_decoderKey = 0;				// 本 Player 所属物理卡下标（DXGI 适配器索引），即其 PR 键；单一语义

	std::deque<STextureFrame*> m_queVideoData;				// 视频数据
	std::deque<AUDIO_DATA*>			 m_queAudioData;				// 音频数据

	microseconds			m_lastVideoPts;

	// 解码器获取到的数据
	microseconds			m_videoFrameTime;
	__int64					m_videoFps;

	float                   m_audioSpeed;							// 当前音频的播放速度

	std::mutex m_resaveMutex;
	std::queue<STextureFrame*> m_queResaveVideoData;				// 回收后可继续使用的视频纹理

	bool m_bInitdata = false;
	TEXTURES m_vecTexs;
	ComPtr<ID3D11Device> m_pD3dDevice = nullptr;
	ComPtr<ID3D11DeviceContext> m_pD3dDeviceContext = nullptr;
	ComPtr<ID3D11Query>			m_pFlushQuery = nullptr;

	std::atomic_bool		m_bEnd;

	// 取帧调试日志（目录 C:\VABasePlayer 存在时开启）
	bool					m_bDebugLog = false;
	std::vector<std::string>	m_vecLogStr;
	std::string				m_debugLogName;
};
