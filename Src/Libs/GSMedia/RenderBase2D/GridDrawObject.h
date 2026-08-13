#pragma once
#include "RenderBase2DGlobal.h"
#include "GSMedia\RenderDevice\DeviceResource.h"
#include "GSMedia\RenderCommon\CommonFunction.h"
#include "GSMedia\RenderCommon\Model.h"
#include "GSMedia\RenderCommon\Vertex.h"
#include "GSMedia/RenderCommon/TextureRender.h"
#include "GSMedia\RenderCommon\PrimitiveBatch.h"
#include "Common/Common/ICustomDataInterface.h"

#include "IRenderBase2DInterface.h"
#include "BasicEffect.h"

class CGridDrawObject : public CD3DResourceChild, public IGridDrawObject
{
public:
	CGridDrawObject(UINT nAdapterIndex, BasicEffect* pBasicEffect);
	~CGridDrawObject();

	CGridDrawObject(const CGridDrawObject& other) = delete;
	CGridDrawObject& operator=(const CGridDrawObject& other) = delete;

public:
	HRESULT		DrawAllGrid();
	HRESULT		DrawGrid(int nIndex);
	HRESULT		InitGridTexture(SScreenGrid& screenGrid,const D2D1_RECT_D& ScreenRc,std::vector<D2D1_RECT_D>& vecScreenInfo, std::chrono::milliseconds& msLastTime);

	void Reset();

protected:
	void InitTexture(std::vector<D2D1_RECT_D>& vecScreenInfo);
	void CalcGridTextureOffset(SScreenGrid& screenGrid, const D2D1_RECT_D& ScreenRc, const std::array<SGridLinePty, EGT_Count> &arGridPty, std::vector<D2D1_RECT_D>& vecScreenInfo);

	//计算网格
	void CalcGridPts(const D2D1_RECT_D& rcScreen, SScreenGrid& ScreenGrid,
		std::vector<std::vector<CPointD>>& vecOriginPts, std::vector<std::vector<CPointD>>& vecEndPts);

protected:
	Model									m_ModelOri = {};			// 1:1贴图时使用：DrawGrid(int nIndex)，模型不需要改变

	Model									m_Model = {};				// 非1:1贴图时使用，用于创意显示绘制：DrawAllGrid();
	float									m_totalWidth, m_totalHeight;
	std::vector<CTextureRender>				m_vecBkTexture;
	std::vector<std::vector<CPointD>>		m_arrTextureOffset;			// 网格纹理偏移<纹理线索引，纹理线偏移>

	std::unique_ptr<DirectX::PrimitiveBatch<VertexPosColor>> m_pPrimitiveBatch = nullptr;
	BasicEffect*					m_pBasicEffect = nullptr;

	std::vector <ComPtr<ID2D1RenderTarget>> m_vecd2dRenderTarget;

	bool m_bInited = false;
};

