#pragma once

#include "VideoDecoder.h"

namespace PlayCore
{
	class CVideoDecoderHW : public CVideoDecoder
	{
	public:
		enum CopyMode {
			ZeroCopy,
			OptimizedCopy,
			GenericCopy
		};
		CVideoDecoderHW();
		virtual ~CVideoDecoderHW();
		int getThreads() const;
		void setThreads(int value);
		void setCopyMode(CopyMode value);
		CopyMode copyMode() const;
		virtual VideoDecoderId id() const = 0;

		bool enableFrameRef() const Q_DECL_OVERRIDE { return false; }
		virtual bool prepare();
		virtual void restore();
		AVPixelFormat getFormat(struct AVCodecContext *p_context, const AVPixelFormat *pi_fmt);

		virtual void* setup(AVCodecContext* avctx) = 0;
		virtual bool getBuffer(void **opaque, uint8_t **data) = 0;
		virtual void releaseBuffer(void *opaque, uint8_t *data) = 0;
		virtual AVPixelFormat vaPixelFormat() const = 0;

		int codedWidth(AVCodecContext *avctx) const;
		int codedHeight(AVCodecContext *avctx) const;

		AVPixelFormat(*get_format)(struct AVCodecContext *s, const AVPixelFormat * fmt);
		int(*get_buffer)(struct AVCodecContext *c, AVFrame *pic);
		void(*release_buffer)(struct AVCodecContext *c, AVFrame *pic);
		int(*reget_buffer)(struct AVCodecContext *c, AVFrame *pic);
		int(*get_buffer2)(struct AVCodecContext *s, AVFrame *frame, int flags);

		AVPixelFormat pixfmt;
		std::string description;
		int threads;
		CVideoDecoderHW::CopyMode copy_mode;
		int defaultThreadCount;
		int defaultThreadSafeCallbacks;

	private:
		int hw_w;
		int hw_h;
		int hw_profile;
	};
}
