#pragma once
#include "ResourceDefine.h"
#include "AVCommon.h"
using namespace std::chrono_literals;

#if 0
enum EDecodePlayState
{
	EDPS_Invalid = -1,		// 无效
	EDPS_Prepare,			// 准备
	EDPS_Play,				// 播放
	EDPS_Pause,				// 暂停
	EDPS_Seek,				// 跳转
	EDPS_Stop,				// 退出
	EDPS_Freeze,			// 冻结
};
#endif

// 初始化对比数据
struct SCellCompareParam
{
	GUID clipID;			// 素材ID
	int	nClipType;			// 类型
	LONGLONG startPts;		// 节目开始播放点
	LONGLONG playLenth;		// 播放时长
	int nLayerType;			// 节目图层类型

	SCellCompareParam()
	{
		clipID = GUID_NULL;
		nClipType = ERT_Invalid;
		startPts = 0;
		nLayerType = -1;
	}

	BOOL operator == (const SCellCompareParam& other)
	{
		return (this->clipID == other.clipID &&\
			this->startPts == other.startPts &&\
			this->playLenth == other.playLenth&&\
			this->nLayerType == other.nLayerType);
	}

	BOOL IsStreamClip()
	{
		return nClipType == ERT_VideoCapture || nClipType == ERT_AudioCapture || nClipType == ERT_Ndi || nClipType == ERT_WebMedia;
	}

	BOOL HasAudioClip()
	{
		return nClipType == ERT_AudioCapture || nClipType == ERT_Ndi || nClipType == ERT_Audio || nClipType == ERT_Video || nClipType == ERT_WebMedia;
	}
};

// 外部与解码控制模块交互数据
struct SCellDecodeState
{
	// 播放单元 ID
	QUuid idCell;

	// 播放模式(EnPlayMode)：EPM_Normal
	int mode = 0x10;

	// seek time
	LONGLONG seek = 0;

	// seek time (seek,seekTs待整理，取其中一个)
	std::chrono::milliseconds seekTs = 0ms;

	//解码单元覆盖的设备数量（多显卡）
	int nCrossScreen = 0;

	SCellDecodeState& operator=(const SCellDecodeState& other)
	{
		memcpy(this, &other, sizeof(SCellDecodeState));
		return *this;
	}
};

