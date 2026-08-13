#include "GridDrawObject.h"
#include "CommonFunction.h"

#include "GSMedia/TimeServer/TimeServerExport.h"

#include <d2d1.h>
#include <wrl/client.h>

// 使用模板别名(C++11)简化类型名
template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

using namespace DirectX;

#//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

std::vector<VertexPosTex> defaultVertices = {
	{ XMFLOAT3(-1.0f, -1.0f, RENDER_ZPLANE), XMFLOAT2(0.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, RENDER_ZPLANE), XMFLOAT2(0.0f, 0.0f) } ,
	{ XMFLOAT3(1.0f, 1.0f, RENDER_ZPLANE), XMFLOAT2(1.0f, 0.0f) } ,
	{ XMFLOAT3(1.0f, -1.0f, RENDER_ZPLANE), XMFLOAT2(1.0f, 1.0f) } };
std::vector<DWORD> defalutIndexs = { 0,1,2,2,3,0 };

CGridDrawObject::CGridDrawObject(UINT nAdpaterIndex, BasicEffect* pBasicEffect)
{
	InitD3DResource(nAdpaterIndex);

	m_pBasicEffect = pBasicEffect;

	m_pPrimitiveBatch= std::make_unique<PrimitiveBatch<VertexPosColor>>(m_pD3DDeviceContext.Get());

	m_ModelOri.AddMesh(m_pD3DDevice.Get(), defaultVertices, defalutIndexs);
}

CGridDrawObject::~CGridDrawObject()
{
	m_vecBkTexture.clear();
}

HRESULT CGridDrawObject::InitGridTexture(SScreenGrid& screenGrid, const D2D1_RECT_D& ScreenRc, 
	std::vector<D2D1_RECT_D>& vecScreenInfo, std::chrono::milliseconds& msLastTime)
{
	if (screenGrid.GetIsGridRoll())
	{
		std::chrono::milliseconds ms;
		TimeServer_GetCurTime(ms);
		CPointD ptOffset = screenGrid.GetGridOffset();
		ptOffset.x += 2;
		ptOffset.y += 1;

		if (ms - msLastTime >= 2 * screenGrid.GetRollInterval())
		{
			screenGrid.SetGridOffset(ptOffset);
			msLastTime = ms;			//减小误差
		}
		else if (ms - msLastTime >= screenGrid.GetRollInterval())
		{
			screenGrid.SetGridOffset(ptOffset);
			msLastTime += screenGrid.GetRollInterval();			//减小误差
		}
	}
	else if (msLastTime.count() != 0)
		msLastTime = std::chrono::milliseconds(0);

	//初始化Texture和View数组
	InitTexture(vecScreenInfo);

	//计算纹理顶点
	auto arGridPty = screenGrid.GetGridPty();
	CPointD ptOffset = screenGrid.GetGridOffset();

	int maxWH = std::max(ScreenRc.Width(), ScreenRc.Height());

	//+1是往外扩大一个间隔的距离，方便移动
	D2D1_RECT_F rectangle = D2D1_RECT_F();
	rectangle.top -= (arGridPty[EGT_Horizon].nInterval + 1);
	rectangle.bottom = (float)(maxWH + arGridPty[EGT_Horizon].nInterval + 1);
	rectangle.left -= (arGridPty[EGT_Vertical].nInterval + 1);
	rectangle.right = (float)(maxWH + arGridPty[EGT_Vertical].nInterval + 1);

	std::vector <std::vector<CPointD>> vecOrigin, vecEnd;
	CalcGridPts(rectangle, screenGrid, vecOrigin, vecEnd);
	
	//计算纹理偏移
	CalcGridTextureOffset(screenGrid, ScreenRc, arGridPty, vecScreenInfo);

	for (int i = 0; i < vecScreenInfo.size(); i++)
	{
		float backClr[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_vecBkTexture[i].Begin(m_pD3DDeviceContext.Get(), backClr);

		auto pd2dRenderTarget = m_vecd2dRenderTarget[i];
		if (!pd2dRenderTarget)
			continue;

  		D2D1::Matrix3x2F translation = D2D1::Matrix3x2F::Translation((FLOAT)ptOffset.x, (FLOAT)ptOffset.y);
 		pd2dRenderTarget->SetTransform(translation);
 		pd2dRenderTarget->BeginDraw();
 		pd2dRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));			//刷新纹理底
 
		XMFLOAT4 vecClrW = XMFLOAT4(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f);
 		ComPtr<ID2D1SolidColorBrush> pVecSolid;
 		HR(pd2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),pVecSolid.GetAddressOf()));
 		D2D1_POINT_2F ptOrigin = D2D1_POINT_2F();
 		D2D1_POINT_2F ptEnd = D2D1_POINT_2F();
 		D2D1_RECT_D rcTexture = vecScreenInfo[i];
 		int nHeight = rcTexture.bottom - rcTexture.top;
 		for (int j = EGT_Horizon; j < (int)vecOrigin.size(); j++)						//遍历每一种网格
 		{
 			if (!arGridPty[j].bShow)
 				continue;

			if (abs(arGridPty[j].fLineWidth - 1.0) < FLT_EPSILON)		//D3D draw 1.0 line width,D2D draw other
				continue;
 
 			//左斜线网格前nCount不需要绘制
 			for (size_t k = 0; k < vecOrigin[j].size(); k++)						//对每一种网格增加偏移和判断数量
 			{
 				ptOrigin = CommonFunction::CPointD_2_D2DPoint2F(vecOrigin[j][k] + m_arrTextureOffset[i][j]);
 				ptEnd = CommonFunction::CPointD_2_D2DPoint2F(vecEnd[j][k] + m_arrTextureOffset[i][j]);

				int nCount = (j == EGT_Horizon || j == EGT_Vertical) ? 2 : 1;
 				while (nCount>0)
 				{
 					pd2dRenderTarget->DrawLine(ptOrigin, ptEnd, pVecSolid.Get(), arGridPty[j].fLineWidth);
 					nCount--;
 				}
 			}
 		}
 
 		pd2dRenderTarget->EndDraw();

		int nWidth = vecScreenInfo[i].right - vecScreenInfo[i].left;
		DirectX::XMMATRIX translation3d = XMMatrixTranslation((float)(ptOffset.x - nWidth / 2), (float)(-ptOffset.y - nHeight / 2), 0.0f);
		DirectX::XMMATRIX scale = XMMatrixScaling(2.0f / nWidth, 2.0f / nHeight, 1.0f);

		m_pBasicEffect->SetWorldMatrix(translation3d*scale);
		m_pBasicEffect->SetViewMatrix(XMMatrixIdentity());
		m_pBasicEffect->SetProjMatrix(XMMatrixIdentity());
		m_pBasicEffect->SetRenderState(m_pD3DDeviceContext.Get(),
			BasicEffect::VS2D,
			BasicEffect::PS2D,
			BasicEffect::BMCover,
			nullptr,
			BasicEffect::SamplerAnisotropic);

		m_pBasicEffect->Apply(m_pD3DDeviceContext.Get());
		m_pPrimitiveBatch->Begin();

		for (int j = EGT_Horizon; j < EGT_Count; j++)
		{
			if (!arGridPty[j].bShow)
				continue;

			if(abs(arGridPty[j].fLineWidth - 1.0) > FLT_EPSILON)
				continue;

			for (size_t k = 0; k < vecOrigin[j].size(); k++)						//对每一种网格增加偏移和判断数量
			{
				ptOrigin = CommonFunction::CPointD_2_D2DPoint2F(vecOrigin[j][k] + m_arrTextureOffset[i][j]);
				ptEnd = CommonFunction::CPointD_2_D2DPoint2F(vecEnd[j][k] + m_arrTextureOffset[i][j]);
				
				VertexPosColor verticesOrigin = VertexPosColor(XMFLOAT3(ptOrigin.x, ptOrigin.y, RENDER_ZPLANE), vecClrW);
				VertexPosColor verticesEnd = VertexPosColor(XMFLOAT3(ptEnd.x, ptEnd.y, RENDER_ZPLANE), vecClrW);

				m_pPrimitiveBatch->DrawLine(verticesOrigin, verticesEnd);
			}
		}

		m_pPrimitiveBatch->End();
		m_vecBkTexture[i].End(m_pD3DDeviceContext.Get());
	}

// #ifdef DEBUG
// 		std::wostringstream stringStream;
// 		stringStream << "D:\\" << "Grid" << ".png";
// 		std::wstring str = stringStream.str();
// 		SaveD3DTextureToFile(m_pD3DDevice,m_pD3DDeviceContext,m_vecBkTexture[0].GetTexture2D(), str.c_str());
// #endif

	m_bInited = true;

	return S_OK;
}

void CGridDrawObject::Reset()
{
	if (!m_bInited)
		return;
	m_bInited = false;

	m_vecd2dRenderTarget.clear();
	m_vecBkTexture.clear();
	m_Model.modelParts.clear();
}

void CGridDrawObject::InitTexture(std::vector<D2D1_RECT_D>& vecScreenInfo)
{
	float nScreenWidth = vecScreenInfo.back().right;
	if (vecScreenInfo.back().right == 0)
		nScreenWidth = vecScreenInfo.back().Width();

	float nScreenHeight = vecScreenInfo.back().bottom;
	if (vecScreenInfo.back().bottom == 0)
		nScreenHeight = vecScreenInfo.back().Height();

	//if (m_totalWidth == nScreenWidth && m_totalHeight == nScreenHeight)
	//	return;

	m_vecBkTexture.resize(vecScreenInfo.size());
	m_vecd2dRenderTarget.resize(vecScreenInfo.size());
	m_Model.modelParts.clear();

	m_totalWidth = nScreenWidth;
	m_totalHeight = nScreenHeight;

	float halfTotalWidth = nScreenWidth / 2.0f;
	float halfTotalHeight = nScreenHeight / 2.0f;

	std::vector<D2D1_RECT_D>::iterator iter = vecScreenInfo.begin();
	for (size_t index = 0; index < vecScreenInfo.size(); index ++)
	{
		D2D1_RECT_D rcTexture = vecScreenInfo[index];

		if (rcTexture.Width() != m_vecBkTexture[index].GetTexWidth() || rcTexture.Height() != m_vecBkTexture[index].GetTexHeight())
		{
			int texWidth = rcTexture.Width() > 16384 ? 16384 : rcTexture.Width();
			int texHeight = rcTexture.Height() > 16384 ? 16384 : rcTexture.Height();

			m_vecBkTexture[index].InitResource(m_pD3DDevice.Get(), texWidth, texHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
			m_vecBkTexture[index].SetDebugObjectName("InputGridBkTexture" + std::to_string(index));

			HR(CreateD2DRenderTargetFromTexture(m_pD2DFactory.Get(), m_vecBkTexture[index].GetTexture2D(), m_vecd2dRenderTarget[index].ReleaseAndGetAddressOf()));
		}

		std::vector<VertexPosTex> vertices = defaultVertices;

		vertices[0].pos.x = rcTexture.left / halfTotalWidth - 1.0f;
		vertices[0].pos.y = -rcTexture.bottom / halfTotalHeight + 1.0f;
		vertices[1].pos.x = rcTexture.left / halfTotalWidth - 1.0f;
		vertices[1].pos.y = -rcTexture.top / halfTotalHeight + 1.0f;
		vertices[2].pos.x = rcTexture.right / halfTotalWidth - 1.0f;
		vertices[2].pos.y = -rcTexture.top / halfTotalHeight + 1.0f;;
		vertices[3].pos.x = rcTexture.right / halfTotalWidth - 1.0f;
		vertices[3].pos.y = -rcTexture.bottom / halfTotalHeight + 1.0f;;

		m_Model.AddMesh(m_pD3DDevice.Get(), vertices, defalutIndexs);
	}
}

void CGridDrawObject::CalcGridTextureOffset(SScreenGrid& screenGrid, const D2D1_RECT_D& ScreenRc,
	const std::array<SGridLinePty, EGT_Count>& arGridPty, std::vector<D2D1_RECT_D>& vecScreenInfo)
{
	m_arrTextureOffset.clear();

	for (auto iter : vecScreenInfo)
	{
		D2D1_RECT_D rcTexture = iter;
		rcTexture.top -= ScreenRc.top;
		rcTexture.left -= ScreenRc.left;
		rcTexture.bottom -= ScreenRc.top;
		rcTexture.right -= ScreenRc.left;
		int nTextureWidth = int(rcTexture.right - rcTexture.left) + arGridPty[EGT_Vertical].nInterval * 2;
		int nTextureHeight = int(rcTexture.bottom - rcTexture.top) + arGridPty[EGT_Horizon].nInterval * 2;

		std::vector<CPointD>vecPoints(EGT_Count);

		//水平网格
		//if (arGridPty[EGT_Horizon))
		{
			CPointD pt = CPointD();
			if (rcTexture.top != 0)			
				pt.y = -(int(rcTexture.top)% arGridPty[EGT_Horizon].nInterval);

			vecPoints[EGT_Horizon] = pt;			
		}

		//垂直网格
		//if (arGridPty[EGT_Vertical))
		{
			CPointD pt = CPointD();
			//int nLeftOffset = (int)(rcTexture.left - pScreenItem->GetRectF().left);
			if (rcTexture.left != 0)
				pt.x = -(int(rcTexture.left)% arGridPty[EGT_Vertical].nInterval);

			vecPoints[EGT_Vertical] = pt;
		}

		//右斜网格
		//if (arGridPty[EGT_RightOblique))
		{
			CPointD pt = CPointD();
			if (rcTexture.top != 0)
				pt.x = -(int(rcTexture.top) % arGridPty[EGT_RightOblique].nInterval);
			if(rcTexture.left != 0)
				pt.y = -(int(rcTexture.left)% arGridPty[EGT_RightOblique].nInterval);

			vecPoints[EGT_RightOblique] = pt;
		}

		//左斜网格
		//if (arGridPty[EGT_LeftOblique))
		{
			CPointD pt = CPointD();
			if (rcTexture.left != 0)
				pt.x = -(int(rcTexture.left)% arGridPty[EGT_LeftOblique].nInterval);
			if(rcTexture.top != 0)
				pt.y = -(int(rcTexture.top)% arGridPty[EGT_LeftOblique].nInterval);
			
			vecPoints[EGT_LeftOblique] = pt;
		}

		m_arrTextureOffset.push_back(vecPoints);
	}

}

//计算网格
void CGridDrawObject::CalcGridPts(const D2D1_RECT_D& rcScreen, SScreenGrid& ScreenGrid,
	std::vector<std::vector<CPointD>>& vecOriginPts, std::vector<std::vector<CPointD>>& vecEndPts)
{
	auto arGridPty = ScreenGrid.GetGridPty();

	vecOriginPts.resize(EGT_Count);
	vecEndPts.resize(EGT_Count);

	if (arGridPty[EGT_Horizon].bShow)			//横线
	{
		int nInterval = arGridPty[EGT_Horizon].nInterval + 1;
		int nHorLine = int((rcScreen.bottom - rcScreen.top) / nInterval + 1);
		if (int(rcScreen.bottom - rcScreen.top) % nInterval != 0)
			nHorLine += 1;
		CPointD ptLeft((float)rcScreen.left, (float)rcScreen.top - 1.0f);
		CPointD ptRight((float)rcScreen.right, (float)rcScreen.top - 1.0f);

		std::vector<CPointD> vecOrigin, vecEnd;
		for (int i = 0; i < nHorLine; i++)
		{
			vecOrigin.push_back(ptLeft);
			vecEnd.push_back(ptRight);
			ptLeft.y += nInterval;
			ptRight.y += nInterval;
		}

		vecOriginPts[EGT_Horizon] = vecOrigin;
		vecEndPts[EGT_Horizon] = vecEnd;
	}

	if (arGridPty[EGT_Vertical].bShow)			//竖线
	{
		int nInterval = arGridPty[EGT_Vertical].nInterval + 1;
		int nVerLine = int((rcScreen.right - rcScreen.left) / nInterval + 1);
		if (int(rcScreen.right - rcScreen.left) % nInterval != 0)
			nVerLine += 1;
		CPointD ptTop((float)rcScreen.left + 1, (float)rcScreen.top);
		CPointD ptBottom((float)rcScreen.left + 1, (float)rcScreen.bottom);

		std::vector<CPointD> vecVOrigin, vecVEnd;
		for (int i = 0; i < nVerLine; i++)
		{
			vecVOrigin.push_back(ptTop);
			vecVEnd.push_back(ptBottom);
			ptTop.x += nInterval;
			ptBottom.x += nInterval;
		}

		vecOriginPts[EGT_Vertical] = vecVOrigin;
		vecEndPts[EGT_Vertical] = vecVEnd;
	}

	if (arGridPty[EGT_RightOblique].bShow)
	{
		int nInterval = arGridPty[EGT_RightOblique].nInterval + 1;
		assert(nInterval != 0);
		if (nInterval == 0)
			return;

		//3D右斜线
		int width = int(rcScreen.right - rcScreen.left);
		int height = int(rcScreen.bottom - rcScreen.top);
		int nLine = (width + height) / nInterval;							//网格数
		if ((width + height) % nInterval != 0)
			nLine += 1;
		FLOAT offset = FLOAT(nInterval - height % nInterval);					//左上网格从左下角开始计算，需要和其他网格在左上角对齐
		CPointD pt3DOrigin((float)rcScreen.left, (float)rcScreen.bottom + offset);
		CPointD pt3DEnd((float)rcScreen.left - offset, (float)rcScreen.bottom);

		std::vector<CPointD> vec3DRTOrigin, vec3DRTEnd;
		for (int i = 0; i < nLine; i++)
		{
			pt3DOrigin.y -= nInterval;
			pt3DEnd.x += nInterval;
			if (pt3DOrigin.y < rcScreen.top)
			{
				if (rcScreen.top - pt3DOrigin.y < nInterval)
					pt3DOrigin.x = pt3DOrigin.x + (float)rcScreen.top - pt3DOrigin.y;
				else
					pt3DOrigin.x += nInterval;
				pt3DOrigin.y = (float)rcScreen.top;
			}
			if (pt3DEnd.x > rcScreen.right)
			{
				if (pt3DEnd.x - rcScreen.right < nInterval)
					pt3DEnd.y = pt3DEnd.y - (float)(pt3DEnd.x - rcScreen.right);
				else
					pt3DEnd.y -= nInterval;
				pt3DEnd.x = (float)rcScreen.right;
			}

			vec3DRTOrigin.push_back(pt3DOrigin);
			vec3DRTEnd.push_back(pt3DEnd);
		}

		vecOriginPts[EGT_RightOblique] = vec3DRTOrigin;
		vecEndPts[EGT_RightOblique] = vec3DRTEnd;
	}

	if (arGridPty[EGT_LeftOblique].bShow)
	{
		int width = int(rcScreen.right - rcScreen.left);
		int height = int(rcScreen.bottom - rcScreen.top);
		int nInterval = arGridPty[EGT_LeftOblique].nInterval + 1;

		assert(nInterval != 0);
		if (nInterval == 0)
			return;
	
		int nLine = (width + height) / nInterval;							//网格数
		if ((width + height) % nInterval != 0)
			nLine += 1;
		FLOAT offset = FLOAT(nInterval - height % nInterval);							//左上网格从左下角开始计算，需要和其他网格在左上角对齐
		offset = 0;

		//3D左斜线
		CPointD pt3DOrigin((float)rcScreen.left , (float)rcScreen.top + offset);
		CPointD pt3DEnd((float)rcScreen.left - offset, (float)rcScreen.top );

		std::vector<CPointD> vec3DLBOrigin, vec3DLBEnd;
		for (int i = 0; i < nLine; i++)
		{
			pt3DOrigin.y += nInterval;
			pt3DEnd.x += nInterval;
			if (pt3DOrigin.y > rcScreen.bottom)
			{
				if (pt3DOrigin.y - rcScreen.bottom < nInterval)
					pt3DOrigin.x = pt3DOrigin.x + pt3DOrigin.y - (float)rcScreen.bottom;
				else
					pt3DOrigin.x += nInterval;
				pt3DOrigin.y = (float)rcScreen.bottom;
			}

			if (pt3DEnd.x > rcScreen.right)
			{
				if (pt3DEnd.x - rcScreen.right < nInterval)
					pt3DEnd.y = pt3DEnd.y + pt3DEnd.x - (float)rcScreen.right;
				else
					pt3DEnd.y += nInterval;
				pt3DEnd.x = (float)rcScreen.right;
			}
			vec3DLBOrigin.push_back(pt3DOrigin);
			vec3DLBEnd.push_back(pt3DEnd);
		}

		vecOriginPts[EGT_LeftOblique] = vec3DLBOrigin;
		vecEndPts[EGT_LeftOblique] = vec3DLBEnd;
	}
}

HRESULT CGridDrawObject::DrawAllGrid()
{
	if (m_vecBkTexture.empty() || !m_bInited)
		return S_OK;

	m_pBasicEffect->SetRenderState(m_pD3DDeviceContext.Get(), BasicEffect::VS3D, BasicEffect::PSBasic, BasicEffect::BMCover, nullptr, BasicEffect::SamplerLineWrap);
	
	float fHalfW = (float)m_totalWidth / 2.0f;
	float fHalfH = (float)m_totalHeight / 2.0f;

	DirectX::XMMATRIX matrixTranslation = XMMatrixTranslation(fHalfW, -fHalfH, 0.0f);
	DirectX::XMMATRIX matrixScale = XMMatrixScaling(fHalfW, fHalfH, 1.0f);
	DirectX::XMMATRIX matrix = matrixScale * matrixTranslation;
	m_pBasicEffect->SetWorldMatrix(matrix);
	m_pBasicEffect->Apply(m_pD3DDeviceContext.Get());

	UINT strides = m_Model.vertexStride;
	UINT offsets = 0;

	for (int nIndex = 0; nIndex < m_Model.modelParts.size(); nIndex++)
	{
		m_pD3DDeviceContext->IASetVertexBuffers(0, 1, m_Model.modelParts[nIndex].vertexBuffer.GetAddressOf(), &strides, &offsets);
		m_pD3DDeviceContext->IASetIndexBuffer(m_Model.modelParts[nIndex].indexBuffer.Get(), m_Model.modelParts[0].indexFormat, 0);

		ComPtr<ID3D11ShaderResourceView> textureView = m_vecBkTexture[nIndex].GetTextureShaderView();
		m_pD3DDeviceContext->PSSetShaderResources(0, 1, textureView.GetAddressOf());
		m_pD3DDeviceContext->DrawIndexed((UINT)m_Model.modelParts[nIndex].indexCount, 0, 0);
	}

	return S_OK;
}

HRESULT CGridDrawObject::DrawGrid(int nIndex)
{
	if (m_vecBkTexture.empty() || !m_bInited)
		return S_OK;

	m_pBasicEffect->SetRenderState(m_pD3DDeviceContext.Get(), BasicEffect::VSBasic, BasicEffect::PSBasic, BasicEffect::BMCover, nullptr, BasicEffect::SamplerPointWrap);

	UINT strides = m_ModelOri.vertexStride;
	UINT offsets = 0;
	m_pD3DDeviceContext->IASetVertexBuffers(0, 1, m_ModelOri.modelParts[0].vertexBuffer.GetAddressOf(), &strides, &offsets);
	m_pD3DDeviceContext->IASetIndexBuffer(m_ModelOri.modelParts[0].indexBuffer.Get(), m_ModelOri.modelParts[0].indexFormat, 0);

	ComPtr<ID3D11ShaderResourceView> textureView = m_vecBkTexture[nIndex].GetTextureShaderView();

	m_pD3DDeviceContext->PSSetShaderResources(0, 1, textureView.GetAddressOf());
	m_pD3DDeviceContext->DrawIndexed((UINT)m_ModelOri.modelParts[0].indexCount, 0, 0);

	return S_OK;
}
