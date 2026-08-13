#include "MediaInfoDemuxer.h"
#include "PixelConverter.h"

#include <codecvt>

#include <iostream>
#include <algorithm>
#include <QFile>

#include "Common/Common/CommonFunctionFileOp.h"

#include "Common/CommonFunctionApi.h"
#include "Common/TexFrameCommon.h"
#include "UICommon/UICommonSizeDefine.h"
#include "Common/Common/AVCommon.h"
#include "Common/Common/ResourceDefine.h"
#include "Common/Common/MediaCommon.h"
#include "Common/Common/VideoDataCommon.h"

#include <QDir>
#include <QCryptographicHash>
#include "Exif.h"

extern "C"
{
#include <libavutil/eval.h>
#include <libavutil/display.h>
#include <libavutil/opt.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
}

#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "delayimp.lib")

#include <opencv2\opencv.hpp>
#include <opencv2\imgproc\imgproc.hpp>

#ifdef _DEBUG
#pragma comment(lib,"opencv_world450d.lib")
#else
#pragma comment(lib,"opencv_world450.lib")
#endif

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

//#ifdef Q_CC_MSVC
//#pragma execution_character_set("utf-8")
//#endif

// 视频解析
CVideoDemuxer::CVideoDemuxer() : m_avFmtCtx(nullptr), m_bHasAudio(false), m_videoCodecCtx(nullptr)
, m_audioCodecCtx(nullptr), m_videoParam(nullptr), m_audioParam(nullptr)
, m_videoIndex(AVERROR_STREAM_NOT_FOUND), m_audioIndex(AVERROR_STREAM_NOT_FOUND)
, m_hwFormat(-1), m_nRotateAngle(0), m_bTryFilter(false), m_pGraph(nullptr)
, m_pInContext(nullptr), m_pContext(nullptr), m_pOutContext(nullptr), m_nFilterType(0)
, m_bSupportHDR(false)
{
}

CVideoDemuxer::~CVideoDemuxer()
{
	if (m_pInContext)
	{
		avfilter_free(m_pInContext);
		m_pInContext = NULL;
	}
	if (m_pContext)
	{
		avfilter_free(m_pContext);
		m_pContext = NULL;
	}

	if (m_pOutContext)
	{
		avfilter_free(m_pOutContext);
		m_pOutContext = NULL;
	}

	if (m_pGraph)
		avfilter_graph_free(&m_pGraph);

	if (m_videoCodecCtx)
	{
		avcodec_flush_buffers(m_videoCodecCtx);
		avcodec_free_context(&m_videoCodecCtx);
	}

	if (m_audioCodecCtx)
		avcodec_free_context(&m_audioCodecCtx);

	if (m_avFmtCtx)
	{
		if (m_avFmtCtx->pb)
		{
			av_freep(&(m_avFmtCtx->pb->buffer));
			avio_closep(&m_avFmtCtx->pb);
			avio_context_free(&m_avFmtCtx->pb);
		}

		avformat_close_input(&m_avFmtCtx);
		avformat_free_context(m_avFmtCtx);
	}
}

bool CVideoDemuxer::Open(const std::string &path)
{
	AVDictionary* opts = nullptr;
	if (av_dict_set(&opts, "probesize", "20000000", AV_DICT_MATCH_CASE) < 0)
		return false;

	if (av_dict_set(&opts, "seek2any", "1", AV_DICT_MATCH_CASE) < 0)
		return false;

	if ((avformat_open_input(&m_avFmtCtx, path.c_str(), nullptr, &opts)) < 0)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	av_format_inject_global_side_data(m_avFmtCtx);

	if (avformat_find_stream_info(m_avFmtCtx, nullptr) < 0)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	return true;
}

bool CVideoDemuxer::Init(const std::string &path)
{
	if (!Open(path))
		return false;

	m_videoIndex = FindStreamIndex(AVMEDIA_TYPE_VIDEO);
	if (m_videoIndex == AVERROR_STREAM_NOT_FOUND)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	// 音频信息
	m_audioIndex = FindStreamIndex(AVMEDIA_TYPE_AUDIO);
	if (m_audioIndex != AVERROR_STREAM_NOT_FOUND)
	{
		AVStream* audioStream = m_avFmtCtx->streams[m_audioIndex];
		if (audioStream)
		{
			const AVCodec* codec = avcodec_find_decoder(audioStream->codecpar->codec_id);
			if (codec)
			{
				m_bHasAudio = true;
				m_audioCodecCtx = avcodec_alloc_context3(codec);
				m_audioParam = audioStream->codecpar;
				avcodec_parameters_to_context(m_audioCodecCtx, m_audioParam);
			}
		}
	}

	// 视频信息
	AVStream* videoStream = m_avFmtCtx->streams[m_videoIndex];
	if (!videoStream)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	m_videoParam = videoStream->codecpar;
	if (!m_videoParam)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	const AVCodec* codec = avcodec_find_decoder(m_videoParam->codec_id);
	if (!codec)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	m_videoCodecCtx = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(m_videoCodecCtx, m_videoParam);
	avcodec_open2(m_videoCodecCtx, codec, nullptr);

	av_seek_frame(m_avFmtCtx, -1, videoStream->start_time, AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);

	if (videoStream->codecpar->color_primaries == AVCOL_PRI_BT2020)
	{
		if (videoStream->codecpar->color_trc == AVCOL_TRC_SMPTE2084 || videoStream->codecpar->color_trc == AVCOL_TRC_ARIB_STD_B67)
			m_bSupportHDR = true;
	}

	return true;
}

bool CVideoDemuxer::Demuxer(std::shared_ptr<SResFileDemuxInfo>& baseInfo)
{
	auto info = std::dynamic_pointer_cast<SMediaInfo>(baseInfo);
	if (!info)
		return false;

	if (!Init(info->path))
		return false;

	// 异常视频，视频时长无效
	if (m_avFmtCtx->duration == AV_NOPTS_VALUE)
		return false;

	std::string path = info->path;

	//int64_t ms = (m_avFmtCtx->duration/* / AV_TIME_BASE*/) * 1000 + ((m_avFmtCtx->duration % AV_TIME_BASE) * 100) / AV_TIME_BASE;

	m_nRotateAngle = GetRotateAngle();

	int64_t ms = m_avFmtCtx->duration / 1000;
	info->duration = ms;
	if (m_nRotateAngle == 90 || m_nRotateAngle == 270)
	{
		info->width = m_videoParam->height;
		info->height = m_videoParam->width;
	}
	else
	{
		info->width = m_videoParam->width;
		info->height = m_videoParam->height;
	}
	info->name = path.substr(0, path.find_last_of("."));
	info->strFormat = m_avFmtCtx->iformat->long_name;

	info->videoBitrate = m_avFmtCtx->bit_rate;
	info->vidioFormat = avcodec_get_name(m_videoParam->codec_id);
	info->nColorRange == (m_videoParam->color_range == AVCOL_RANGE_JPEG) ? CR_Full : CR_Limit;

	if (m_avFmtCtx->streams[m_videoIndex]->avg_frame_rate.den != 0)
		info->videoFramerate = m_avFmtCtx->streams[m_videoIndex]->avg_frame_rate.num / (double)m_avFmtCtx->streams[m_videoIndex]->avg_frame_rate.den;
	else
		info->videoFramerate = m_avFmtCtx->streams[m_videoIndex]->r_frame_rate.num / (double)m_avFmtCtx->streams[m_videoIndex]->r_frame_rate.den;

	if (m_audioParam)
	{
		info->audiFormat = avcodec_get_name(m_audioParam->codec_id);
		info->audioSamplerate = m_audioParam->sample_rate;
		info->audioChannels = m_audioParam->channels;
	}

	info->bSupportHWDEC = IsSupportHWDecode();
	info->m_bSupportHDR = m_bSupportHDR;

	QFile file(QString::fromStdString(info->path));
	if (file.open(QIODevice::ReadOnly))
		info->filesize = file.size();
	file.close();

	info->pixmap = GetThumbnail(QString::fromStdString(path));

	return true;
}

int CVideoDemuxer::FindStreamIndex(AVMediaType type)
{
	return av_find_best_stream(m_avFmtCtx, type, -1, -1, 0, 0);
}

bool CVideoDemuxer::IsSupportHWDecode()
{
	const AVCodec* decoder = NULL;
	int nIndex = av_find_best_stream(m_avFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
	if (m_videoIndex != nIndex)
		return false;

	enum AVHWDeviceType type = AV_HWDEVICE_TYPE_D3D11VA;
	for (int i = 0;; i++)
	{
		const AVCodecHWConfig *config = avcodec_get_hw_config(decoder, i);
		if (!config)
			return false;

		if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == type)
		{
			m_hwFormat = config->pix_fmt;
			break;
		}
	}

	return true;
}

QPixmap CVideoDemuxer::GetThumbnail(const QString& path)
{
	QString tempPath = GetImageThumbnailPath();
	QByteArray md5 = QCryptographicHash::hash(path.toLocal8Bit(), QCryptographicHash::Md5);
	QString MD5 = QString::fromLocal8Bit(md5.toHex());

	QString name = path.right(path.length() - path.lastIndexOf('/') - 1);
	name = name.left(name.lastIndexOf('.'));
	name = name + MD5 + QString(".png");
	QString pathtemp = tempPath + name;

	QFileInfo file(pathtemp);
	if (file.isFile())
	{
		QImage image(pathtemp);
		return QPixmap::fromImage(image);
	}


	if (m_nRotateAngle == 90 || m_nRotateAngle == 270)
		m_nFilterType |= FILTER_ROTATE;

	if (!m_bTryFilter && m_nFilterType != FILETER_NONE)
	{
		InitFilter();
		m_bTryFilter = true;
	}

	AVPacket* packet = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();
	frame->key_frame = 0;

	QPixmap pix;
	int ret = -1;

	int64_t maxWaitTime = 5000;

	int64_t begin = av_gettime();
	int64_t end = av_gettime();
	int nKeyFrameIndex = 2;
	while ((end - begin) < maxWaitTime && (nKeyFrameIndex != 0))
	{
		if (!frame)
			frame = av_frame_alloc();

		do {
			ret = av_read_frame(m_avFmtCtx, packet);
			if (ret < 0)
				break;

			if (packet->stream_index != m_videoIndex)
				continue;

			ret = avcodec_send_packet(m_videoCodecCtx, packet);
			if (ret < 0)
				break;

		} while (avcodec_receive_frame(m_videoCodecCtx, frame));

		if (ret == AVERROR_EOF)
			break;

		if (ret >= 0 && frame->key_frame == 1)
		{
			nKeyFrameIndex--;
		}

		if (m_pGraph && ret >= 0)
		{
			int nResult = av_buffersrc_add_frame(m_pInContext, frame);
			if (nResult < 0)
			{
				continue;
			}

			nResult = av_buffersink_get_frame_flags(m_pOutContext, frame, 0);
			if (nResult < 0)
				continue;
		}

		frame->key_frame = 0;
		end = av_gettime();
	}

	av_packet_free(&packet);

	if (ret < 0)
	{
		av_frame_free(&frame);
		QImage image(":/dark/images/ResourceWnd/Thumbmail/Normal/Video.svg");
		image.save(pathtemp);
		return QPixmap();
	}

	if (!frame || !frame->data[0])
	{
		QImage image(":/dark/images/ResourceWnd/Thumbmail/Normal/Video.svg");
		image.save(pathtemp);
		return QPixmap();
	}

	CPixelConverter converter;
	AVFrame* dstFrame = converter.Convert(frame, Res_ThumbnailWidth, Res_ThumbnailHeight, AV_PIX_FMT_RGBA);
	av_frame_free(&frame);
	if (!dstFrame || !dstFrame->data[0])
	{
		QImage image(":/dark/images/ResourceWnd/Thumbmail/Normal/Video.svg");
		image.save(pathtemp);
		return QPixmap();
	}

	QImage image = QImage(dstFrame->data[0], Res_ThumbnailWidth, Res_ThumbnailHeight, QImage::Format_RGBA8888);	// QImage 使用buffer转化时，在QImage整个生命期，buffer不能被释放

	pix = QPixmap::fromImage(image);
	pix.save(pathtemp);

	av_free(dstFrame->data[0]);
	av_frame_free(&dstFrame);

	return pix;
}

int CVideoDemuxer::GetRotateAngle()
{
	AVDictionaryEntry * tag = av_dict_get(m_avFmtCtx->streams[m_videoIndex]->metadata, "rotate", NULL, 0);
	uint8_t* displaymatrix = av_stream_get_side_data(m_avFmtCtx->streams[m_videoIndex], AV_PKT_DATA_DISPLAYMATRIX, NULL);

	double theta = 0;
	if (tag && *tag->value && strcmp(tag->value, 0))
	{
		char *tail;
		theta = av_strtod(tag->value, &tail);
		if (*tail)
			theta = 0;
	}

	if (displaymatrix && !theta)
		theta = -av_display_rotation_get((int32_t*)displaymatrix);

	theta -= 360 * floor(theta / 360 + 0.9 / 360);

	int angle = 0;
	if (fabs(theta - 90) < 1.0)
		angle = 90;
	else if (fabs(theta - 180) < 1.0 || fabs(theta + 180) < 1.0)
		angle = 180;
	else if (fabs(theta - 270) < 1.0 || fabs(theta + 90) < 1.0)
		angle = 270;

	return angle;
}

void CVideoDemuxer::InitFilter()
{
	char sz[256];
	int nResult;
	AVPixelFormat pix_fmts[] = { m_videoCodecCtx->pix_fmt, AV_PIX_FMT_NONE };

	do
	{
		m_pGraph = avfilter_graph_alloc();
		if (!m_pGraph)
		{
			break;
		}

		sprintf_s(
			sz,
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d",
			m_videoCodecCtx->width,
			m_videoCodecCtx->height,
			m_videoCodecCtx->pix_fmt,
			m_avFmtCtx->streams[m_videoIndex]->time_base.num,
			m_avFmtCtx->streams[m_videoIndex]->time_base.den);

		const AVFilter *src_filter = avfilter_get_by_name("buffer");
		nResult = avfilter_graph_create_filter(
			&m_pInContext,
			src_filter,
			"in",
			sz,
			NULL,
			m_pGraph);
		if (nResult < 0)
		{
			char sz[256];
			av_strerror(nResult, sz, 256);
			break;
		}

		nResult = avfilter_graph_create_filter(
			&m_pOutContext,
			avfilter_get_by_name("buffersink"),
			"ou",
			NULL,
			NULL,
			m_pGraph);
		if (nResult < 0)
		{
			break;
		}

		nResult = av_opt_set_int_list(
			m_pOutContext,
			"pix_fmts",
			pix_fmts,
			AV_PIX_FMT_NONE,
			AV_OPT_SEARCH_CHILDREN);
		if (nResult < 0)
		{
			break;
		}

		AVFilterInOut *inputs = avfilter_inout_alloc();
		AVFilterInOut *ouputs = avfilter_inout_alloc();

		inputs->name = av_strdup("out");
		inputs->filter_ctx = m_pOutContext;
		inputs->pad_idx = 0;
		inputs->next = NULL;

		ouputs->name = av_strdup("in");
		ouputs->filter_ctx = m_pInContext;
		ouputs->pad_idx = 0;
		ouputs->next = NULL;

		std::string strfilter_desc;
		if (m_nFilterType & FILTER_INTERLACED)
			strfilter_desc = "yadif=mode=send_frame:parity=auto:deint=interlaced";
		if (m_nFilterType & FILTER_ROTATE)
		{
			std::string strFilterRotate;
			if (m_nRotateAngle == 270)
				strFilterRotate = "transpose=dir=cclock";
			else if (m_nRotateAngle == 90)
				strFilterRotate = "transpose=dir=clock";

			if (strfilter_desc.empty())
				strfilter_desc = strFilterRotate;
			else
				strfilter_desc = strfilter_desc + "," + strFilterRotate;
		}

		nResult = avfilter_graph_parse_ptr(m_pGraph, strfilter_desc.c_str(), &inputs, &ouputs, NULL);
		if (nResult < 0)
		{
			break;
		}

		nResult = avfilter_graph_config(m_pGraph, NULL);
		if (nResult < 0)
		{
			break;
		}

		return;
	} while (FALSE);

	avfilter_free(m_pInContext);
	m_pInContext = NULL;
	avfilter_free(m_pContext);
	m_pContext = NULL;
	avfilter_free(m_pOutContext);
	m_pOutContext = NULL;
	avfilter_graph_free(&m_pGraph);
}

// 音频解析
CAudioDemuxer::CAudioDemuxer() : m_avFmtCtx(nullptr), m_audioParam(nullptr)
{

}

CAudioDemuxer::~CAudioDemuxer()
{
	if (m_avFmtCtx)
	{
		if (m_avFmtCtx->pb)
		{
			av_freep(&(m_avFmtCtx->pb->buffer));
			avio_closep(&m_avFmtCtx->pb);
			avio_context_free(&m_avFmtCtx->pb);
		}

		avformat_close_input(&m_avFmtCtx);
		avformat_free_context(m_avFmtCtx);
	}
}

bool CAudioDemuxer::Demuxer(std::shared_ptr<SResFileDemuxInfo>& baseInfo)
{
	auto info = std::dynamic_pointer_cast<SMediaInfo>(baseInfo);
	if (!info)
		return false;

	if (!Init(info->path))
		return false;

	std::string path = info->path;

	//int64_t ms = (m_avFmtCtx->duration / AV_TIME_BASE) * 1000 + ((m_avFmtCtx->duration % AV_TIME_BASE) * 100) / AV_TIME_BASE;
	int64_t ms = m_avFmtCtx->duration / 1000;
	info->duration = ms;
	info->name = path.substr(0, path.find_last_of("."));
	info->pixmap = GetThumbnail();
	info->audioChannels = m_audioParam->channels;
	info->audiFormat = avcodec_get_name(m_audioParam->codec_id);
	info->width = THUMBNAIL_AUDIO_WIDTH;
	info->height = THUMBNAIL_AUDIO_HEIGHT;

	info->audioSamplerate = m_audioParam->sample_rate;

	QFile file(QString::fromStdString(info->path));
	if (file.open(QIODevice::ReadOnly))
		info->filesize = file.size();
	file.close();

	return true;
}

bool CAudioDemuxer::Init(const std::string& path)
{
	m_avFmtCtx = avformat_alloc_context();
	if (avformat_open_input(&m_avFmtCtx, path.c_str(), nullptr, nullptr) < 0)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	if (avformat_find_stream_info(m_avFmtCtx, nullptr) < 0)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	int audioIndex = FindStreamIndex(AVMEDIA_TYPE_AUDIO);
	if (audioIndex == AVERROR_STREAM_NOT_FOUND)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	m_audioParam = m_avFmtCtx->streams[audioIndex]->codecpar;
	return true;
}

int CAudioDemuxer::FindStreamIndex(AVMediaType type)
{
	return av_find_best_stream(m_avFmtCtx, type, -1, -1, 0, 0);
}

QPixmap CAudioDemuxer::GetThumbnail()
{
	return QPixmap();
}

// 图片解析
CPictureDemuxer::CPictureDemuxer() : m_avFmtCtx(nullptr), m_videoCodecCtx(nullptr), m_videoParam(nullptr)
, m_videoIndex(AVERROR_STREAM_NOT_FOUND)
{
}

CPictureDemuxer::~CPictureDemuxer()
{
	if (m_avFmtCtx)
	{
		if (m_avFmtCtx->pb)
		{
			av_freep(&(m_avFmtCtx->pb->buffer));
			avio_closep(&m_avFmtCtx->pb);
			avio_context_free(&m_avFmtCtx->pb);
		}

		avformat_close_input(&m_avFmtCtx);
		avformat_free_context(m_avFmtCtx);
	}
}

bool CPictureDemuxer::Demuxer(std::shared_ptr<SResFileDemuxInfo>& baseInfo)
{
	auto info = std::dynamic_pointer_cast<SMediaInfo>(baseInfo);
	if (!info)
		return false;

	QString path = QString::fromStdString(info->path);

	std::string tmpPath = info->path;
	info->name = tmpPath.substr(0, tmpPath.find_last_of("."));
	info->strFormat = tmpPath.substr(tmpPath.find_last_of(".") + 1, tmpPath.size());

	QFile file(path);
	if (file.open(QIODevice::ReadOnly))
		info->filesize = file.size();
	file.close();

	if (info->strFormat == "tiff" || info->strFormat == "tif")
	{
		cv::Mat img = cv::imread(path.toLocal8Bit().toStdString(), cv::IMREAD_UNCHANGED);
		if (!img.empty())
		{
			info->width = img.cols;
			info->height = img.rows;

			cv::Mat img1 = img.clone();
			cv::resize(img, img1, cv::Size(Res_ThumbnailWidth, Res_ThumbnailHeight));

			cv::Mat img2 = img.clone();
			cv::cvtColor(img1, img2, cv::COLOR_BGRA2RGBA);

			QImage dst((const uchar*)img2.data, img2.cols, img2.rows, (int)img2.step, QImage::Format_RGBA8888);
			dst.bits();

			info->pixmap = QPixmap::fromImage(dst);

			img.release();
			img1.release();
			img2.release();
			return true;
		}

		img.release();
	}

	{
		// 检测exif信息
		FILE *fp = _wfopen(path.toStdWString().c_str(), L"rb");
		if (!fp)
			return false;

		// 获取图片 size data
		fseek(fp, 0, SEEK_END);
		unsigned long fsize = ftell(fp);
		rewind(fp);
		unsigned char *buf = new unsigned char[fsize];
		if (fread(buf, 1, fsize, fp) != fsize)
		{
			delete[] buf;
			return false;
		}

		fclose(fp);

		easyexif::EXIFInfo exifInfo;
		int code = exifInfo.parseFrom(buf, fsize);
		delete[] buf;
		if (code)
		{
			printf("can't prasing exif info.\n");
		}
		else
		{
			info->nExifOrientation = exifInfo.Orientation;
			printf("orientation   %d.\n", exifInfo.Orientation);
		}
	}

	if (!Init(info->path))
		return false;

	info->width = m_videoParam->width;
	info->height = m_videoParam->height;
	info->nColorRange = m_videoParam->color_range;

	if (ToLowerCase(info->strFormat) == "gif")
	{
		int64_t ms = m_avFmtCtx->duration / 1000;
		info->duration = ms;
	}

	GetThumbnail(path, info);

	avcodec_free_context(&m_videoCodecCtx);

	return true;
}

bool CPictureDemuxer::Open(const std::string& path)
{
	m_avFmtCtx = avformat_alloc_context();
	int ret = 0;
	if (ret = (avformat_open_input(&m_avFmtCtx, path.c_str(), nullptr, nullptr)) < 0)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	if (avformat_find_stream_info(m_avFmtCtx, nullptr) < 0)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	return true;
}

bool CPictureDemuxer::Init(const std::string& path)
{
	if (!Open(path))
		return false;

	m_videoIndex = FindStreamIndex(AVMEDIA_TYPE_VIDEO);
	if (m_videoIndex == AVERROR_STREAM_NOT_FOUND)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	// 视频信息
	AVStream* videoStream = m_avFmtCtx->streams[m_videoIndex];
	if (!videoStream)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	m_videoParam = videoStream->codecpar;
	if (!m_videoParam)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	const AVCodec* codec = avcodec_find_decoder(m_videoParam->codec_id);
	if (!codec)
	{
		avformat_close_input(&m_avFmtCtx);
		return false;
	}

	m_videoCodecCtx = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(m_videoCodecCtx, m_videoParam);
	avcodec_open2(m_videoCodecCtx, codec, nullptr);

	return true;
}

int CPictureDemuxer::FindStreamIndex(AVMediaType type)
{
	return av_find_best_stream(m_avFmtCtx, type, -1, -1, 0, 0);
}

void CPictureDemuxer::GetThumbnail(const QString& path, std::shared_ptr<SMediaInfo>& info)
{
	QString tempPath = GetImageThumbnailPath();
	QByteArray md5 = QCryptographicHash::hash(path.toLocal8Bit(), QCryptographicHash::Md5);
	QString MD5 = QString::fromLocal8Bit(md5.toHex());

	QString name = path.right(path.length() - path.lastIndexOf('/') - 1);
	name = name.left(name.lastIndexOf('.'));
	name = name + MD5 + QString(".png");
	QString pathtemp = tempPath + name;

	QFileInfo file(pathtemp);
	if (file.isFile())
	{
		QImage image(pathtemp);
		info->pixmap = QPixmap::fromImage(image);
	}

	AVPacket* packet = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();

	int ret = -1;
	do {

		ret = av_read_frame(m_avFmtCtx, packet);
		if (ret < 0)
			break;

		if (packet->stream_index != m_videoIndex)
			continue;

		ret = avcodec_send_packet(m_videoCodecCtx, packet);
		if (ret < 0)
			break;

	} while (avcodec_receive_frame(m_videoCodecCtx, frame));

	av_packet_free(&packet);
	avcodec_flush_buffers(m_videoCodecCtx);

	if (ret < 0)
	{
		av_frame_free(&frame);
		return;
	}

	CPixelConverter converter;
	AVFrame* dstFrame = converter.Convert(frame, Res_ThumbnailWidth, Res_ThumbnailHeight, AV_PIX_FMT_RGBA);
	if (!dstFrame || !dstFrame->data[0])
	{
		av_frame_free(&frame);
		return;
	}

	// 保存缩略图
	QImage image(dstFrame->data[0], Res_ThumbnailWidth, Res_ThumbnailHeight, QImage::Format_RGBA8888);

	switch (info->nExifOrientation)
	{
	case EExifOrientation::EEO_2:
	{
		image = image.mirrored(true, false);
	}
	break;
	case EExifOrientation::EEO_3:
	{
		image = image.mirrored(true, true);
	}
	break;
	case EExifOrientation::EEO_4:
	{
		image = image.mirrored(false, true);
	}
	break;
	case EExifOrientation::EEO_5:
	{
		std::swap(info->width, info->height);

		QMatrix matrix;
		matrix.rotate(90.0);
		image = image.transformed(matrix, Qt::FastTransformation);
		image = image.mirrored(true, false);
	}
	break;
	case EExifOrientation::EEO_6:
	{
		std::swap(info->width, info->height);
		QMatrix matrix;
		matrix.rotate(90.0);
		image = image.transformed(matrix, Qt::FastTransformation);
	}
	break;
	case EExifOrientation::EEO_7:
	{
		std::swap(info->width, info->height);
		QMatrix matrix;
		matrix.rotate(90.0);
		image = image.transformed(matrix, Qt::FastTransformation);
		image = image.mirrored(false, true);
	}
	break;
	case EExifOrientation::EEO_8:
	{
		std::swap(info->width, info->height);
		QMatrix matrix;
		matrix.rotate(-90.0);
		image = image.transformed(matrix, Qt::FastTransformation);
	}
	break;
	default:
		break;
	}

	image.save(pathtemp);
	info->pixmap = QPixmap::fromImage(image);

	av_free(dstFrame->data[0]);
	av_frame_free(&dstFrame);

	// 图片优化
	PicTrans(path, m_videoParam->width, m_videoParam->height, info->OptFilePaths, frame);
	av_frame_free(&frame);
}

void CPictureDemuxer::PicTrans(const QString& path, int width, int height, std::map<int, QString>& OptFilePaths, AVFrame* frame)
{
	QFileInfo file(path);
	if (!file.isFile())
		return;

	int nMinOpt = GetOptSize(width, height);

	QString tempPath = GetAppDataPath() + "PicOptimizeCache/";
	QDir dir(tempPath);
	if (!dir.exists())
		QDir().mkpath(tempPath);

	QFile pdfFile(path);
	if (!pdfFile.exists())
		return;
	QString MD5;
	if (pdfFile.open(QIODevice::ReadOnly)) {
		// 计算文件的哈希值
		QCryptographicHash hash(QCryptographicHash::Md5);
		hash.addData(&pdfFile);

		// 将哈希值转换为十六进制字符串，作为唯一标识符
		MD5 = QString(hash.result().toHex());
	}
	pdfFile.close();

	for (int i = EPOT_1_2; i <= nMinOpt; i++)
	{
		int nWidth = width;
		int nHeight = height;

		TransSize(nWidth, nHeight, i);

		QString name = path.right(path.length() - path.lastIndexOf('/') - 1);
		name = name.left(name.lastIndexOf('.'));
		name = name + QString("_%1_%2_").arg(nWidth).arg(nHeight) + MD5 + QString(".png");
		QString pathtemp = tempPath + QString("/") + name;

		OptFilePaths[i] = name;

		QFileInfo file(pathtemp);
		if (!file.isFile() && frame)
		{
			CPixelConverter converter;
			AVFrame* dstFrame = converter.Convert(frame, nWidth, nHeight, AV_PIX_FMT_RGBA);
			if (!dstFrame || !dstFrame->data[0])
				continue;

			QImage image(dstFrame->data[0], nWidth, nHeight, QImage::Format_RGBA8888);
			image.save(pathtemp);

			av_free(dstFrame->data[0]);
			av_frame_free(&dstFrame);
		}
	}

	//自动
	if (nMinOpt > EPOT_1_1)
	{
		int nWidth = AUTO_PIC_OPT_WIDTH;
		int nHeight = AUTO_PIC_OPT_HEIGHT;

		QString name = path.right(path.length() - path.lastIndexOf('/') - 1);
		name = name.left(name.lastIndexOf('.'));
		name = name + QString("_%1_%2_").arg(nWidth).arg(nHeight) + MD5 + QString(".png");
		QString pathtemp = tempPath + QString("/") + name;

		OptFilePaths[EPOT_Auto] = name;

		QFileInfo f(pathtemp);
		if (!f.isFile() && frame)
		{
			CPixelConverter converter;
			AVFrame* dstFrame = converter.Convert(frame, nWidth, nHeight, AV_PIX_FMT_RGBA);
			if (!dstFrame || !dstFrame->data[0])
				return;

			QImage image(dstFrame->data[0], nWidth, nHeight, QImage::Format_RGBA8888);
			image.save(pathtemp);
			image = QImage();

			av_free(dstFrame->data[0]);
			av_frame_free(&dstFrame);
		}
	}
}

int CPictureDemuxer::GetOptSize(int nWidth, int nHeight)
{
	if (nWidth < AUTO_PIC_OPT_WIDTH&&nHeight < AUTO_PIC_OPT_HEIGHT)
		return  EPOT_1_1;

	nWidth = std::min(nWidth, PIC_MAX_SIZE);
	nHeight = std::min(nHeight, PIC_MAX_SIZE);

	int nOpt = EPOT_1_1;
	for (; nOpt < EPOT_Auto; nOpt++)
	{
		int nTmpWidth = nWidth >> nOpt;
		int nTmpHeight = nHeight >> nOpt;

		if (nTmpWidth <= MIN_PIC_OPT_WIDTH && nTmpHeight <= MIN_PIC_OPT_HEIGHT)
			return nOpt;
	}

	return nOpt;
}

void CPictureDemuxer::TransSize(int& nWidth, int& nHeight, int type)const
{
	if (nWidth > PIC_MAX_SIZE || nHeight > PIC_MAX_SIZE)
	{
		float radio = (float)nWidth / (float)nHeight;

		if (nWidth > nHeight)
		{
			nWidth = PIC_MAX_SIZE;
			nHeight = std::max(1, int(PIC_MAX_SIZE / radio));
		}
		else
		{
			nWidth = std::max(1, int(PIC_MAX_SIZE * radio));
			nHeight = PIC_MAX_SIZE;
		}
	}

	nWidth = nWidth >> type;
	nHeight = nHeight >> type;
}

