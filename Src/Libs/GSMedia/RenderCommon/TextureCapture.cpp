#include "pch.h"
#include "TextureCapture.h"
#include "d3dUtil.h"
#include "DXTrace.h"

using namespace DirectX;
using namespace Microsoft::WRL;

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CTextureCaptureColor::CTextureCaptureColor(uint32_t i) { m_i = i; }
CTextureCaptureColor::CTextureCaptureColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	m_i = 0;
	m_i += r << 24;
	m_i += g << 16;
	m_i += b << 8;
	m_i += a;
}
DirectX::XMFLOAT4 CTextureCaptureColor::ToDXColor()
{
	return DirectX::XMFLOAT4(uint8_t(m_i >> 24) / 255.f,
		uint8_t(m_i >> 16) / 255.f,
		uint8_t(m_i >> 8) / 255.f,
		uint8_t(m_i) / 255.f);
}
bool operator==(const CTextureCaptureColor& a, const CTextureCaptureColor& b)
{
	return a.m_i == b.m_i;
}
bool operator<(const CTextureCaptureColor& a, const CTextureCaptureColor& b)
{
	return a.m_i < b.m_i;
}


HRESULT TextureCapture::InitResource(ID3D11Device* device, int texWidth, int texHeight)
{
	// 防止重复初始化造成内存泄漏
	ResetResource();

	std::unique_lock<std::shared_mutex> lock(m_mutex);
	m_nWidth = texWidth;
	m_nHeight = texHeight;
	m_ImageData.resize(texWidth*texHeight * 4);

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = texWidth;
	texDesc.Height = texHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Usage = D3D11_USAGE_STAGING;
	texDesc.BindFlags = 0;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	texDesc.MiscFlags = 0;
	HR(device->CreateTexture2D(&texDesc, nullptr, m_pTexture.ReleaseAndGetAddressOf()));

#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	D3D11SetDebugObjectName(m_pTexture.Get(), "TextureCapture.Texture");
#endif

	return S_OK;
}

void TextureCapture::ResetResource()
{
	std::unique_lock<std::shared_mutex> lock(m_mutex);
	m_pTexture.Reset();
	m_ImageData.clear();
	m_nWidth = 0;
	m_nHeight = 0;
}

void TextureCapture::CaptureTexture(ID3D11DeviceContext * deviceContext, ID3D11Texture2D* pTexture)
{
	std::shared_lock<std::shared_mutex> lock(m_mutex);

#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	D3D11_TEXTURE2D_DESC desc;
	pTexture->GetDesc(&desc);
	assert(desc.Width == m_nWidth);
	assert(desc.Height == m_nHeight);
	assert(desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM);
#endif

	deviceContext->CopyResource(m_pTexture.Get(), pTexture);

	m_ImageData.clear();
	m_ImageData.resize(m_nWidth*m_nHeight * 4);
	D3D11_MAPPED_SUBRESOURCE mappedData;
	deviceContext->Map(m_pTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedData);
	uint8_t* pData = reinterpret_cast<uint8_t*>(mappedData.pData);

	assert(pData);

	if (!pData)
		return;

	for (int i = 0; i < m_nHeight; ++i)
	{
		memcpy_s(&m_ImageData[i * m_nWidth * 4], m_nWidth * 4, pData, m_nWidth * 4);
		pData += mappedData.RowPitch;
	}
	deviceContext->Unmap(m_pTexture.Get(), 0);
}

CTextureCaptureColor TextureCapture::GetColor(int x, int y)
{
	std::shared_lock<std::shared_mutex> lock(m_mutex);
	//
	//assert(x < m_nWidth&&x >= 0);
	//assert(y < m_nHeight&&y >= 0);

	int r = y * m_nWidth * 4 + x * 4;
	int g = r + 1;
	int b = g + 1;
	int a = b + 1;

	//assert(r < m_ImageData.size());
	//assert(g < m_ImageData.size());
	//assert(b < m_ImageData.size());
	//assert(a < m_ImageData.size());

	return CTextureCaptureColor(m_ImageData[r], m_ImageData[g], m_ImageData[b], m_ImageData[a]);
}

bool TextureCapture::CopyImageData(std::vector<uint8_t>& outRgba)
{
	std::shared_lock<std::shared_mutex> lock(m_mutex);
	if (m_ImageData.empty() || m_nWidth <= 0 || m_nHeight <= 0)
		return false;
	outRgba = m_ImageData;
	return true;
}
