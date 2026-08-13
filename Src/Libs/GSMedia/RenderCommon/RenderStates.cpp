#include "d3dUtil.h"
#include "RenderStates.h"
#include "DXTrace.h"

using namespace Microsoft::WRL;

bool RenderStates::IsInit()
{
	// 一般来说初始化操作会把所有的状态都创建出来
	return RSWireframe != nullptr;
}

void RenderStates::InitAll(ID3D11Device* device)
{
	// 先前初始化过的话就没必要重来了
	if (IsInit())
		return;
	// ******************
	// 初始化光栅化器状态
	//
	CD3D11_RASTERIZER_DESC rasterizerDesc(CD3D11_DEFAULT{});

	// 线框模式
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	HR(device->CreateRasterizerState(&rasterizerDesc, RSWireframe.GetAddressOf()));

	// 无背面剔除模式
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	HR(device->CreateRasterizerState(&rasterizerDesc, RSNoCull.GetAddressOf()));

	// 顺时针剔除模式
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = true;
	HR(device->CreateRasterizerState(&rasterizerDesc, RSCullClockWise.GetAddressOf()));

	// 深度偏移模式
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.SlopeScaledDepthBias = 1.0f;
	HR(device->CreateRasterizerState(&rasterizerDesc, RSShadow.GetAddressOf()));

	CD3D11_RASTERIZER_DESC rastDesc(D3D11_FILL_SOLID, D3D11_CULL_NONE, FALSE,
		D3D11_DEFAULT_DEPTH_BIAS, D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, TRUE, TRUE);
	HR(device->CreateRasterizerState(&rastDesc, RSGridGround.GetAddressOf()));

	// ******************
	// 初始化采样器状态
	//
	CD3D11_SAMPLER_DESC sampDesc(CD3D11_DEFAULT{});

	// 点过滤与Clamp模式
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	HR(device->CreateSamplerState(&sampDesc, SSPointClamp.GetAddressOf()));

	// 线性过滤与Clamp模式
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	HR(device->CreateSamplerState(&sampDesc, SSLinearClamp.GetAddressOf()));

	// 2倍各向异性过滤与Clamp模式
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 2;
	HR(device->CreateSamplerState(&sampDesc, SSAnistropicClamp2x.GetAddressOf()));

	// 4倍各向异性过滤与Clamp模式
	sampDesc.MaxAnisotropy = 4;
	HR(device->CreateSamplerState(&sampDesc, SSAnistropicClamp4x.GetAddressOf()));

	// 8倍各向异性过滤与Clamp模式
	sampDesc.MaxAnisotropy = 8;
	HR(device->CreateSamplerState(&sampDesc, SSAnistropicClamp8x.GetAddressOf()));

	// 16倍各向异性过滤与Clamp模式
	sampDesc.MaxAnisotropy = 16;
	HR(device->CreateSamplerState(&sampDesc, SSAnistropicClamp16x.GetAddressOf()));

	// 线性过滤与Wrap模式
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MaxAnisotropy = 0;
	HR(device->CreateSamplerState(&sampDesc, SSLinearWrap.GetAddressOf()));

	// 点过滤与Wrap模式
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	HR(device->CreateSamplerState(&sampDesc, SSPointWrap.GetAddressOf()));

	// 16倍各向异性过滤与Wrap模式
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 16;
	HR(device->CreateSamplerState(&sampDesc, SSAnistropicWrap16x.GetAddressOf()));

	// 线性过滤与Border模式
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.BorderColor[0] = 0.0f;
	sampDesc.BorderColor[1] = 0.0f;
	sampDesc.BorderColor[2] = 0.0f;
	sampDesc.BorderColor[3] = 0.0f;
	HR(device->CreateSamplerState(&sampDesc, SSLinearBorder.GetAddressOf()));

	// 采样器状态：深度比较与Border模式
	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	sampDesc.MaxAnisotropy = 1;
	sampDesc.MinLOD = 0.0f;
	sampDesc.MaxLOD = 0.0f;
	sampDesc.BorderColor[0] = 0.0f;
	sampDesc.BorderColor[1] = 0.0f;
	sampDesc.BorderColor[2] = 0.0f;
	sampDesc.BorderColor[3] = 1.0f;
	HR(device->CreateSamplerState(&sampDesc, SSShadowPCF.GetAddressOf()));

	// ******************
	// 初始化混合状态
	//
	CD3D11_BLEND_DESC blendDesc(CD3D11_DEFAULT{});
	auto& rtDesc = blendDesc.RenderTarget[0];

	// Alpha-To-Coverage模式
	blendDesc.AlphaToCoverageEnable = true;
	HR(device->CreateBlendState(&blendDesc, BSAlphaToCoverage.GetAddressOf()));

	// 覆盖混合模式
	// Color = SrcAlpha * SrcColor + （1- SrcAlpha） * DestColor 
	// Alpha = SrcAlpha+DestAlpha
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	rtDesc.BlendEnable = true;
	rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtDesc.DestBlendAlpha = D3D11_BLEND_ONE;
	rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HR(device->CreateBlendState(&blendDesc, BSCover.GetAddressOf()));

	// 透明混合模式
	// Color = SrcAlpha * SrcColor + (1 - SrcAlpha) * DestColor 
	// Alpha = SrcAlpha
	blendDesc.AlphaToCoverageEnable = false;
	rtDesc.BlendEnable = true;
	rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	HR(device->CreateBlendState(&blendDesc, BSTransparent.GetAddressOf()));

	// 预乘 Alpha 混合（源 RGB 已含透明度，如 RenderItem 淡化烘焙后的媒体 RT）
	// Color = SrcColor + (1 - SrcAlpha) * DestColor
	// Alpha = SrcAlpha + (1 - SrcAlpha) * DestAlpha
	blendDesc.AlphaToCoverageEnable = false;
	rtDesc.BlendEnable = true;
	rtDesc.SrcBlend = D3D11_BLEND_ONE;
	rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtDesc.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	HR(device->CreateBlendState(&blendDesc, BSPremulAlpha.GetAddressOf()));

	// 加法混合模式
	// Color = SrcColor + DestColor
	// Alpha = SrcAlpha + DestAlpha
	rtDesc.SrcBlend = D3D11_BLEND_ONE;
	rtDesc.DestBlend = D3D11_BLEND_ONE;
	rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtDesc.DestBlendAlpha = D3D11_BLEND_ONE;
	rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	HR(device->CreateBlendState(&blendDesc, BSAdditive.GetAddressOf()));

	// 带Alpha权重的加法混合模式
	// Color = SrcAlpha * SrcColor + DestColor
	// Alpha = SrcAlpha
	rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtDesc.DestBlend = D3D11_BLEND_ONE;
	rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	HR(device->CreateBlendState(&blendDesc, BSAlphaWeightedAdditive.GetAddressOf()));

	// 叠加模式
	// Color = SrcAlpha * SrcColor + DestAlpha * DestColor 
	// Alpha = SrcAlpha + DestAlpha
	rtDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtDesc.DestBlend = D3D11_BLEND_DEST_ALPHA;
	rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtDesc.DestBlendAlpha = D3D11_BLEND_ONE;
	rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	HR(device->CreateBlendState(&blendDesc, BSTowAlphaWeightedAdditive.GetAddressOf()));

	// ******************
	// 初始化深度/模板状态
	//
	CD3D11_DEPTH_STENCIL_DESC dsDesc(CD3D11_DEFAULT{});
	// 仅允许深度值一致的像素进行写入的深度/模板状态
	// 没必要写入深度
	HR(device->CreateDepthStencilState(&dsDesc, DSSDefault.GetAddressOf()));

	// 关闭深度测试的深度/模板状态
	// 若绘制非透明物体，务必严格按照绘制顺序
	// 绘制透明物体则不需要担心绘制顺序
	// 而默认情况下模板测试就是关闭的
	dsDesc.DepthEnable = false;
	HR(device->CreateDepthStencilState(&dsDesc, DSSNoDepthTest.GetAddressOf()));

	// 开启默认深度测试
	// 模版测试总是通过 且 通过深度/模版测试时写1
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	dsDesc.StencilEnable = TRUE;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	HR(device->CreateDepthStencilState(&dsDesc, DSSStencilAlways.GetAddressOf()));

	// 开启默认深度测试
	// 模版测试总是通过 且 通过深度/模版测试时写1 深度失败时不写入
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	dsDesc.StencilEnable = TRUE;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	HR(device->CreateDepthStencilState(&dsDesc, DSSStencilAlwaysDepthFail.GetAddressOf()));

	// 关闭深度测试
	// 关闭模版写入
	// 模版测试不相等时通过
	dsDesc.DepthEnable = FALSE;

	dsDesc.StencilEnable = TRUE;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = 0x00;

	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	HR(device->CreateDepthStencilState(&dsDesc, DSSNoDepthStencilNotEqual.GetAddressOf()));

	// 默认深度测试
	// 关闭模版写入
	// 模版测试不相等时通过
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	dsDesc.StencilEnable = TRUE;
	dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = 0x00;

	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	HR(device->CreateDepthStencilState(&dsDesc, DSSStencilNotEqual.GetAddressOf()));


	// ******************
	// 设置调试对象名
	//
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	SetDebugObjectName(RSCullClockWise.Get(), "RSCullClockWise");
	SetDebugObjectName(RSNoCull.Get(), "RSNoCull");
	SetDebugObjectName(RSWireframe.Get(), "RSWireframe");
	SetDebugObjectName(RSShadow.Get(), "RSShadow");
	SetDebugObjectName(RSGridGround.Get(), "RSGridGround");

	SetDebugObjectName(SSPointClamp.Get(), "SSPointClamp");
	SetDebugObjectName(SSLinearWrap.Get(), "SSLinearWrap");
	SetDebugObjectName(SSLinearClamp.Get(), "SSLinearClamp");
	SetDebugObjectName(SSAnistropicWrap16x.Get(), "SSAnistropicWrap16x");
	SetDebugObjectName(SSAnistropicClamp2x.Get(), "SSAnistropicClamp2x");
	SetDebugObjectName(SSAnistropicClamp4x.Get(), "SSAnistropicClamp4x");
	SetDebugObjectName(SSAnistropicClamp8x.Get(), "SSAnistropicClamp8x");
	SetDebugObjectName(SSAnistropicClamp16x.Get(), "SSAnistropicClamp16x");
	SetDebugObjectName(SSShadowPCF.Get(), "SSShadowPCF");

	SetDebugObjectName(BSAlphaToCoverage.Get(), "BSAlphaToCoverage");
	SetDebugObjectName(BSTransparent.Get(), "BSTransparent");
	SetDebugObjectName(BSPremulAlpha.Get(), "BSPremulAlpha");
	SetDebugObjectName(BSAdditive.Get(), "BSAdditive");

	SetDebugObjectName(DSSDefault.Get(), "DSSDefault");
	SetDebugObjectName(DSSNoDepthTest.Get(), "DSSNoDepthTest");
	SetDebugObjectName(DSSStencilAlways.Get(), "DSSStencilAlways");
	SetDebugObjectName(DSSStencilAlwaysDepthFail.Get(), "DSSStencilAlwaysDepthFail");
	SetDebugObjectName(DSSNoDepthStencilNotEqual.Get(), "NoDepthStencilNotEqual");
	SetDebugObjectName(DSSStencilNotEqual.Get(), "StencilNotEqual");
#endif
}

void RenderStates::ReleaseAll()
{
	RSNoCull.Reset();
	RSWireframe.Reset();
	RSCullClockWise.Reset();
	RSShadow.Reset();
	RSGridGround.Reset();

	SSPointClamp.Reset();
	SSPointWrap.Reset();
	SSLinearWrap.Reset();
	SSLinearClamp.Reset();
	SSLinearBorder.Reset();
	SSAnistropicWrap16x.Reset();
	SSAnistropicClamp2x.Reset();
	SSAnistropicClamp4x.Reset();
	SSAnistropicClamp8x.Reset();
	SSAnistropicClamp16x.Reset();
	SSShadowPCF.Reset();

	BSCover.Reset();
	BSAlphaToCoverage.Reset();
	BSTransparent.Reset();
	BSPremulAlpha.Reset();
	BSAdditive.Reset();
	BSAlphaWeightedAdditive.Reset();
	BSTowAlphaWeightedAdditive.Reset();

	DSSDefault.Reset();
	DSSNoDepthTest.Reset();
	DSSStencilAlways.Reset();
	DSSStencilAlwaysDepthFail.Reset();
	DSSNoDepthStencilNotEqual.Reset();
	DSSStencilNotEqual.Reset();
}