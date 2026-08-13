#include "FxRenderItem.h"

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

#include "D3DEffectRender.h"

#include "GSMedia/TimeServer/TimeServerExport.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end


CFxRenderItem::CFxRenderItem(std::shared_ptr<SMediaPty> pMediaPty, CD3DResourceChild* pD3dRes, bool bUseCopyTexRender)
	: CBaseRenderItem(pMediaPty, pD3dRes, bUseCopyTexRender)
{
	m_pFxRenderData = std::make_shared<SFxRenderData>();
	m_pEffectRender = new CD3DEffectRender(pD3dRes->m_pD3DDevice.Get(), m_pMediaPty, m_pFxRenderData);
}


CFxRenderItem::~CFxRenderItem()
{
	if (m_pEffectRender)
	{
		delete m_pEffectRender;
		m_pEffectRender = nullptr;
	}
}

void CFxRenderItem::UpdateDrawData()
{
	__super::UpdateDrawData();
	microseconds ts;
	TimeServer_GetCurTime(ts);
	m_pEffectRender->UpdateFxRenderData(ts);
}

void CFxRenderItem::SetBasicEffect(BasicEffect* pEffect)
{
	CBaseRenderItem::SetBasicEffect(pEffect);
	m_pEffectRender->SetBasicEffect(pEffect);
}

void CFxRenderItem::UpdateMediaPtyByRedoUndo(std::shared_ptr<SMediaPty> pMediaPty)
{
	__super::UpdateMediaPtyByRedoUndo(pMediaPty);
	m_pEffectRender->UpdateMediaPty(pMediaPty);
}

void CFxRenderItem::UpdateVertexData(const STexDatas& texNode)
{
	SVertexPosTexData vertexNode = CBaseRenderItem::GetVertexTexData(QSize(m_drawData.uvWidth, m_drawData.uvHeight));

	m_bScroll = m_pEffectRender->GetFxVertex(vertexNode);

	VertexFillTypeDeal(vertexNode, texNode);

	m_vecTexRender[0]->SetVertexBuffer(m_pD3dRes->m_pD3DDeviceContext.Get(), GetVertexBufferByPosTexData(vertexNode, QSize(m_drawData.uvWidth, m_drawData.uvHeight)));
}

void CFxRenderItem::DrawTexture(const SColorPty& globalPty, std::vector<SDrawTexture> vecFrameTexs)
{
	if (vecFrameTexs.empty() || vecFrameTexs[0].textureFrame.m_texs.empty())
		return;
	ResetTexRender(vecFrameTexs[0].textureFrame.m_texs[0]);

	float backClr[4] = { 0.0f, 0.0f, 0 / 255.0f, 1.0f };
	m_vecTexRender[0]->Begin(m_pD3dRes->m_pD3DDeviceContext.Get(), backClr, true);

	int nTexIndex = 0;
	for (auto it : vecFrameTexs)
	{
		auto& texs = it.textureFrame;
		for (auto iter : texs.m_texs)
		{
			if (!iter.texNode0.texHandle)
				continue;

			std::vector<ID3D11ShaderResourceView*> vecSharedView = GetShaderResView(iter, texs.m_TexColorPty);
			if (!vecSharedView.empty())
			{
				//特效
				m_pEffectRender->MediaFxRender(m_pD3dRes, vecSharedView, iter, texs.m_TexColorPty, texs.m_pts.count());

				UpdateVertexData(iter);
				m_vecTexRender[0]->SetShaderViews(vecSharedView);
			}
			else
				int temp = 1;

			UpdateEffectPty(texs.m_TexColorPty, globalPty,
				m_bScroll ? BasicEffect::SamplerLineWrap : BasicEffect::SamplerLineClamp,
				it.transFxData);

			m_vecTexRender[0]->Draw(m_pD3dRes->m_pD3DDeviceContext.Get());

			nTexIndex++;
		}
	}

	if (nTexIndex == 0)
		int temp = 1;

	m_vecTexRender[0]->End(m_pD3dRes->m_pD3DDeviceContext.Get());
}
