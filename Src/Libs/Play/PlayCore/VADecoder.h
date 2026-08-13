#pragma once

#include "VABaseDecoder.h"
#include <future>
#include <iostream> 

#include <atomic>
#include <mutex>
#include <thread>

#include "Common/AutoLock.h"
#include "AudioChecker.h"

struct SwsContext;
struct SwrContext;
class CMMDecoder;
class CDropJudgment;
class CVADecoder : public CVABaseDecoder
{
public:
	CVADecoder();
	~CVADecoder();

public:
	bool Init(
		const char *pFile, 
		EDecoderType emDecType, 
		SPlayerResource playerResource, 
		bool bNeedVideo, 
		bool needAudio,
		bool bLocate = false,
		std::chrono::milliseconds position = std::chrono::milliseconds(0)) override;
	bool UnInit();

	// 获取PacketSize
	int GetVideoDecodePacketSize() override;
	// 获取FrameSize
	int GetVideoDecodeFrameSize() override;

	// 参数调整必须指定 gpuIndex（发起方物理卡下标，即 PR 表键），即哪个播放器控制
	bool		 Seek(int gpuIndex, milliseconds position, bool bExact, bool bReset) override;

	bool		 GetDecType(int &pType) override;
	bool		 GetAudioPty(int gpuIndex, SAudioPlayerProp& pty) override;

	microseconds GetFrameTime() override;
	microseconds GetAudioWaitTime() override;

	// 是否有视频
	int			HasVideo() override;
	// 是否有音频
	int			HasAudio() override;

	bool		IsComplete() override;

public:
	void				Thread();							// 播放准备线程函数

protected:
	//static DWORD WINAPI ThreadProc(LPVOID p);				// 准备线程
	void				DealAudioData(AVFrame * pFrame);	// 重采样音频数据并给AUDIO_DATA赋值
	void				DealVideoData(AVFrame * pFrame);	// 将视频帧数据转换成纹理视图并放入队列
	//void				DealHapVideoData(SHapFrame &frame); // 处理压缩纹理
	bool				TryReadStereo(AVFrame *pFrame, STextureFrame *pVideoData);

private:
	CMMDecoder*				m_pDecoder = nullptr;					// 解码器

	EM_STATUS				m_emStatus;					// 播放状态
	std::atomic_bool		m_bEnd;						// 解码结束
	std::atomic_bool		m_bSeeking = false;
	std::atomic_bool		m_bOccurError = false;

	// 建播创建者物理卡下标（Init 时写入）；共享多 PR 时仅该卡可 Seek 底层解码器
	UINT					m_creatorGpuIndex = UINT_MAX;

	//HANDLE					m_hPlayerThread;		// 资源生成线程
	std::unique_ptr<std::thread> m_audioDealThrd, m_videoDealThrd;

	bool					m_bPrepareCallback;		// seek 之后回调

private:
	CCritSec				m_csVideoLock;			// video Lock
	SwsContext*				m_pVideoSwrctx;			// 视频格式转换

	CCritSec				m_csAudioLock;			// audio Lock
	bool					m_bAudioForceRefresh;	// seek 之后强制刷新

	LONGLONG				m_llAudioClk;
	SwrContext*				m_pAudioSwrctx;			// 重采样上下文

	SAudioCheckProperty		m_sAudioProperty;

private:
	unsigned int				m_nTexWidth;
	unsigned int				m_nTexHeight;

	ComPtr<ID3D11Device>		m_pHWDevice;
	ComPtr<ID3D11DeviceContext> m_pHWDeviceContex;
	ComPtr<ID3D11Query>			m_pFlushQuery;
	bool						m_bFirstTimeDealFrame;

	ComPtr<ID3D11Texture2D>		m_pVideoTexture0 = nullptr;
	//HANDLE						m_handleTexture0 = nullptr;

	//ComPtr<ID3D11Texture2D>		m_pVideoTexture1 = nullptr;
	//HANDLE						m_handleTexture1 = nullptr;

	//ComPtr<ID3D11Texture2D>		m_pVideoTexture2 = nullptr;
	//HANDLE						m_handleTexture2 = nullptr;
private:
	CDropJudgment*				m_pDropJudge = nullptr;
	IDECCallBacker*				m_pDECCallback = nullptr;
};
