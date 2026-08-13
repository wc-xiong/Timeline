#pragma once

#include "BaseRenderItem.h"

#include <mutex>
#include <QPointF>
#include <QLineF>

class CD3DTransationRender;
class CD3DScrollRender;
struct STransInfo;
struct VertexPosTex;

// 媒体对应的RenderItem
class RENDERITEM_API CMediaScrollRenderItem : public CBaseRenderItem,public IMediaScrollRenderItem
{
public:
	CMediaScrollRenderItem(std::shared_ptr<SMediaPty> pMediaPty, CD3DResourceChild* pD3dRes, bool bUseCopyTexRender = true);

	virtual ~CMediaScrollRenderItem();

	virtual void UpdateDrawData() override;

	//设置effect
	void SetBasicEffect(BasicEffect* pEffect) override;

	virtual void UpdateMediaPtyByRedoUndo(std::shared_ptr<SMediaPty> pMediaPty) override;

protected:
	bool UpdateVertexData(float X, float Y, int W, int H, const QRect& offscreenRc);

	void DrawTexture(const SColorPty& globalPty, std::vector<SDrawTexture> vecFrameTexs) override;

	void UpdateTextureColor(int index);

	virtual void ResetTexRender(const STexDatas& texNode) override;

protected:

	std::shared_ptr<SMediaScrollParam>	m_pMediaScrollParam = nullptr; // 媒体滚动数据

	CD3DTransationRender* m_pTransationRender = nullptr;	// 转场特效绘制

	CD3DScrollRender* m_pScrollRender = nullptr;	// 滚动数据获取

	std::shared_ptr<SRenderScrollData> m_pScrollData = nullptr;	// 滚动数据（Render）

	QColor m_clrBk = { 1,0,0,0 };
	std::vector<ID3D11ShaderResourceView*> m_vecBkSharedView;
};

