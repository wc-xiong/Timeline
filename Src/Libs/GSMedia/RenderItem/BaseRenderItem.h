#pragma once
#include "RenderItemGlobal.h"
#include "Common/Common/IRenderItemInterface.h"
#include "Common/Common/MediaCommon.h"
#include "Common/Common/VideoDataCommon.h"
#include "RenderDrawDataDefine.h"

#include <d3d11_1.h>
#include <wrl/client.h>
#pragma comment(lib, "d3d11.lib")

#include <mutex>
#include <QPointF>
#include <QLineF>
#include <queue>

// 媒体对应的RenderItem

class CD3DResourceChild;
class CTextureRender;
class CRenderResCommonTexs;
class BasicEffect;
struct STexDatas;

class RENDERITEM_API CBaseRenderItem
	: public IBaseRenderItem
{
public:
	CBaseRenderItem(std::shared_ptr<SMediaPty> pMediaPty, CD3DResourceChild* pD3dRes, bool bUseCopyTexRender = true);
	virtual ~CBaseRenderItem();

	QUuid GetId() const override
	{
		return m_mediaId;
	}

	int GetType() override;
	void SetBasicEffect(BasicEffect* pEffect) override;
	void Render(const SDrawTextureData& texNode, const SColorPty& globalPty) override;

	SRenderItemTex GetDrawData() override;
	bool ConsumePublishTexDirty() override;

	virtual void UpdateDrawData() override {}
	virtual void UpdateMediaPtyByRedoUndo(std::shared_ptr<SMediaPty> pMediaPty) override;

	std::shared_ptr<SMediaPty> GetMediaPty()
	{
		return m_pMediaPty;
	}

protected:
	// transFx：时间线转场；淡入淡出在此统一乘到 cellClr.alpha，供 Base/Fx/Scroll 共用
	void UpdateEffectPty(
		const Video_ClrMetedata& frameType,
		const SColorPty& globalPty,
		int nSampleRate,
		const STransInfo& transFx = STransInfo());

	virtual void DrawTexture(
		const SColorPty& globalPty,
		std::vector<SDrawTexture> vecFrameTexs);

	virtual void UpdateVertexData(
		std::vector<VertexPosTex>& vertices,
		std::vector<DWORD>& indexs,
		const STexDatas& texNode);

	virtual void UpdateVertexData(const STexDatas& texNode);

	virtual void ResetTexRender(const STexDatas& texNode);
	void UpdateTexRenderByWidthHeight(int nWidth, int nHeight);

	// 根据纹理值获取GPU对应顶点、像素数据
	std::vector<VertexPosTex> GetVertexBufferByPosTexData(const SVertexPosTexData& vertexNode, const QSize& offscreenRc);
	// 获取初始顶点、像素数据
	SVertexPosTexData GetVertexTexData(const QSize& sz, const std::vector<VertexPosTex>& vertices = std::vector<VertexPosTex>());
	// 获取填充模式处理后的纹理顶点、像素数据
	void VertexFillTypeDeal(SVertexPosTexData& vertexNode, const STexDatas& texNode);
	// 获取纹理着色器资源
	std::vector<ID3D11ShaderResourceView*> GetShaderResView(STexDatas& data, Video_ClrMetedata& metaData, int index = 0);

protected:
	QUuid m_mediaId;
	std::shared_ptr<SMediaPty> m_pMediaPty = nullptr;

	Video_ClrMetedata m_clrMeteData;

	std::vector<CTextureRender*> m_vecTexRender;
	bool m_bUseCopyTexRender = true;
	BasicEffect* m_pBasicEffect = nullptr;
	CD3DResourceChild* m_pD3dRes = nullptr;

	std::vector<SRenderShaderResView> m_sViewNode;

	SDrawTextureData m_sDrawTexData;
	SRenderItemTex m_drawData;
	bool m_bPublishTexDirty = false;
};
