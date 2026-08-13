#pragma once

#include <map>
#include <atomic>
#include <shared_mutex>
#include <string>

#include "Common/AVCommon.h"
#include "Common/AudioCommon.h"
#include "Common/TexFrameCommon.h"
#include <QReadWriteLock>

#include <d3d11.h>
#include <dxgi.h>

#include <wrl/client.h>
template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

struct SPlayerResource
{
	UINT idxDevice = 0;
	UINT idxAdapter = 0;
	UINT gpuIndex = 0;				// 物理卡下标，供硬解 findAdapter 跟卡
	UINT arrivalSeq = 0;			// 挂接到达序号（仅共享软解路径由管理器赋值），先来后到音频/控制归属依据
	bool bMultiGpusMode = false;	// 多显卡模式状态，供 CVAPlayer::InitPlayer 内部判定软解复用分流
	std::string shareKey;			// 软解复用共享键 layerId|mediaId|filePath（空则不共享）
	float texX = 0.0f, texY = 0.0f, texW = 1.0f, texH = 1.0f; // 纹理拆分, 左上角+宽高
	ComPtr<ID3D11Device> pD3DDevice;
	LPCRITICAL_SECTION pCriticalSection		= nullptr;
	IVACallBacker* pVACallbacker			= nullptr;
	IVAPresent* pVAPresent					= nullptr;
	IDECCallBacker* pDECCallback			= nullptr;
};

struct STextureFrame;
class CVABaseDecoder
{
public:
	virtual ~CVABaseDecoder() {};
	//解码初始化
	virtual bool Init(
		const char *pFile, 
		EDecoderType emDecTypebool, 
		SPlayerResource playerResource, 
		bool NeedVideo, 
		bool needAudio,
		bool bLocate = false,
		std::chrono::milliseconds position = std::chrono::milliseconds(0)) { return false; }

	// 序列帧解码器初始化
	virtual bool Init(std::vector<std::wstring> const &files, int fps) { return false; }

	virtual bool UnInit() = 0;

	// 注册显卡资源 (新的播放器)
	// PR 表键恒为 pr.gpuIndex（物理卡下标，单一语义）；idxDevice 保持设备角色 Device_* 语义不参与键控
	// 返回当前注册的数量
	virtual int AddPlayerResource(SPlayerResource playerResource);
	// 取消注显卡资源 (新的播放器)
	// @param gpuIndex 物理卡下标（PR 表键）
	// 返回当前注册的数量
	virtual int RemovePlayerResources(int gpuIndex);

	// 清空全部注册资源（解码器实例销毁前调用）
	void ClearPlayerResources();

	//seek（gpuIndex：发起方物理卡下标，即 PR 表键）
	virtual bool Seek(int gpuIndex, milliseconds position, bool bExact, bool bReset = false) = 0;

	// 获取解码类型
	virtual bool		GetDecType(int& pType) = 0;

	// 获取音频属性（gpuIndex：发起方物理卡下标，即 PR 表键）
	virtual bool		GetAudioPty(int gpuIndex, SAudioPlayerProp& pty) = 0;

	//获取帧时间
	virtual microseconds GetFrameTime() = 0;

	//获取音频等待时间
	virtual microseconds GetAudioWaitTime() = 0;

	// 获取PacketSize
	virtual int GetVideoDecodePacketSize() = 0;

	// 获取FrameSize
	virtual int GetVideoDecodeFrameSize() = 0;

	// 是否有视频
	virtual int HasVideo() = 0;
	// 是否有音频
	virtual int HasAudio() = 0;

	virtual bool IsComplete() = 0;

protected:
	// 更新纹理到所有的播放器, 必须手动加锁 m_mtxPlayerResource
	void PresentTextures(std::map<int, STextureFrame*> & videoDatas);

	bool IsVideoEnough();

	std::map<int, STextureFrame*> FetchVideoData(std::chrono::microseconds pts);

	bool IsFirstPR(int gpuIndex);

	void ClearAVData();

	// 音频归属键：先来后到——到达序号 arrivalSeq 最小的 PR。
	// 创建者天然首到；Owner 退出后自动移交给次早到达者；空表返回 UINT_MAX。
	// 注意：Seek 写者与此无关，见 CVADecoder::m_creatorGpuIndex。
	UINT GetAudioOwnerKey();

protected:
	QReadWriteLock m_mtxPlayerResource;
	std::map<int, std::pair<int, SPlayerResource>> m_playerResources;

	std::atomic<float> m_fSpeed = 1.0f; // 播放速度
};