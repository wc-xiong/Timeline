#pragma once
#include <atlcomcli.h>
#include <d2d1.h>
#include <dwrite.h>
#include "RenderCommonGlobal.h"

RENDERCOMMON_API HRESULT CreateD3DTextureFromWICFile(ID3D11Device* pd3dDevice, const wchar_t* szFileName, ID3D11Texture2D** texture, BOOL bShared = FALSE);
RENDERCOMMON_API HRESULT CreateD3DTextureFromTGAFile(ID3D11Device* pd3dDevice, const wchar_t* szFileName, ID3D11Texture2D** texture, BOOL bShared = FALSE);
RENDERCOMMON_API void CreateD3DTextureFromMemory(ID3D11Device* pd3dDevice, const BYTE *data, UINT width, UINT height, DXGI_FORMAT format, ID3D11Texture2D **texture, BOOL bShared = FALSE);

RENDERCOMMON_API HRESULT CreateD3DTextureFromMemory(ID3D11Device* pd3dDevice, const BYTE *data, UINT width, UINT height, DXGI_FORMAT format, UINT linesize, ID3D11Texture2D **texture, BOOL bShared = FALSE);

RENDERCOMMON_API void CreateD3DTextureForDraw(ID3D11Device* pd3dDevice, UINT width, UINT height, UINT textBit, UINT bindFlag, ID3D11Texture2D **texture);

RENDERCOMMON_API HRESULT CreateD3DTextureShaderView(ID3D11Device* pd3dDevice, const ID3D11Texture2D* texture, const D3D11_SHADER_RESOURCE_VIEW_DESC* desc, ID3D11ShaderResourceView** textureView);
RENDERCOMMON_API HRESULT CreateD3DTextureRenderTargetView(ID3D11Device* pd3dDevice, const ID3D11Texture2D* texture, const D3D11_RENDER_TARGET_VIEW_DESC* desc, ID3D11RenderTargetView** textureView);

RENDERCOMMON_API HRESULT CreateD3DTextureShaderView(ID3D11Device* pd3dDevice, const ID3D11Texture2D* texture, const D3D11_SHADER_RESOURCE_VIEW_DESC* desc, ID3D11ShaderResourceView** textureView);
RENDERCOMMON_API HRESULT CreateD3DTextureRenderTargetView(ID3D11Device* pd3dDevice, const ID3D11Texture2D* texture, const D3D11_RENDER_TARGET_VIEW_DESC* desc, ID3D11RenderTargetView** textureView);
RENDERCOMMON_API HRESULT CreateD2DRenderTargetFromTexture(ID2D1Factory* pd2dFactory, ID3D11Texture2D* texture, ID2D1RenderTarget** textRenderTarget);

RENDERCOMMON_API void SaveD3DTextureToFile(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, ID3D11Texture2D *texture, const wchar_t* szFileName);