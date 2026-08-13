#pragma once

#include <QPixmap>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libavfilter/avfilter.h>
}

#include "BaseDemuxer.h"


// 视频解复用
class CVideoDemuxer : public IBaseDemuxer
{
public:
	CVideoDemuxer();
	virtual ~CVideoDemuxer();

	virtual bool Demuxer(std::shared_ptr<SResFileDemuxInfo>& info) override;

protected:
	bool	Open(const std::string &path);
	bool	Init(const std::string &path);
	int		FindStreamIndex(AVMediaType type);

	bool	IsSupportHWDecode();		// 判断是否支持硬解码

	QPixmap GetThumbnail(const QString& path);

	//bool GetFrame(std::shared_ptr<CFrame> &pFrame);
	//QPixmap GetThumbnailByFrame(std::shared_ptr<CFrame> pFrame);

	int GetRotateAngle();	// 获取旋转角度
	void InitFilter();		// 视频滤镜

protected:
	AVFormatContext*	m_avFmtCtx;
	AVCodecContext*		m_videoCodecCtx;
	AVCodecContext*		m_audioCodecCtx;
	AVCodecParameters*	m_videoParam;
	AVCodecParameters*	m_audioParam;

	int					m_videoIndex;
	int					m_audioIndex;

	bool				m_bHasAudio;

	int					m_hwFormat;

	int					m_nRotateAngle;		// 旋转角度
	bool				m_bTryFilter;		// 滤镜相关 
	int					m_nFilterType;

	bool				m_bSupportHDR;		//是否支持HDR

	AVFilterGraph		* m_pGraph;
	AVFilterContext		* m_pInContext;
	AVFilterContext		* m_pContext;
	AVFilterContext		* m_pOutContext;
};

// 音频解复用
class CAudioDemuxer : public IBaseDemuxer
{
public:
	CAudioDemuxer();
	virtual ~CAudioDemuxer();

	virtual bool Demuxer(std::shared_ptr<SResFileDemuxInfo>& info) override;

protected:
	bool	Init(const std::string& path);
	int		FindStreamIndex(AVMediaType type);
	QPixmap GetThumbnail();

protected:
	AVFormatContext*	m_avFmtCtx;
	AVCodecParameters*	m_audioParam;
};


// 图片解复用
class CPictureDemuxer : public IBaseDemuxer
{
public:
	CPictureDemuxer();
	virtual ~CPictureDemuxer();

	virtual bool Demuxer(std::shared_ptr<SResFileDemuxInfo>& info) override;

protected:
	bool	Open(const std::string &path);
	bool	Init(const std::string &path);
	int		FindStreamIndex(AVMediaType type);
	void	GetThumbnail(const QString& path, std::shared_ptr<SMediaInfo>& info);

	void PicTrans(const QString& path, int width, int height, std::map<int, QString>& OptFilePaths, AVFrame* frame);
	int GetOptSize(int nWidth, int nHeight);
	void TransSize(int& nWidth, int& nHeight, int type)const;

protected:
	const int PIC_MAX_SIZE = 16384;

	AVFormatContext*	m_avFmtCtx;
	AVCodecContext*		m_videoCodecCtx;
	AVCodecParameters*	m_videoParam;
	int					m_videoIndex;
};