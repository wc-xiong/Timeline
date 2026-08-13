#pragma once

#include <string>
#include <chrono>
#include <dxgi.h>
#include "ChannelMap.h"

#include <iostream>

using namespace std::chrono;

#include <QString>

//素材原始音频属性
struct SAudioPlayerInfo
{
	//音频通道信息
	int nChannels;
	//采样率
	int nSampleRate;
};

//AudioRender音频播放信息
struct SAudioRenderInfo
{
	// 采样率
	int	m_nSampleRate;
	// 声道数
	int	m_nChannels;
	// audio buffer size in samples
	int	m_nSample;
	// 每秒的声音数据字节数
	int	m_nBytesPerSec;
	// 静音长度
	int	m_nSilenceLen;
	// 超时时间
	std::chrono::milliseconds m_waitTime;
};

struct SAudioPlayerProp
{
	int sampleRate = 0;		// 采样率
	int channels = 0;		// 通道数
	int nSamples = 0;		// 单次回调采样数
	int bitsPerSample = 16; // 样本比特数
	QString speaker = ""; // 空则使用默认设备

	// 单次回调缓冲区大小
	int GetBufferSize()
	{
		return nSamples * channels * (bitsPerSample / 8);
	}

	void SetDefault()
	{
		sampleRate = 0;
		channels = 0;
		nSamples = 0;
		bitsPerSample = 16;
		speaker = "";
	}

	QString GetAudioPtyString()
	{
		return  QString("pty:sampleRate = %1, channels= %2,nSamples= %3, bitsPerSample = %4").arg(QString::number(sampleRate))
			.arg(QString::number(channels)).arg(nSamples).arg(bitsPerSample);
	}
};

static inline bool operator<(const SAudioPlayerProp &l, const SAudioPlayerProp &r)
{
	if (l.speaker != r.speaker)
		return l.speaker < r.speaker;
	//	if (l.channels != r.channels)
	//		return l.channels < r.channels;
	if (l.nSamples != r.nSamples)
		return l.nSamples < r.nSamples;
	if (l.bitsPerSample != r.bitsPerSample)
		return l.bitsPerSample < r.bitsPerSample;
	return l.sampleRate < r.sampleRate;
}

struct SAudioPlayData
{
	BYTE*	pAudioData; // 音频数据
	float	fVolume; // 音量
	SChannelMap ChannelMap; // 通道映射

	SAudioPlayData()
	{
		pAudioData = nullptr;
		fVolume = 1.0f;
	}

	SAudioPlayData(BYTE* pData, float fvol, SChannelMap map)
	{
		pAudioData = pData;
		fVolume = fvol;
		ChannelMap = map;
	}

	~SAudioPlayData()
	{
		////此处不能释放！！！否则重复释放pAudioData
		//if (pAudioData)
		//	delete[] pAudioData;

		//pAudioData = nullptr;
	}

	SAudioPlayData(const SAudioPlayData& other)
	{
		this->pAudioData = other.pAudioData;
		this->fVolume = other.fVolume;
		this->ChannelMap = other.ChannelMap;
	}
};

struct SAudioDecodeData
{
	int		nLenth;
	BYTE*	pData;

	SAudioDecodeData()
	{
		nLenth = 0;
		pData = nullptr;
	}

	~SAudioDecodeData()
	{
		delete[] pData;
		pData = nullptr;
		nLenth = 0;
	}
};

// 音频幅值控件
#define SAMPLE_LIFE_CICLE	50
#define SINGLE_BYTE			127
#define DOUBLE_BYTE			32767
#define MIN_VOLUME			-58.0f
#define DISTANCE			2.0f
#define CHANELWIDTH			16.0f

static struct
{
	DWORD BrushClr;
	float BrushPos;
} g_Brush_Clr_Pos[4] = {
	{0xFF368D46, 0.0f},
	{0xFF2EC13C, 0.3f},
	{0xFFC6C93C, 0.6f},
	{0xFFE2554E, 1.0f}
};

struct SAudioDB
{
	float leftAudioDB = 0.0f;	// 左声道分贝
	float rightAudioDB = 0.0f;	// 右声道分贝
	LONGLONG curPts = 0;		// 当前微秒

	SAudioDB()
	{
		SetDefault();
	}

	SAudioDB& operator=(const SAudioDB& other)
	{
		leftAudioDB = other.leftAudioDB;
		rightAudioDB = other.rightAudioDB;
		curPts = other.curPts;
		return (*this);
	}

	void SetDefault()
	{
		leftAudioDB = MIN_VOLUME;
		rightAudioDB = MIN_VOLUME;
	}

	bool operator==(const SAudioDB &other)
	{
		return leftAudioDB == other.leftAudioDB
			&& rightAudioDB == other.rightAudioDB;
	}
 };

//参考https://blog.csdn.net/qinglongzhan/article/details/82909430
static float  CalcuDB(float value) { return 20.0f * log10f(value); }