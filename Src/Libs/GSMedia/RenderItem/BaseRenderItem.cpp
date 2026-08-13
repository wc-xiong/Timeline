#include "BaseRenderItem.h"

#include "Common/Common/CommonDefine.h"
#include "Common/Common/MediaCommon.h"
#include "Common/Common/TransationTypeDefine.h"
#include "Common/Common/TexFrameCommon.h"
#include "Common/Common/XEffectCommon.h"
#include "Common/Common/VideoDataCommon.h"
#include "Common/Common/ResourceDefine.h"
#include "Common/Common/IMediaPropertyInterface.h"

#include "GSMedia/RenderCommon/TextureRender.h"
#include "GSMedia/RenderCommon/CommonFunction.h"
#include "GSMedia/RenderBase2D/BasicEffect.h"
#include "GSMedia/RenderDevice/DeviceResource.h"

#include <QPixmap>
#include <vector>
#include <QDebug>
#include <time.h>

#include "RenderDrawDataDefine.h"

#include "RenderCommonApi.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

#include <DirectXMath.h>
#include <QMatrix4x4>
#include <QVector4D>
#include <algorithm>

// 将DirectX的XMMATRIX转换为Qt的QMatrix4x4
static QMatrix4x4 XMMatrixToQMatrix4x4(const DirectX::XMMATRIX& dxMatrix)
{
	// 第一步：将SIMD格式的XMMATRIX导出到普通浮点数组格式的XMFLOAT4X4
	DirectX::XMFLOAT4X4 floatMat;
	DirectX::XMStoreFloat4x4(&floatMat, dxMatrix);

	// 第二步：直接构造QMatrix4x4（避免中间数组拷贝）
	// 注意：XMFLOAT4X4的成员命名规则：_行号列号
	QMatrix4x4 qtMatrix(
		// 第1列（对应XMMATRIX的第1行）
		floatMat._11, floatMat._21, floatMat._31, floatMat._41,
		// 第2列（对应XMMATRIX的第2行）
		floatMat._12, floatMat._22, floatMat._32, floatMat._42,
		// 第3列（对应XMMATRIX的第3行）
		floatMat._13, floatMat._23, floatMat._33, floatMat._43,
		// 第4列（对应XMMATRIX的第4行）
		floatMat._14, floatMat._24, floatMat._34, floatMat._44
	);

	// 【核心】行优先→列优先的转换：你的原代码通过转置实现，这里直接构造时就匹配列优先，无需额外转置
	// （两种方式效果等价，直接构造更高效）

	return qtMatrix;
}

CBaseRenderItem::CBaseRenderItem(std::shared_ptr<SMediaPty> pMediaPty, CD3DResourceChild* pD3dRes, bool bUseCopyTexRender)
	: m_mediaId(pMediaPty->pBasePty->GetMediaId())
	, m_bUseCopyTexRender(bUseCopyTexRender)
{
	m_pMediaPty = pMediaPty;
	m_pD3dRes = pD3dRes;

	switch (pMediaPty->pBasePty->GetResType())
	{
	case ERT_Video:
	case ERT_Picture:
	{
		QSizeF szRes = pMediaPty->pBasePty->GetOriginalSize();
		UpdateTexRenderByWidthHeight(szRes.width(), szRes.height());
	}
	break;
	default:
	{
		QSizeF mediaSize = pMediaPty->pBasePty->GetSize();
		UpdateTexRenderByWidthHeight(mediaSize.width(), mediaSize.height());
	}
	break;
	}
}

CBaseRenderItem::~CBaseRenderItem()
{
	m_pMediaPty = nullptr;
	for (auto pTexRender : m_vecTexRender)
	{
		delete pTexRender;
	}
	m_vecTexRender.clear();
	m_pD3dRes = nullptr;
}

int	CBaseRenderItem::GetType() 
{
	if (m_pMediaPty->pBasePty)
		return m_pMediaPty->pBasePty->GetResType();
	return -1;
}

void CBaseRenderItem::SetBasicEffect(BasicEffect* pEffect)
{
	m_pBasicEffect = pEffect;
}

void CBaseRenderItem::Render(const SDrawTextureData& texNode, const SColorPty & globalPty)
{
	m_sDrawTexData = texNode;

	for (const auto& it : m_vecTexRender)
		it->SetShaderViews(std::vector<ID3D11ShaderResourceView*>());

	DrawTexture(globalPty, m_sDrawTexData.vecTextureFrame);
}

SRenderItemTex CBaseRenderItem::GetDrawData()
{
	return m_drawData;
}

bool CBaseRenderItem::ConsumePublishTexDirty()
{
	if (!m_bPublishTexDirty)
		return false;
	m_bPublishTexDirty = false;
	return true;
}

void CBaseRenderItem::UpdateMediaPtyByRedoUndo(std::shared_ptr<SMediaPty> pMediaPty)
{
	m_pMediaPty = pMediaPty;
}

void CBaseRenderItem::ResetTexRender(const STexDatas& texNode)
{
	return;

	if (!texNode.texNode0.pTex)
		return;

	int nTexWidth = texNode.texNode0.texDesc.Width;
	int nTexHeight = texNode.texNode0.texDesc.Height;

	int newWidth = std::min(nTexWidth, MAX_RESOURCE_WIDTH);
	int newHeight = std::min(nTexHeight, MAX_RESOURCE_HEIGHT);

	UpdateTexRenderByWidthHeight(newWidth, newHeight);
}

void CBaseRenderItem::UpdateTexRenderByWidthHeight(int newWidth, int newHeight)
{
	if (abs(m_drawData.uvWidth - newWidth) < 1e-2 && abs(m_drawData.uvHeight - newHeight) < 1e-2)
	{
		return;
	}

	const int tileCountX = (newWidth + MAX_RESOURCE_WIDTH - 1) / MAX_RESOURCE_WIDTH;
	const int tileCountY = (newHeight + MAX_RESOURCE_HEIGHT - 1) / MAX_RESOURCE_HEIGHT;
	const int totalTiles = tileCountX * tileCountY;

	while (static_cast<int>(m_vecTexRender.size()) > totalTiles)
	{
		delete m_vecTexRender.back();
		m_vecTexRender.pop_back();
	}
	m_vecTexRender.resize(totalTiles);

	m_drawData.uvWidth = static_cast<float>(newWidth);
	m_drawData.uvHeight = static_cast<float>(newHeight);
	m_drawData.texs.resize(totalTiles);

	for (int tileY = 0; tileY < tileCountY; tileY++)
	{
		for (int tileX = 0; tileX < tileCountX; tileX++)
		{
			const int tileWidth = (tileX == tileCountX - 1) ? newWidth - tileX * MAX_RESOURCE_WIDTH : MAX_RESOURCE_WIDTH;
			const int tileHeight = (tileY == tileCountY - 1) ? newHeight - tileY * MAX_RESOURCE_HEIGHT : MAX_RESOURCE_HEIGHT;
			const int oneDimIndex = tileX + tileY * tileCountX;

			CTextureRender*& pTexRender = m_vecTexRender.at(oneDimIndex);
			const bool texReady = pTexRender
				&& pTexRender->GetTexWidth() == tileWidth
				&& pTexRender->GetTexHeight() == tileHeight
				&& pTexRender->IsUseCopyTex() == m_bUseCopyTexRender
				&& pTexRender->GetTexHandle() != nullptr;

			if (texReady)
			{
				continue;
			}

			if (!pTexRender)
			{
				pTexRender = new CTextureRender(m_pD3dRes->m_pD3DDevice.Get());
			}

			// 拷贝模式由 CTextureRender 内部强制主 RT 非共享，并创建 SHARED 拷贝面
			const bool bSharedMain = !m_bUseCopyTexRender;
			pTexRender->InitResource(
				m_pD3dRes->m_pD3DDevice.Get(),
				tileWidth,
				tileHeight,
				DXGI_FORMAT_R8G8B8A8_UNORM,
				bSharedMain,
				false,
				m_bUseCopyTexRender);

			SRenderItemSegmentTex& segTex = m_drawData.texs.at(oneDimIndex);
			segTex.texHandle = pTexRender->GetTexHandle();
			m_bPublishTexDirty = true;

			segTex.rcTex = D2D1_RECT_D(
				tileX * MAX_RESOURCE_WIDTH,
				tileY * MAX_RESOURCE_HEIGHT,
				tileX * MAX_RESOURCE_WIDTH + tileWidth,
				tileY * MAX_RESOURCE_HEIGHT + tileHeight);
		}
	}
}

void CBaseRenderItem::UpdateEffectPty(
	const Video_ClrMetedata& frameType,
	const SColorPty& globalPty,
	int nSampleRate,
	const STransInfo& transFx)
{
	m_pBasicEffect->SetWorldMatrix(DirectX::XMMatrixIdentity());
	m_pBasicEffect->SetViewMatrix(DirectX::XMMatrixIdentity());
	m_pBasicEffect->SetProjMatrix(DirectX::XMMatrixIdentity());

	m_clrMeteData = frameType;
	SColorPty cellClr = m_pMediaPty->pEffectPty->GetColorProperty();

	// 时间线重叠区淡入淡出：统一在此乘 alpha，Base/Fx/Scroll 的 DrawTexture 均可生效
	if (transFx.nTransType == ETFT_Fade && transFx.stage != ETFS_Normal)
	{
		const float factor = (transFx.stage == ETFS_In)
			? transFx.fProgress
			: (1.0f - transFx.fProgress);
		cellClr.alpha *= factor;
	}

	// 不透明度 < 1 时强制 Alpha 混合（含转场淡入淡出）
	const float effectAlpha = cellClr.alpha;
	const auto blendMode = (effectAlpha < 0.999f)
		? BasicEffect::BMBlend
		: BasicEffect::BlendMode(m_pMediaPty->pBasePty->GetBlendType());
	m_pBasicEffect->SetRenderState(
		m_pD3dRes->m_pD3DDeviceContext.Get(), 
		BasicEffect::VS3D, BasicEffect::PS3D, 
		blendMode, 
		nullptr, BasicEffect::SamplerState(nSampleRate)
	);
	m_pBasicEffect->SetCellPty(frameType);
	m_pBasicEffect->SetCellClr(cellClr);
	m_pBasicEffect->SetGlobalClr(globalPty);

	m_pBasicEffect->Apply(m_pD3dRes->m_pD3DDeviceContext.Get());
}

void CBaseRenderItem::DrawTexture(const SColorPty & globalPty, std::vector<SDrawTexture> vecFrameTexs)
{
	//ResetTexRender();

	float backClr[4] = { 0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 0.0f };
	for (const auto& renderItem : m_vecTexRender)
	{
		renderItem->Begin(m_pD3dRes->m_pD3DDeviceContext.Get(), backClr, true);

		for (auto it : vecFrameTexs)
		{
			auto& texs = it.textureFrame;
			for (auto iter : texs.m_texs)
			{
				UpdateVertexData(iter);

				auto vecSharedView = GetShaderResView(iter, texs.m_TexColorPty);
				renderItem->SetShaderViews(vecSharedView);

				UpdateEffectPty(texs.m_TexColorPty, globalPty, BasicEffect::SamplerLineClamp, it.transFxData);

				renderItem->Draw(m_pD3dRes->m_pD3DDeviceContext.Get());
			}
		}
		renderItem->End(m_pD3dRes->m_pD3DDeviceContext.Get());
	}
}

void CBaseRenderItem::UpdateVertexData(std::vector<VertexPosTex>& vertices, std::vector<DWORD>& indexs, const STexDatas& texNode)
{
	SVertexPosTexData vertexNode = CBaseRenderItem::GetVertexTexData(QSize(m_drawData.uvWidth, m_drawData.uvHeight), vertices);

	VertexFillTypeDeal(vertexNode, texNode);

	m_vecTexRender.at(0)->SetVertexBuffer(m_pD3dRes->m_pD3DDevice.Get(), GetVertexBufferByPosTexData(vertexNode, QSize(m_drawData.uvWidth, m_drawData.uvHeight)), indexs);
}

void CBaseRenderItem::UpdateVertexData(const STexDatas& texNode)
{
	SVertexPosTexData vertexNode = CBaseRenderItem::GetVertexTexData(QSize(m_drawData.uvWidth, m_drawData.uvHeight));

	VertexFillTypeDeal(vertexNode, texNode);

	m_vecTexRender.at(0)->SetVertexBuffer(m_pD3dRes->m_pD3DDeviceContext.Get(), GetVertexBufferByPosTexData(vertexNode, QSize(m_drawData.uvWidth, m_drawData.uvHeight)));
}

std::vector<VertexPosTex> CBaseRenderItem::GetVertexBufferByPosTexData(const SVertexPosTexData & vertexNode, const QSize& offscreenRc)
{
	// 镜像、旋转属性每个媒体都有，且必须放在最后，故集成在基类中
	std::vector<VertexPosTex> vecVertices;
	vecVertices.resize(std::max(vertexNode.vecVertexPos.size(), vertexNode.vecVertexTex.size()));

	QMatrix4x4 viewMatrix = XMMatrixToQMatrix4x4(DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		DirectX::XMVectorSet(0.0f, 0.0f, RENDER_ZPLANE, 0.0f),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));

	QMatrix4x4 projectionMatrix = XMMatrixToQMatrix4x4(DirectX::XMMatrixOrthographicOffCenterLH((float)0, (float)offscreenRc.width(), (float)offscreenRc.height(), (float)0, RENDER_ZPLANE, FLT_MAX));


	// 设置绘制位置的坐标缓冲区
	int nIndex = 0;
	for (auto vertex : vertexNode.vecVertexPos)
	{
		vertex = viewMatrix.map(vertex);
		vertex = projectionMatrix.map(vertex);
		vecVertices[nIndex++].pos = DirectX::XMFLOAT3(vertex.x(), vertex.y(), RENDER_ZPLANE);
	}

	// 设置绘制纹理的坐标缓冲区
	nIndex = 0;
	for (auto vertex : vertexNode.vecVertexTex)
	{
		vecVertices[nIndex++].tex = DirectX::XMFLOAT2(vertex.x(), vertex.y());
	}

	return vecVertices;
}

SVertexPosTexData CBaseRenderItem::GetVertexTexData(const QSize& sz, const std::vector<VertexPosTex>& vertices)
{
	SVertexPosTexData vertexNode;
	if (vertices.empty())
		vertexNode.InitVertexData(QPoint(0, 0), sz);
	else
		vertexNode.InitVertexData(vertices, QPoint(0, 0), sz);

	return vertexNode;
}

void CBaseRenderItem::VertexFillTypeDeal(SVertexPosTexData &vertexNode, const STexDatas& texNode)
{
	if (!texNode.texNode0.texHandle)
		return;

	if (!IsSupportFillType(GetType()))
		return;

	if (EMFT_FullWindow == m_pMediaPty->pBasePty->GetFillType())	// 全屏不处理
		return;

	int nTexWidth = texNode.texNode0.texDesc.Width;
	int nTexHeight = texNode.texNode0.texDesc.Height;

	if (nTexWidth == 0 || nTexHeight == 0)
		return;

	QPointF pt/* = m_pMediaPty->pBasePty->GetPos()*/;
	QSizeF sz = m_pMediaPty->pBasePty->GetSize();

	QMatrix4x4 keepradioPos, noscalePos;//在舞台坐标系上计算
	QMatrix4x4 noscaleTex;

	switch (m_pMediaPty->pBasePty->GetFillType())
	{
	case EMFT_KeepRatio:
	{
		//计算 媒体宽与素材宽之比 和  媒体高与素材高之比 的比
		//radio小于1 则 媒体宽与素材宽之比小于媒体高与素材高之比，纵向进行缩放来保持素材宽高之比，否则反之
		QRectF rc(pt, sz);
		float radio = abs(rc.width() / nTexWidth / (rc.height() / nTexHeight));

		keepradioPos.setToIdentity();
		keepradioPos.translate(nTexWidth * 0.5, nTexHeight * 0.5);
		radio < 1.0f ? keepradioPos.scale(1.0f, radio) : keepradioPos.scale(1.0f / radio, 1.0f);
		keepradioPos.translate(-nTexWidth * 0.5, -nTexHeight * 0.5);
	}
	break;
	case EMFT_NoScale:
	{
		//radio小于1，改变纹理取样点，大于1，改变位置点
		float radioX = sz.width() / (float)nTexWidth;
		float radioY = sz.height() / (float)nTexHeight;

		//tempPos是实际点位置的变换，tempTex是纹理取样点的变化
		QRectF rc(pt, sz);
		noscaleTex.translate(0.5, 0.5);
		noscalePos.translate(nTexWidth * 0.5, nTexHeight * 0.5);
		radioX < 1.0f ? noscaleTex.scale(radioX, 1.0f) : noscalePos.scale(1.0f / radioX, 1.0f);
		radioY < 1.0f ? noscaleTex.scale(1.0f, radioY) : noscalePos.scale(1.0f, 1.0f / radioY);
		noscaleTex.translate(-0.5, -0.5);
		noscalePos.translate(-nTexWidth * 0.5, -nTexHeight * 0.5);
	}
	break;
	default:
		return;
		break;
	}

	for (auto &vertex : vertexNode.vecVertexPos)
	{
		vertex = keepradioPos.map(vertex);
		vertex = noscalePos.map(vertex);
	}

	for (auto &vertex : vertexNode.vecVertexTex)
	{
		vertex = noscaleTex.map(vertex);
	}
}

std::vector<ID3D11ShaderResourceView*> CBaseRenderItem::GetShaderResView(STexDatas& data, Video_ClrMetedata& metaData, int index)
{
	std::vector<ID3D11ShaderResourceView*> vecViews;
	if (index < 0)
		return vecViews;
	if (index >= m_sViewNode.size())
		m_sViewNode.resize(index + 1);

	CreateShaderResView(m_pD3dRes->m_pD3DDevice, data, metaData, m_sViewNode[index]);

	if (m_sViewNode[index].m_pView0)
		vecViews.push_back(m_sViewNode[index].m_pView0.Get());
	if (m_sViewNode[index].m_pView1)
		vecViews.push_back(m_sViewNode[index].m_pView1.Get());
	if (m_sViewNode[index].m_pView2)
		vecViews.push_back(m_sViewNode[index].m_pView2.Get());

	return vecViews;
}
