#include "pch.h"
#include "CommonFunction.h"
#include <algorithm>

#include "DXTrace.h"
#include "d3dUtil.h"
#include "WICTextureLoader.h"

using namespace ATL;
using namespace std;
using namespace DirectX;
template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;


RENDERCOMMON_API
HRESULT CreateD3DTextureFromWICFile(ID3D11Device* pd3dDevice, const wchar_t* szFileName, ID3D11Texture2D** texture, BOOL bShared)
{
	return CreateWICTextureFromFileEx(
		pd3dDevice,
		nullptr,
		szFileName,
		0,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE,
		0,
		(bShared ? D3D11_RESOURCE_MISC_SHARED : bShared),
		WIC_LOADER_IGNORE_SRGB,
		(ID3D11Resource**)texture,
		nullptr);
}

RENDERCOMMON_API
HRESULT CreateD3DTextureFromTGAFile(ID3D11Device* pd3dDevice, const wchar_t* szFileName, ID3D11Texture2D** texture, BOOL bShared)
{
	ScratchImage image;
	LoadFromTGAFile(szFileName, nullptr, image);

	return CreateTextureEx(pd3dDevice
		, image.GetImages()
		, image.GetImageCount()
		, image.GetMetadata()
		, D3D11_USAGE_DEFAULT
		, D3D11_BIND_SHADER_RESOURCE
		, 0
		, (bShared ? D3D11_RESOURCE_MISC_SHARED : bShared)
		, TRUE
		, (ID3D11Resource**)texture);
}

RENDERCOMMON_API
void CreateD3DTextureFromMemory(ID3D11Device* pd3dDevice, const BYTE *data, UINT width, UINT height, DXGI_FORMAT format, ID3D11Texture2D **texture, BOOL bShared)
{
	CreateD3DTextureFromMemory(pd3dDevice, data, width, height, format, 0, texture, bShared);
}

RENDERCOMMON_API HRESULT CreateD3DTextureFromMemory(ID3D11Device* pd3dDevice, const BYTE * data, UINT width, UINT height, DXGI_FORMAT format, UINT linesize, ID3D11Texture2D ** texture, BOOL bShared)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = bShared ? D3D11_RESOURCE_MISC_SHARED : bShared;

	HRESULT hr = S_OK;
	if (data != NULL)
	{
		if (linesize <= 0)
		{
			linesize = width * 4;
		}

		D3D11_SUBRESOURCE_DATA texData;
		texData.pSysMem = data;
		texData.SysMemPitch = linesize;
		texData.SysMemSlicePitch = height * linesize;

		hr = pd3dDevice->CreateTexture2D(&texDesc, &texData, texture);
	}
	else
	{
		hr = pd3dDevice->CreateTexture2D(&texDesc, nullptr, texture);
	}

	return hr;
}

RENDERCOMMON_API
void CreateD3DTextureForDraw(ID3D11Device* pd3dDevice, UINT width, UINT height, UINT bitDepth, UINT bindFlag, ID3D11Texture2D **texture)
{
	D3D11_TEXTURE2D_DESC texDesc; //纹理描述
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1; //希望它不产生mipmap，则应当指定为1(只包含最大的位图本身)
	texDesc.ArraySize = 1;
	texDesc.Format = (bitDepth == 8) ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R16G16B16A16_UNORM;
	texDesc.SampleDesc.Count = 1; //不采用多重采样
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT; //GPU读写
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; //作为渲染目标和着色器资源
	texDesc.BindFlags |= bindFlag;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	HR(pd3dDevice->CreateTexture2D(&texDesc, NULL, texture));
}

RENDERCOMMON_API
HRESULT CreateD3DTextureShaderView(ID3D11Device* pd3dDevice, const ID3D11Texture2D* texture, const D3D11_SHADER_RESOURCE_VIEW_DESC* desc, ID3D11ShaderResourceView** textureView)
{
	if (texture == nullptr)
		return E_POINTER;

	return pd3dDevice->CreateShaderResourceView((ID3D11Resource*)texture, desc, textureView);
}

RENDERCOMMON_API
HRESULT CreateD3DTextureRenderTargetView(ID3D11Device* pd3dDevice, const ID3D11Texture2D* texture, const D3D11_RENDER_TARGET_VIEW_DESC* desc, ID3D11RenderTargetView** textureView)
{
	return pd3dDevice->CreateRenderTargetView((ID3D11Resource*)texture, desc, textureView);
}

RENDERCOMMON_API 
HRESULT CreateD2DRenderTargetFromTexture(ID2D1Factory* pd2dFactory, ID3D11Texture2D* texture, ID2D1RenderTarget** textRenderTarget)
{
	ComPtr<IDXGISurface> pDxgiSurf;
	texture->QueryInterface(__uuidof(IDXGISurface), (void**)pDxgiSurf.ReleaseAndGetAddressOf());

	if (!pDxgiSurf)
		return E_POINTER;

	D2D1_RENDER_TARGET_PROPERTIES props;
	props.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
	props.dpiX = 0;
	props.dpiY = 0;
	props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	props.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
	props.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
	props.usage = D2D1_RENDER_TARGET_USAGE_NONE;

	return pd2dFactory->CreateDxgiSurfaceRenderTarget(pDxgiSurf.Get(), &props, textRenderTarget);
}

RENDERCOMMON_API
HRESULT GetD3DTextHandleFromTexture(ID3D11Texture2D* pTexture, HANDLE* pTexHandle)
{
	if (pTexture == nullptr)
		return E_POINTER;

	if (pTexHandle == nullptr)
		return E_POINTER;

	ComPtr<IDXGIResource> pDXGIResource;
	HRESULT hr = pTexture->QueryInterface(__uuidof(IDXGIResource), (LPVOID*)&pDXGIResource);
	if (pDXGIResource)
		hr = pDXGIResource->GetSharedHandle(pTexHandle);

	if (SUCCEEDED(hr) && *pTexHandle != nullptr)
		return S_OK;

	return E_POINTER;
}

RENDERCOMMON_API
void SaveD3DTextureToFile(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, ID3D11Texture2D *texture, const wchar_t* szFileName)
{
	DirectX::ScratchImage pImage;
	HRESULT hr = S_OK;
	hr = DirectX::CaptureTexture(pd3dDevice, pd3dImmediateContext, texture, pImage);
	if (SUCCEEDED(hr))
	{
		const DirectX::Image *pRealImage = pImage.GetImage(0, 0, 0);
		hr = DirectX::SaveToWICFile(*pRealImage, DirectX::WIC_FLAGS_NONE, GUID_ContainerFormatPng, szFileName);
	}
}


