#include "Custom2DDrawHelper.h"

#include "Common/Common/CommonFunctionMath.h"
#include "Model/SliceModel/SliceModelExport.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CCustom2DDrawHelper::CCustom2DDrawHelper()
{
	m_points = new D2D1_POINT_2F[m_pointNum];
}
CCustom2DDrawHelper::~CCustom2DDrawHelper()
{
	delete[]m_points;
}

void CCustom2DDrawHelper::Clear(ID2D1Factory* pFactory)
{
	pFactory->CreatePathGeometry(m_pPathGeometry.ReleaseAndGetAddressOf());
	m_pPathGeometry->Open(m_pSink.ReleaseAndGetAddressOf());	
}

bool CCustom2DDrawHelper::Add(const QUuid& id, bool bInput)
{
	auto pData = g_pSliceModel->GetSliceTreeNode(id).lock();
	if (!pData)
		return true;

	auto pSlice = std::dynamic_pointer_cast<ISliceTreeSliceNode>(pData);
	if (!pSlice)
		return false;

	std::vector<CPointD> pts;
	pSlice->GetPoints(bInput, pts);
	AddPoints(pts);
	return true;
}

bool CCustom2DDrawHelper::AddPoints(const std::vector<CPointD>& pts)
{
	if (pts.empty())
		return false;

	m_pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

	m_pSink->BeginFigure(
		D2D1::Point2F((float)pts[0].x, (float)pts[0].y),
		D2D1_FIGURE_BEGIN_FILLED
	);

	int ptCount = (int)pts.size();
	if (m_pointNum < ptCount)
	{
		m_pointNum = ptCount;
		delete[]m_points;
		m_points = new D2D1_POINT_2F[m_pointNum];
	}

	for (INT_PTR i = 0; i < ptCount; i++)
	{
		m_points[i].x = (float)pts.at(i).x;
		m_points[i].y = (float)pts.at(i).y;
	}

	m_pSink->AddLines(m_points, (UINT32)ptCount);
	m_pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
	return true;
}

bool CCustom2DDrawHelper::AddEdge(const CEdgeD& e)
{
	m_pSink->BeginFigure(D2D1::Point2F(e.pMin.x, e.pMin.y), D2D1_FIGURE_BEGIN_HOLLOW);
	m_pSink->AddLine(D2D1::Point2F(e.pMax.x, e.pMax.y));
	m_pSink->EndFigure(D2D1_FIGURE_END_OPEN); // 不闭合
	return false;
}

void CCustom2DDrawHelper::Draw(ID2D1RenderTarget* pd2dRenderTarget,	ID2D1Brush *brush,	float strokeWidth, ID2D1StrokeStyle* strokeStyle)
{
	m_pSink->Close();

	pd2dRenderTarget->DrawGeometry(m_pPathGeometry.Get(), brush, strokeWidth, strokeStyle);
}

void CCustom2DDrawHelper::Fill(ID2D1RenderTarget* pd2dRenderTarget)
{
	m_pSink->Close();

	if (!m_pBrush)
	{
		pd2dRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),
			m_pBrush.ReleaseAndGetAddressOf());
	}

	pd2dRenderTarget->FillGeometry(m_pPathGeometry.Get(), m_pBrush.Get());
}

void CCustom2DDrawHelper::Fill(ID2D1RenderTarget* pD2DRenderTarget, ID2D1Brush *brush)
{
	m_pSink->Close();
	pD2DRenderTarget->FillGeometry(m_pPathGeometry.Get(), brush, NULL);
}