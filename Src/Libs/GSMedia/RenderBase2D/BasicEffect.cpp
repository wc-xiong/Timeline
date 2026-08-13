#include "pch.h"
#include "BasicEffect.h"
#include "GSMedia/RenderCommon/d3dHelper.h"
#include "EffectHelper.h"
#include "GSMedia/RenderCommon/DXTrace.h"
#include "GSMedia/RenderCommon/Vertex.h"
using namespace DirectX;

template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

# pragma warning(disable: 26812)
#define RENDER_LIGHT_COUNT			8

//
// BasicEffect::Impl 需要先于BasicEffect的定义
//

class BasicEffect::Impl : public AlignedType<BasicEffect::Impl>
{
public:

	//
	// 这些结构体对应HLSL的结构体。需要按16字节对齐
	//

	__declspec(align(16)) struct CBVSChangesEveryDrawing
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInvTranspose;
		CBVSChangesEveryDrawing()
		{
			world = DirectX::XMMatrixIdentity();
			worldInvTranspose = DirectX::XMMatrixIdentity();
		}
	};

	__declspec(align(16)) struct CBVSChangesEveryFrame
	{
		DirectX::XMMATRIX view;
		DirectX::XMVECTOR eyePos;
		CBVSChangesEveryFrame()
		{
			view = DirectX::XMMatrixIdentity();
			eyePos = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		}
	};

	__declspec(align(16)) struct CBVSChangesOnResize
	{
		DirectX::XMMATRIX proj;
		CBVSChangesOnResize()
		{
			proj = DirectX::XMMatrixIdentity();
		}
	};

	__declspec(align(16)) struct CBPSChangesEveryDrawing
	{
		SColorPty cellClr;
	};

	__declspec(align(16)) struct CBPSChangesCellPty
	{
		Video_ClrMetedata cellMeta;
	};

	__declspec(align(16)) struct CBPSChangesEveryFrame
	{
		SColorPty globalClr;
		SColorPty screenClr;
		int		  srnHdrPty;					// 屏幕HDR属性

		CBPSChangesEveryFrame()
		{
			srnHdrPty = 0x11;
		}
	};

	// 离屏渲染特效数据  CBXEffect


	// 曲面细分结构体
	__declspec(align(16)) struct STessConstantBuffer
	{
		float tessFactor = 64.0f;
	};


	__declspec(align(16)) struct CBLightHelperBuffer
	{
		DirectionalLight dirLight[RENDER_LIGHT_COUNT];
		PointLight pointLight[RENDER_LIGHT_COUNT];
		SpotLight spotLight[RENDER_LIGHT_COUNT];
		PhongMaterial material;
	};

public:
	// 必须显式指定
	Impl() : m_IsDirty() {}
	~Impl() = default;

public:
	// 需要16字节对齐的优先放在前面
	CBufferObject<0, CBVSChangesEveryDrawing>	m_CBVSDrawing;			// 每次绘制的顶点常量缓冲区
	CBufferObject<1, CBVSChangesEveryFrame>		m_CBVSFrame;			// 每帧绘制的顶点常量缓冲区
	CBufferObject<2, CBVSChangesOnResize>		m_CBVSOnResize;			// 每次窗口切换顶点常量缓冲区

	CBufferObject<3, CBPSChangesEveryDrawing>	m_CBPSDrawing;			// 每次绘制状态改变的常量缓冲区
	CBufferObject<4, CBPSChangesCellPty>		m_CBPSCellPty;			// 每次绘制状态改变的常量缓冲区
	CBufferObject<5, CBPSChangesEveryFrame>		m_CBPSFrame;			// 每帧绘制状态改变的常量缓冲区
	CBufferObject<6, CBXEffect>					m_CBXEffect;			// 特效常量缓冲区
	CBufferObject<7, STessConstantBuffer>		m_CBHSTessellation;		// 曲面细分常量缓冲区
	CBufferObject<8, CBLightHelperBuffer>		m_CBLightHelper;		// 灯光和材质常量缓冲区

	BOOL m_IsDirty;											            // 是否有值变更
	std::vector<CBufferBase*> m_pCBuffers;					            // 统一管理上面所有的常量缓冲区

	ComPtr<ID3D11VertexShader> m_pBasicVS;
	ComPtr<ID3D11VertexShader> m_p3DVS;
	// 曲面细分顶点着色器
	ComPtr<ID3D11VertexShader> m_pTessellationVertexShader = nullptr;

	// 曲面细分外壳着色器
	ComPtr<ID3D11HullShader> m_pTessellationHullShader = nullptr;

	// 曲面细分域着色器
	ComPtr<ID3D11DomainShader> m_pTessellationDomainShader = nullptr;

	ComPtr<ID3D11PixelShader>  m_pBasicPS;
	ComPtr<ID3D11PixelShader>  m_p3DPS;
	ComPtr<ID3D11PixelShader>  m_pEffectPS;
	ComPtr<ID3D11PixelShader>  m_pPhwNTPS;
	ComPtr<ID3D11VertexShader> m_pPNTVS;

	ComPtr<ID3D11InputLayout> m_pVertexTexLayout;
	ComPtr<ID3D11InputLayout> m_pVertexPosNormalTexLayout;

	ComPtr<ID3D11VertexShader> m_p2DVS;
	ComPtr<ID3D11PixelShader>  m_p2DPS;
	ComPtr<ID3D11InputLayout> m_pVertexClrLayout;
};

//
// BasicEffect
//

BasicEffect::BasicEffect()
{
	pImpl = std::make_unique<BasicEffect::Impl>();
}

BasicEffect::~BasicEffect()
{
	m_RenderStates.ReleaseAll();
}

BasicEffect::BasicEffect(BasicEffect && moveFrom) noexcept
{
	pImpl.swap(moveFrom.pImpl);
}

BasicEffect & BasicEffect::operator=(BasicEffect && moveFrom) noexcept
{
	pImpl.swap(moveFrom.pImpl);
	return *this;
}

bool BasicEffect::InitAll(ID3D11Device * device)
{
	if (!device)
		return false;

	if (!pImpl->m_pCBuffers.empty())
		return true;

	if (!m_RenderStates.IsInit())
		m_RenderStates.InitAll(device);

	// ******************
	// 创建顶点着色器
	//
	ComPtr<ID3DBlob> blob;
	const std::wstring hlslDir = GetModuleHLSLDir();

	// 缺 HLSL 时返回 false（预览可回落），避免 Debug 下 HR 弹框/进调试器
	auto loadShader = [&](const wchar_t* csoName, const wchar_t* hlslName,
		LPCSTR entry, LPCSTR model) -> bool
	{
		const std::wstring csoPath = hlslDir + csoName;
		const std::wstring hlslPath = hlslDir + hlslName;
		const HRESULT hr = CreateShaderFromFile(csoPath.c_str(), hlslPath.c_str(), entry, model, blob.ReleaseAndGetAddressOf());
		if (FAILED(hr) || !blob)
		{
			OutputDebugStringW(L"[BasicEffect] CreateShaderFromFile failed: ");
			OutputDebugStringW(hlslPath.c_str());
			OutputDebugStringW(L"\n");
			return false;
		}
		return true;
	};

	// ******************
	// 创建顶点着色器
	// Mapping：优先 exe\HLSL\*.cso；无则绝对路径编译 *.hlsl（勿用 CWD 相对路径）
	//
	if (!loadShader(L"VS_Basic.cso", L"VS_Basic.hlsl", "VS", "vs_5_0"))
		return false;
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pBasicVS.GetAddressOf()));
	
	if (!loadShader(L"VS_3D.cso", L"VS_3D.hlsl", "VS", "vs_5_0"))
		return false;
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_p3DVS.GetAddressOf()));
	// 创建顶点布局
	HR(device->CreateInputLayout(VertexPosTex::inputLayout, 2, blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexTexLayout.GetAddressOf()));


	if (!loadShader(L"VS_2D.cso", L"VS_2D.hlsl", "VS", "vs_5_0"))
		return false;
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_p2DVS.GetAddressOf()));
	HR(device->CreateInputLayout(VertexPosColor::inputLayout2, 2, blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexClrLayout.GetAddressOf()));

	if (!loadShader(L"VS_PNT.cso", L"VS_PNT.hlsl", "VS", "vs_5_0"))
		return false;
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pPNTVS.GetAddressOf()));
	HR(device->CreateInputLayout(VertexPosNormalTex::inputLayout, 3, blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosNormalTexLayout.GetAddressOf()));

	
	
	

	// ******************
	// 创建像素着色器
	//
	if (!loadShader(L"PS_Basic.cso", L"PS_Basic.hlsl", "PS", "ps_5_0"))
		return false;
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pBasicPS.GetAddressOf()));

	if (!loadShader(L"PS_3D.cso", L"PS_3D.hlsl", "PS", "ps_5_0"))
		return false;
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_p3DPS.GetAddressOf()));

	if (!loadShader(L"PS_PhwNT.cso", L"PS_PhwNT.hlsl", "PS", "ps_5_0"))
		return false;
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pPhwNTPS.GetAddressOf()));

	if (!loadShader(L"PS_2D.cso", L"PS_2D.hlsl", "PS", "ps_5_0"))
		return false;
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_p2DPS.GetAddressOf()));

	if (!loadShader(L"XEffect.cso", L"XEffect.hlsl", "PS", "ps_5_0"))
		return false;
	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pEffectPS.GetAddressOf()));

	// 曲面细分相关的着色器
	if (!loadShader(L"VS_Tessellation.cso", L"VS_Tessellation.hlsl", "VS", "vs_5_0"))
		return false;
	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pTessellationVertexShader.GetAddressOf()));
	if (!loadShader(L"HS_Transfer.cso", L"HS_Transfer.hlsl", "HS", "hs_5_0"))
		return false;
	HR(device->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pTessellationHullShader.GetAddressOf()));
	if (!loadShader(L"DS_Liner.cso", L"DS_Liner.hlsl", "DS", "ds_5_0"))
		return false;
	HR(device->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pTessellationDomainShader.GetAddressOf()));

	pImpl->m_pCBuffers.assign({
		&pImpl->m_CBVSDrawing,
		&pImpl->m_CBVSFrame,
		&pImpl->m_CBVSOnResize,
		&pImpl->m_CBPSDrawing,
		&pImpl->m_CBPSCellPty,
		&pImpl->m_CBPSFrame,
		&pImpl->m_CBXEffect,
		&pImpl->m_CBHSTessellation,
		&pImpl->m_CBLightHelper});

	// 创建常量缓冲区
	for (auto& pBuffer : pImpl->m_pCBuffers)
	{
		HR(pBuffer->CreateBuffer(device));
	}

	SetViewMatrix(DirectX::XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, RENDER_ZPLANE, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));

	// 设置调试对象名
	D3D11SetDebugObjectName(pImpl->m_pVertexTexLayout.Get(), "BasicEffect.VertexTexLayout");
	D3D11SetDebugObjectName(pImpl->m_pVertexPosNormalTexLayout.Get(), "BasicEffect.VertexPosNormalTexLayout");
	D3D11SetDebugObjectName(pImpl->m_pVertexClrLayout.Get(), "BasicEffect.VertexClrLayout");

	D3D11SetDebugObjectName(pImpl->m_pCBuffers[0]->cBuffer.Get(), "BasicEffect.m_CBVSDrawing");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[1]->cBuffer.Get(), "BasicEffect.m_CBVSFrame");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[2]->cBuffer.Get(), "BasicEffect.m_CBVSOnResize");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[3]->cBuffer.Get(), "BasicEffect.m_CBPSDrawing");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[4]->cBuffer.Get(), "BasicEffect.m_CBPSCellPty");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[5]->cBuffer.Get(), "BasicEffect.m_CBPSFrame");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[6]->cBuffer.Get(), "BasicEffect.m_CBXEffect");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[7]->cBuffer.Get(), "BasicEffect.m_CBHSTessellation");
	D3D11SetDebugObjectName(pImpl->m_pCBuffers[8]->cBuffer.Get(), "BasicEffect.m_CBLightHelperBuffer");

	D3D11SetDebugObjectName(pImpl->m_pBasicVS.Get(), "BasicEffect.Basic_VS");
	D3D11SetDebugObjectName(pImpl->m_pBasicPS.Get(), "BasicEffect.Basic_PS");
	D3D11SetDebugObjectName(pImpl->m_p3DVS.Get(), "BasicEffect.3D_VS");
	D3D11SetDebugObjectName(pImpl->m_p3DPS.Get(), "BasicEffect.3D_PS");
	D3D11SetDebugObjectName(pImpl->m_p2DVS.Get(), "BasicEffect.2D_VS");
	D3D11SetDebugObjectName(pImpl->m_p2DPS.Get(), "BasicEffect.2D_PS");
	D3D11SetDebugObjectName(pImpl->m_pPhwNTPS.Get(), "BasicEffect.PhwNT_PS");
	D3D11SetDebugObjectName(pImpl->m_pPNTVS.Get(), "BasicEffect.PNT_VS");
	D3D11SetDebugObjectName(pImpl->m_pEffectPS.Get(), "BasicEffect.Effect_PS");

	D3D11SetDebugObjectName(pImpl->m_pTessellationVertexShader.Get(), "BasicEffect.TessellationVertexShader");
	D3D11SetDebugObjectName(pImpl->m_pTessellationHullShader.Get(), "BasicEffect.TessellationHullShader");
	D3D11SetDebugObjectName(pImpl->m_pTessellationDomainShader.Get(), "BasicEffect.TessellationDomainShader");

#if defined(_DEBUG)
	OutputDebugStringW(L"[BasicEffect2D] InitAll created input layouts\n");
#endif

	return true;
}

void BasicEffect::SetRenderState(ID3D11DeviceContext * deviceContext, VertexShader vsType, PixelShader pstype, BlendMode bmMode, float* bmFactor, SamplerState sample)
{
	deviceContext->OMSetDepthStencilState(m_RenderStates.DSSNoDepthTest.Get(), 0);
	deviceContext->RSSetState(m_RenderStates.RSNoCull.Get());

	switch (vsType)
	{
	case BasicEffect::VSPNT:
	{
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);			//图元类型
		deviceContext->IASetInputLayout(pImpl->m_pVertexPosNormalTexLayout.Get());						//输入布局
		deviceContext->VSSetShader(pImpl->m_pPNTVS.Get(), nullptr, 0);
	}
		break;
	case BasicEffect::VS3D:
	{
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);			//图元类型
		deviceContext->IASetInputLayout(pImpl->m_pVertexTexLayout.Get());						//输入布局
		deviceContext->VSSetShader(pImpl->m_p3DVS.Get(), nullptr, 0);
	}
		break;
	case BasicEffect::VSTransfer:
	{
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);			//图元类型
		deviceContext->IASetInputLayout(pImpl->m_pVertexTexLayout.Get());						//输入布局
		deviceContext->VSSetShader(pImpl->m_pTessellationVertexShader.Get(), nullptr, 0);
	}
		break;
	case BasicEffect::VS2D:
	{
		deviceContext->IASetInputLayout(pImpl->m_pVertexClrLayout.Get());						//输入布局
		deviceContext->VSSetShader(pImpl->m_p2DVS.Get(), nullptr, 0);
	}
		break;
	case BasicEffect::VSNoChange:
		break;
	default:
	{
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);			//图元类型
		deviceContext->IASetInputLayout(pImpl->m_pVertexTexLayout.Get());						//输入布局
		deviceContext->VSSetShader(pImpl->m_pBasicVS.Get(), nullptr, 0);
	}
		break;
	}

	switch (pstype)
	{
	case BasicEffect::PSPNT:
		deviceContext->PSSetShader(pImpl->m_pPhwNTPS.Get(), nullptr, 0);
		break;
	case BasicEffect::PS3D:
		deviceContext->PSSetShader(pImpl->m_p3DPS.Get(), nullptr, 0);
		break;
	case BasicEffect::PS2D:
		deviceContext->PSSetShader(pImpl->m_p2DPS.Get(), nullptr, 0);
		break;
	case BasicEffect::PSXEffect:
		deviceContext->PSSetShader(pImpl->m_pEffectPS.Get(), nullptr, 0);
		break;
	case BasicEffect::PSNoChange:
		break;
	default:
		deviceContext->PSSetShader(pImpl->m_pBasicPS.Get(), nullptr, 0);
		break;
	}

	switch (bmMode)
	{
	case BasicEffect::BMCover:
		deviceContext->OMSetBlendState(m_RenderStates.BSCover.Get(), bmFactor, 0xFFFFFFFF);
		break;
	case BasicEffect::BMBlend:
		deviceContext->OMSetBlendState(m_RenderStates.BSTransparent.Get(), bmFactor, 0xFFFFFFFF);
		break;
	case BasicEffect::BMScreen:
		deviceContext->OMSetBlendState(m_RenderStates.BSAdditive.Get(), bmFactor, 0xFFFFFFFF);
		break;
	case BasicEffect::BMPremulAlpha:
		deviceContext->OMSetBlendState(m_RenderStates.BSPremulAlpha.Get(), bmFactor, 0xFFFFFFFF);
		break;
	case BasicEffect::BMNoChange:
		break;
	}

	switch (sample)
	{
	case BasicEffect::SamplerPointClamp:
		deviceContext->PSSetSamplers(0, 1, m_RenderStates.SSPointClamp.GetAddressOf());
		break;
	case BasicEffect::SamplerPointWrap:
		deviceContext->PSSetSamplers(0, 1, m_RenderStates.SSPointWrap.GetAddressOf());
		break;
	case BasicEffect::SamplerLineClamp:
		deviceContext->PSSetSamplers(0, 1, m_RenderStates.SSLinearClamp.GetAddressOf());
		break;
	case BasicEffect::SamplerLineBorder:
		deviceContext->PSSetSamplers(0, 1, m_RenderStates.SSLinearBorder.GetAddressOf());
		break;
	case BasicEffect::SamplerLineWrap:
		deviceContext->PSSetSamplers(0, 1, m_RenderStates.SSLinearWrap.GetAddressOf());
		break;
	case BasicEffect::SamplerAnisotropic:
		deviceContext->PSSetSamplers(0, 1, m_RenderStates.SSAnistropicClamp4x.GetAddressOf());
		break;
	case BasicEffect::SampleNoChange:
		break;
	default:
		break;
	}
}

DirectX::FXMMATRIX XM_CALLCONV BasicEffect::SetWorldMatrix(const DirectX::FXMMATRIX& W)
{
	auto& cBuffer = pImpl->m_CBVSDrawing;
	DirectX::FXMMATRIX oldW = XMMatrixTranspose(cBuffer.data.world);
	cBuffer.data.world = XMMatrixTranspose(W);
	cBuffer.data.worldInvTranspose = XMMatrixInverse(nullptr, W);
	pImpl->m_IsDirty = cBuffer.isDirty = true;
	return oldW;
}

DirectX::FXMMATRIX XM_CALLCONV BasicEffect::SetViewMatrix(const FXMMATRIX& V)
{
	auto& cBuffer = pImpl->m_CBVSFrame;
	DirectX::FXMMATRIX oldV = XMMatrixTranspose(cBuffer.data.view);
	cBuffer.data.view = XMMatrixTranspose(V);
	pImpl->m_IsDirty = cBuffer.isDirty = true;
	return oldV;
}

DirectX::FXMMATRIX XM_CALLCONV BasicEffect::SetProjMatrix(const FXMMATRIX& P)
{
	auto& cBuffer = pImpl->m_CBVSOnResize;
	DirectX::FXMMATRIX oldP = XMMatrixTranspose(cBuffer.data.proj);
	cBuffer.data.proj = XMMatrixTranspose(P);
	pImpl->m_IsDirty = cBuffer.isDirty = true;
	return oldP;
}

void BasicEffect::SetTessFactor(float tessFactor)
{
	auto& cBuffer = pImpl->m_CBHSTessellation;
	cBuffer.data.tessFactor = tessFactor;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetStretchingMode(ID3D11DeviceContext * deviceContext)
{
	// 四控制点输入
	deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	deviceContext->IASetInputLayout(pImpl->m_pVertexTexLayout.Get());
	deviceContext->OMSetDepthStencilState(m_RenderStates.DSSNoDepthTest.Get(), 0);

	// 设置曲面细分专用着色器
	deviceContext->VSSetShader(pImpl->m_pTessellationVertexShader.Get(), nullptr, 0);
	deviceContext->HSSetShader(pImpl->m_pTessellationHullShader.Get(), nullptr, 0);
	deviceContext->DSSetShader(pImpl->m_pTessellationDomainShader.Get(), nullptr, 0);
}

void BasicEffect::ReleaseStretchingMode(ID3D11DeviceContext * deviceContext)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);			//图元类型
	deviceContext->IASetInputLayout(pImpl->m_pVertexTexLayout.Get());						//输入布局
	deviceContext->OMSetDepthStencilState(m_RenderStates.DSSNoDepthTest.Get(), 0);
	deviceContext->RSSetState(m_RenderStates.RSNoCull.Get());

	deviceContext->VSSetShader(nullptr, nullptr, 0);
	deviceContext->HSSetShader(nullptr, nullptr, 0);
	deviceContext->DSSetShader(nullptr, nullptr, 0);
}


void BasicEffect::SetGlobalClr(const SColorPty& clrPty)
{
	auto& cBuffer = pImpl->m_CBPSFrame;
	cBuffer.data.globalClr = clrPty;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetScreenClr(const SColorPty& clrPty, int hdrPty)
{
	auto& cBuffer = pImpl->m_CBPSFrame;
	cBuffer.data.screenClr = clrPty;
	cBuffer.data.srnHdrPty = hdrPty;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetCellClr(const SColorPty& clrPty)
{
	auto& cBuffer = pImpl->m_CBPSDrawing;
	cBuffer.data.cellClr = clrPty;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetCellPty(const Video_ClrMetedata& clrMete)
{
	auto& cBuffer = pImpl->m_CBPSCellPty;
	cBuffer.data.cellMeta = clrMete;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetXEffectPty(const CBXEffect& pty)
{
	auto& cBuffer = pImpl->m_CBXEffect;
	memcpy(&cBuffer.data, &pty, sizeof(CBXEffect));
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetDirLight(size_t pos, const DirectionalLight & dirLight)
{
	auto& cBuffer = pImpl->m_CBLightHelper;
	cBuffer.data.dirLight[pos] = dirLight;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetPointLight(size_t pos, const PointLight & pointLight)
{
	auto& cBuffer = pImpl->m_CBLightHelper;
	cBuffer.data.pointLight[pos] = pointLight;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetSpotLight(size_t pos, const SpotLight & spotLight)
{
	auto& cBuffer = pImpl->m_CBLightHelper;
	cBuffer.data.spotLight[pos] = spotLight;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::SetMaterial(const PhongMaterial & material)
{
	auto& cBuffer = pImpl->m_CBLightHelper;
	cBuffer.data.material = material;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void XM_CALLCONV BasicEffect::SetEyePos(FXMVECTOR eyePos)
{
	auto& cBuffer = pImpl->m_CBVSFrame;
	cBuffer.data.eyePos = eyePos;
	pImpl->m_IsDirty = cBuffer.isDirty = true;
}

void BasicEffect::Apply(ID3D11DeviceContext * deviceContext)
{
	auto& pCBuffers = pImpl->m_pCBuffers;

	// 将缓冲区绑定到渲染管线上
	pCBuffers[0]->BindVS(deviceContext);
	pCBuffers[1]->BindVS(deviceContext);
	pCBuffers[2]->BindVS(deviceContext);

	pCBuffers[3]->BindPS(deviceContext);
	pCBuffers[4]->BindPS(deviceContext);
	pCBuffers[5]->BindPS(deviceContext);
	pCBuffers[6]->BindPS(deviceContext);

	pCBuffers[7]->BindHS(deviceContext);
	pCBuffers[8]->BindPS(deviceContext);

	if (pImpl->m_IsDirty)
	{
		pImpl->m_IsDirty = false;
		for (auto& pCBuffer : pCBuffers)
		{
			pCBuffer->UpdateBuffer(deviceContext);
		}
	}
}


