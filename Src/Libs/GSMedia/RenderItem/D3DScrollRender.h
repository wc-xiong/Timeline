#pragma once

#include <wrl/client.h>
#include "GSMedia/RenderCommon/TextureRender.h"
#include <unordered_map>
#include <memory>

#include <QRect>

struct VertexPosTex;
struct SColorPty;
struct SDrawTexture;
struct STransInfo;
class IVertexCalculator;
class IMediaBaseProperty;
struct SVertexPosTexData;
struct SMediaScrollParam;
struct SRenderScrollData;

class CD3DScrollRender
{
public:
	// 使用模板别名(C++11)简化类型名
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	CD3DScrollRender();
	CD3DScrollRender(std::shared_ptr<IMediaBaseProperty> pBasePty, 
		std::shared_ptr<SMediaScrollParam> pScrollParam, 
		std::shared_ptr<SRenderScrollData> pScrollData);
	~CD3DScrollRender();

	// 获取普通素材是否滚动到窗口外
	bool GetTransationVertex(SVertexPosTexData &vertexNode, 
		float X, float Y, int W, int H, const QSize& offscreenSz);

	// 获取拼接合集是否滚动到窗口外
	bool GetStitchGrpTransationVertex(SVertexPosTexData &vertexNode,
		float X, float Y, int W, int H);

	// 更新当前滚动位置
	void UpdateScrollPos(bool bTexScrolling);

	void UpdateScrollParam(std::shared_ptr<SMediaScrollParam> pScrollParam);
	void UpdateBaseMediaPty(std::shared_ptr<IMediaBaseProperty> pMediaBasePty);

protected:
	void UpdatePosNotEndToEnd(int scrollDir,int nTexWidth, int nTexHeight, int nWndWidth, int nWndHeight);

protected:


	std::shared_ptr<IMediaBaseProperty> m_pBasePty = nullptr;	// 媒体基础属性
	std::shared_ptr<SMediaScrollParam> m_pScrollParam = nullptr;	// 媒体滚动数据

	std::shared_ptr<SRenderScrollData> m_pScrollData = nullptr;	// 媒体滚动数据（Render）

	//首尾相接切换位置同步,在首尾相接的时候记录无首尾相接的位置，在切换到无首尾相接时候，更新位置
	int m_nLastScrollType;
	bool m_bLastIsEndToEnd;
	float m_fCurPosY = 0.0;
	float m_fCurPosX = 0.0;
};