#include "pch.h"
#include "TextureRender.h"
#include "d3dUtil.h"

#pragma warning(disable: 26812)

using namespace DirectX;
using namespace Microsoft::WRL;

// 默认顶点像素着色器设置、顶点绘制顺序
static std::vector<VertexPosTex> g_vecDefalutVertices = {
{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
{ DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) } ,
{ DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) } ,
{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) } };

static std::vector<DWORD> g_vecDefaultIndexs = { 0,1,2,2,3,0 };

CTextureRender::CTextureRender(ID3D11Device * device)
{
	m_Model.AddMesh(device, g_vecDefalutVertices, g_vecDefaultIndexs);
}

HRESULT CTextureRender::InitResource(ID3D11Device* device, int texWidth, int texHeight,
	DXGI_FORMAT format, bool bShared, bool generateMips, bool bUseCopyTex)
{
	// 启用拷贝时主绘制面强制非共享，对外通过拷贝面 SHARED handle 发布
	if (bUseCopyTex)
		bShared = false;

	const bool copyReady = !bUseCopyTex || (m_pCopyTexture && m_handleCopyTex);
	if (m_nTexWidth == texWidth && m_nTexHeight == texHeight
		&& m_bUseCopyTex == bUseCopyTex && copyReady)
		return S_OK;

	// 防止重复初始化造成内存泄漏
	ResetResource();

	m_GenerateMips = generateMips;
	m_bUseCopyTex = bUseCopyTex;
	HRESULT hr;
	// ******************
	// 1. 创建纹理
	//
	m_nTexWidth = texWidth;
	m_nTexHeight = texHeight;

	D3D11_TEXTURE2D_DESC texDesc;

	texDesc.Width = texWidth;
	texDesc.Height = texHeight;
	texDesc.MipLevels = (m_GenerateMips ? 0 : 1);	// 0为完整mipmap链
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = format;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | (bShared ? D3D11_RESOURCE_MISC_SHARED : 0);

	// 现在texture用于新建纹理
	hr = device->CreateTexture2D(&texDesc, nullptr, m_pOutputTexture.GetAddressOf());
	if (FAILED(hr))
		return hr;
	// ******************
	// 2. 创建纹理对应的渲染目标视图
	//
	m_handleTex = nullptr;
	if (bShared)
	{
		ComPtr<IDXGIResource> pDXGIResource = nullptr;
		hr = m_pOutputTexture->QueryInterface(__uuidof(IDXGIResource), (LPVOID*)&pDXGIResource);
		if (FAILED(hr))
			return hr;
		hr = pDXGIResource->GetSharedHandle(&m_handleTex);
		if (FAILED(hr))
			return hr;
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	hr = device->CreateRenderTargetView(m_pOutputTexture.Get(), &rtvDesc, m_pOutputTextureRTV.GetAddressOf());
	if (FAILED(hr))
		return hr;

	// ******************
	// 3. 创建纹理对应的着色器资源视图
	//

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;	// 使用所有的mip等级

	hr = device->CreateShaderResourceView(m_pOutputTexture.Get(), &srvDesc, m_pOutputTextureSRV.GetAddressOf());
	if (FAILED(hr))
		return hr;

	// ******************
	// 4. 创建与纹理等宽高的深度/模板缓冲区和对应的视图
	//

	texDesc.Width = texWidth;
	texDesc.Height = texHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> depthTex;
	hr = device->CreateTexture2D(&texDesc, nullptr, depthTex.GetAddressOf());
	if (FAILED(hr))
		return hr;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = texDesc.Format;
	dsvDesc.Flags = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	hr = device->CreateDepthStencilView(depthTex.Get(), &dsvDesc,
		m_pOutputTextureDSV.GetAddressOf());
	if (FAILED(hr))
		return hr;

	// ******************
	// 5. 初始化视口
	//
	m_OutputViewPort.TopLeftX = 0.0f;
	m_OutputViewPort.TopLeftY = 0.0f;
	m_OutputViewPort.Width = static_cast<float>(texWidth);
	m_OutputViewPort.Height = static_cast<float>(texHeight);
	m_OutputViewPort.MinDepth = 0.0f;
	m_OutputViewPort.MaxDepth = 1.0f;

	// ******************
	// 6. 若启用拷贝，创建 SHARED 拷贝面
	//
	if (m_bUseCopyTex)
	{
		hr = CreateCopyTexture(device, texWidth, texHeight, format);
		if (FAILED(hr))
			return hr;
	}

	return S_OK;
}

HRESULT CTextureRender::CreateCopyTexture(ID3D11Device* device, int texWidth, int texHeight, DXGI_FORMAT format)
{
	ClearCopyTexture();

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = texWidth;
	texDesc.Height = texHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Format = format;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_SHARED;

	HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, m_pCopyTexture.GetAddressOf());
	if (FAILED(hr))
		return hr;

	ComPtr<IDXGIResource> pDXGIResource;
	hr = m_pCopyTexture->QueryInterface(__uuidof(IDXGIResource), reinterpret_cast<void**>(pDXGIResource.GetAddressOf()));
	if (FAILED(hr))
	{
		ClearCopyTexture();
		return hr;
	}

	hr = pDXGIResource->GetSharedHandle(&m_handleCopyTex);
	if (FAILED(hr))
	{
		ClearCopyTexture();
		return hr;
	}

	return S_OK;
}

void CTextureRender::ClearCopyTexture()
{
	m_pCopyTexture.Reset();
	m_handleCopyTex = nullptr;
}

void CTextureRender::SetShaderViews(const std::vector<ID3D11ShaderResourceView*>& vec)
{
	m_vecShaderViews = vec;
}

void CTextureRender::SetVertexBuffer(ID3D11DeviceContext* deviceContext, const std::vector<VertexPosTex>& vertices)
{
	m_Model.UpdataVertexBuffer(deviceContext, 0, vertices.data());
}

void CTextureRender::SetVertexBuffer(ID3D11Device* device, const std::vector<VertexPosTex>& vertices, const std::vector<DWORD>& indices)
{
	m_Model.modelParts.clear();
	m_Model.AddMesh(device, vertices, indices);
}

void CTextureRender::ResetResource()
{
	m_pOutputTextureSRV.Reset();
	m_pOutputTextureRTV.Reset();
	m_pOutputTexture.Reset();
	m_pOutputTextureDSV.Reset();
	m_pCacheRTV.Reset();
	m_pCacheDSV.Reset();
	m_handleTex = nullptr;
	ClearCopyTexture();
	m_bUseCopyTex = false;
	m_nTexWidth = -1;
	m_nTexHeight = -1;
}

void CTextureRender::Begin(ID3D11DeviceContext* deviceContext, const FLOAT backgroundColor[4], BOOL bClear)
{
	// 缓存渲染目标和深度模板视图
	deviceContext->OMGetRenderTargets(1, m_pCacheRTV.GetAddressOf(), m_pCacheDSV.GetAddressOf());
	// 缓存视口
	UINT num_Viewports = 1;
	deviceContext->RSGetViewports(&num_Viewports, &m_CacheViewPort);

	// 清空缓冲区
	if (bClear)
	{
		deviceContext->ClearRenderTargetView(m_pOutputTextureRTV.Get(), backgroundColor);
		deviceContext->ClearDepthStencilView(m_pOutputTextureDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
	// 设置渲染目标和深度模板视图
	deviceContext->OMSetRenderTargets(1, m_pOutputTextureRTV.GetAddressOf(), m_pOutputTextureDSV.Get());
	// 设置视口
	deviceContext->RSSetViewports(1, &m_OutputViewPort);
}

void CTextureRender::End(ID3D11DeviceContext* deviceContext)
{
	// 恢复默认设定
	deviceContext->RSSetViewports(1, &m_CacheViewPort);
	deviceContext->OMSetRenderTargets(1, m_pCacheRTV.GetAddressOf(), m_pCacheDSV.Get());

	// 若之前有指定需要mipmap链，则生成
	if (m_GenerateMips)
	{
		deviceContext->GenerateMips(m_pOutputTextureSRV.Get());
	}

	// 启用拷贝时，将主绘制面同步到 SHARED 拷贝面
	if (m_bUseCopyTex && m_pCopyTexture && m_pOutputTexture)
	{
		deviceContext->CopyResource(m_pCopyTexture.Get(), m_pOutputTexture.Get());
	}

	// 清空临时缓存的渲染目标视图和深度模板视图
	m_pCacheDSV.Reset();
	m_pCacheRTV.Reset();
}

void CTextureRender::Draw(ID3D11DeviceContext* deviceContext)
{
	// 设置顶点/索引缓冲区
	UINT strides = m_Model.vertexStride;
	UINT offsets = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_Model.modelParts[0].vertexBuffer.GetAddressOf(), &strides, &offsets);
	deviceContext->IASetIndexBuffer(m_Model.modelParts[0].indexBuffer.Get(), m_Model.modelParts[0].indexFormat, 0);

	deviceContext->PSSetShaderResources(0, (int)m_vecShaderViews.size(), m_vecShaderViews.data());
	deviceContext->DrawIndexed((UINT)m_Model.modelParts[0].indexCount, 0, 0);
}


ID3D11ShaderResourceView* CTextureRender::GetTextureShaderView()
{
	return m_pOutputTextureSRV.Get();
}

ID3D11Texture2D* CTextureRender::GetTexture2D()
{
	return m_pOutputTexture.Get();
}

ID3D11RenderTargetView*    CTextureRender::GetTextureTargetView()
{
	return m_pOutputTextureRTV.Get();
}

ID3D11DepthStencilView * CTextureRender::GetDepthStencilView()
{
	return m_pOutputTextureDSV.Get();
}

void CTextureRender::SetDebugObjectName(const std::string& name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	D3D11SetDebugObjectName(m_pOutputTextureDSV.Get(), name + ".TextureDSV");
	D3D11SetDebugObjectName(m_pOutputTextureSRV.Get(), name + ".TextureSRV");
	D3D11SetDebugObjectName(m_pOutputTextureRTV.Get(), name + ".TextureRTV");
	D3D11SetDebugObjectName(m_pOutputTexture.Get(), name + ".Texture");

	if (m_pOutputTextureUAV)
	{
		D3D11SetDebugObjectName(m_pOutputTextureUAV.Get(), name + ".TextureUAV");
	}
#else
	UNREFERENCED_PARAMETER(name);
#endif
}
