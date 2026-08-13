#pragma once
#include <unordered_map>
#include "Common/TexFrameCommon.h"
#include "Common/VideoDataCommon.h"
#include "Common/DxEnsure.h"
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
}

#include "AVCommon.h"

struct SPixTex
{
	DXGI_FORMAT dxgiFormat;
	int			textType;
	int			colorBit;
};

static const std::unordered_map<AVPixelFormat, SPixTex> PIX_TEX = {
	{AV_PIX_FMT_RGB0, {DXGI_FORMAT_R8G8B8A8_UNORM, ETT_RGB0, 8}},
	{AV_PIX_FMT_RGBA, {DXGI_FORMAT_R8G8B8A8_UNORM, ETT_RGBA, 8}},
	{AV_PIX_FMT_BGRA, {DXGI_FORMAT_B8G8R8A8_UNORM, ETT_BGRA, 8}},
	{AV_PIX_FMT_BGR0, {DXGI_FORMAT_B8G8R8A8_UNORM, ETT_BGR0, 8}},

	{AV_PIX_FMT_ARGB, {DXGI_FORMAT_R8G8B8A8_UNORM, ETT_ARGB, 8}},
	{AV_PIX_FMT_0RGB, {DXGI_FORMAT_R8G8B8A8_UNORM, ETT_0RGB, 8}},
	{AV_PIX_FMT_ABGR, {DXGI_FORMAT_R8G8B8A8_UNORM, ETT_ABGR, 8}},
	{AV_PIX_FMT_0BGR, {DXGI_FORMAT_R8G8B8A8_UNORM, ETT_0BGR, 8}},

	{AV_PIX_FMT_YUV420P, {DXGI_FORMAT_R8_UNORM, ETT_YUV, 8}},
	{AV_PIX_FMT_YUV422P, {DXGI_FORMAT_R8_UNORM, ETT_YUV, 8}},
	{AV_PIX_FMT_YUV444P, {DXGI_FORMAT_R8_UNORM, ETT_YUV, 8}},

	{AV_PIX_FMT_YUV420P10, {DXGI_FORMAT_R16_UNORM, ETT_YUV, 10}},
	{AV_PIX_FMT_YUV422P10, {DXGI_FORMAT_R16_UNORM, ETT_YUV, 10}},
	{AV_PIX_FMT_YUV444P10, {DXGI_FORMAT_R16_UNORM, ETT_YUV, 10}},

	{AV_PIX_FMT_YUV420P12, {DXGI_FORMAT_R16_UNORM, ETT_YUV, 12}},
	{AV_PIX_FMT_YUV422P12, {DXGI_FORMAT_R16_UNORM, ETT_YUV, 12}},
	{AV_PIX_FMT_YUV444P12, {DXGI_FORMAT_R16_UNORM, ETT_YUV, 12}},

	{AV_PIX_FMT_YUV420P14, {DXGI_FORMAT_R16_UNORM, ETT_YUV, 14}},
	{AV_PIX_FMT_YUV422P14, {DXGI_FORMAT_R16_UNORM, ETT_YUV, 14}},
	{AV_PIX_FMT_YUV444P14, {DXGI_FORMAT_R16_UNORM, ETT_YUV, 14}},

	{AV_PIX_FMT_YUV420P16, {DXGI_FORMAT_R16_UNORM, ETT_YUV, 16}},
	{AV_PIX_FMT_YUV422P16, {DXGI_FORMAT_R16_UNORM, ETT_YUV, 16}},
	{AV_PIX_FMT_YUV444P16, {DXGI_FORMAT_R16_UNORM, ETT_YUV, 16}},
};

static int TransColorSpace(const AVColorSpace&  clrSpace)
{
	if (AVCOL_SPC_BT2020_NCL == clrSpace || AVCOL_SPC_BT2020_CL == clrSpace)
		return CS_2020;
	else if (AVCOL_SPC_BT470BG == clrSpace || AVCOL_SPC_SMPTE170M == clrSpace)
		return CS_601;
	else
		return CS_709;
}

static int TransColorRange(const AVColorRange&  clrRange)
{
	return (clrRange == AVCOL_RANGE_JPEG) ? CR_Full : CR_Limit;
}

static int TransColorTrc(const AVColorTransferCharacteristic& clrTrc)
{
	if (clrTrc == AVCOL_TRC_SMPTE2084)
		return CG_HDR10;
	else if (clrTrc == AVCOL_TRC_ARIB_STD_B67 || clrTrc == AVCOL_TRC_BT2020_10)
		return CG_HLG;
	else
		return CG_SDR;
}

static void	GetVideoProperty(AVFrame *frame, int nDecodeType, Video_ClrMetedata& pty)
{
	if (!frame)
		return;

	pty.color_range = TransColorRange(frame->color_range);
	pty.color_gamma = TransColorTrc(frame->color_trc);
	pty.color_space = TransColorSpace(frame->colorspace);

	// 首帧
	if (nDecodeType == EDecoderType::EDT_HARDWARD)
	{
		ID3D11VideoDecoderOutputView* srcSurface = (ID3D11VideoDecoderOutputView*)(uintptr_t)frame->data[3];
		if (!srcSurface)
			return;

		D3D11_VIDEO_DECODER_OUTPUT_VIEW_DESC viewDesc;
		srcSurface->GetDesc(&viewDesc);

		ComPtr<ID3D11Texture2D> pHWTexture;
		srcSurface->GetResource((ID3D11Resource**)pHWTexture.GetAddressOf());

		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		pHWTexture->GetDesc(&desc);

		switch (desc.Format)
		{
		case DXGI_FORMAT_P016:
			pty.color_bit = 16;
			break;
		case DXGI_FORMAT_P010:
			pty.color_bit = 10;
			break;
		case  DXGI_FORMAT_NV12:
			pty.color_bit = 8;
			break;
		default:
			assert(0);
			pty.color_bit = 8;
			break;
		}

		if (desc.Format == DXGI_FORMAT_NV12)
		{
			pty.color_type = ETT_NV12;
		}
		else if (desc.Format == DXGI_FORMAT_P010)
		{
			pty.color_type = ETT_P010;
		}
		else if (desc.Format == DXGI_FORMAT_YUY2)
		{
			pty.color_type = ETT_YV12;
		}
		else if (desc.Format == DXGI_FORMAT_R16_UNORM || desc.Format == DXGI_FORMAT_R8_UNORM)
		{
			pty.color_type = ETT_YUV;
		}
		else if (desc.Format == DXGI_FORMAT_BC3_UNORM)
		{
			pty.color_type = ETT_BC3BC4;
		}
		else if (desc.Format == DXGI_FORMAT_G8R8_G8B8_UNORM)
		{
			pty.color_type = ETT_YUYV;
		}
	}
	else
	{
		auto iter = PIX_TEX.find((AVPixelFormat)frame->format);
		if (iter != PIX_TEX.end())
		{
			pty.color_bit = iter->second.colorBit;
			pty.color_type = iter->second.textType;
		}
		else
		{
			pty.color_bit = 8;
			pty.color_type = ETT_YUV;
		}
	}
}

static HRESULT CreateTexture(uint8_t* const pdata, UINT lineSize, int width, int height, DXGI_FORMAT format, ID3D11Texture2D** ppTexture, ComPtr<ID3D11Device> pD3DDevice)
{
	D3D11_TEXTURE2D_DESC sharedTexDesc;		// 纹理描述
	sharedTexDesc.Width = width;
	sharedTexDesc.Height = height;
	sharedTexDesc.MipLevels = 1;
	sharedTexDesc.ArraySize = 1;
	sharedTexDesc.Format = format;
	sharedTexDesc.SampleDesc.Count = 1;
	sharedTexDesc.SampleDesc.Quality = 0;
	sharedTexDesc.Usage = D3D11_USAGE_DEFAULT;
	sharedTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	sharedTexDesc.CPUAccessFlags = 0;
	sharedTexDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;			// 共享纹理

	HRESULT hr = S_OK;
	if (pdata != NULL)
	{
		D3D11_SUBRESOURCE_DATA texData;
		texData.pSysMem = pdata;
		texData.SysMemPitch = lineSize;
		texData.SysMemSlicePitch = 0;

		hr = pD3DDevice->CreateTexture2D(&sharedTexDesc, &texData, ppTexture);
	}
	else
	{
		hr = pD3DDevice->CreateTexture2D(&sharedTexDesc, nullptr, ppTexture);
	}

	DX_CHECK(hr);

	return hr;
}
