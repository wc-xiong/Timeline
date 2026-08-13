#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "TextureManager.h"
#include "d3dUtil.h"
#include "DXTrace.h"
#include "CommonFunction.h"
//#include "ImGuiLog.h"
#include "DDSTextureLoader.h"
#include <filesystem>

using namespace Microsoft::WRL;

namespace
{
    // TextureManager单例
    TextureManager* s_pInstance = nullptr;
}

TextureManager::TextureManager()
{
}

TextureManager::~TextureManager()
{
}

TextureManager* TextureManager::Get()
{
	static TextureManager s_Instance;
    return &s_Instance;
}

void TextureManager::Init(ID3D11Device* device)
{
    m_pDevice = device;
    m_pDevice->GetImmediateContext(m_pDeviceContext.ReleaseAndGetAddressOf());

    // 加一张1x1的空纹理
    ComPtr<ID3D11Texture2D> pTex;
    CD3D11_TEXTURE2D_DESC nullTexDesc(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1);
    D3D11_SUBRESOURCE_DATA initData{};
    uint32_t white = (uint32_t)-1;
    initData.pSysMem = &white;
    initData.SysMemPitch = 4;
    m_pDevice->CreateTexture2D(&nullTexDesc, &initData, pTex.GetAddressOf());

    ComPtr<ID3D11ShaderResourceView> pSRV;
    m_pDevice->CreateShaderResourceView(pTex.Get(), nullptr, pSRV.GetAddressOf());

    {
        std::unique_lock<std::shared_mutex> Locker(m_mtxTex);
        m_TextureSRVs.try_emplace(0, pSRV.Get()).second;
    }
}

void TextureManager::UnInit()
{
    std::unique_lock<std::shared_mutex> locker(m_mtxTex);
    m_TextureSRVs.clear();
    m_VariableTexture.clear();
    m_pDeviceContext.Reset();
    m_pDevice.Reset();
}

ID3D11ShaderResourceView * TextureManager::CreateFromFile(QString filePath, bool enableMips, bool forceSRGB)
{
	// 写锁：会插入/更新 SRV，并可能从 m_VariableTexture 剔除（文件贴图不应被 ClearTexture 清掉）
	std::unique_lock<std::shared_mutex> Locker(m_mtxTex);

	const QByteArray pathLocal = filePath.toLocal8Bit();
	const std::string filename(pathLocal.constData(), pathLocal.size());
	XID fileID = StringToID(filename);

	auto itExisting = m_TextureSRVs.find(fileID);
	if (itExisting != m_TextureSRVs.end())
	{
		if (itExisting->second)
		{
			// RemoveTexture 会把 ID 记入 VariableTexture；文件贴图重建后必须剔除，否则每帧 ClearTexture 会清掉
			m_VariableTexture.erase(fileID);
			return itExisting->second.Get();
		}
		// 空缓存条目：允许重试加载
		m_TextureSRVs.erase(itExisting);
	}

	auto& res = m_TextureSRVs[fileID];
	std::wstring wstr = filePath.toStdWString();

	ComPtr<ID3D11Texture2D> pTex;
	if (SUCCEEDED(CreateD3DTextureFromWICFile(m_pDevice.Get(), wstr.c_str(), pTex.ReleaseAndGetAddressOf(), true)))
	{
		// 创建SRV
		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D,
			forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM);
		HR(m_pDevice->CreateShaderResourceView(pTex.Get(), NULL, res.ReleaseAndGetAddressOf()));
		// 生成mipmap
		if (enableMips)
			m_pDeviceContext->GenerateMips(res.Get());
		D3D11SetDebugObjectName(res.Get(), std::filesystem::path(filename).filename().string());
	}
	else if (FAILED(DirectX::CreateDDSTextureFromFileEx(m_pDevice.Get(),
		enableMips ? m_pDeviceContext.Get() : nullptr,
		wstr.c_str(), 0, D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE, 0, 0,
		forceSRGB, nullptr, res.ReleaseAndGetAddressOf())))
	{
		int width, height, comp;

		stbi_uc* img_data = stbi_load(filename.c_str(), &width, &height, &comp, STBI_rgb_alpha);
		if (img_data)
		{
			CD3D11_TEXTURE2D_DESC texDesc(forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
				width, height, 1,
				enableMips ? 0 : 1,
				D3D11_BIND_SHADER_RESOURCE | (enableMips ? D3D11_BIND_RENDER_TARGET : 0),
				D3D11_USAGE_DEFAULT, 0, 1, 0,
				enableMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
			Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
			HR(m_pDevice->CreateTexture2D(&texDesc, nullptr, tex.GetAddressOf()));
			// 上传纹理数据
			m_pDeviceContext->UpdateSubresource(tex.Get(), 0, nullptr, img_data, width * sizeof(uint32_t), 0);
			CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D,
				forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM);
			// 创建SRV
			HR(m_pDevice->CreateShaderResourceView(tex.Get(), &srvDesc, res.ReleaseAndGetAddressOf()));
			// 生成mipmap
			if (enableMips)
				m_pDeviceContext->GenerateMips(res.Get());
			stbi_image_free(img_data);
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
			D3D11SetDebugObjectName(res.Get(), std::filesystem::path(filename).filename().string());
#endif
		}
		else
		{
			std::string warning = "[Warning]: TextureManager::CreateFromFile, couldn't find \"";
			warning += filename;
			warning += "\"\n";

			/*if (ImGuiLog::HasInstance())
			{
				ImGuiLog::Get().AddLog(warning.c_str());
			}
			else*/
			{
				OutputDebugStringA(warning.c_str());
			}

		}
	}

	ID3D11ShaderResourceView* pResult = res.Get();
	if (pResult)
		m_VariableTexture.erase(fileID);
	else
		m_TextureSRVs.erase(fileID);

	return pResult;
}

ID3D11ShaderResourceView* TextureManager::CreateFromFile(std::string_view filename, bool enableMips, bool forceSRGB)
{
	std::unique_lock<std::shared_mutex> Locker(m_mtxTex);

	XID fileID = StringToID(filename);
	auto itExisting = m_TextureSRVs.find(fileID);
	if (itExisting != m_TextureSRVs.end())
	{
		if (itExisting->second)
		{
			m_VariableTexture.erase(fileID);
			return itExisting->second.Get();
		}
		m_TextureSRVs.erase(itExisting);
	}

	auto& res = m_TextureSRVs[fileID];
	std::wstring wstr = LocalToWString(filename);

	ComPtr<ID3D11Texture2D> pTex;
	if (SUCCEEDED(CreateD3DTextureFromWICFile(m_pDevice.Get(), wstr.c_str(), pTex.ReleaseAndGetAddressOf(), true)))
	{
		// 创建SRV
		CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D,
			forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM);
		HR(m_pDevice->CreateShaderResourceView(pTex.Get(), NULL, res.ReleaseAndGetAddressOf()));
		// 生成mipmap
		if (enableMips)
			m_pDeviceContext->GenerateMips(res.Get());
		D3D11SetDebugObjectName(res.Get(), std::filesystem::path(filename).filename().string());
	}
	else if (FAILED(DirectX::CreateDDSTextureFromFileEx(m_pDevice.Get(),
		enableMips ? m_pDeviceContext.Get() : nullptr,
		wstr.c_str(), 0, D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE, 0, 0,
		forceSRGB, nullptr, res.ReleaseAndGetAddressOf())))
	{
		int width, height, comp;

		// string_view 未必以 '\0' 结尾，拷贝后再交给 stbi
		const std::string filenameOwned(filename);
		stbi_uc* img_data = stbi_load(filenameOwned.c_str(), &width, &height, &comp, STBI_rgb_alpha);
		if (img_data)
		{
			CD3D11_TEXTURE2D_DESC texDesc(forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
				width, height, 1,
				enableMips ? 0 : 1,
				D3D11_BIND_SHADER_RESOURCE | (enableMips ? D3D11_BIND_RENDER_TARGET : 0),
				D3D11_USAGE_DEFAULT, 0, 1, 0,
				enableMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
			Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
			HR(m_pDevice->CreateTexture2D(&texDesc, nullptr, tex.GetAddressOf()));
			// 上传纹理数据
			m_pDeviceContext->UpdateSubresource(tex.Get(), 0, nullptr, img_data, width * sizeof(uint32_t), 0);
			CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D,
				forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM);
			// 创建SRV
			HR(m_pDevice->CreateShaderResourceView(tex.Get(), &srvDesc, res.ReleaseAndGetAddressOf()));
			// 生成mipmap
			if (enableMips)
				m_pDeviceContext->GenerateMips(res.Get());
			stbi_image_free(img_data);
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
			D3D11SetDebugObjectName(res.Get(), std::filesystem::path(filename).filename().string());
#endif
		}
		else
		{
			std::string warning = "[Warning]: TextureManager::CreateFromFile, couldn't find \"";
			warning += filenameOwned;
			warning += "\"\n";

			/*if (ImGuiLog::HasInstance())
			{
				ImGuiLog::Get().AddLog(warning.c_str());
			}
			else*/
			{
				OutputDebugStringA(warning.c_str());
			}

		}
	}

	ID3D11ShaderResourceView* pResult = res.Get();
	if (pResult)
		m_VariableTexture.erase(fileID);
	else
		m_TextureSRVs.erase(fileID);

	return pResult;
}

ID3D11ShaderResourceView* TextureManager::CreateFromMemory(std::string_view name, void* data, size_t byteWidth, bool enableMips, bool forceSRGB)
{
    std::shared_lock<std::shared_mutex> Locker(m_mtxTex);

    XID fileID = StringToID(name);
    if (m_TextureSRVs.count(fileID))
        return m_TextureSRVs[fileID].Get();

    auto& res = m_TextureSRVs[fileID];
    int width, height, comp;
    stbi_uc* img_data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(data), (int)byteWidth, &width, &height, &comp, STBI_rgb_alpha);
    if (img_data)
    {
        CD3D11_TEXTURE2D_DESC texDesc(forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
            width, height, 1,
            enableMips ? 0 : 1,
            D3D11_BIND_SHADER_RESOURCE | (enableMips ? D3D11_BIND_RENDER_TARGET : 0),
            D3D11_USAGE_DEFAULT, 0, 1, 0,
            enableMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
        Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
        HR(m_pDevice->CreateTexture2D(&texDesc, nullptr, tex.GetAddressOf()));
        // 上传纹理数据
        m_pDeviceContext->UpdateSubresource(tex.Get(), 0, nullptr, img_data, width * sizeof(uint32_t), 0);
        CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D,
            forceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM);
        // 创建SRV
        HR(m_pDevice->CreateShaderResourceView(tex.Get(), &srvDesc, res.ReleaseAndGetAddressOf()));
        // 生成mipmap
        if (enableMips)
            m_pDeviceContext->GenerateMips(res.Get());
        stbi_image_free(img_data);
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
        // 避免 std::filesystem::path(name) 在本地代码页无法映射字符时抛 system_error(1113)。
        std::string debugName(name);
        const size_t sepPos = debugName.find_last_of("/\\");
        if (sepPos != std::string::npos && sepPos + 1 < debugName.size())
        {
            debugName = debugName.substr(sepPos + 1);
        }
        D3D11SetDebugObjectName(res.Get(), debugName);
#endif
    }
    else
    {
        std::string warning = "[Warning]: TextureManager::CreateFromMemory, failed to create texture \"";
        warning += name;
        warning += "\"\n";
        OutputDebugStringA(warning.c_str());
    }
    return res.Get();
}

bool TextureManager::AddTexture(std::string_view name, ID3D11ShaderResourceView* texture, bool bVariableTexture)
{
    std::unique_lock<std::shared_mutex> Locker(m_mtxTex);

    XID nameID = StringToID(name);

    if (bVariableTexture)
        m_VariableTexture.insert(nameID);

    return m_TextureSRVs.try_emplace(nameID, texture).second;
}

void TextureManager::RemoveTexture(std::string_view name)
{
    std::unique_lock<std::shared_mutex> Locker(m_mtxTex);

    XID nameID = StringToID(name);
    m_TextureSRVs.erase(nameID);
    m_VariableTexture.insert(nameID);
}

void TextureManager::ClearTexture(bool bOnlyVariableTexture)
{
    std::unique_lock<std::shared_mutex> Locker(m_mtxTex);

    if (!bOnlyVariableTexture)
    {
        m_TextureSRVs.clear();
    }
    else
    {
        std::for_each(m_VariableTexture.begin(), m_VariableTexture.end(), [&](const auto& it) {
            m_TextureSRVs.erase(it);
        });
    }

    m_VariableTexture.clear();
}

ID3D11ShaderResourceView* TextureManager::GetTexture(std::string_view filename)
{
    std::shared_lock<std::shared_mutex> Locker(m_mtxTex);

    XID fileID = StringToID(filename);
    if (m_TextureSRVs.count(fileID))
        return m_TextureSRVs[fileID].Get();
    return nullptr;
}

ID3D11ShaderResourceView* TextureManager::GetNullTexture()
{
    std::shared_lock<std::shared_mutex> Locker(m_mtxTex);
    return m_TextureSRVs[0].Get();
}
