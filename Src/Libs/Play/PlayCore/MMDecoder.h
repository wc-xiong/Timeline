#pragma once

#include <list>
#include <set>

#pragma warning(disable : 4819)

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/hwcontext.h>
#include <libavfilter/avfilter.h>
#include <libavutil/opt.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavutil/stereo3d.h>
#include <libavutil/mastering_display_metadata.h>
#include <libavutil/pixdesc.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
}

#include <libavutil/hwcontext_d3d11va.h>
#include <libavcodec/d3d11va.h>

#include "CDropJudgment.h"

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avfilter.lib")

#include <unordered_map>
#include "Common/TexFrameCommon.h"
#include "Common/AVCommon.h"
#include "Common/AutoLock.h"

#include "VideoDecode/VideoDecoder.h"

#include <mutex>
#include <QReadWriteLock>

class CMMDecoder
{
public:
	CMMDecoder();
	~CMMDecoder();

public:
	// 拿缩略图的时候不要尝试 HAP
	bool Init(
		const char * pFile, 
		bool bNeedVideo, 
		bool bDiscardAudio, 
		EDecoderType emType, 
		UINT uAdapter, 
		bool tryHap = false, 
		IDECCallBacker* pDECCallback = nullptr,
		bool bLocate = false,
		std::chrono::milliseconds position = std::chrono::milliseconds(0),
		UINT gpuIndex = 0);
	void UnInit();

	AVFrame* GetVideoFrame();
	AVFrame* GetAudioFrame();
	void	 Seek(LONGLONG pts, bool bExact); //pts 毫秒
	/** Seek 后保持 m_bSeeking，出帧就绪后再 FinishSeek 才允许后续逻辑视为 Seek 完成 */
	void	 FinishSeek();
	bool	 IsSeeking() const { return m_bSeeking.load(); }
	bool	 IsComplete();
	bool	 IsVideoComplete() { return m_bReadEOF && m_bVideoDecoderEOF && m_videoFrameList.empty(); }
	bool	 IsAudioComplete() { return m_bReadEOF && m_bAudioDecoderEOF && m_audioFrameList.empty(); }
	bool	 IsDropDecEnable() { return ((m_pDECCallback != nullptr) && (m_pDECCallback != nullptr)); }

	int64_t GetVideoBitrate();
	std::string GetVideoCodecName();
	std::string GetContainerFormat();

	int GetAudioChannels();
	int64_t GetAudioSampleRate();
	std::string GetAudioCodecName();

	//bool IsHapVideo() { return m_bUseHap; }
	//SHapFrame GetHapVideoFrame();

	int GetVideoPackSize();
	int GetVideoFrameSize();

public:
	bool	 IsInterlaced();
	LONG	 GetWidth();
	LONG	 GetHeight();
	LONGLONG GetFrameTime();

	EDecoderType    GetDecoderType();
	int				GetRotateAngle();
	bool			HasVideo();
	bool			HasAudio();
	__int64			GetDuration();
	AVPixelFormat   GetPixformat();
	AVColorSpace	GetColorSpace();
	AVColorRange	GetColorRange();
	AVColorTransferCharacteristic  GetColorTrc();

public:
	void Read();
	void VideoDecoder();
	void AudioDecoder();

protected:
	static DWORD WINAPI ReadProc(LPVOID p);
	static DWORD WINAPI VideoDecoderProc(LPVOID p);
	static DWORD WINAPI AudioDecoderProc(LPVOID p);
	bool OpenVideoStreamHardWare(UINT uAdapter);
	bool OpenVideoStreamSoftWare();
	bool OpenAudioStream();
	void GetVideoSizeAndRate();
	void FlushPackets();
	void FlushFrames();
	bool IsVideoPacketsEnough();
	bool IsAudioPacketsEnough();
	bool ReceiveVideoFrame();
	bool SendVideoPacket();
	bool ReceiveAudioFrame();
	bool SendAudioPacket();
	void InitFilter();

	//bool DecodeHapVideoPacket();

	//bool SetupDecodeThreads();

public:
	bool m_bEnd;								// Engine关闭
	AVCodecContext * m_pAudioCodecContext = nullptr;		// 音频解码器 Context

private:
	int m_nVideoStreamIndex;					// 视频流index
	int m_nAudioStreamIndex;					// 音频流index

	AVFormatContext * m_pAVFormatContext = nullptr;
	AVCodecContext * m_pVideoCodecContext = nullptr;		// 视频解码器Context

	EDecoderType m_emDecoderType;				// 解码类型

	LONG m_width;								// 视频分辨率宽
	LONG m_height;								// 视频分辨率高
	int	 m_nRotate;								// 视频旋转角度
	LONGLONG m_frameTime;						// 视频帧间隔 单位：us
	bool m_bMP3;								// 部分音频会带有封面,影响HasVideo判断

	AVPixelFormat m_pixformat;					// 像素格式
	AVColorSpace  m_colorSpace;					// 颜色空间
	AVColorRange  m_colorRange;					// limit / full
	AVColorTransferCharacteristic m_colorTrc;	// Gamma

	std::list<AVPacket*> m_videoPacketList;			// 视频Packet
	std::list<AVPacket*> m_audioPacketList;			// 音频Packet

	LONGLONG m_videoPacketDuration;
	LONGLONG m_audioPacketDuration;

	bool m_bReadEOF;							// 文件读取结束
	bool m_bVideoDecoderEOF;					// 视频解码结束
	bool m_bAudioDecoderEOF;					// 音频解码结束

	HANDLE m_hRead = nullptr;								// 文件读取线程
	HANDLE m_hVideoDecoder = nullptr;						// 视频解码线程
	HANDLE m_hAudioDecoder = nullptr;						// 音频解码线程

	CCritSec m_csReadLock;						// 文件读取锁
	QReadWriteLock m_csVideoDecoderLock;				// 视频解码锁
	QReadWriteLock m_csAudioDecoderLock;				// 音频解码锁

	QReadWriteLock m_csVideoPacketListLock;			// packetList 锁
	QReadWriteLock m_csAudioPacketListLock;			// packetlist 锁

	std::list<AVFrame*> m_videoFrameList;			// Frame list
	std::list<AVFrame*> m_audioFrameList;			// Frame list

	QReadWriteLock m_csVideoFrameListLock;
	QReadWriteLock m_csAudioFrameListLock;

	long long m_lastGotFrameTs = 0;

	LONGLONG m_seekingPTS;
	bool m_bSeekingForVideo;
	bool m_bKeyFrameValid;
	bool m_bSeekingForAudio;

	//滤镜相关
	bool m_bTryFilter;
	int	 m_nFilterType;
	AVFilterGraph * m_pGraph = nullptr;
	AVFilterContext * m_pInContext = nullptr;
	AVFilterContext * m_pContext = nullptr;
	AVFilterContext * m_pOutContext = nullptr;

	PlayCore::CVideoDecoder* m_pDecoder = nullptr;

	UINT m_gpuIndex = 0;

	std::atomic_bool m_bSeeking = false;
	std::atomic_bool m_bNeedKeyframe = false;

public:
	CDropJudgment*	m_pDropJudge = nullptr;
	IDECCallBacker* m_pDECCallback = nullptr;
};

inline LONG		CMMDecoder::GetWidth() { return m_width; }
inline LONG		CMMDecoder::GetHeight() { return m_height; }
inline LONGLONG	CMMDecoder::GetFrameTime() { return m_frameTime; }
inline bool		CMMDecoder::IsInterlaced() { return m_bTryFilter; }
inline EDecoderType CMMDecoder::GetDecoderType() { return m_emDecoderType; }

inline AVPixelFormat   CMMDecoder::GetPixformat() { return m_pixformat; }
inline AVColorSpace   CMMDecoder::GetColorSpace() { return m_colorSpace; }
inline AVColorRange   CMMDecoder::GetColorRange() { return m_colorRange; }
inline AVColorTransferCharacteristic   CMMDecoder::GetColorTrc() { return m_colorTrc; }

