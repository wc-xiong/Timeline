//***************************************************************************************
// RenderStates.h by X_Jun(MKXJun) (C) 2018-2022 All Rights Reserved.
// Licensed under the MIT License.
//
// 提供一些渲染状态
// Provide some render states.
//***************************************************************************************

#pragma once

#ifndef RENDER_STATES_H
#define RENDER_STATES_H

#include "RenderCommonGlobal.h"
#include <wrl/client.h>
#include <d3d11_1.h>


class RENDERCOMMON_API RenderStates
{
public:
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	bool IsInit();

	void InitAll(ID3D11Device * device);
	// 使用ComPtr无需手工释放

	void RenderStates::ReleaseAll();

public:
	ComPtr<ID3D11RasterizerState> RSWireframe = nullptr;		            // 光栅化器状态：线框模式
	ComPtr<ID3D11RasterizerState> RSNoCull = nullptr;			            // 光栅化器状态：无背面裁剪模式
	ComPtr<ID3D11RasterizerState> RSCullClockWise = nullptr;	            // 光栅化器状态：顺时针裁剪模式
	ComPtr<ID3D11RasterizerState> RSShadow = nullptr;						// 光栅化器状态：深度偏移模式
	ComPtr<ID3D11RasterizerState> RSGridGround = nullptr;					// 光栅化器状态：地面网格

	ComPtr<ID3D11SamplerState> SSPointClamp = nullptr;						// 采样器状态：点过滤与Clamp模式
	ComPtr<ID3D11SamplerState> SSPointWrap = nullptr;						// 采样器状态：点过滤与Wrap模式
	ComPtr<ID3D11SamplerState> SSLinearWrap = nullptr;			            // 采样器状态：线性过滤与Wrap模式
	ComPtr<ID3D11SamplerState> SSLinearClamp = nullptr;						// 采样器状态：线性过滤与Clamp模式
	ComPtr<ID3D11SamplerState> SSLinearBorder = nullptr;					// 采样器状态：线性过滤与Border模式
	ComPtr<ID3D11SamplerState> SSAnistropicWrap16x = nullptr;		        // 采样器状态：16倍各向异性过滤与Wrap模式
	ComPtr<ID3D11SamplerState> SSAnistropicClamp2x = nullptr;		        // 采样器状态：2倍各向异性过滤与Clamp模式
	ComPtr<ID3D11SamplerState> SSAnistropicClamp4x = nullptr;		        // 采样器状态：4倍各向异性过滤与Clamp模式
	ComPtr<ID3D11SamplerState> SSAnistropicClamp8x = nullptr;		        // 采样器状态：8倍各向异性过滤与Clamp模式
	ComPtr<ID3D11SamplerState> SSAnistropicClamp16x = nullptr;		        // 采样器状态：16倍各向异性过滤与Clamp模式
	ComPtr<ID3D11SamplerState> SSShadowPCF = nullptr;						// 采样器状态：深度比较与Border模式

	ComPtr<ID3D11BlendState> BSCover = nullptr;								// 混合状态：覆盖混合
	ComPtr<ID3D11BlendState> BSTransparent = nullptr;		                // 混合状态：透明混合
	ComPtr<ID3D11BlendState> BSPremulAlpha = nullptr;						// 混合状态：预乘 Alpha（RenderItem 已烘焙透明度的纹理）
	ComPtr<ID3D11BlendState> BSAlphaToCoverage = nullptr;	                // 混合状态：Alpha-To-Coverage
	ComPtr<ID3D11BlendState> BSAdditive = nullptr;			                // 混合状态：加法混合
	ComPtr<ID3D11BlendState> BSAlphaWeightedAdditive = nullptr;             // 混合状态：带Alpha权重的加法混合模式
	ComPtr<ID3D11BlendState> BSTowAlphaWeightedAdditive = nullptr;          // 混合状态：带Alpha权重的加法混合模式
	ComPtr<ID3D11DepthStencilState> DSSDefault = nullptr;						// 深度/模板状态：默认
	ComPtr<ID3D11DepthStencilState> DSSNoDepthTest = nullptr;					// 深度/模板状态：关闭深度测试
	ComPtr<ID3D11DepthStencilState> DSSStencilAlways = nullptr;					// 深度/模板状态：模版测试总是通过
	ComPtr<ID3D11DepthStencilState> DSSStencilAlwaysDepthFail = nullptr;        // 深度/模板状态：模版测试总是通过，深度失败时不写入
	ComPtr<ID3D11DepthStencilState> DSSNoDepthStencilNotEqual = nullptr;        // 深度/模板状态：模版测试不相等时通过
	ComPtr<ID3D11DepthStencilState> DSSStencilNotEqual = nullptr;				// 深度/模板状态: 默认深度测试，模版测试不相等时通过
};



#endif
