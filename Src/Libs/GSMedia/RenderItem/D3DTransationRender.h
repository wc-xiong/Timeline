#pragma once

#include <wrl/client.h>
#include <unordered_map>
#include "GSMedia/RenderCommon/TextureRender.h"
#include "GSMedia/RenderCommon/Model.h"
#include "Common/TexFrameCommon.h"
#include "Common/MediaCommon.h"

struct VertexPosTex;
struct SColorPty;
struct SDrawTexture;
class IVertexCalculator;
class IMediaBaseProperty;
struct SVertexPosTexData;
struct STexDatas;
class CD3DResourceChild;
class BasicEffect;

class CD3DTransationRender
{
public:
	// 使用模板别名(C++11)简化类型名
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	CD3DTransationRender(ID3D11Device * device);
	~CD3DTransationRender();


	//设置effect
	void SetBasicEffect(BasicEffect* pEffect);

	void TransationFxRender(CD3DResourceChild* deviced3d, 
		SDrawTexture &texData, 
		const STransInfo &transFx,
		std::vector<ID3D11ShaderResourceView*> &vecViews, 
		SColorPty &pty, 
		std::vector<VertexPosTex> &vertices, 
		std::vector<DWORD> &indexs);

	//初始化缓冲区
	void InitOffScreenTexs(CD3DResourceChild* deviced3d,int w, int y, DXGI_FORMAT format);

	// 计算顶点
	void CalculaVertex(std::vector<VertexPosTex>& vertices, std::vector<DWORD>& indexs);

	// 绘制
	void Draw(CD3DResourceChild* deviced3d, const STexDatas &tex);

protected:
	Model	m_Model;

	std::vector<ID3D11ShaderResourceView*> m_vecShaderViews;	// 纹理

	std::unique_ptr<CTextureRender>  m_offscreenTexsFront;
	std::unique_ptr<CTextureRender>  m_offscreenTexsBack;

	STransInfo m_sTransInfo;

	std::unordered_map<int, IVertexCalculator*>	m_mapVertexCalculator;		//顶点计算器映射表

	std::map<QUuid, std::vector<ComPtr<ID3D11ShaderResourceView>>> m_mapTransTextures; //转场纹理<特效标识，纹理>

	// 绘制转场特效临时纹理数据
	int m_nTexWidth = -1;
	int m_nTexHeight = -1;
	DXGI_FORMAT m_nTexFromat = DXGI_FORMAT_UNKNOWN;

	BasicEffect* m_pBasicEffect = nullptr;
};