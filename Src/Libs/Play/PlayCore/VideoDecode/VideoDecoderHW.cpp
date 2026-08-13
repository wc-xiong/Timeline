#include "VideoDecoderHW.h"
#include <algorithm>

#ifndef Q_UNLIKELY
#define Q_UNLIKELY(x) (!!(x))
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

namespace PlayCore
{
	static enum AVPixelFormat ffmpeg_get_va_format(
		AVCodecContext *c
		, const enum AVPixelFormat * ff)
	{
		CVideoDecoderHW *va = (CVideoDecoderHW*)c->opaque;
		AVPixelFormat fmt = va->getFormat(c, ff);
		return fmt;
	}

	typedef struct ffmpeg_va_ref_t {
		CVideoDecoderHW *va;
		void *opaque;
	} ffmpeg_va_ref_t;

	static void ffmpeg_release_va_buffer2(void *opaque, uint8_t *data)
	{
		ffmpeg_va_ref_t *ref = (ffmpeg_va_ref_t*)opaque;
		ref->va->releaseBuffer(ref->opaque, data);
		delete ref;
		ref = nullptr;
	}

	static int ffmpeg_get_va_buffer2(
		struct AVCodecContext *ctx
		, AVFrame *frame, int flags)
	{
		Q_UNUSED(flags);
		if (!ctx)
			return -1;

		for (unsigned i = 0; i < AV_NUM_DATA_POINTERS; i++) {
			frame->data[i] = nullptr;
			frame->linesize[i] = 0;
			frame->buf[i] = nullptr;
		}

		CVideoDecoderHW *va = (CVideoDecoderHW*)ctx->opaque;
		if (!va || !va->getBuffer(&frame->opaque, &frame->data[0]))
		{
			std::cout << "va->getBuffer failed" << std::endl;
			return -1;
		}
		ffmpeg_va_ref_t *ref = new ffmpeg_va_ref_t;
		ref->va = va;
		ref->opaque = frame->opaque;
		frame->data[3] = frame->data[0];
		frame->buf[0] = av_buffer_create(frame->data[0], 0
			, ffmpeg_release_va_buffer2, ref, 0);
		if (Q_UNLIKELY(!frame->buf[0]))
		{
			ffmpeg_release_va_buffer2(ref, frame->data[0]);
			return -1;
		}
		Q_ASSERT(frame->data[0] != nullptr);
		return 0;
	}

	CVideoDecoderHW::CVideoDecoderHW()
		: get_format(nullptr)
		, get_buffer(nullptr)
		, release_buffer(nullptr)
		, reget_buffer(nullptr)
		, get_buffer2(nullptr)
		, threads(0)
		, copy_mode(CVideoDecoderHW::OptimizedCopy)
		, hw_w(0)
		, hw_h(0)
		, hw_profile(0)
		, defaultThreadCount(0)
		, defaultThreadSafeCallbacks(0)
	{

	}

	CVideoDecoderHW::~CVideoDecoderHW()
	{
		int i = 0;
		int j = 4;
	}

	void CVideoDecoderHW::setThreads(int value)
	{
		if (this->threads == value)
		{
			return;
		}
		this->threads = value;
		if (codec_ctx)
		{
			av_opt_set_int(codec_ctx, "threads", (int64_t)value, 0);
		}
	}

	int CVideoDecoderHW::getThreads() const
	{
		return threads;
	}

	void CVideoDecoderHW::setCopyMode(CopyMode value)
	{
		if (copy_mode == value)
		{
			return;
		}
		copy_mode = value;
	}

	CVideoDecoderHW::CopyMode CVideoDecoderHW::copyMode() const
	{
		return copy_mode;
	}

	bool CVideoDecoderHW::prepare()
	{
		defaultThreadSafeCallbacks = codec_ctx->thread_safe_callbacks;
		codec_ctx->opaque = this;
		pixfmt = codec_ctx->pix_fmt;
		get_format = codec_ctx->get_format;
		get_buffer2 = codec_ctx->get_buffer2;

		codec_ctx->thread_count = 1;
		codec_ctx->get_format = ffmpeg_get_va_format;
		codec_ctx->get_buffer2 = ffmpeg_get_va_buffer2;
		codec_ctx->thread_safe_callbacks = 1;
		return true;
	}

	void CVideoDecoderHW::restore()
	{
		codec_ctx->thread_count = defaultThreadCount;
		codec_ctx->thread_safe_callbacks = defaultThreadSafeCallbacks;
		codec_ctx->pix_fmt = pixfmt;
		codec_ctx->opaque = nullptr;
		codec_ctx->get_format = get_format;
		codec_ctx->get_buffer2 = get_buffer2;
	}

	AVPixelFormat CVideoDecoderHW::getFormat(struct AVCodecContext *avctx, const AVPixelFormat *pi_fmt)
	{
		bool can_hwaccel = false;

		const enum AVPixelFormat *p;
		for(p=pi_fmt; *p != -1;p++)
		{
			const AVPixFmtDescriptor *dsc = av_pix_fmt_desc_get(*p/*pi_fmt[i]*/);
			if (nullptr == dsc)
			{
				continue;
			}
			bool hwaccel = 0 != (dsc->flags & AV_PIX_FMT_FLAG_HWACCEL);
			//std::string prefix = hwaccel ? "hard" : "soft";
			//std::cout << "available " << prefix << "ware decoder output format "
			//	<< *p << " (" << dsc->name << ")" << std::endl;
			if (hwaccel)
			{
				can_hwaccel = true;
				break;
			}
		}
		if (!can_hwaccel)
		{
			std::cout << "hardware acceleration is not available" << std::endl; 
			return avcodec_default_get_format(avctx, pi_fmt);
		}
		//for (size_t i = 0; pi_fmt[i] != AV_PIX_FMT_NONE; i++)
		for (p = pi_fmt; *p != -1; p++)
		{
			if (vaPixelFormat() != *p)
			{
				continue;
			}
			if (hw_w == codedWidth((avctx)) && hw_h == codedHeight(avctx)
				&& hw_profile == avctx->profile // update decoder if profile changed. but now only surfaces are updated
				&& avctx->hwaccel_context)
			{
				return *p;
			}
			// TODO: manage uswc here for x86 (surface size is decoder dependent)
			avctx->hwaccel_context = setup(avctx);
			if (!avctx->hwaccel_context)
			{
				std::cout << "acceleration setup failure" << std::endl;
				break;
			}
			hw_w = codedWidth((avctx));
			hw_h = codedHeight(avctx);
			hw_profile = avctx->profile;
			//std::cout << "Using " << description << " for hardware decoding." << std::endl;
			return *p;
		}
		close();
		std::cout << "hardware acceleration is not available" << std::endl;//纭欢鍔犻€熶笉鍙敤
		return avcodec_default_get_format(avctx, pi_fmt);
	}

	int CVideoDecoderHW::codedWidth(AVCodecContext *avctx) const
	{
		if (avctx->coded_width > 0)
		{
			return avctx->coded_width;
		}
		return avctx->width;
	}

	int CVideoDecoderHW::codedHeight(AVCodecContext *avctx) const
	{
		if (avctx->coded_height > 0)
		{
			return avctx->coded_height;
		}
		return avctx->height;
	}

}