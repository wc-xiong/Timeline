#include "PreviewFxPass.h"
#include "PreviewGpuPipeline.h"

#include "Common/Common/IMediaPropertyInterface.h"
#include "Common/Common/IFxInterface.h"
#include "Common/Common/XEffectCommon.h"
#include "Common/Common/VideoDataCommon.h"
#include "GSMedia/RenderDevice/DeviceResource.h"
#include "GSMedia/RenderBase2D/BasicEffect.h"
#include "GSMedia/RenderItem/D3DEffectRender.h"
#include "GSMedia/RenderItem/RenderDrawDataDefine.h"
#include "GSMedia/RenderItem/VertexCalculator.h"
#include "GSMedia/RenderCommon/Vertex.h"
#include "Common/Common/TransationTypeDefine.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "Model/MediaModel/MediaNode.h"

#include <QPointF>
#include <QSizeF>
#include <cmath>
#include <unordered_map>

struct CPreviewFxPass::Impl
{
	CD3DResourceChild d3d;
	std::shared_ptr<SMediaPty> mediaPty;
	std::shared_ptr<SFxRenderData> fxData;
	std::unique_ptr<CD3DEffectRender> effectRender;
	std::unordered_map<int, IVertexCalculator*> mapVertexCalc;
	STransInfo transInfo;
	QUuid clipId;
	bool bReady = false;
};

CPreviewFxPass& CPreviewFxPass::Instance()
{
	static CPreviewFxPass s;
	return s;
}

CPreviewFxPass::CPreviewFxPass()
	: m_pImpl(new Impl)
{
	m_pImpl->mediaPty = std::make_shared<SMediaPty>();
	m_pImpl->fxData = std::make_shared<SFxRenderData>();
}

CPreviewFxPass::~CPreviewFxPass()
{
	Clear();
	for (auto& kv : m_pImpl->mapVertexCalc)
		delete kv.second;
	m_pImpl->mapVertexCalc.clear();
	delete m_pImpl;
	m_pImpl = nullptr;
}

bool CPreviewFxPass::EnsureInit()
{
	if (m_pImpl->bReady)
		return true;
	if (!CPreviewGpuPipeline::Instance().IsDeviceReady())
		return false;

	// 必须与硬解同 Device_Decode，否则无法绑定解码输出 SRV
	m_pImpl->d3d.InitD3DResource(Device_Decode, 0);
	if (!m_pImpl->d3d.m_pD3DDevice)
		return false;

	BasicEffect* pBe = CPreviewGpuPipeline::Instance().GetBasicEffect();
	if (!pBe)
		return false;

	m_pImpl->effectRender = std::make_unique<CD3DEffectRender>(
		m_pImpl->d3d.m_pD3DDevice.Get(), m_pImpl->mediaPty, m_pImpl->fxData);
	m_pImpl->effectRender->SetBasicEffect(pBe);
	m_pImpl->bReady = true;
	return true;
}

void CPreviewFxPass::Clear()
{
	m_pImpl->effectRender.reset();
	m_pImpl->clipId = QUuid();
	m_pImpl->mediaPty->pBasePty = nullptr;
	m_pImpl->mediaPty->pEffectPty = nullptr;
	m_pImpl->mediaPty->pSharePty = nullptr;
	m_pImpl->mediaPty->pFxPty = nullptr;
	m_pImpl->bReady = false;
}

void CPreviewFxPass::BindClip(const QUuid& clipId)
{
	if (clipId.isNull())
	{
		Unbind();
		return;
	}
	if (!EnsureInit())
		return;

	auto pMedia = std::dynamic_pointer_cast<CMediaNode>(
		g_pMediaModel->GetMediaNode(clipId).lock());
	if (!pMedia)
	{
		Unbind();
		return;
	}

	m_pImpl->clipId = clipId;
	m_pImpl->mediaPty->pBasePty = pMedia->GetBaseProperty();
	m_pImpl->mediaPty->pEffectPty = pMedia->GetEffectProperty();
	m_pImpl->mediaPty->pSharePty = pMedia->GetShareProperty();
	m_pImpl->mediaPty->pFxPty = pMedia->GetFxProperty();
	if (m_pImpl->effectRender)
		m_pImpl->effectRender->UpdateMediaPty(m_pImpl->mediaPty);
}

void CPreviewFxPass::Unbind()
{
	m_pImpl->clipId = QUuid();
	m_pImpl->mediaPty->pBasePty = nullptr;
	m_pImpl->mediaPty->pEffectPty = nullptr;
	m_pImpl->mediaPty->pSharePty = nullptr;
	m_pImpl->mediaPty->pFxPty = nullptr;
}

bool CPreviewFxPass::HasBoundMedia() const
{
	return m_pImpl && !m_pImpl->clipId.isNull() && m_pImpl->mediaPty
		&& m_pImpl->mediaPty->pFxPty;
}

void CPreviewFxPass::SetTransInfo(const STransInfo& info)
{
	m_pImpl->transInfo = info;
}

const STransInfo& CPreviewFxPass::GetTransInfo() const
{
	return m_pImpl->transInfo;
}

bool CPreviewFxPass::BuildTransVertices(std::vector<VertexPosTex>& verts, std::vector<unsigned int>& indices)
{
	verts.clear();
	indices.clear();
	const STransInfo& ti = m_pImpl->transInfo;
	if (ti.stage == ETFS_Normal || ti.nTransType == ETFT_Fade || ti.nTransType < ETFT_LeftMove
		|| ti.nTransType > ETFT_PerspectiveRotationBottom)
		return false;

	verts = g_vecDefalutVertices;
	std::vector<DWORD> idxDw = g_vecDefaultIndexs;

	IVertexCalculator* pCalc = nullptr;
	auto it = m_pImpl->mapVertexCalc.find(ti.nTransType);
	if (it == m_pImpl->mapVertexCalc.end())
	{
		pCalc = CreateVertexCalculator(ti.nTransType);
		if (pCalc)
			m_pImpl->mapVertexCalc[ti.nTransType] = pCalc;
	}
	else
		pCalc = it->second;

	if (!pCalc)
		return false;

	STransInfo mutableInfo = ti;
	pCalc->DoCalVertex(mutableInfo, verts, idxDw);
	indices.assign(idxDw.begin(), idxDw.end());
	return !verts.empty() && !indices.empty();
}

ID3D11ShaderResourceView* CPreviewFxPass::Apply(
	ID3D11ShaderResourceView* inSrv,
	ID3D11Texture2D* inTex,
	int w, int h,
	int64_t ptsUs)
{
	if (!inSrv || !inTex || w <= 0 || h <= 0)
		return inSrv;
	if (!HasBoundMedia() || !EnsureInit() || !m_pImpl->effectRender)
		return inSrv;

	const microseconds mediaTs(ptsUs);
	m_pImpl->effectRender->UpdateFxRenderData(mediaTs);

	// 像素特效（Keying/Feather/Blur…）才走离屏 MediaFxRender；Cut/Scroll 仅更新矩阵
	bool bPixelFx = false;
	for (auto& factor : m_pImpl->mediaPty->pFxPty->GetAllFxFactor())
	{
		if (!factor || !factor->GetData() || !factor->GetData()->visiable)
			continue;
		const auto t = factor->GetType();
		if (t != EEffectType::Cut && t != EEffectType::Scroll && t != EEffectType::Shake)
		{
			bPixelFx = true;
			break;
		}
	}
	if (!bPixelFx)
		return inSrv;

	STexDatas tex;
	// ComPtr::Attach 不 AddRef；析构会 Release，故先 AddRef 保持与管线纹理平衡
	inTex->AddRef();
	tex.texNode0.pTex.Attach(inTex);
	inTex->GetDesc(&tex.texNode0.texDesc);

	std::vector<ID3D11ShaderResourceView*> views;
	views.push_back(inSrv);

	Video_ClrMetedata meta(ETT_BGRA);
	CRITICAL_SECTION* pCs = m_pImpl->d3d.m_pD3DCSContext;
	if (pCs)
		EnterCriticalSection(pCs);
	m_pImpl->effectRender->MediaFxRender(&m_pImpl->d3d, views, tex, meta, ptsUs);
	if (pCs)
		LeaveCriticalSection(pCs);

	if (!views.empty() && views[0])
		return views[0];
	return inSrv;
}

bool CPreviewFxPass::GetCropUv(float& u0, float& v0, float& u1, float& v1)
{
	u0 = 0.f; v0 = 0.f; u1 = 1.f; v1 = 1.f;
	if (!HasBoundMedia() || !m_pImpl->fxData || !m_pImpl->fxData->bSupportCrop)
		return false;

	const QMatrix4x4& m = m_pImpl->fxData->cropTex;
	const QPointF tl = m.map(QPointF(0.0, 0.0));
	const QPointF br = m.map(QPointF(1.0, 1.0));
	u0 = static_cast<float>(tl.x());
	v0 = static_cast<float>(tl.y());
	u1 = static_cast<float>(br.x());
	v1 = static_cast<float>(br.y());
	return true;
}

bool CPreviewFxPass::GetPreviewTransform(float& offsetXNdc, float& offsetYNdc,
	float& scaleX, float& scaleY, float& rotZDeg)
{
	offsetXNdc = 0.f;
	offsetYNdc = 0.f;
	scaleX = 1.f;
	scaleY = 1.f;
	rotZDeg = 0.f;
	if (!m_pImpl->mediaPty || !m_pImpl->mediaPty->pBasePty)
		return false;

	auto pBase = m_pImpl->mediaPty->pBasePty;
	const QSizeF sz = pBase->GetSize();
	const QSizeF keep = pBase->GetSizeKeepAspectRatio();
	const float refW = keep.width() > 1.f ? static_cast<float>(keep.width()) : 1920.f;
	const float refH = keep.height() > 1.f ? static_cast<float>(keep.height()) : 1080.f;
	if (sz.width() > 0.f && refW > 0.f)
		scaleX = static_cast<float>(sz.width() / refW);
	if (sz.height() > 0.f && refH > 0.f)
		scaleY = static_cast<float>(sz.height() / refH);

	const QPointF pos = pBase->GetPos();
	// 以参考画布归一化到约 [-1,1] 量级（预览 letterbox 内相对偏移）
	offsetXNdc = static_cast<float>(pos.x() / (refW * 0.5));
	offsetYNdc = static_cast<float>(-pos.y() / (refH * 0.5));
	rotZDeg = pBase->GetRotate();
	return true;
}
