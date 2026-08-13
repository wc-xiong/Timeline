#include "VideoDecoderSW.h"

#define DEFAULT_DECODER_THREAD_COUNT 4

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

namespace PlayCore {
	VideoDecoderId CVideoDecoderSW::id() const
	{
		return VideoDecoderId_SW;
	}


	bool CVideoDecoderSW::openInternal()
	{
		codec_ctx->thread_count = DEFAULT_DECODER_THREAD_COUNT;
		codec_ctx->thread_type = FF_THREAD_FRAME;
		return true;
	}

	CVideoDecoderSW::CVideoDecoderSW()
	{

	}
}
