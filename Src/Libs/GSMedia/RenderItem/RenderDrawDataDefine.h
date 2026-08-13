#pragma once
#include <DirectXMath.h>
#include <vector>

#include <wrl/client.h>
#include <d3d11_1.h>

#include "Common/VideoDataCommon.h"
#include "GSMedia/RenderCommon/Vertex.h"

#include <QMatrix4x4>

#include <mutex>
#include <set>
#include <QDebug>
#include <shared_mutex>

static QPointF PtTransViewToView(const QPointF& pt, const QSizeF& sz, const VertexPosTex& pos)
{
	QMatrix4x4 temp;
	temp.translate(pt.x() + sz.width() / 2, pt.y() + sz.height() / 2);
	temp.scale(sz.width() / 2, sz.height() / 2);

	return temp.map(QPointF(pos.pos.x, -pos.pos.y));
}

// 纹理绘制顶点、像素数据
struct SVertexPosTexData
{
	QVector<QPointF> vecVertexPos; // 顶点信息

	QVector<QPointF> vecVertexTex;	// 纹理信息

	SVertexPosTexData(QVector<QPointF> vecPos, QVector<QPointF> vecTex)
		: vecVertexPos(vecPos)
		, vecVertexTex(vecTex)
	{

	}

	SVertexPosTexData()
	{

	}

	void InitVertexData(QPointF pt, QSize sz)
	{
		vecVertexPos.clear();
		vecVertexTex.clear();

		vecVertexPos.push_back(QPointF(pt.x(), pt.y()));
		vecVertexPos.push_back(QPointF(pt.x(), pt.y() + sz.height()));
		vecVertexPos.push_back(QPointF(pt.x() + sz.width(), pt.y() + sz.height()));
		vecVertexPos.push_back(QPointF(pt.x() + sz.width(), pt.y()));

		vecVertexTex.push_back(QPointF(0.0f, 0.0f));
		vecVertexTex.push_back(QPointF(0.0f, 1.0f));
		vecVertexTex.push_back(QPointF(1.0f, 1.0f));
		vecVertexTex.push_back(QPointF(1.0f, 0.0f));
	}

	void InitVertexData(const std::vector<VertexPosTex>& vertices, QPointF pt, QSize sz)
	{
		vecVertexPos.clear();
		vecVertexTex.clear();

		for (auto pos : vertices)
		{
			vecVertexPos.push_back(PtTransViewToView(pt, sz, pos));
		}

		for (auto tex : vertices)
		{
			vecVertexTex.push_back(QPointF(tex.tex.x, tex.tex.y));
		}
	}
};

// 纹理着色器资源
struct SRenderShaderResView
{
	//源纹理
	ComPtr<ID3D11ShaderResourceView> m_pView0 = nullptr;
	ComPtr<ID3D11ShaderResourceView> m_pView1 = nullptr;
	ComPtr<ID3D11ShaderResourceView> m_pView2 = nullptr;

	HANDLE m_texHandle0 = nullptr;
	HANDLE m_texHandle1 = nullptr;
	HANDLE m_texHandle2 = nullptr;

	void Reset()
	{
		m_pView0.Reset();
		m_pView1.Reset();
		m_pView2.Reset();

		m_texHandle0 = nullptr;
		m_texHandle1 = nullptr;
		m_texHandle2 = nullptr;
	}

	~SRenderShaderResView()
	{
		Reset();
	}
};


// 默认顶点像素着色器设置、顶点绘制顺序
static std::vector<VertexPosTex> g_vecDefalutVertices = {
{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) } ,
{ DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) } ,
{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) } };

static std::vector<DWORD> g_vecDefaultIndexs = { 0,1,2,2,3,0 };


// 媒体特效数据共享
struct SFxRenderData
{
	// 当前媒体播放进度
	microseconds curTs = microseconds(0);

	// 循环滚动
	bool bSupportScroll = false;
	double fScrollXPos = 0.0f;
	double fScrollYPos = 0.0f;
	QMatrix4x4 scrollTex;//在（0,0）~（1,1）上计算

	// 抖动
	bool bSupportShakePos = false;
	QMatrix4x4 shakePos;

	// 裁剪
	bool bSupportCrop = false;
	QMatrix4x4 cropTex;
	QMatrix4x4 cropPos;

	void SetDefault()
	{
		bSupportScroll = false;
		bSupportShakePos = false;
		bSupportCrop = false;

		//scrollTex = QMatrix4x4();
		//shakePos = QMatrix4x4();

		//cropTex = QMatrix4x4();
		//cropPos = QMatrix4x4();
	}
};

struct SRenderScrollData
{
	int m_nNowX = 0;				//当前X
	int m_nNowY = 0;				//当前Y

	int m_nScrollWndWidth = 0;
	int m_nScrollWndHeight = 0;

	// 拼接合集
	bool m_bHorStitch = true;	// 拼接方向是否横向，反之竖向
	bool m_bHorScroll = true;	// 滚动方向是否横向，反之竖直滚，要是静止需提前判断

	int m_nRealTotalWidth = 1920;	// 纹理实际总宽度
	int m_nRealTotalHeight = 1080;	// 纹理实际总高度
};


static bool isInterset(float min1, float max1, float min2, float max2)
{
	return std::max(min1, min2) < std::min(max1, max2);
}
