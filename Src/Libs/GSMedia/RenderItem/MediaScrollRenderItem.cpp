#include "MediaScrollRenderItem.h"

#include "Common/Common/CommonDefine.h"
#include "Common/Common/MediaCommon.h"
#include "Common/Common/TexFrameCommon.h"
#include "Common/Common/XEffectCommon.h"
#include "Common/Common/VideoDataCommon.h"
#include "Common/Common/ResourceDefine.h"
#include "Common/Common/IMediaPropertyInterface.h"
#include "GSMedia/RenderDevice/DeviceResource.h"
#include "GSMedia/RenderBase2D/BasicEffect.h"

#include <QPixmap>
#include <vector>
#include <QDebug>
#include <time.h>

#include "D3DScrollRender.h"
#include "D3DEffectRender.h"
#include "D3DTransationRender.h"
#include "RenderDrawDataDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CMediaScrollRenderItem::CMediaScrollRenderItem(std::shared_ptr<SMediaPty> pMediaPty, CD3DResourceChild* pD3dRes, bool bUseCopyTexRender)
	:CBaseRenderItem(pMediaPty, pD3dRes, bUseCopyTexRender)
{
	m_pTransationRender = new CD3DTransationRender(pD3dRes->m_pD3DDevice.Get());

	m_pScrollData = std::make_shared<SRenderScrollData>();

	m_pMediaScrollParam = m_pMediaPty->pSharePty->GetScrollParam();
	m_pMediaScrollParam->nScrollTypeCahce = m_pMediaScrollParam->nScrollType;

	m_pScrollRender = new CD3DScrollRender(m_pMediaPty->pBasePty, m_pMediaScrollParam, m_pScrollData);
}

CMediaScrollRenderItem::~CMediaScrollRenderItem()
{
	if (m_pTransationRender)
	{
		delete m_pTransationRender;
		m_pTransationRender = nullptr;
	}

	if (m_pScrollRender)
	{
		delete m_pScrollRender;
		m_pScrollRender = nullptr;
	}
}

void CMediaScrollRenderItem::UpdateDrawData()
{
	if(!m_sDrawTexData.vecTextureFrame.empty())
		m_pScrollRender->UpdateScrollPos(m_sDrawTexData.vecTextureFrame[0].textureFrame.m_bScrollState);
}

void CMediaScrollRenderItem::SetBasicEffect(BasicEffect* pEffect)
{
	CBaseRenderItem::SetBasicEffect(pEffect);
	m_pTransationRender->SetBasicEffect(pEffect);
}

void CMediaScrollRenderItem::UpdateMediaPtyByRedoUndo(std::shared_ptr<SMediaPty> pMediaPty)
{
	__super::UpdateMediaPtyByRedoUndo(pMediaPty);

	m_pMediaScrollParam = m_pMediaPty->pSharePty->GetScrollParam();
	m_pMediaScrollParam->nScrollTypeCahce = m_pMediaScrollParam->nScrollType;

	m_pScrollRender->UpdateScrollParam(m_pMediaScrollParam);
	m_pScrollRender->UpdateBaseMediaPty(pMediaPty->pBasePty);
}

bool CMediaScrollRenderItem::UpdateVertexData(float X, float Y, int W, int H, const QRect& offscreenRc)
{
	QSize offscreemRc = offscreenRc.size();

	SVertexPosTexData vertexNode = GetVertexTexData(offscreemRc);

	if (!m_pScrollRender->GetTransationVertex(vertexNode, X - offscreenRc.left(), Y - offscreenRc.top(), W, H, offscreemRc))
		return false;

	for (int i = 0; i < m_vecTexRender.size(); ++i)
	{
		m_vecTexRender.at(i)->SetVertexBuffer(m_pD3dRes->m_pD3DDeviceContext.Get(), GetVertexBufferByPosTexData(vertexNode, offscreemRc));
	}

	return true;
}

void CMediaScrollRenderItem::DrawTexture(const SColorPty& globalPty, std::vector<SDrawTexture> vecFrameTexs)
{
	if (vecFrameTexs.empty() || vecFrameTexs[0].textureFrame.m_texs.empty())
		return;
	ResetTexRender(vecFrameTexs[0].textureFrame.m_texs[0]);

	float backClr[4] = { 0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 0.0f };

	for (int j = 0; j < m_vecTexRender.size(); ++j)
	{
		const auto& rcTex = m_drawData.texs.at(j).rcTex;
		QRect offscreenRc = QRect(rcTex.left, rcTex.top, rcTex.Width(), rcTex.Height());

		m_vecTexRender.at(j)->Begin(m_pD3dRes->m_pD3DDeviceContext.Get(), backClr, true);

		UpdateTextureColor(j);

		int nTexIndex = 0;
		for (auto it : vecFrameTexs)
		{
			auto pty = m_pMediaPty->pEffectPty->GetColorProperty();
			pty.alpha *= m_pMediaPty->pEffectPty->GetFadeAlpha();

			auto& texs = it.textureFrame;

			switch (m_pMediaScrollParam->nScrollTypeCahce)
			{
			case EST_Right2Left:
			case EST_Left2Right:
			case EST_Top2Bottom:
			case EST_Bottom2Top:
			{
				float tempX = m_pMediaScrollParam->fNowX;
				float tempY = m_pMediaScrollParam->fNowY;

				bool bHor = m_pMediaScrollParam->nScrollTypeCahce == EST_Right2Left || m_pMediaScrollParam->nScrollTypeCahce == EST_Left2Right;
				bool bVer = m_pMediaScrollParam->nScrollTypeCahce == EST_Top2Bottom || m_pMediaScrollParam->nScrollTypeCahce == EST_Bottom2Top;

				do
				{
					for (int i = 0; i < texs.m_texs.size(); i++)
					{
						if (!texs.m_texs[i].texNode0.texHandle)
							continue;

						int nTexWidth = texs.m_texs[i].texNode0.texDesc.Width;
						int nTexHeight = texs.m_texs[i].texNode0.texDesc.Height;
													
						if (UpdateVertexData(tempX, tempY, nTexWidth, nTexHeight, offscreenRc))
						{
							auto vecSharedView = GetShaderResView(texs.m_texs[i], texs.m_TexColorPty, i);
							m_vecTexRender.at(j)->SetShaderViews(vecSharedView);

							UpdateEffectPty(texs.m_TexColorPty, globalPty, BasicEffect::SamplerLineClamp, it.transFxData);

							m_vecTexRender.at(j)->Draw(m_pD3dRes->m_pD3DDeviceContext.Get());
						}

						if (bHor)
							tempX += nTexWidth;

						if (bVer)
							tempY += nTexHeight;
					}

					if (m_pMediaScrollParam->bEndToEnd)
					{
						if (bHor)
							tempX += m_pMediaScrollParam->nEndSpace;

						if (bVer)
							tempY += m_pMediaScrollParam->nEndSpace;
					}
				} while (m_pMediaScrollParam->bEndToEnd &&
					((tempX < m_pScrollData->m_nScrollWndWidth && bHor) || (tempY < m_pScrollData->m_nScrollWndHeight && bVer)));
			}
			break;
			case EST_Stop:
			{
				if (texs.m_texs.empty())
					break;

				if (!texs.m_texs[0].texNode0.texHandle)
					continue;

				std::vector<ID3D11ShaderResourceView*> 	vecSharedView = GetShaderResView(texs.m_texs[0], texs.m_TexColorPty, nTexIndex);

				std::vector<VertexPosTex> vertices;
				std::vector<DWORD> indexs;

				m_pTransationRender->TransationFxRender(m_pD3dRes, it, it.transFxData, vecSharedView, pty, vertices, indexs);

				CBaseRenderItem::UpdateVertexData(vertices, indexs, texs.m_texs[0]);

				m_vecTexRender.at(j)->SetShaderViews(vecSharedView);

				UpdateEffectPty(texs.m_TexColorPty, globalPty, BasicEffect::SamplerLineBorder, it.transFxData);

				m_vecTexRender.at(j)->Draw(m_pD3dRes->m_pD3DDeviceContext.Get());

				nTexIndex++;
			}
			break;
			default:
				break;
			}
		}

		m_vecTexRender.at(j)->End(m_pD3dRes->m_pD3DDeviceContext.Get());
	}
}

void CMediaScrollRenderItem::UpdateTextureColor(int index)
{
	if (!m_pMediaScrollParam->bBkClrEnable)
		return;

	if (m_clrBk != m_pMediaScrollParam->bgColor)
	{
		m_clrBk = m_pMediaScrollParam->bgColor;
		uint8_t r = m_pMediaScrollParam->bgColor.red();
		uint8_t g = m_pMediaScrollParam->bgColor.green();
		uint8_t b = m_pMediaScrollParam->bgColor.blue();
		uint8_t a = m_pMediaScrollParam->bgColor.alpha();

		// Combine them into a single UINT32 value (ABGR format)
		UINT32 newColor = (static_cast<UINT32>(a) << 24) | (static_cast<UINT32>(b) << 16) |
			(static_cast<UINT32>(g) << 8) | static_cast<UINT32>(r);

		// 创建一个纹理数据数组，因为是纯色，不用管大小
		std::vector<UINT32> textureData(100, newColor); // ABGR 格式

		// 创建纹理描述
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = 10;
		texDesc.Height = 10;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;

		// 创建纹理
		ID3D11Texture2D* pTexture2D = nullptr;
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = textureData.data();
		initData.SysMemPitch = 10 * sizeof(UINT32);
		
		auto hr = m_pD3dRes->m_pD3DDevice->CreateTexture2D(&texDesc, &initData, &pTexture2D);
		if (FAILED(hr))
			return;

		// 创建Shader Resource View (SRV)
		ID3D11ShaderResourceView* pSRV = nullptr;
		hr = m_pD3dRes->m_pD3DDevice->CreateShaderResourceView(pTexture2D, nullptr, &pSRV);
		if (FAILED(hr))
			return;

		// 使用纯红色纹理替代原有纹理
		m_vecBkSharedView = { pSRV };

		// 清理纹理资源
		if (pTexture2D) pTexture2D->Release();
	}
	
	QSize offscreenRc = QSize(
		m_drawData.texs.at(index).rcTex.Width(),
		m_drawData.texs.at(index).rcTex.Height()
	);

	m_vecTexRender.at(index)->SetVertexBuffer(m_pD3dRes->m_pD3DDeviceContext.Get(), GetVertexBufferByPosTexData(CBaseRenderItem::GetVertexTexData(offscreenRc), offscreenRc));
	m_vecTexRender.at(index)->SetShaderViews(m_vecBkSharedView);
	UpdateEffectPty(Video_ClrMetedata(), SColorPty(), BasicEffect::SamplerLineClamp);
	m_vecTexRender.at(index)->Draw(m_pD3dRes->m_pD3DDeviceContext.Get());
}

void CMediaScrollRenderItem::ResetTexRender(const STexDatas& texNode)
{
	int nTexWidth = texNode.texNode0.texDesc.Width;
	int nTexHeight = texNode.texNode0.texDesc.Height;

	int newWidth = 0;
	int newHeight = 0;
	switch (m_pMediaScrollParam->nScrollTypeCahce)
	{
	case EST_Stop:
	{
		newWidth = std::min(nTexWidth, MAX_RESOURCE_WIDTH);
		newHeight = std::min(nTexHeight, MAX_RESOURCE_HEIGHT);
	}
	break;
	case EST_Right2Left:
	case EST_Left2Right:
	{
		newHeight = std::min(m_pScrollData->m_nScrollWndHeight, MAX_RESOURCE_HEIGHT);
		newWidth = m_pScrollData->m_nScrollWndWidth;
	}
	break;
	case EST_Top2Bottom:
	case EST_Bottom2Top:
	{
		newWidth = std::min(m_pScrollData->m_nScrollWndWidth, MAX_RESOURCE_WIDTH);
		newHeight = m_pScrollData->m_nScrollWndHeight;
	}
	break;
	default:
		assert(0);
		break;
	}

	UpdateTexRenderByWidthHeight(newWidth, newHeight);
}
