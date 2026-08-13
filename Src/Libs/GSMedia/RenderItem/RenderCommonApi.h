#pragma once
#include <DirectXMath.h>
#include <vector>

#include <wrl/client.h>
#include <d3d11_1.h>

#include "Common/VideoDataCommon.h"
#include "GSMedia/RenderCommon/Vertex.h"

#include "RenderDrawDataDefine.h"

// 创建着色器资源：先 Open/Create 全部成功后再替换旧 SRV，失败则保留旧视图（避免闪绿）
static HRESULT CreateShaderResView(ComPtr<ID3D11Device> pD3DDevice, STexDatas& tex, Video_ClrMetedata& metaData, SRenderShaderResView& viewData)
{
	HRESULT hr = S_OK;

	bool bReset = (viewData.m_texHandle0 != tex.texNode0.texHandle)
		|| (viewData.m_texHandle1 != tex.texNode1.texHandle)
		|| (viewData.m_texHandle2 != tex.texNode2.texHandle);

	ComPtr<ID3D11Texture2D> sharedTexture0 = nullptr;
	ComPtr<ID3D11Texture2D> sharedTexture1 = nullptr;
	ComPtr<ID3D11Texture2D> sharedTexture2 = nullptr;
	ComPtr<ID3D11ShaderResourceView> newView0 = nullptr;
	ComPtr<ID3D11ShaderResourceView> newView1 = nullptr;
	ComPtr<ID3D11ShaderResourceView> newView2 = nullptr;

	if (bReset)
	{
		if (!tex.texNode0.texHandle)
			return S_FALSE;

		hr = pD3DDevice->OpenSharedResource(tex.texNode0.texHandle, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(sharedTexture0.GetAddressOf()));
		if (!SUCCEEDED(hr))
			return S_FALSE;

		if (tex.texNode1.texHandle)
		{
			hr = pD3DDevice->OpenSharedResource(tex.texNode1.texHandle, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(sharedTexture1.GetAddressOf()));
			if (!SUCCEEDED(hr))
				return S_FALSE;
		}

		if (tex.texNode2.texHandle)
		{
			hr = pD3DDevice->OpenSharedResource(tex.texNode2.texHandle, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(sharedTexture2.GetAddressOf()));
			if (!SUCCEEDED(hr))
				return S_FALSE;
		}
	}

	switch (tex.texNode0.texDesc.Format)
	{
	case DXGI_FORMAT_NV12:
	{
		metaData.color_type = ETT_NV12;

		if (bReset)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC lumnDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(sharedTexture0.Get(), D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8_UNORM);
			hr = pD3DDevice->CreateShaderResourceView(sharedTexture0.Get(), &lumnDesc, newView0.ReleaseAndGetAddressOf());
			if (!SUCCEEDED(hr))
				return S_FALSE;

			D3D11_SHADER_RESOURCE_VIEW_DESC chroDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(sharedTexture0.Get(), D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8_UNORM);
			hr = pD3DDevice->CreateShaderResourceView(sharedTexture0.Get(), &chroDesc, newView1.ReleaseAndGetAddressOf());
			if (!SUCCEEDED(hr))
				return S_FALSE;
		}
	}
	break;
	case DXGI_FORMAT_P010:
	{
		metaData.color_type = ETT_P010;

		if (bReset)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC lumnDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(sharedTexture0.Get(), D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R16_UNORM);
			hr = pD3DDevice->CreateShaderResourceView(sharedTexture0.Get(), &lumnDesc, newView0.ReleaseAndGetAddressOf());
			if (!SUCCEEDED(hr))
				return S_FALSE;
			D3D11_SHADER_RESOURCE_VIEW_DESC chroDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(sharedTexture0.Get(), D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R16G16_UNORM);
			hr = pD3DDevice->CreateShaderResourceView(sharedTexture0.Get(), &chroDesc, newView1.ReleaseAndGetAddressOf());
			if (!SUCCEEDED(hr))
				return S_FALSE;
		}
	}
	break;
	case DXGI_FORMAT_YUY2:
	{
		metaData.color_type = ETT_YV12;

		if (bReset)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC chroDesc = CD3D11_SHADER_RESOURCE_VIEW_DESC(sharedTexture0.Get(), D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM);
			hr = pD3DDevice->CreateShaderResourceView(sharedTexture0.Get(), &chroDesc, newView0.ReleaseAndGetAddressOf());
			if (!SUCCEEDED(hr))
				return S_FALSE;
		}
	}
	break;
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R8_UNORM:
	{
		metaData.color_type = ETT_YUV;
		if (bReset)
		{
			hr = pD3DDevice->CreateShaderResourceView(sharedTexture0.Get(), NULL, newView0.ReleaseAndGetAddressOf());
			if (!SUCCEEDED(hr))
				return S_FALSE;
			if (sharedTexture1)
			{
				hr = pD3DDevice->CreateShaderResourceView(sharedTexture1.Get(), NULL, newView1.ReleaseAndGetAddressOf());
				if (!SUCCEEDED(hr))
					return S_FALSE;
			}
			if (sharedTexture2)
			{
				hr = pD3DDevice->CreateShaderResourceView(sharedTexture2.Get(), NULL, newView2.ReleaseAndGetAddressOf());
				if (!SUCCEEDED(hr))
					return S_FALSE;
			}
		}
	}
	break;
	case DXGI_FORMAT_BC3_UNORM:
	{
		metaData.color_type = ETT_BC3;

		if (bReset)
		{
			hr = pD3DDevice->CreateShaderResourceView(sharedTexture0.Get(), NULL, newView0.ReleaseAndGetAddressOf());
			if (!SUCCEEDED(hr))
				return S_FALSE;

			if (sharedTexture1.Get())
			{
				metaData.color_type = ETT_BC3BC4;
				hr = pD3DDevice->CreateShaderResourceView(sharedTexture1.Get(), NULL, newView1.ReleaseAndGetAddressOf());
				if (!SUCCEEDED(hr))
					return S_FALSE;
			}
		}
	}
	break;
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	{
		metaData.color_type = ETT_YUYV;

		if (bReset)
		{
			hr = pD3DDevice->CreateShaderResourceView(sharedTexture0.Get(), NULL, newView0.ReleaseAndGetAddressOf());
			if (!SUCCEEDED(hr))
				return S_FALSE;
		}
	}
	break;
	default:
	{
		if (bReset)
		{
			hr = pD3DDevice->CreateShaderResourceView(sharedTexture0.Get(), NULL, newView0.ReleaseAndGetAddressOf());
			if (!SUCCEEDED(hr))
				return S_FALSE;
		}
	}
	break;
	}

	if (bReset)
	{
		viewData.m_pView0 = newView0;
		viewData.m_pView1 = newView1;
		viewData.m_pView2 = newView2;
		viewData.m_texHandle0 = tex.texNode0.texHandle;
		viewData.m_texHandle1 = tex.texNode1.texHandle;
		viewData.m_texHandle2 = tex.texNode2.texHandle;
	}

	return S_OK;
}
