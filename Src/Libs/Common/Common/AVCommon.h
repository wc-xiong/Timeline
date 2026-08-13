#pragma once

#include <iostream>
#include "CommonDefine.h"
//#include "hap/hap.h"

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04  // 40ms
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1   // 100ms

/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.040
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 3
//#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))
//#define FRAME_QUEUE_SIZE 3

/* polls for possible required screen refresh at least this often, should be less than 1/fps */
#define REFRESH_RATE 0.01

#define MIN_FRAMES 10	// packet size

#define CPU_IDLE_INTERNAL_MS 5

enum EDecodePlayState
{
	EDPS_Invalid = -1,		// 无效
	EDPS_Prepare,			// 准备
	EDPS_Play,				// 播放
	EDPS_Pause,				// 暂停
	EDPS_Seek,				// 跳转
	EDPS_Stop,				// 退出
	EDPS_Frezze,			// 冻结
};

typedef enum _EM_STATUS
{
	PLAY = 0,			// 播放
	SEEK,				// 跳转
	PAUSE,				// 暂停
	STOP				// 停止
}EM_STATUS;

// 解码类型（与 EDecoding 同序：Auto / Soft / Hard）
enum EDecoderType
{
	EDT_FAILED = -1,
	EDT_AUTO = 0,
	EDT_SOFTWARW,
	EDT_HARDWARD
};

// EDecoding -> EDecoderType（禁止裸转）
inline EDecoderType ToDecoderType(EDecoding mode)
{
	switch (mode)
	{
	case ED_Auto: return EDT_AUTO;
	case ED_Sort: return EDT_SOFTWARW;
	case ED_Hard: return EDT_HARDWARD;
	default: return EDT_AUTO;
	}
}

inline EDecoderType ToDecoderType(int decodingMode)
{
	return ToDecoderType(static_cast<EDecoding>(decodingMode));
}

typedef enum _EM_DECODER_FILTER
{
	FILETER_NONE = 0x00,
	FILTER_INTERLACED = 0x01,
	FILTER_ROTATE = 0x02,
}EM_DECODER_FILTER;

//struct SHapFrame
//{
//	void *buffers[2];
//	std::size_t sizes[2];
//	HapTextureFormat formats[2];
//	int64_t pts;
//
//	SHapFrame()
//	{
//		memset(this, 0, sizeof(*this));
//	}
//
//	void DeleteBuffers()
//	{
//		if (buffers[0]) delete buffers[0];
//		if (buffers[1]) delete buffers[1];
//		memset(buffers, 0, 2 * sizeof(void*));
//	}
//};

