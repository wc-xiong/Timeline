#pragma once
#include "RenderBase2DGlobal.h"
#include <QUuid>
#include <d2d1.h>
#include <wrl/client.h>
#include "IRenderBase2DInterface.h"


// 使用模板别名(C++11)简化类型名
template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class ISliceTree;

class CCustom2DDrawHelper:public ICustom2DDrawHelper
{
public:
	CCustom2DDrawHelper();
	~CCustom2DDrawHelper();

	void Clear(ID2D1Factory* pFactory);

	virtual bool Add(const QUuid& sliceID, bool bInput);

	bool AddPoints(const std::vector<CPointD>& vecPoints) override;

	virtual bool AddEdge(const CEdgeD& e);

	virtual void Draw(ID2D1RenderTarget* pD2DRenderTarget, ID2D1Brush *brush, float strokeWidth = 1.0f, ID2D1StrokeStyle* strokeStyle = nullptr) override;

	virtual void Fill(ID2D1RenderTarget* pD2DRenderTarget);

	virtual void Fill(ID2D1RenderTarget* pD2DRenderTarget, ID2D1Brush *brush);

	int m_pointNum = 100;
	D2D1_POINT_2F* m_points = nullptr;

	ComPtr<ID2D1PathGeometry> m_pPathGeometry = nullptr;
	ComPtr<ID2D1GeometrySink> m_pSink = nullptr;
	ComPtr<ID2D1SolidColorBrush> m_pBrush = nullptr;
};

