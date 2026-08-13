#pragma once
#include "BaseRenderItem.h"
// 媒体对应的RenderItem

class CRenderDeviceD3D;
class IRenderSwapchain;
class CPlayerRenderItemImpl;
struct STexDatas;
class CD3DTextureRender;
class CD2DRender;
class CD3DEffectRender;
class CTextureRender;
struct SFxRenderData;

class RENDERITEM_API CFxRenderItem : public CBaseRenderItem
{
public:
	CFxRenderItem(std::shared_ptr<SMediaPty> pMediaPty, CD3DResourceChild* pD3dRes, bool bUseCopyTexRender = true);
	virtual ~CFxRenderItem();

	virtual void UpdateDrawData() override;

	//设置effect
	void SetBasicEffect(BasicEffect* pEffect) override;

	virtual void UpdateMediaPtyByRedoUndo(std::shared_ptr<SMediaPty> pMediaPty) override;
	
protected:
	void UpdateVertexData(const STexDatas& texNode) override;

	void DrawTexture(const SColorPty& globalPty, std::vector<SDrawTexture> vecFrameTexs) override;

protected:
	CD3DEffectRender*			m_pEffectRender;	// 特效渲染器
	std::shared_ptr<SFxRenderData> m_pFxRenderData = nullptr;

	bool m_bScroll = false;	// 当前是否添加滚动类特效（渲染时设置采样器类型有关）
};

