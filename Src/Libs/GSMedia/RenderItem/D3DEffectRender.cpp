#include "D3DEffectRender.h"
#include "Common/Common/XEffectCommon.h"
#include "Common/Common/MediaCommon.h"
#include "Common/Common/IMediaPropertyInterface.h"
#include "GSMedia/RenderCommon/CommonFunction.h"
#include "GSMedia/RenderBase2D/BasicEffect.h"
#include "GSMedia/RenderDevice/DeviceResource.h"

#include <QMatrix4x4>
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

CD3DEffectRender::CD3DEffectRender(ID3D11Device* device, std::shared_ptr<SMediaPty> pMediaPty, std::shared_ptr<SFxRenderData> pFxRenderData)
	: m_pMediaPty(pMediaPty)
	, m_pFxRenderData(pFxRenderData)
{
	assert(m_pMediaPty);

	m_Model.AddMesh(device, g_vecDefalutVertices, g_vecDefaultIndexs);

	m_offscreenTexsBack = std::make_unique<CTextureRender>();
	m_offscreenTexsFront = std::make_unique<CTextureRender>();
	m_pEffect = std::shared_ptr<SFxFactor>(new SFxFactor);
}

CD3DEffectRender::~CD3DEffectRender()
{
	m_vecShaderViews.clear();
	m_vecShaderViews.shrink_to_fit();

	m_Model.Reset();

	m_pTexView.Reset();
	for (auto iter : m_mapMask)
		iter.second.Reset();

	m_offscreenTexsBack.reset();
	m_offscreenTexsFront.reset();
	m_pEffect.reset();
}

void CD3DEffectRender::SetBasicEffect(BasicEffect* pEffect)
{
	m_pBasicEffect = pEffect;
}

void CD3DEffectRender::UpdateFxRenderData(microseconds mediaTs)
{
	std::lock_guard<std::mutex> locker(m_mtxVertexFxDraw);

	m_pFxRenderData->SetDefault();

	auto vecFxfactor = m_pMediaPty->pFxPty->GetAllFxFactor();
	for (auto factor : vecFxfactor)
	{
		if (!factor)
			continue;

		if (!factor->GetData()->visiable)
			continue;

		switch (factor->GetType())
		{
		case EEffectType::Shake:
		{
			m_pFxRenderData->bSupportShakePos = true;
			m_pFxRenderData->shakePos.setToIdentity();

			std::shared_ptr<SShakeFactor> pShake = std::dynamic_pointer_cast<SShakeFactor>(factor->GetData());

			SShakeFactor factor = pShake->ToEffect();
			long long cycle = (long long)(1000000 * (1.0f / factor.frequency));
			float ratio = sinf(float(1.0 * (mediaTs.count() % cycle) / cycle * 2 * 3.1415926f));
			QPointF center(factor.xOffset * ratio, factor.yOffset * ratio);

			auto szMedia = m_pMediaPty->pBasePty->GetSize();
			m_pFxRenderData->shakePos.translate(center.x() * szMedia.width(), -center.y() * szMedia.height());
		}
		break;
		case EEffectType::Scroll:
		{
			m_pFxRenderData->bSupportScroll = true;

			if (mediaTs == m_pFxRenderData->curTs)
				break;

			m_pFxRenderData->curTs = mediaTs;
			m_pFxRenderData->scrollTex.setToIdentity();

			std::shared_ptr<SScrollFactor> pScroll = std::dynamic_pointer_cast<SScrollFactor>(factor->GetData());

			SScrollFactor factor = pScroll->ToEffect();

			double x, y;
			switch (factor.dir)
			{
			case 0: // 右->左
				x = factor.speed * (m_pMediaPty->pBasePty->GetHorizontal() ? -1 : 1);
				m_pFxRenderData->fScrollXPos = fmod(m_pFxRenderData->fScrollXPos + 0.016 * x, 2.0f);
				m_pFxRenderData->fScrollYPos = 0;
				break;
			case 1: // 左->右
				x = -factor.speed * (m_pMediaPty->pBasePty->GetHorizontal() ? -1 : 1);
				m_pFxRenderData->fScrollXPos = fmod(m_pFxRenderData->fScrollXPos + 0.016 * x, 2.0f);
				m_pFxRenderData->fScrollYPos = 0;
				break;
			case 2: // 下->上
				y = factor.speed * (m_pMediaPty->pBasePty->GetVertical() ? -1 : 1);
				m_pFxRenderData->fScrollXPos = 0;
				m_pFxRenderData->fScrollYPos = fmod(m_pFxRenderData->fScrollYPos + 0.016 * y, 2.0f);
				break;
			case 3: // 上->下
				y = -factor.speed * (m_pMediaPty->pBasePty->GetVertical() ? -1 : 1);
				m_pFxRenderData->fScrollXPos = 0;
				m_pFxRenderData->fScrollYPos = fmod(m_pFxRenderData->fScrollYPos + 0.016 * y, 2.0f);
				break;

			default:
				break;
			}

			m_pFxRenderData->scrollTex.translate(m_pFxRenderData->fScrollXPos, m_pFxRenderData->fScrollYPos);
		}
		break;
		case EEffectType::Cut:
		{
			m_pFxRenderData->bSupportCrop = true;

			std::shared_ptr<SCropFactor> pCrop = std::dynamic_pointer_cast<SCropFactor>(factor->GetData());

			SCropFactor factor = pCrop->ToEffect();
			if (1.f - factor.r <= factor.l || 1.f - factor.b <= factor.t)
			{
				m_pFxRenderData->cropPos = m_pFxRenderData->cropPos * 0;
				continue;
			}

			m_pFxRenderData->cropPos.setToIdentity();
			//不填充，改位置
			if (!factor.stretch)
			{
				QRectF rc = m_pMediaPty->pBasePty->GetSizeRect();

				m_pFxRenderData->cropPos.translate(((1.f - factor.r + factor.l) / 2 - 0.5f) * rc.width(),
					((1.f - factor.b + factor.t) / 2 - 0.5f) * rc.height());

				m_pFxRenderData->cropPos.translate(rc.center().x(), rc.center().y());
				m_pFxRenderData->cropPos.scale(1.f - factor.r - factor.l, 1.f - factor.b - factor.t);
				m_pFxRenderData->cropPos.translate(-rc.center().x(), -rc.center().y());
			}

			m_pFxRenderData->cropTex.setToIdentity();
			m_pFxRenderData->cropTex.translate((1.f - factor.r + factor.l) / 2 - 0.5f, (1.f - factor.b + factor.t) / 2 - 0.5f);
			m_pFxRenderData->cropTex.translate(0.5f, 0.5f);
			m_pFxRenderData->cropTex.scale(1.f - factor.r - factor.l, 1.f - factor.b - factor.t);
			m_pFxRenderData->cropTex.translate(-0.5f, -0.5f);
		}
		break;
		default:
			break;
		}
	}
}

bool CD3DEffectRender::GetFxVertex(SVertexPosTexData &vertexNode)
{
	std::lock_guard<std::mutex> locker(m_mtxVertexFxDraw);

	auto vecFxfactor = m_pMediaPty->pFxPty->GetAllFxFactor();
	for (auto &vertex : vertexNode.vecVertexPos)
	{
		if(m_pFxRenderData->bSupportCrop)
			vertex = m_pFxRenderData->cropPos.map(vertex);

		if (m_pFxRenderData->bSupportShakePos)
			vertex = m_pFxRenderData->shakePos.map(vertex);
	}

	for (auto &vertex : vertexNode.vecVertexTex)
	{
		if(m_pFxRenderData->bSupportCrop)
			vertex = m_pFxRenderData->cropTex.map(vertex);

		if (m_pFxRenderData->bSupportScroll)
			vertex = m_pFxRenderData->scrollTex.map(vertex);
	}

	return m_pFxRenderData->bSupportScroll;
}

void CD3DEffectRender::MediaFxRender(CD3DResourceChild* deviced3d, 
	std::vector<ID3D11ShaderResourceView*> &vecViews, 
	const STexDatas &tex,
	Video_ClrMetedata &texColorPty,
	__int64 pts)
{

	if (tex.texNode0.pTex)
	{
		InitOffScreenTexs(deviced3d, tex.texNode0.texDesc.Width, tex.texNode0.texDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM);
	}

	auto vecFxfactor = m_pMediaPty->pFxPty->GetAllFxFactor();
	for (auto factor : vecFxfactor)
	{
		if (!factor)
			continue;

		if (!factor->GetData()->visiable)
			continue;

		m_pts = pts;
		m_pEffect = factor->GetData();

		m_vecShaderViews = vecViews;
		m_pBasicEffect->SetCellPty(texColorPty);

		Draw(deviced3d);

		vecViews = m_vecShaderViews;
		texColorPty.color_type = ETT_RGBA;
	}
}

void CD3DEffectRender::InitOffScreenTexs(CD3DResourceChild* deviced3d,int w, int h, DXGI_FORMAT format)
{
	if (m_width == w && m_height == h && m_format == format)
		return;

	m_width = w;
	m_height = h;
	m_format = format;

	HRESULT hr = m_offscreenTexsBack->InitResource(deviced3d->m_pD3DDevice.Get(), w, h, format);
	hr = m_offscreenTexsFront->InitResource(deviced3d->m_pD3DDevice.Get(), w, h, format);
}

void CD3DEffectRender::Draw(CD3DResourceChild* deviced3d)
{
	// 设置顶点/索引缓冲区
	UINT strides = m_Model.vertexStride;
	UINT offsets = 0;
	
	deviced3d->m_pD3DDeviceContext->IASetVertexBuffers(0, 1, m_Model.modelParts[0].vertexBuffer.GetAddressOf(), &strides, &offsets);
	deviced3d->m_pD3DDeviceContext->IASetIndexBuffer(m_Model.modelParts[0].indexBuffer.Get(), m_Model.modelParts[0].indexFormat, 0);

	deviced3d->m_pD3DDeviceContext->PSSetShaderResources(0, (int)m_vecShaderViews.size(), m_vecShaderViews.data());

	// 离屏特效 Filter（KeyingFilter/FeatherFilter 等）：
	// 全屏四边形顶点已是 NDC[-1,1]，用 VSBasic 直通，避免 VS3D 依赖残留 View/Proj 导致画空黑屏。
	// 抠像额外使用 SamplerPointClamp。
	const auto sampler = (m_pEffect->type == EEffectType::Keying)
		? BasicEffect::SamplerPointClamp
		: BasicEffect::SamplerLineClamp;
	m_pBasicEffect->SetRenderState(deviced3d->m_pD3DDeviceContext.Get(),
		BasicEffect::VSBasic, BasicEffect::PSXEffect, BasicEffect::BMCover, nullptr, sampler);

	switch (m_pEffect->type)
	{
	case EEffectType::Denoise:
	{
		CBXEffect data = m_pEffect->ToCBXEffect();

		int n = data.denoise.weight;
		do 
		{
			Denoise(deviced3d);

			n--;
			if (n > 0)
			{
				m_offscreenTexsFront.swap(m_offscreenTexsBack);

				m_vecShaderViews.clear();
				m_vecShaderViews.push_back(m_offscreenTexsFront->GetTextureShaderView());
				deviced3d->m_pD3DDeviceContext->PSSetShaderResources(0, (int)m_vecShaderViews.size(), m_vecShaderViews.data());

				m_pBasicEffect->SetCellPty(Video_ClrMetedata());
			}

		} while (n > 0);
	}
		break;
	case EEffectType::Blur:
		Blur(deviced3d);
		break;
	case EEffectType::Mosaic:
		Mosaic(deviced3d);
		break;
	case EEffectType::HSV:
		HSV(deviced3d);
		break;
	case EEffectType::Frame:
		Frame(deviced3d);
		break;
	case EEffectType::Marquees:
		Marquees(deviced3d);
		break;
	case EEffectType::Mask:
		Mask(deviced3d);
		break;
	case EEffectType::Blink:
		Blink(deviced3d);
		break;
	case EEffectType::Shadow:
		Shadow(deviced3d);
		break;
	default:
	{
		m_pBasicEffect->SetXEffectPty(m_pEffect->ToCBXEffect());
		m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
		float bk[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_offscreenTexsBack->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
		deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
		m_offscreenTexsBack->End(deviced3d->m_pD3DDeviceContext.Get());
	}
		break;
	}

	m_offscreenTexsFront.swap(m_offscreenTexsBack);

	m_vecShaderViews.clear();
	m_vecShaderViews.push_back(m_offscreenTexsFront->GetTextureShaderView());
}

void CD3DEffectRender::Denoise(CD3DResourceChild* deviced3d)
{
	CBXEffect data = m_pEffect->ToCBXEffect();
	float bk[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	if (data.denoise.type == 0)
	{
		m_pBasicEffect->SetXEffectPty(data);
		m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
		m_offscreenTexsBack->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
		deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
		m_offscreenTexsBack->End(deviced3d->m_pD3DDeviceContext.Get());
	}
	else
	{
		std::unique_ptr<CTextureRender> renderTarget1 = std::make_unique<CTextureRender>();
		renderTarget1->InitResource(deviced3d->m_pD3DDevice.Get(), m_width, m_height, m_format);

		//水平一次
		data.denoise.x = 1;
		m_pBasicEffect->SetXEffectPty(data);
		m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
		renderTarget1->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
		deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
		renderTarget1->End(deviced3d->m_pD3DDeviceContext.Get());

		// 垂直一次
		data.denoise.x = 0;
		m_pBasicEffect->SetXEffectPty(data);
		m_pBasicEffect->SetCellPty(Video_ClrMetedata());
		ComPtr<ID3D11ShaderResourceView> pResourceView1 = renderTarget1->GetTextureShaderView();
		deviced3d->m_pD3DDeviceContext->PSSetShaderResources(0, 1, pResourceView1.GetAddressOf());
		m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
		m_offscreenTexsBack->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
		deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
		m_offscreenTexsBack->End(deviced3d->m_pD3DDeviceContext.Get());
	}
}

void CD3DEffectRender::Blur(CD3DResourceChild* deviced3d)
{
	CBXEffect data = m_pEffect->ToCBXEffect();
	if (data.type == EEffectType::None)
	{
		m_pBasicEffect->SetXEffectPty(m_pEffect->ToCBXEffect());
		m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
		float bk[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_offscreenTexsBack->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
		deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
		m_offscreenTexsBack->End(deviced3d->m_pD3DDeviceContext.Get());
		return;
	}

	float bk[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	std::shared_ptr<SBlurFactor> pBlur = std::dynamic_pointer_cast<SBlurFactor>(m_pEffect);
	int width = int(pBlur->scale * m_width);
	int height = int(pBlur->scale * m_height);
	if (width <= 32)
	{
		width = 32;
		height = int(m_height / ((float)m_width / width));
	}

	//初始化两张较小的纹理用于绘制
	std::unique_ptr<CTextureRender> renderTarget1 = std::make_unique<CTextureRender>();
	std::unique_ptr<CTextureRender> renderTarget2 = std::make_unique<CTextureRender>();
	renderTarget1->InitResource(deviced3d->m_pD3DDevice.Get(), width, height, m_format);
	renderTarget2->InitResource(deviced3d->m_pD3DDevice.Get(), width, height, m_format);

	//将原纹理转移至小尺寸纹理上
	data.type = EEffectType::None;
	m_pBasicEffect->SetXEffectPty(data);
	m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
	renderTarget1->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
	deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
	renderTarget1->End(deviced3d->m_pD3DDeviceContext.Get());

	//模糊处理
	data.type = EEffectType::Blur;
	m_pBasicEffect->SetXEffectPty(data);
	m_pBasicEffect->SetCellPty(Video_ClrMetedata());
	for (int i = 0; i < pBlur->pass; i++)
	{
		// 水平一次
		data.blur.x = 1;
		m_pBasicEffect->SetXEffectPty(data);
		m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
		ComPtr<ID3D11ShaderResourceView> pResourceView1 = renderTarget1->GetTextureShaderView();
		deviced3d->m_pD3DDeviceContext->PSSetShaderResources(0, 1, pResourceView1.GetAddressOf());
		m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
		renderTarget2->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
		deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
		renderTarget2->End(deviced3d->m_pD3DDeviceContext.Get());

		// 垂直一次
		data.blur.x = 0;
		m_pBasicEffect->SetXEffectPty(data);
		m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
		ComPtr<ID3D11ShaderResourceView> pResourceView2 = renderTarget2->GetTextureShaderView();
		deviced3d->m_pD3DDeviceContext->PSSetShaderResources(0, 1, pResourceView2.GetAddressOf());
		m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
		renderTarget1->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
		deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
		renderTarget1->End(deviced3d->m_pD3DDeviceContext.Get());
	}

	//再次将纹理转移至大纹理上
	data.type = EEffectType::None;
	m_pBasicEffect->SetXEffectPty(data);
	ComPtr<ID3D11ShaderResourceView> pResourceView1 = renderTarget1->GetTextureShaderView();
	deviced3d->m_pD3DDeviceContext->PSSetShaderResources(0, 1, pResourceView1.GetAddressOf());
	m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
	m_offscreenTexsBack->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
	deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
	m_offscreenTexsBack->End(deviced3d->m_pD3DDeviceContext.Get());
}

void CD3DEffectRender::Mosaic(CD3DResourceChild* deviced3d)
{
	CBXEffect data = m_pEffect->ToCBXEffect();
	m_pBasicEffect->SetXEffectPty(data);
	m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
	float bk[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_offscreenTexsBack->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
	deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
	m_offscreenTexsBack->End(deviced3d->m_pD3DDeviceContext.Get());

	std::shared_ptr<SMosaicFactor> pMosaic = std::dynamic_pointer_cast<SMosaicFactor>(m_pEffect);
	if (data.mosaic.size <= 0 || pMosaic->width <= 0) return;

	ComPtr<ID2D1RenderTarget> pTarget;
	CreateD2DRenderTargetFromTexture(deviced3d->m_pD2DFactory.Get(), m_offscreenTexsBack->GetTexture2D(), &pTarget);
	if (nullptr == pTarget)
		return;

	auto targetSize = pTarget->GetSize();

	auto l = pMosaic->x1 * targetSize.width;
	auto t = pMosaic->y1 * targetSize.height;
	auto r = pMosaic->x2 * targetSize.width;
	auto b = pMosaic->y2 * targetSize.height;

	if (l >= r || t >= b) return;

	auto base = std::max(targetSize.width, targetSize.height);

	auto x_size = data.mosaic.size * base * pMosaic->width;
	auto y_size = data.mosaic.size * base * pMosaic->width;
	auto x_gap = data.mosaic.size * base;
	auto y_gap = data.mosaic.size * base;
	auto x_start = l + x_gap;
	auto y_start = t + y_gap;

	ComPtr<ID2D1SolidColorBrush> pBrush;
	pTarget->CreateSolidColorBrush(D2D1::ColorF(pMosaic->r, pMosaic->g, pMosaic->b, 1.0f), &pBrush);
	if (!pBrush)
		return;

	pTarget->BeginDraw();
	if (x_size > 0)
	{
		auto half = x_size / 2;
		{
			auto size = half;
			auto start = l + half / 2.0f;
			pTarget->DrawLine(D2D1::Point2F(start, t), D2D1::Point2F(start, b), pBrush.Get(), size);
		}
		while (x_start + half <= r)
		{
			pTarget->DrawLine(D2D1::Point2F(x_start, t), D2D1::Point2F(x_start, b), pBrush.Get(), x_size);
			x_start += x_gap;
		}
		if (x_start - half < r)
		{
			auto size = r - (x_start - half);
			auto start = r - size / 2.0f;
			pTarget->DrawLine(D2D1::Point2F(start, t), D2D1::Point2F(start, b), pBrush.Get(), size);
		}
	}
	if (y_size > 0)
	{
		auto half = y_size / 2;
		{
			auto size = half;
			auto start = t + half / 2.0f;
			pTarget->DrawLine(D2D1::Point2F(l, start), D2D1::Point2F(r, start), pBrush.Get(), size);
		}
		while (y_start + half <= b)
		{
			pTarget->DrawLine(D2D1::Point2F(l, y_start), D2D1::Point2F(r, y_start), pBrush.Get(), y_size);
			y_start += y_gap;
		}
		if (y_start - half < b)
		{
			auto size = b - (y_start - half);
			auto start = b - size / 2.0f;
			pTarget->DrawLine(D2D1::Point2F(l, start), D2D1::Point2F(r, start), pBrush.Get(), size);
		}
	}
	pTarget->EndDraw();
}

void CD3DEffectRender::Frame(CD3DResourceChild* deviced3d)
{
	CBXEffect data = m_pEffect->ToCBXEffect();
	m_pBasicEffect->SetXEffectPty(data);
	m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
	float bk[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_offscreenTexsBack->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
	deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
	m_offscreenTexsBack->End(deviced3d->m_pD3DDeviceContext.Get());

	std::shared_ptr<SFrameFactor> pFrame = std::dynamic_pointer_cast<SFrameFactor>(m_pEffect);
	if (data.frame.size <= 0) return;

	ComPtr<IDXGISurface> pDxgiSurf;
	m_offscreenTexsBack->GetTexture2D()->QueryInterface(__uuidof(IDXGISurface), (void**)pDxgiSurf.ReleaseAndGetAddressOf());
	if (!pDxgiSurf)
		return;

	ComPtr<ID2D1RenderTarget> pTarget;
	D2D1_RENDER_TARGET_PROPERTIES props;
	props.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
	props.dpiX = 0;
	props.dpiY = 0;
	props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	props.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
	props.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
	props.usage = D2D1_RENDER_TARGET_USAGE_NONE;
	deviced3d->m_pD2DFactory->CreateDxgiSurfaceRenderTarget(pDxgiSurf.Get(), &props, pTarget.GetAddressOf());
	if (!pTarget)
		return;

	auto minSize = std::min(m_width, m_height) * data.frame.size;
	auto sizeX = minSize / m_width;
	auto sizeY = minSize / m_height;

	auto targetSize = pTarget->GetSize();
	auto frameSizeX = targetSize.width * sizeX;
	auto frameSizeY = targetSize.height * sizeY;
	ComPtr<ID2D1SolidColorBrush> pBrush;
	D2D1_COLOR_F clr;
	clr.a = pFrame->a;
	clr.r = pFrame->r;
	clr.g = pFrame->g;
	clr.b = pFrame->b;
	pTarget->CreateSolidColorBrush(clr, &pBrush);
	if (!pBrush)
		return;

	ComPtr<ID2D1RectangleGeometry> geoInside, geoOutside;
	deviced3d->m_pD2DFactory->CreateRectangleGeometry(D2D1::RectF(0.0f, 0.0f, targetSize.width, targetSize.height), &geoOutside);
	deviced3d->m_pD2DFactory->CreateRectangleGeometry(D2D1::RectF(frameSizeX, frameSizeY, targetSize.width - frameSizeX, targetSize.height - frameSizeY), &geoInside);
	ComPtr<ID2D1GeometryGroup> geoGroup;
	ID2D1Geometry *geos[2] = { geoOutside.Get(), geoInside.Get() };
	deviced3d->m_pD2DFactory->CreateGeometryGroup(D2D1_FILL_MODE_ALTERNATE, geos, 2, &geoGroup);

	pTarget->BeginDraw();
	pTarget->FillGeometry(geoGroup.Get(), pBrush.Get());
	pTarget->EndDraw();
}

void CD3DEffectRender::HSV(CD3DResourceChild* deviced3d)
{
	CBXEffect data = m_pEffect->ToCBXEffect();
	data.hsv.h = (fmod(m_pts / 1000000.0f, float(data.hsv.h)) / float(data.hsv.h) - 0.5f) * 2.0f;
	if (data.hsv.h == 0.0f)
		data.type = EEffectType::None;

	m_pBasicEffect->SetXEffectPty(data);
	m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
	float bk[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_offscreenTexsBack->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
	deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
	m_offscreenTexsBack->End(deviced3d->m_pD3DDeviceContext.Get());
}

void CD3DEffectRender::Marquees(CD3DResourceChild* deviced3d)
{
	std::shared_ptr<SMarqueesFactor> pMarquees = std::dynamic_pointer_cast<SMarqueesFactor>(m_pEffect);
	CBXEffect data = m_pEffect->ToCBXEffect();

	auto minSize = std::min(m_width, m_height) * pMarquees->size / 2;
	auto sizeX = minSize / m_width;
	auto sizeY = minSize / m_height;

	data.marquees.dir = pMarquees->dir;
	data.marquees.x = sizeX;
	data.marquees.y = sizeY;
	data.marquees.l = 1.0f / pMarquees->length;
	data.marquees.d0 = fmod(pMarquees->speed * m_pts / 1000000.0f, data.marquees.l);
	if (data.marquees.x > 0 && data.marquees.x <= 0.5f&&data.marquees.y > 0 && data.marquees.y <= 0.5f)
	{
		std::vector<UINT8> colors;
		colors.reserve(pMarquees->nColorCount * 4);
		for (int i = pMarquees->nColorCount - 1; i >= 0; i--)
		{
			colors.push_back(UINT8(pMarquees->colors[i].red()));
			colors.push_back(UINT8(pMarquees->colors[i].green()));
			colors.push_back(UINT8(pMarquees->colors[i].blue()));
			colors.push_back(255);
		}

		if (m_colors != colors)
		{
			m_colors = colors;

			ComPtr<ID3D11Texture2D> pTex;
			CreateD3DTextureFromMemory(deviced3d->m_pD3DDevice.Get(), colors.data(), (UINT)pMarquees->nColorCount, 1, DXGI_FORMAT_R8G8B8A8_UNORM, pTex.ReleaseAndGetAddressOf());
			if (pTex)
				deviced3d->m_pD3DDevice->CreateShaderResourceView((ID3D11Resource*)pTex.Get(), nullptr, m_pTexView.ReleaseAndGetAddressOf());
		}
			
		deviced3d->m_pD3DDeviceContext->PSSetShaderResources(3, 1, m_pTexView.GetAddressOf());
	}

	m_pBasicEffect->SetRenderState(deviced3d->m_pD3DDeviceContext.Get(), BasicEffect::VSBasic, BasicEffect::PSXEffect, BasicEffect::BMCover, nullptr, BasicEffect::SamplerLineWrap);
	m_pBasicEffect->SetXEffectPty(data);
	m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
	float bk[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_offscreenTexsBack->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
	deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
	m_offscreenTexsBack->End(deviced3d->m_pD3DDeviceContext.Get());
}

void CD3DEffectRender::Mask(CD3DResourceChild* deviced3d)
{
	deviced3d->m_pD3DDeviceContext->PSSetShaderResources(3, 1, GetMaskView(deviced3d).GetAddressOf());

	m_pBasicEffect->SetXEffectPty(m_pEffect->ToCBXEffect());
	m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
	float bk[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_offscreenTexsBack->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
	deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
	m_offscreenTexsBack->End(deviced3d->m_pD3DDeviceContext.Get());
}

void CD3DEffectRender::Blink(CD3DResourceChild* deviced3d)
{
	CBXEffect data = m_pEffect->ToCBXEffect();
	std::shared_ptr<SBlinkFactor> pBlink = std::dynamic_pointer_cast<SBlinkFactor>(m_pEffect);

	float shadow = pBlink->shadow / 4.0f;
	float freq = std::max(0.1f, pBlink->freq * 10.0f);
	auto T = 1000000 / freq;
	auto tLast = pBlink->last * T;
	auto tCur = (float)fmod(m_pts, T);
	auto tIn = tLast * shadow;
	auto tOut = tLast - tIn;
	auto alpha = 0.0f;
	if (tCur < tIn)
	{
		alpha = tCur / tIn;
	}
	else if (tCur < tOut)
	{
		alpha = 1.0f;
	}
	else if (tCur < tLast)
	{
		alpha = 1.0f - (tCur - tOut) / tIn;
	}
	else
		data.type = EEffectType::None;

	data.blink.a = alpha;


	m_pBasicEffect->SetXEffectPty(data);
	m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
	float bk[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_offscreenTexsBack->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);
	deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
	m_offscreenTexsBack->End(deviced3d->m_pD3DDeviceContext.Get());
}

void CD3DEffectRender::Shadow(CD3DResourceChild* deviced3d)
{
	CBXEffect data = m_pEffect->ToCBXEffect();
	std::shared_ptr<SShadowFactor> pShadow = std::dynamic_pointer_cast<SShadowFactor>(m_pEffect);

	std::vector<VertexPosTex> vertices = {
{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) } ,
{ DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) } ,
{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) } };
	std::vector<DWORD> indexs = { 0,1,2,2,3,0 };

	//根据阴影数据修改顶点数据
	auto scale = 1.0f / pShadow->size;
	auto rotate = (pShadow->rotate / 360.0f) * 2 * 3.1415926f;
	auto distance = pShadow->distance * (1.0f - scale);
	auto x = cos(rotate) * distance, y = sin(rotate) * distance;

	for (auto &v : vertices)
	{
		v.pos.x = v.pos.x * scale;
		v.pos.y = v.pos.y * scale;
	}
	for (auto &v : vertices)
	{
		v.pos.x = v.pos.x + x;
		v.pos.y = v.pos.y + y;
	}

	//由于阴影特效需要绘制两次，第一次仅绘制阴影，无偏移，第二次绘制纹理，带偏移
	if (m_Model.modelParts.size() < 2)
		m_Model.AddMesh(deviced3d->m_pD3DDevice.Get(), vertices, indexs);
	else
		m_Model.UpdataVertexBuffer(deviced3d->m_pD3DDeviceContext.Get(), 1, vertices.data());
	
	UINT strides = m_Model.vertexStride;
	UINT offsets = 0;
	float bk[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_offscreenTexsBack->Begin(deviced3d->m_pD3DDeviceContext.Get(), bk, true);

	//绘制纹理
	data.type = EEffectType::None;
	m_pBasicEffect->SetXEffectPty(data);
	m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
	deviced3d->m_pD3DDeviceContext->IASetVertexBuffers(0, 1, m_Model.modelParts[1].vertexBuffer.GetAddressOf(), &strides, &offsets);
	deviced3d->m_pD3DDeviceContext->IASetIndexBuffer(m_Model.modelParts[1].indexBuffer.Get(), m_Model.modelParts[1].indexFormat, 0);
	deviced3d->m_pD3DDeviceContext->PSSetShaderResources(0, (int)m_vecShaderViews.size(), m_vecShaderViews.data());
	deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[1].indexCount, 0, 0);

	//绘制阴影
	data.type = EEffectType::Shadow;
	m_pBasicEffect->SetXEffectPty(data);
	m_pBasicEffect->Apply(deviced3d->m_pD3DDeviceContext.Get());
	deviced3d->m_pD3DDeviceContext->IASetVertexBuffers(0, 1, m_Model.modelParts[0].vertexBuffer.GetAddressOf(), &strides, &offsets);
	deviced3d->m_pD3DDeviceContext->IASetIndexBuffer(m_Model.modelParts[0].indexBuffer.Get(), m_Model.modelParts[0].indexFormat, 0);
	deviced3d->m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);

	m_offscreenTexsBack->End(deviced3d->m_pD3DDeviceContext.Get());
}

void CD3DEffectRender::UpdateMediaPty(std::shared_ptr<SMediaPty> pMediaPty)
{
	m_pMediaPty = pMediaPty;
}

ComPtr<ID3D11ShaderResourceView> CD3DEffectRender::GetMaskView(CD3DResourceChild* deviced3d)
{
	std::shared_ptr<SMaskFactor> pMask = std::dynamic_pointer_cast<SMaskFactor>(m_pEffect);

	if (pMask->MaskType == EMT_CustomLayer)
	{
		if (pMask->filename=="-")
			m_strFileName = pMask->filename;
		else if (m_strFileName == pMask->filename)
		{
			if (m_mapMask[pMask->MaskType])
			{
				return m_mapMask[pMask->MaskType];
			}
		}
		else
		{
			QString path = pMask->filename;
			QImage image(path);

			if (!image.isNull())
			{
				ComPtr<ID3D11Texture2D> pMaskTex = nullptr;
				CreateD3DTextureFromMemory(deviced3d->m_pD3DDevice.Get(), image.bits(), image.width(), image.height(),
					DXGI_FORMAT_B8G8R8A8_UNORM, pMaskTex.ReleaseAndGetAddressOf());

				ComPtr<ID3D11ShaderResourceView> pView;
				deviced3d->m_pD3DDevice->CreateShaderResourceView((ID3D11Resource*)pMaskTex.Get(), nullptr, pView.ReleaseAndGetAddressOf());
				m_mapMask[pMask->MaskType] = pView;
				m_strFileName = pMask->filename;

				return pView;
			}
		}
	}

	/*if (m_mapMask.find(pMask->MaskType) != m_mapMask.end()&& !pMask->bDirty)
		return m_mapMask[pMask->MaskType];*/

	pMask->bDirty = false;
	ComPtr<ID2D1Geometry> pGeo = CreateMaskGeo(deviced3d->m_pD2DFactory.Get(), pMask->MaskType);
	if (nullptr == pGeo)
		return nullptr;

#define  Mask_Width		500
#define  Mask_Height	500

	ComPtr<ID3D11Texture2D> pMaskTex = nullptr;
	CreateD3DTextureForDraw(deviced3d->m_pD3DDevice.Get(), Mask_Width, Mask_Height, DXGI_FORMAT_A8_UNORM, D3D11_BIND_RENDER_TARGET, pMaskTex.ReleaseAndGetAddressOf());
	if (nullptr == pMaskTex)
		return nullptr;

	ComPtr<ID2D1RenderTarget> pTarget = nullptr;
	CreateD2DRenderTargetFromTexture(deviced3d->m_pD2DFactory.Get(), pMaskTex.Get(), &pTarget);
	if (nullptr == pTarget)
		return nullptr;

	ComPtr<ID2D1SolidColorBrush> pBrush;
	pTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &pBrush);
	if (nullptr == pBrush)
		return nullptr;

	pTarget->BeginDraw();
	if (pMask->MaskType == EMT_NoneType || pMask->MaskType == EMT_CustomLayer)
	{
		pTarget->SetTransform(
			D2D1::Matrix3x2F::Scale(pTarget->GetSize())
		);
	}
	else
	{
		D2D1_POINT_2F pt = { pMask->x, pMask->y };
		pTarget->SetTransform(
			D2D1::Matrix3x2F::Translation(pMask->x - 0.5f, pMask->y - 0.5f)*
			D2D1::Matrix3x2F::Scale(pMask->w, pMask->h, pt)*
			D2D1::Matrix3x2F::Scale(pTarget->GetSize())
		);
	}
	pTarget->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
	pTarget->FillGeometry(pGeo.Get(), pBrush.Get());
	pTarget->EndDraw();

	ComPtr<ID3D11ShaderResourceView> pView;
	deviced3d->m_pD3DDevice->CreateShaderResourceView((ID3D11Resource*)pMaskTex.Get(), nullptr, pView.ReleaseAndGetAddressOf());
	m_mapMask[pMask->MaskType] = pView;

	return pView;
}

// 绘制类似 SVG 的路径:
//    - L: 直线, 一个点一组
//    - Q: 二次贝塞尔曲线, 二个点一组
//    - C: 三次贝塞尔曲线, 三个点一组
static ComPtr<ID2D1Geometry> CreateGeometry(ComPtr<ID2D1Factory> pD2DFactory, D2D1_POINT_2F m, std::vector<std::pair<char, std::vector<D2D1_POINT_2F>>> const &segments)
{
	ComPtr<ID2D1GeometrySink> pSink;
	ComPtr<ID2D1PathGeometry> pPathGeo;
	pD2DFactory->CreatePathGeometry(pPathGeo.ReleaseAndGetAddressOf());
	if (pPathGeo)
	{
		pPathGeo->Open(pSink.ReleaseAndGetAddressOf());
	}
	if (!pSink)
		return nullptr;

	pSink->BeginFigure(m, D2D1_FIGURE_BEGIN_FILLED);

	for (auto &segment : segments)
	{
		switch (segment.first)
		{
		case 'L':
			pSink->AddLines(segment.second.data(), (UINT32)segment.second.size());
			break;
		case 'Q':
			assert(segment.second.size() && segment.second.size() % 2 == 0);
			pSink->AddQuadraticBeziers((D2D1_QUADRATIC_BEZIER_SEGMENT*)segment.second.data(), (UINT32)segment.second.size() / 2);
			break;
		case 'C':
			assert(segment.second.size() && segment.second.size() % 3 == 0);
			pSink->AddBeziers((D2D1_BEZIER_SEGMENT*)segment.second.data(), (UINT32)segment.second.size() / 3);
			break;
		}
	}

	pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

	pSink->Close();
	return pPathGeo;
}

static inline ComPtr<ID2D1Geometry> CreateLinesGeometry(ComPtr<ID2D1Factory> pD2DFactory, D2D1_POINT_2F const *pts, size_t n)
{
	if (!pts || !n)
		return nullptr;
	auto segment = std::make_pair('L', std::vector<D2D1_POINT_2F>(pts, pts + n));
	return CreateGeometry(pD2DFactory, pts[n - 1], { segment });
}

ComPtr<ID2D1Geometry> CD3DEffectRender::CreateMaskGeo(ComPtr<ID2D1Factory> pD2DFactory, int nType)
{
	switch (nType)
	{
	case EMT_NoneType:
	{
		D2D1_POINT_2F pts[] = {
			D2D1::Point2F(0.0f, 0.0f),
			D2D1::Point2F(0.0f, 1.0f),
			D2D1::Point2F(1.0f, 1.0f),
			D2D1::Point2F(1.0f, 0.0f),
		};
		return CreateLinesGeometry(pD2DFactory, pts, _countof(pts));
	}
	case EMT_CustomLayer:
	{
		D2D1_POINT_2F pts[] = {
			D2D1::Point2F(0.0f, 0.0f),
			D2D1::Point2F(0.0f, 1.0f),
			D2D1::Point2F(1.0f, 1.0f),
			D2D1::Point2F(1.0f, 0.0f),
		};
		return CreateLinesGeometry(pD2DFactory, pts, _countof(pts));
	}
	case EMT_3:
	{
		D2D1_POINT_2F pts[] = {
			D2D1::Point2F(0.5f, 0.0f),
			D2D1::Point2F(1.0f, 1.0f),
			D2D1::Point2F(0.0f, 1.0f),
		};
		return CreateLinesGeometry(pD2DFactory, pts, _countof(pts));
	}
	case EMT_flower:
	{
		// 五个圆形

		// 0.5, 0.5
		//auto s = 0.6f;
		//D2D1_POINT_2F pts[] = { // copy from sss
		//	D2D1::Point2F(0.5f, 0.5f),
		//	D2D1::Point2F(0.5, 0),
		//	D2D1::Point2F(1, 0.381966),
		//	D2D1::Point2F(0.809017, 1),
		//	D2D1::Point2F(0.190983, 1),
		//	D2D1::Point2F(0, 0.381966),
		//};
		//for (int i = 1; i < 6; i++)
		//{
		//	auto &p = pts[i];
		//	p.x = (p.x - 0.5f) * s + 0.5f;
		//	p.y = (p.y - 0.5f) * s + 0.5f;
		//}
		//std::stringstream ss;
		//for (auto &p : pts)
		//{
		//	ss << "D2D1::Point2F(" << p.x << "f, " << p.y << "f),\n";
		//}
		//auto sss = ss.str();

		D2D1_POINT_2F centers[] = { // copy from sss
			D2D1::Point2F(0.5f, 0.5f),
			D2D1::Point2F(0.5f, 0.2f),
			D2D1::Point2F(0.8f, 0.42918f),
			D2D1::Point2F(0.68541f, 0.8f),
			D2D1::Point2F(0.31459f, 0.8f),
			D2D1::Point2F(0.2f, 0.42918f),
		};
		auto r = centers[1].y;
		auto rc = r * 1.15f;

		ComPtr<ID2D1EllipseGeometry> ellipses[6];
		// 覆盖中心
		pD2DFactory->CreateEllipseGeometry(D2D1::Ellipse(centers[0], rc, rc), ellipses[0].ReleaseAndGetAddressOf());
		for (int i = 1; i < 6; i++)
		{
			pD2DFactory->CreateEllipseGeometry(D2D1::Ellipse(centers[i], r, r), ellipses[i].ReleaseAndGetAddressOf());
		}
		ComPtr<ID2D1GeometryGroup> pGroup;
		pD2DFactory->CreateGeometryGroup(D2D1_FILL_MODE_WINDING, (ID2D1Geometry**)ellipses[0].GetAddressOf(), 6, pGroup.ReleaseAndGetAddressOf());
		return pGroup;
	}
	case EMT_4:
	{
		D2D1_POINT_2F pts[] = {
			D2D1::Point2F(0.5f, 0.0f),
			D2D1::Point2F(1.0f, 0.5f),
			D2D1::Point2F(0.5f, 1.0f),
			D2D1::Point2F(0.0f, 0.5f),
		};
		return CreateLinesGeometry(pD2DFactory, pts, _countof(pts));
	}
	case EMT_heart:
	{
		// 心形曲线
		auto segment = std::make_pair('C', std::vector<D2D1_POINT_2F>{
			{ 0.537f, 0.9582f },
			{ 0.7489f,	0.7194f },
			{ 0.8813f,	0.5703f },
			{ 1.0f,		0.4367f },
			{ 0.9974f,	0.2418f },
			{ 0.8896f,	0.121f },
			{ 0.7817f,	0.0f },
			{ 0.6073f,	0.0005f },
			{ 0.5f,		0.122f },
			{ 0.3926f,	0.0005f },
			{ 0.2182f,	0.0f },
			{ 0.1104f,	0.1209f },
			{ 0.0025f,	0.2417f },
			{ 0.0f,		0.4366f },
			{ 0.1186f,	0.5703f },
			{ 0.251f,	0.7194f },
			{ 0.4629f,	0.9582f },
			{ 0.5f,		1.0f },
		});
		return CreateGeometry(pD2DFactory, { 0.5f, 1.0f }, { segment });
	}
	case EMT_6:
	{
		//auto R = 1.0f / 2.0f / sin(PI / 3.0f);
		//D2D1_POINT_2F pts[6];
		//pts[0].x = 0.5f - R / 2.0f;
		//pts[0].y = 0.0f;
		//pts[1].x = 0.5f + R / 2.0f;
		//pts[1].y = pts[0].y;
		//pts[2].x = 0.5f + R;
		//pts[2].y = 0.5f;
		//pts[5].x = 0.5f - R;
		//pts[5].y = pts[2].y;
		//pts[3].x = pts[1].x;
		//pts[3].y = 1.0f;
		//pts[4].x = pts[0].x;
		//pts[4].y = pts[3].y;
		//auto x = pts[5].x;
		//for (auto &p : pts)
		//{
		//	p.x -= x;
		//}
		//x = 1.0f / pts[2].x;
		//for (auto &p : pts)
		//{
		//	p.x *= x;
		//}
		//std::stringstream ss;
		//for (auto &p : pts)
		//{
		//	ss << "\t\t\t\t\t\tD2D1::Point2F(" << p.x << ", " << p.y << "),\n";
		//}
		//auto sss = ss.str();
		D2D1_POINT_2F pts[] = { // copy from sss
			D2D1::Point2F(0.25, 0),
			D2D1::Point2F(0.75, 0),
			D2D1::Point2F(1, 0.5),
			D2D1::Point2F(0.75, 1),
			D2D1::Point2F(0.25, 1),
			D2D1::Point2F(0, 0.5),
		};
		return CreateLinesGeometry(pD2DFactory, pts, _countof(pts));
	}
	case EMT_ellipse:
	{
		ComPtr<ID2D1EllipseGeometry> pEllipse;
		pD2DFactory->CreateEllipseGeometry(D2D1::Ellipse(D2D1::Point2F(0.5f, 0.5f), 0.5f, 0.45f), pEllipse.ReleaseAndGetAddressOf());
		return pEllipse;
	}
	case EMT_5:
	{
		//auto r = 2.0f * PI / 5.0f;
		//auto r18 = PI / 2.0f - r;
		//auto r54 = r - r18;
		//auto r36 = r / 2.0f;
		//auto R1 = 0.45f, R2 = R1 * sin(r18) / cos(r36);
		//D2D1_POINT_2F pts[10]; // 中心为原点
		//pts[0] = D2D1::Point2F(0.0f, R1);
		//pts[1] = D2D1::Point2F(R2 * cos(r54), R2 * sin(r54));
		//pts[9] = D2D1::Point2F(-pts[1].x, pts[1].y);
		//pts[2] = D2D1::Point2F(R1 * cos(r18), R1 * sin(r18));
		//pts[8] = D2D1::Point2F(-pts[2].x, pts[2].y);
		//pts[3] = D2D1::Point2F(R2 * cos(r18), -R2 * sin(r18));
		//pts[7] = D2D1::Point2F(-pts[3].x, pts[3].y);
		//pts[4] = D2D1::Point2F(R1 * cos(r54), -R1 * sin(r54));
		//pts[6] = D2D1::Point2F(-pts[4].x, pts[4].y);
		//pts[5] = D2D1::Point2F(0.0f, -R2);
		//for (auto &p : pts) // 纹理坐标系
		//{
		//	p.x += 0.5f;
		//	p.y -= 0.5f;
		//	p.y = -p.y;
		//}
		//auto x = pts[8].x;
		//auto y = pts[0].y;
		//for (auto &p : pts)
		//{
		//	p.y -= y;
		//	p.x -= x;
		//}
		//x = 1.0f / pts[2].x;
		//y = 1.0f / pts[4].y;
		//for (auto &p : pts)
		//{
		//	p.x *= x;
		//	p.y *= y;
		//}
		//std::stringstream ss;
		//for (auto &p : pts)
		//{
		//	ss << "D2D1::Point2F(" << p.x << ", " << p.y << "),\n";
		//}
		//auto sss = ss.str();
		D2D1_POINT_2F pts[] = { // copy from sss
			D2D1::Point2F(0.5f, 0.0f),
			D2D1::Point2F(0.618034f, 0.381966f),
			D2D1::Point2F(1.0f, 0.381966f),
			D2D1::Point2F(0.690983f, 0.618034f),
			D2D1::Point2F(0.809017f, 1.0f),
			D2D1::Point2F(0.5f, 0.763932f),
			D2D1::Point2F(0.190983f, 1.0f),
			D2D1::Point2F(0.309017f, 0.618034f),
			D2D1::Point2F(0.0f, 0.381966f),
			D2D1::Point2F(0.381966f, 0.381966f),
		};
		return CreateLinesGeometry(pD2DFactory, pts, _countof(pts));
	}
	case EMT_rrect:
	{
		ComPtr<ID2D1RoundedRectangleGeometry> pRoundedRect;
		pD2DFactory->CreateRoundedRectangleGeometry(D2D1::RoundedRect(D2D1::RectF(0.0f, 0.0f, 1.0f, 1.0f), 0.2f, 0.2f), pRoundedRect.ReleaseAndGetAddressOf());
		return pRoundedRect;
	}
	case EMT_circle:
	{
		ComPtr<ID2D1EllipseGeometry> pEllipse;
		pD2DFactory->CreateEllipseGeometry(D2D1::Ellipse(D2D1::Point2F(0.5f, 0.5f), 0.5f, 0.5f), pEllipse.ReleaseAndGetAddressOf());
		return pEllipse;
	}
	default:
	{
		assert(!"未知的蒙版类型!");
		return nullptr;
	}
	}
	return nullptr;
}
