#include "PixelConverter.h"
#include "Common/IDemuxerInterface.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CPixelConverter::CPixelConverter() : m_pContext(nullptr)
{
}

CPixelConverter::~CPixelConverter()
{

}

AVFrame* CPixelConverter::Convert(AVFrame* frame, int /*AVPixelFormat*/ dstFmt, int align)
{
	if (!frame)
		return nullptr;

	return Convert(frame, frame->width, frame->height, dstFmt, align);
}

AVFrame* CPixelConverter::Convert(AVFrame* frame, int nDstWidth, int nDstHeight, int/*AVPixelFormat*/ dstFmt, int align)
{
	if (!frame || nDstWidth <=0 || nDstHeight <= 0)
		return nullptr;

	//if (nDstWidth == frame->width && nDstHeight == frame->height
	//	&& dstFmt == frame->format && frame->linesize[0] % frame->width == 0)
	//	return nullptr;

	AVFrame* dstFrame = av_frame_alloc();
	dstFrame->width = nDstWidth;
	dstFrame->height = nDstHeight;
	dstFrame->format = dstFmt;
	dstFrame->color_range = frame->color_range;
	dstFrame->color_trc = frame->color_trc;
	dstFrame->colorspace = frame->colorspace;
	
	AVPixelFormat srcPixFmt = (AVPixelFormat)ConvertFmt(/*(AVPixelFormat)*/frame->format);

	m_pContext = sws_getContext(frame->width, frame->height, srcPixFmt/*frame->format*/, nDstWidth, nDstHeight, (AVPixelFormat)dstFmt, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
	if (!m_pContext)
		return nullptr;

	av_image_alloc(dstFrame->data, dstFrame->linesize, nDstWidth, nDstHeight, (AVPixelFormat)dstFmt, align);

	int ret = sws_scale(m_pContext, (const uint8_t* const*)frame->data, frame->linesize, 0, frame->height, dstFrame->data, dstFrame->linesize);
	if (ret < 0)
	{
		if (dstFrame->data[0])
			av_free(dstFrame->data[0]);

		av_frame_free(&dstFrame);
		sws_freeContext(m_pContext);
		return nullptr;
	}

	sws_freeContext(m_pContext);
	return dstFrame;
}

AVFrame* CPixelConverter::Copy(AVFrame* frame, int align)
{
	if (!frame)
		return nullptr;

	AVFrame* dstFrame = av_frame_alloc();

	dstFrame->width = frame->width;
	dstFrame->height = frame->height;

	dstFrame->format = (AVPixelFormat)frame->format;
	av_image_alloc(dstFrame->data, frame->linesize, frame->width, frame->height, (AVPixelFormat)frame->format, align);

	av_frame_copy(dstFrame, frame);

	return dstFrame;
}

AVFrame * CPixelConverter::Copy(QImage& img, int fmt)
{
	if (img.isNull())
		return nullptr;

	int width = img.width();
	int height = img.height();

	AVFrame* dstFrame = av_frame_alloc();
	dstFrame->width = width;
	dstFrame->height = height;
	dstFrame->format = AV_PIX_FMT_RGBA;

	av_frame_get_buffer(dstFrame, 4);
	memcpy(dstFrame->data[0], img.bits(), width* height * 4);

	return dstFrame;
}

AVFrame * CPixelConverter::Copy(uchar* data, int width, int height)
{
	if (data == nullptr)
		return nullptr;

	AVFrame* dstFrame = av_frame_alloc();
	dstFrame->width = width;
	dstFrame->height = height;
	dstFrame->format = AV_PIX_FMT_BGRA;

	av_frame_get_buffer(dstFrame, 0);
	memcpy(dstFrame->data[0], data, width* height * 4);

	return dstFrame;
}

/*AVPixelFormat*/int CPixelConverter::ConvertFmt(const int/*AVPixelFormat*/ & fmt)
{
	int cvtFmt;
	switch (fmt)
	{
	case AV_PIX_FMT_YUVJ420P:
		cvtFmt = AV_PIX_FMT_YUV420P;
		break;
	case AV_PIX_FMT_YUVJ422P:
		cvtFmt = AV_PIX_FMT_YUV422P;
		break;
	case AV_PIX_FMT_YUVJ440P:
		cvtFmt = AV_PIX_FMT_YUV440P;
		break;
	case AV_PIX_FMT_YUVJ444P:
		cvtFmt = AV_PIX_FMT_YUV444P;
		break;
	default:
		cvtFmt = fmt;//AV_PIX_FMT_YUV420P
		break;
	}
	return cvtFmt;
}
