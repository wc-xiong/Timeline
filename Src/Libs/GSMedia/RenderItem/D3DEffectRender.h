#pragma once

#include <wrl/client.h>
#include <d2d1.h>
#include "GSMedia/RenderCommon/TextureRender.h"
#include "GSMedia/RenderCommon/Model.h"
#include "RenderDrawDataDefine.h"

struct SFxRenderData;
struct SFxFactor;
class IFxProperty;
class IMediaBaseProperty;
struct SMediaPty;
class CD3DResourceChild;
class BasicEffect;

class CD3DEffectRender
{
public:
	// 使用模板别名(C++11)简化类型名
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	CD3DEffectRender(ID3D11Device * device, std::shared_ptr<SMediaPty> pMediaPty, std::shared_ptr<SFxRenderData> pFxRenderData);
	~CD3DEffectRender();

	//设置effect
	void SetBasicEffect(BasicEffect* pEffect);

	void UpdateFxRenderData(microseconds mediaTs);

	// 根据特效获取对应顶点特效信息，返回值：当前特效是否滚动
	bool GetFxVertex(SVertexPosTexData &vertexNode);

	void MediaFxRender(CD3DResourceChild* deviced3d,
		std::vector<ID3D11ShaderResourceView*> &vecViews,
		const STexDatas &tex,
		Video_ClrMetedata &texColorPty,
		__int64 pts);


	//初始化缓冲区
	void InitOffScreenTexs(CD3DResourceChild* deviced3d,int w, int y, DXGI_FORMAT format);

	// 绘制
	void Draw(CD3DResourceChild* deviced3d);

	// 绘制特效
	void Denoise(CD3DResourceChild* deviced3d);
	void Blur(CD3DResourceChild* deviced3d);
	void Mosaic(CD3DResourceChild* deviced3d);
	void HSV(CD3DResourceChild* deviced3d);
	void Frame(CD3DResourceChild* deviced3d);
	void Marquees(CD3DResourceChild* deviced3d);
	void Mask(CD3DResourceChild* deviced3d);
	void Blink(CD3DResourceChild* deviced3d);
	void Shadow(CD3DResourceChild* deviced3d);

	void UpdateMediaPty(std::shared_ptr<SMediaPty> pMediaPty);

protected:
	std::shared_ptr<SMediaPty> m_pMediaPty = nullptr;
	std::shared_ptr<SFxRenderData> m_pFxRenderData = nullptr;

	Model	m_Model;

	std::vector<ID3D11ShaderResourceView*> m_vecShaderViews;	// 纹理

	std::unique_ptr<CTextureRender>  m_offscreenTexsFront;
	std::unique_ptr<CTextureRender>  m_offscreenTexsBack;

	long long m_pts = 0;
	int m_width = -1;
	int m_height = -1;
	DXGI_FORMAT m_format=DXGI_FORMAT_R8G8B8A8_UNORM;

	std::shared_ptr<SFxFactor> m_pEffect = nullptr;
	
	//跑马灯
	std::vector<UINT8>					m_colors;
	ComPtr<ID3D11ShaderResourceView>	m_pTexView;

	//蒙版
	QString m_strFileName;
	std::map<int, ComPtr<ID3D11ShaderResourceView>> m_mapMask;

	ComPtr<ID2D1Geometry> CreateMaskGeo(ComPtr<ID2D1Factory> pD2DFactory, int nType);
	ComPtr<ID3D11ShaderResourceView> GetMaskView(CD3DResourceChild* deviced3d);

	std::mutex m_mtxVertexFxDraw;

	BasicEffect* m_pBasicEffect = nullptr;
};