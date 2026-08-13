#pragma once
#include <windows.h>
#include <d2d1.h>
#include <vector>
#include <chrono>
#include <QUuid>
#include "Common/Common/Maths/Matrix3x2.h"
#include "Common/Common/IModelDataInterface.h"
#include "Common/Common/Maths/Edge.h"

struct SScreenGrid;
class IGridDrawObject
{
public:
	virtual ~IGridDrawObject() = default;

	virtual HRESULT		DrawAllGrid() = 0;
	virtual HRESULT		DrawGrid(int nIndex)=0;
	virtual HRESULT		InitGridTexture(SScreenGrid& screenGrid, const D2D1_RECT_D& ScreenRc,
		std::vector<D2D1_RECT_D>& vecScreenInfo, std::chrono::milliseconds& msLastTime) = 0;

	virtual void Reset() = 0;
};

class ISliceTree;

class ICustom2DDrawHelper
{
public:
	virtual ~ICustom2DDrawHelper() = default;

	virtual void Clear(ID2D1Factory* pFactory)=0;
	virtual bool Add(const QUuid& sliceID, bool bInput) = 0;
	virtual bool AddPoints(const std::vector<CPointD> &vecPoints) = 0;
	virtual bool AddEdge(const CEdgeD& e) = 0;
	virtual void Draw(ID2D1RenderTarget* pD2DRenderTarget, ID2D1Brush *brush, float strokeWidth = 1.0f, ID2D1StrokeStyle* strokeStyle = nullptr)= 0;

	virtual	void Fill(ID2D1RenderTarget* pD2DRenderTarget) = 0;
	virtual	void Fill(ID2D1RenderTarget* pD2DRenderTarget, ID2D1Brush *brush) = 0;
};