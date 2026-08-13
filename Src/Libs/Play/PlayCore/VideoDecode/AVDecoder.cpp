#include "AVDecoder.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

static const AVCodec* get_codec(const std::string &name
    , const std::string& hwa
    , AVCodecID cid)
{
    std::string fullname(name);
    if (name.empty())
    {
        if (hwa.empty())
        {
            return avcodec_find_decoder(cid);
        }
        fullname += avcodec_get_name(cid);
        fullname += hwa;
    }

    const AVCodec *codec = avcodec_find_decoder_by_name(fullname.c_str());
    if (codec)
    {
        return codec;
    }

    const AVCodecDescriptor* cd = avcodec_descriptor_get_by_name(fullname.c_str());
    if (cd)
    {
        return avcodec_find_decoder(cd->id);
    }
    return NULL;
}


const char* CAVDecoder::getProfileName(AVCodecID id, int profile)
{
   const AVCodec *codec = avcodec_find_decoder(id);
    if (nullptr == codec)
    {
        return "Unknow";
    }
    return av_get_profile_name(codec, profile);
}


const char* CAVDecoder::getProfileName(const AVCodecContext* ctx)
{
    if (ctx->codec)
    {
        return av_get_profile_name(ctx->codec, ctx->profile);
    }
    return getProfileName(ctx->codec_id, ctx->profile);
}

CAVDecoder::CAVDecoder()
    : codec_ctx(0)
    , available(true)
    , is_open(false)
    , undecoded_size(0)
    , dict(0)
    , decoder(nullptr)
{

}

CAVDecoder::~CAVDecoder()
{
	close();
}


void CAVDecoder::setCodec(const AVCodec *codec)
{
    decoder = codec;
}

std::string CAVDecoder::name() const
{
    return std::string();
}

std::string CAVDecoder::getDescription() const
{
    return std::string();
}

void CAVDecoder::restore()
{
}

bool CAVDecoder::open()
{
    if (!codec_ctx)
    {
        printf("FFmpeg codec context not ready.\n");
        return false;
    }
    if (!openInternal())
    {
		printf("openInternal fail.\n");
		goto FAILED;
    }
    if (!getDecoderConfig())
    {
		printf("getDecoderConfig fail.\n");
		goto FAILED;
    }
    if (!isValidTexture())
    {
		printf("isValidTexture fail.\n");
		goto FAILED;
    }
    int ret = avcodec_open2(codec_ctx, decoder, nullptr);
    if (ret < 0)
    {
		printf("avcodec_open2 fail.\n");
		goto FAILED;
    }
    is_open = true;
    return true;

FAILED:
	restore();
	closeInternal();
	return false;
}

bool CAVDecoder::close()
{
    if (!isOpen())
    {
        return true;
    }
    is_open = false;

    flush();

    if (codec_ctx)
    {
        AV_ENSURE_OK(avcodec_close(codec_ctx), false);
    }
    closeInternal();
    return true;
}

bool CAVDecoder::isOpen() const
{
    return is_open;
}

void CAVDecoder::flush()
{
    if (!isAvailable())
    {
        return;
    }
    if (!isOpen())
    {
        return;
    }
    avcodec_flush_buffers(codec_ctx);
}

void CAVDecoder::setCodecContext(void *codecCtx)
{
    AVCodecContext *ctx = (AVCodecContext*)codecCtx;
    if (codec_ctx == ctx)
    {
        return;
    }

    if (isOpen())
    {
        std::cout << "Can not copy codec properties when it's open" << std::endl;
        close();
    }

    is_open = false;
    if (nullptr == ctx)
    {
        avcodec_free_context(&codec_ctx);
        codec_ctx = nullptr;
        return;
    }
    if (nullptr == codec_ctx)
    {
        codec_ctx = avcodec_alloc_context3(nullptr);
    }
    if (nullptr == codec_ctx)
    {
        std::cout << "avcodec_alloc_context3 failed" << std::endl;
        return;
    }
    //AV_ENSURE_OK(avcodec_copy_context(codec_ctx, ctx));
    codec_ctx = ctx;
}

//TODO: reset other parameters?
void* CAVDecoder::codecContext() const
{
    return codec_ctx;
}

void CAVDecoder::setCodecName(const std::string &name)
{
    if (codec_name == name)
    {
        return;
    }
    codec_name = name;
}

std::string CAVDecoder::codecName() const
{
    return codec_name;
}

bool CAVDecoder::isAvailable() const
{
    return nullptr != codec_ctx;
}

int CAVDecoder::undecodedSize() const
{
    return undecoded_size;
}