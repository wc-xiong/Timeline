#pragma once

#include <vector>
#include <chrono>
#include <QUuid>
using namespace std::chrono;

//音频数据
typedef struct _AUDIO_DATA
{
	microseconds	pts;			// 当前帧的结束时间 pts
	unsigned char* pSeekTo;		// 数据
	int				nDataLen;		// 数据长度 (字节数)
	unsigned char* pBuf;			// 数据备份地址

	std::chrono::microseconds    newpts;         // 重新计算的pts

	_AUDIO_DATA() :
		pts(0)
		, newpts(0)
	{
		pSeekTo = nullptr;
		nDataLen = 0;
		pBuf = nullptr;
	}

	~_AUDIO_DATA()
	{
		if (pBuf)
		{
			delete[] pBuf;
		}
		pBuf = nullptr;
		pSeekTo = nullptr;
	}


}AUDIO_DATA, * LPAUDIO_DATA;

enum class EVideoTextType
{
	// 解码纹理时使用
	No, // 非法
	Src_Normal, // 普通视频
	Src_LongWidth, // 超宽纹理视频
	Src_StereoLeft, // 3D视频纹理，单左画面
	Src_StereoRight, // 3D视频纹理，单右画面

	LongWithStereo, // 超宽3D视频纹理，暂时禁止
};

//回调函数
class IVACallBacker
{
public:
	virtual ~IVACallBacker(){}

	virtual bool		IsVideoEnough(bool bCicleDecode) = 0;		// 视频纹理足够
	virtual bool		IsAudioEnough(bool bCicleDecode) = 0;		// 音频数据足够

	virtual void		ClearVideoData() = 0;
	virtual void		ClearAudioData() = 0;

	virtual	void		Complete() = 0;
	virtual	void		AudioComplete() = 0;
	virtual	void		VideoComplete() = 0;

	virtual bool        HasVideo() = 0;
	virtual bool        HasAudio() = 0;
};

//回调函数
class IDECCallBacker
{
public:
	virtual ~IDECCallBacker(){}

	virtual microseconds GetRenderVideoPts() = 0;
};
struct STextureFrame;
class IVAPresent
{
public:
	virtual ~IVAPresent() {}

	virtual bool		PresentTexture(STextureFrame* videoData) = 0;
	virtual	bool		PresentAudioData(AUDIO_DATA* audioData) = 0;
	virtual STextureFrame*	FetchVideoData() = 0;
};
