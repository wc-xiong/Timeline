#include "D3D11VideoProcessor.h"

#include <iostream>
#include <d3d11_1.h>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CD3D11VideoProcessor::CD3D11VideoProcessor(ComPtr<ID3D11Device> device)
	: m_device(device)
	, m_inputWidth(0)
	, m_inputHeight(0)
	, m_outputWidth(0)
	, m_outputHeight(0)
{
	HRESULT hr = m_device.As(&m_videoDevice);
	if (FAILED(hr))
		std::cout << "video device eeror!\n";
}

CD3D11VideoProcessor::~CD3D11VideoProcessor()
{
	m_outTexture.Reset();
}

void CD3D11VideoProcessor::SetSourceRect(const QRect & rect)
{
	m_rect = rect;
}

void CD3D11VideoProcessor::SetOutput(ID3D11Texture2D * tex, UINT width, UINT height)
{
	m_outTexture = tex;
	m_videoProcessOutputView.Reset();
}

bool CD3D11VideoProcessor::Process(ID3D11Texture2D * srcTexture, int index)
{
	if (!srcTexture || !m_outTexture)
	{
		return false;
	}
	D3D11_TEXTURE2D_DESC desc;
	srcTexture->GetDesc(&desc);
	if (!EnsureResource(desc.Width, desc.Height, desc.Format))
	{
		return false;
	}

	D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inviewDesc = {
		0, D3D11_VPIV_DIMENSION_TEXTURE2D, { 0, (UINT)index }
	};

	ComPtr<ID3D11VideoProcessorInputView> inview;
	HRESULT hr = m_videoDevice->CreateVideoProcessorInputView(
		srcTexture, m_videoProcessEnumerator.Get(), &inviewDesc, &inview);
	// 跨设备纹理或非法 ArraySlice 时会失败；不得继续 Blt
	if (FAILED(hr) || !inview)
		return false;

	ComPtr<ID3D11DeviceContext> ctx;
	ComPtr<ID3D11VideoContext1> videoctx;
	m_device->GetImmediateContext(&ctx);

	hr = ctx.As(&videoctx);
	if (FAILED(hr))
		return false;

	if (!m_rect.isEmpty())
	{
		// RECT 为 left/top/right/bottom，不是 x/y/width/height
		const RECT r = {
			m_rect.x(),
			m_rect.y(),
			m_rect.x() + m_rect.width(),
			m_rect.y() + m_rect.height()
		};
		videoctx->VideoProcessorSetStreamSourceRect(m_videoProcess.Get(), 0, TRUE, &r);
	}
	// disable additional processing. this can fix the output frame is too dark, also make in/out color space parameters work
	//videoctx->VideoProcessorSetStreamAutoProcessingMode(m_videoProcess.Get(), 0, FALSE);

	D3D11_VIDEO_PROCESSOR_STREAM stream;
	ZeroMemory(&stream, sizeof(stream));
	stream.Enable = TRUE;
	stream.pInputSurface = inview.Get();
	hr = videoctx->VideoProcessorBlt(m_videoProcess.Get(), m_videoProcessOutputView.Get(), 0, 1, &stream);
	if (FAILED(hr))
		return false;

	return true;
}

bool CD3D11VideoProcessor::EnsureResource(UINT width, UINT height, DXGI_FORMAT informat)
{
	HRESULT hr = S_OK;

	D3D11_TEXTURE2D_DESC outdesc;
	m_outTexture->GetDesc(&outdesc);

	bool dirty = width != m_inputWidth || height != m_inputHeight || outdesc.Width != m_outputWidth || outdesc.Height != m_outputHeight;
	if (dirty || !m_videoProcessEnumerator)
	{
		D3D11_VIDEO_PROCESSOR_CONTENT_DESC vpdesc;
		ZeroMemory(&vpdesc, sizeof(vpdesc));
		vpdesc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
		vpdesc.InputWidth = width;
		vpdesc.InputHeight = height;
		vpdesc.OutputWidth = outdesc.Width;
		vpdesc.OutputHeight = outdesc.Height;
		vpdesc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;
		hr = m_videoDevice->CreateVideoProcessorEnumerator(&vpdesc, m_videoProcessEnumerator.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			std::cout << "create video process enume fail!\n";
			return false;
		}
	}

	UINT flags = 0;
	hr = m_videoProcessEnumerator->CheckVideoProcessorFormat(informat, &flags);
	if (FAILED(hr))
	{
		std::cout << "check video process input format fail!\n";
		return false;
	}
	// 部分驱动对 flags 不完全准确，最终以 Create*View / Blt 成败为准
	if (!(flags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_INPUT))
		std::cout << "warn: input format may be unsupported:" << informat << std::endl;

	flags = 0;
	hr = m_videoProcessEnumerator->CheckVideoProcessorFormat(outdesc.Format, &flags);
	if (FAILED(hr))
	{
		std::cout << "check video process output format fail!\n";
		return false;
	}
	if (!(flags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT))
		std::cout << "warn: output format may be unsupported:" << outdesc.Format << std::endl;

	if (dirty || !m_videoProcess)
	{
		hr = m_videoDevice->CreateVideoProcessor(m_videoProcessEnumerator.Get() , 0, m_videoProcess.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			std::cout << "CreateVideoProcessor failed!\n";
			return false;
		}
	}

	if (dirty || !m_videoProcessOutputView)
	{
		D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC outputDesc;
		ZeroMemory(&outputDesc, sizeof(outputDesc));
		outputDesc.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
		outputDesc.Texture2D.MipSlice = 0;

		hr = m_videoDevice->CreateVideoProcessorOutputView(m_outTexture.Get()
			, m_videoProcessEnumerator.Get(), &outputDesc, m_videoProcessOutputView.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			std::cout << "CreateVideoProcessorOutputView failed!\n";
			return false;
		}
	}

	m_inputWidth = width;
	m_inputHeight = height;
	m_outputWidth = outdesc.Width;
	m_outputHeight = outdesc.Height;

	return true;
}
