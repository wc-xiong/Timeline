#include "D3DTransationRender.h"
#include "Common/Common/XEffectCommon.h"
#include "Common/Common/TransationTypeDefine.h"
#include "common/Common/MediaCommon.h"
#include "Common/Common/TexFrameCommon.h"
#include "Common/Common/VideoDataCommon.h"
#include "Common/CommonDefine.h"
#include "GSMedia/RenderDevice/DeviceResource.h"
#include "GSMedia/RenderBase2D/BasicEffect.h"

#include "RenderDrawDataDefine.h"
#include "VertexCalculator.h"


//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CD3DTransationRender::CD3DTransationRender(ID3D11Device * device)
{
	std::vector<VertexPosTex> vertices = {
{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) } ,
{ DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) } ,
{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) } };
	std::vector<DWORD> indexs = { 0,1,2,2,3,0 };

	m_Model.AddMesh(device, vertices, indexs);

	m_offscreenTexsBack = std::make_unique<CTextureRender>();
	m_offscreenTexsFront = std::make_unique<CTextureRender>();
}

CD3DTransationRender::~CD3DTransationRender()
{
	for (auto iter : m_mapTransTextures)
	{
		for (auto item : iter.second)
		{
			item.Reset();
		}
	}

	for (auto iter : m_mapVertexCalculator)
	{
		if (iter.second)
		{
			delete iter.second;
			iter.second = nullptr;
		}
	}

	m_vecShaderViews.clear();
	m_vecShaderViews.shrink_to_fit();

	m_Model.Reset();
}

void CD3DTransationRender::SetBasicEffect(BasicEffect* pEffect)
{
	m_pBasicEffect = pEffect;
}

void CD3DTransationRender::TransationFxRender(CD3DResourceChild* deviced3d, 
	SDrawTexture &texData, 
	const STransInfo &transFx,
	std::vector<ID3D11ShaderResourceView*> &vecViews, 
	SColorPty &pty,
	std::vector<VertexPosTex> &vertices,
	std::vector<DWORD> &indexs)
{
	vertices = g_vecDefalutVertices;
	indexs = g_vecDefaultIndexs;

	if (transFx.stage == ETFS_Normal)
		return;

	auto& texs = texData.textureFrame;

	if (texs.m_texs.empty())
		return;

	m_sTransInfo = transFx;
	
	if (transFx.nTransType == ETFT_Fade)
	{
		const float factor = (transFx.stage == ETFS_In)
			? transFx.fProgress
			: (1.0f - transFx.fProgress);
		pty.alpha *= factor;
		m_pBasicEffect->SetCellClr(pty);
	}
	else if (transFx.nTransType >= ETFT_LeftMove && transFx.nTransType <= ETFT_BorderToCenterCross)
	{
		CalculaVertex(vertices, indexs);
	}
	else
	{
		//转场纹理绘制
		m_vecShaderViews = vecViews;
		m_pBasicEffect->SetCellPty(texs.m_TexColorPty);

		Draw(deviced3d, texs.m_texs[0]);

		vecViews = m_vecShaderViews;
		texs.m_TexColorPty.color_type = ETT_RGBA;
	}
}

void CD3DTransationRender::InitOffScreenTexs(CD3DResourceChild* deviced3d,int w, int h, DXGI_FORMAT format)
{
	if (w == m_nTexWidth && h == m_nTexHeight && format == m_nTexFromat)
		return;

	m_nTexWidth = w;
	m_nTexHeight = h;
	m_nTexFromat = format;

	HRESULT hr = m_offscreenTexsBack->InitResource(deviced3d->m_pD3DDevice.Get(), w, h, format);
	if (FAILED(hr)) return;

	hr = m_offscreenTexsFront->InitResource(deviced3d->m_pD3DDevice.Get(), w, h, format);
	if (FAILED(hr)) return;
}

void CD3DTransationRender::CalculaVertex(std::vector<VertexPosTex>& vertices, std::vector<DWORD>& indexs)
{
	int nMode = m_sTransInfo.nTransType;
	if (nMode >= ETFT_LeftMove && nMode <= ETFT_PerspectiveRotationBottom)
	{
		IVertexCalculator* pVertexCalc = nullptr;

		auto iter = m_mapVertexCalculator.find(nMode);
		if (iter == m_mapVertexCalculator.end())
		{
			pVertexCalc = CreateVertexCalculator(nMode);
			m_mapVertexCalculator.insert({ nMode,pVertexCalc });
		}
		else
			pVertexCalc = iter->second;
	
		if(pVertexCalc)
			pVertexCalc->DoCalVertex(m_sTransInfo, vertices, indexs);
	}
}

void CD3DTransationRender::Draw(CD3DResourceChild* deviced3d, const STexDatas &tex)
{
	if (tex.texNode0.pTex)	// 初始化临时纹理数据
	{
		InitOffScreenTexs(deviced3d, tex.texNode0.texDesc.Width, tex.texNode0.texDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM);
	}

	m_Model.modelParts.clear();
	std::vector<VertexPosTex> vertices = {
{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) } ,
{ DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) } ,
{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) } };
	std::vector<DWORD> indexs = { 0,1,2,2,3,0 };

	CalculaVertex(vertices, indexs);
	m_Model.AddMesh(deviced3d->m_pD3DDevice.Get(), vertices, indexs);

	int nMode = m_sTransInfo.nTransType;
	if (nMode >= ETFT_PerspectiveRotationLeft && nMode <= ETFT_PerspectiveRotationBottom)
	{
		m_pBasicEffect->SetRenderState(deviced3d->m_pD3DDeviceContext.Get(), BasicEffect::VS3D, BasicEffect::PSXEffect, BasicEffect::BMCover, nullptr, BasicEffect::SamplerLineClamp);
		m_pBasicEffect->SetWorldMatrix(DirectX::XMMatrixIdentity());
		auto oldV = m_pBasicEffect->SetViewMatrix(DirectX::XMMatrixLookAtLH(
			DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
			DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
		auto oldP = m_pBasicEffect->SetProjMatrix(DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, 1.0f, 1.0f, FLT_MAX));

		deviced3d->m_pD3DDeviceContext->PSSetShaderResources(0, (int)m_vecShaderViews.size(), m_vecShaderViews.data());
		if(m_vecShaderViews.size()>0)
			deviced3d->m_pD3DDeviceContext->PSSetShaderResources(3, 1,&m_vecShaderViews[0]);

		// 设置顶点/索引缓冲区
		UINT strides = m_Model.vertexStride;
		UINT offsets = 0;
		deviced3d->m_pD3DDeviceContext->IASetVertexBuffers(0, 1, m_Model.modelParts[0].vertexBuffer.GetAddressOf(), &strides, &offsets);
		deviced3d->m_pD3DDeviceContext->IASetIndexBuffer(m_Model.modelParts[0].indexBuffer.Get(), m_Model.modelParts[0].indexFormat, 0);

		CBXEffect effectBuffer;
		effectBuffer.type = EEffectType::None;
		m_pBasicEffect->SetXEffectPty(effectBuffer);
		m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
		float bk[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_offscreenTexsBack->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
		deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
		m_offscreenTexsBack->End(deviced3d->m_pD3DDeviceContext.Get());

		m_pBasicEffect->SetViewMatrix(oldV);
		m_pBasicEffect->SetProjMatrix(oldP);
	}

	m_offscreenTexsFront.swap(m_offscreenTexsBack);

	m_vecShaderViews.clear();
	m_vecShaderViews.push_back(m_offscreenTexsFront->GetTextureShaderView());
}
