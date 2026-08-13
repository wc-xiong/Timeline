#pragma once

#include "DemuxerGlobal.h"

#include <memory>
#include <QImage>

// 图片像素格式转换
struct AVFrame;
struct SwsContext;
class DEMUXER_EXPORT CPixelConverter
{
public:
	CPixelConverter();
	virtual ~CPixelConverter();

	// 格式转换
	AVFrame* Convert(AVFrame* frame, int /*AVPixelFormat*/ dstFmt, int align = 4);
	AVFrame* Convert(AVFrame* frame, int nDstWidth, int nDstHeight, int/*AVPixelFormat*/ dstFmt, int align = 4);

	//AVFrame* Copy(std::shared_ptr<CFrame>& frame, int align = 4);
	AVFrame* Copy(AVFrame* frame, int align = 4);
	AVFrame* Copy(QImage& img, int fmt);
	AVFrame* Copy(uchar* data, int width, int height);

protected:
	/*AVPixelFormat*/int ConvertFmt(const int/*AVPixelFormat*/& fmt);

protected:
	struct  SwsContext*	m_pContext;
};

