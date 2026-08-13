//***************************************************************************************
// TextureRender.h by X_Jun(MKXJun) (C) 2018-2020 All Rights Reserved.
// Licensed under the MIT License.
//
// 渲染到纹理类
// Render-To-Texture class.
//***************************************************************************************

#ifndef CTEXTURERENDER_H
#define CTEXTURERENDER_H

#include <d3d11_1.h>
#include <wrl/client.h>
#include <string>
#include "RenderCommonGlobal.h"
#include "Model.h"
#include "Vertex.h"

#pragma warning(disable:4251)

class RENDERCOMMON_API CTextureRender
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	CTextureRender(ID3D11Device* device);

	CTextureRender() = default;
	~CTextureRender() = default;
	// 不允许拷贝，允许移动
	CTextureRender(const CTextureRender&) = delete;
	CTextureRender& operator=(const CTextureRender&) = delete;
	CTextureRender(CTextureRender&&) = default;
	CTextureRender& operator=(CTextureRender&&) = default;

	HRESULT InitResource(ID3D11Device* device,
		int texWidth,
		int texHeight,
		DXGI_FORMAT format,
		bool bShared = false,
		bool generateMips = false,
		bool bUseCopyTex = false);

	// 设置纹理
	void SetShaderViews(const std::vector<ID3D11ShaderResourceView*>& vec);

	// 更新顶点缓冲区
	void SetVertexBuffer(ID3D11DeviceContext* deviceContext, const std::vector<VertexPosTex>& vertices);

	// 设置顶点缓冲区
	void SetVertexBuffer(ID3D11Device* device, const std::vector<VertexPosTex>& vertices, const std::vector<DWORD>& indices);

	void ResetResource();

	// 开始对当前纹理进行渲染
	void Begin(ID3D11DeviceContext * deviceContext, const FLOAT backgroundColor[4], BOOL bClearBk = TRUE);

	// 结束对当前纹理的渲染，还原状态；若启用拷贝则自动 CopyResource 到共享拷贝面
	void End(ID3D11DeviceContext * deviceContext);

	// 绘制（转场特效类中不使用此接口）
	void Draw(ID3D11DeviceContext* deviceContext);

	// 获取渲染好的纹理的着色器资源视图
	// 引用数不增加，仅用于传参
	ID3D11ShaderResourceView * GetTextureShaderView();
	ID3D11Texture2D*		   GetTexture2D();
	ID3D11RenderTargetView*    GetTextureTargetView();
	ID3D11DepthStencilView*	   GetDepthStencilView();

	// 设置调试对象名
	void SetDebugObjectName(const std::string& name);

	int GetTexWidth() {
		return m_nTexWidth;
	}

	int GetTexHeight() {
		return m_nTexHeight;
	}

	// 拷贝模式返回共享拷贝面 handle，否则返回主纹理 handle
	HANDLE GetTexHandle() {
		return m_bUseCopyTex ? m_handleCopyTex : m_handleTex;
	}

	bool IsUseCopyTex() const {
		return m_bUseCopyTex;
	}

protected:
	HRESULT CreateCopyTexture(ID3D11Device* device, int texWidth, int texHeight, DXGI_FORMAT format);
	void ClearCopyTexture();

	HANDLE m_handleTex = nullptr;
	ComPtr<ID3D11Texture2D>				m_pOutputTexture;		// 输出的纹理
	ComPtr<ID3D11ShaderResourceView>	m_pOutputTextureSRV;	// 输出的纹理对应的着色器资源视图
	ComPtr<ID3D11RenderTargetView>		m_pOutputTextureRTV;	// 输出的纹理对应的渲染目标视图


	ComPtr<ID3D11UnorderedAccessView>	m_pOutputTextureUAV;	// 输出的纹理对应的无序访问视图
	ComPtr<ID3D11DepthStencilView>		m_pOutputTextureDSV;	// 输出纹理所用的深度/模板视图

	D3D11_VIEWPORT						m_OutputViewPort = {};	// 输出所用的视口

	ComPtr<ID3D11RenderTargetView>		m_pCacheRTV;		    // 临时缓存的后备缓冲区
	ComPtr<ID3D11DepthStencilView>		m_pCacheDSV;		    // 临时缓存的深度/模板缓冲区
	D3D11_VIEWPORT						m_CacheViewPort = {};	// 临时缓存的视口
	std::vector<ID3D11ShaderResourceView*> m_vecShaderViews;	// 当前绘制着色器资源（外部纹理绘制）

	Model								m_Model;				// 顶点、像素着色器资源

	bool								m_GenerateMips = false;	// 是否生成mipmap链
	bool								m_bUseCopyTex = false;	// 是否使用共享拷贝面对外发布

	ComPtr<ID3D11Texture2D>				m_pCopyTexture;			// SHARED 拷贝面（仅拷贝模式）
	HANDLE								m_handleCopyTex = nullptr;

	int m_nTexWidth = -1;
	int m_nTexHeight = -1;
};

#endif
