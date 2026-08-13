#include "VideoDecoder.h"

#include "VideoDecoderD3D11.h"
#include "VideoDecoderSW.h"

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

	VideoDecoderId VideoDecoderId_SW = 1;
	VideoDecoderId VideoDecoderId_CUDA = 2;
	VideoDecoderId VideoDecoderId_DXVA = 3;
	VideoDecoderId VideoDecoderId_D3D11 = 4;

	QStringList CVideoDecoder::supportedCodecs()
	{
		static QStringList codecs;
		if (!codecs.isEmpty())
		{
			return codecs;
		}

		const AVCodec* c = NULL;
		while ((c = av_codec_iterate((void**)c))) {
			if (!av_codec_is_decoder(c) || c->type != AVMEDIA_TYPE_VIDEO)
				continue;
			codecs.append(QString::fromLatin1(c->name));
		}
		return codecs;
	}

	CVideoDecoder* CVideoDecoder::create(VideoDecoderId id)
	{
		switch (id)
		{
		case 1:
			return new CVideoDecoderSW();
		case 4:
			return new CVideoDecoderD3D11();
		default:
			break;
		}
		return nullptr;
	}

	std::string CVideoDecoder::name() const
	{
		return "";
		//return CVideoDecoder::name();
	}


	CVideoDecoder::CVideoDecoder()
		: width(0)
		, height(0)
	{

	}

	void CVideoDecoder::setGpuIndex(unsigned int gpuIndex)
	{
		m_gpuIndex = gpuIndex;
	}
	CVideoDecoder::~CVideoDecoder()
	{
	}
}