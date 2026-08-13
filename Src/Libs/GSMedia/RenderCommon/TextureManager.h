//***************************************************************************************
// GameObject.h by X_Jun(MKXJun) (C) 2018-2022 All Rights Reserved.
// Licensed under the MIT License.
//
// 管理纹理资源
// Simple game object.
//***************************************************************************************

#pragma once

#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H


#include <unordered_map>
#include <unordered_set>
#include <string>
//#include "WinMin.h"
#include <d3d11_1.h>
#include <wrl/client.h>
#include "d3dUtil.h"
#include "RenderCommonGlobal.h"

#include <QString>
#include <shared_mutex>

class RENDERCOMMON_API TextureManager
{
private:
    TextureManager();

public:
    ~TextureManager();
    TextureManager(TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = default;
    TextureManager& operator=(TextureManager&&) = default;

    static TextureManager* Get();
    void Init(ID3D11Device* device);
    void UnInit();
	ID3D11ShaderResourceView* CreateFromFile(QString filePath, bool enableMips = false, bool forceSRGB = false);
    ID3D11ShaderResourceView* CreateFromFile(std::string_view filename, bool enableMips = false, bool forceSRGB = false);
    ID3D11ShaderResourceView* CreateFromMemory(std::string_view name, void* data, size_t byteWidth, bool enableMips = false, bool forceSRGB = false);
    bool AddTexture(std::string_view name, ID3D11ShaderResourceView* texture, bool bVariableTexture = true);
    void RemoveTexture(std::string_view name);
	void ClearTexture(bool bOnlyVariableTexture = true);
    ID3D11ShaderResourceView* GetTexture(std::string_view filename);
    ID3D11ShaderResourceView* GetNullTexture();

private:

    Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pDeviceContext;

    std::shared_mutex m_mtxTex;
    std::unordered_map<XID, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_TextureSRVs;
    std::unordered_set<XID> m_VariableTexture;
};

#endif
