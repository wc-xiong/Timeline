#pragma once

#include <vector>
#include <chrono>
using namespace std::chrono;

#include <d3d11.h>
#include <dxgi.h>

#include <wrl/client.h>
template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

#include "CommonDefine.h"
#include "MediaCommon.h"
#include "TexFrameCommon.h"
#include "Maths/Matrix3x2.h"

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <string>

#include <rapidjson/document.h>


typedef enum _EM_COLOR_SPACE
{
	CS_601 = 0,	//	601
	CS_709,		//	709
	CS_2020,	//	2020
}EM_COLOR_SAPCE;

typedef enum _EM_COLOR_RANGE
{
	CR_Limit = 0,	//	limit
	CR_Full,		//	full
}EM_COLOR_RANGE;

typedef enum _EM_COLOR_GAMMA
{
	CG_SDR = 0,		//	SDR
	CG_HDR10,		//	HDR10
	CG_HLG,			//	HLG
	CG_HDR10_PLUS,	//	HDR10+
}EM_COLOR_GAMMA;

enum ETextureType	//需要和着色器对应
{
	ETT_Invalid = -1,
	ETT_YUV,
	ETT_YV12,
	ETT_NV12,
	ETT_P010 = ETT_NV12,
	ETT_BC3,
	ETT_BC3BC4,
	ETT_YUYV,
	ETT_RGBA,	// 非YUV格式纹理
	ETT_BGRA,
	ETT_RGB0,
	ETT_BGR0,
	ETT_ARGB,
	ETT_ABGR,
	ETT_0RGB,
	ETT_0BGR,
};

struct Video_ClrMetedata
{
	int		color_bit;				// 颜色bit
	int		color_space;			// 颜色空间
	int		color_range;			// 色彩范围	
	int     color_gamma;			// EOTF
	int		color_type;				// 颜色类型

	Video_ClrMetedata()
	{
		color_bit = 8;				// 默认为8bit
		color_space = CS_709;		// 默认为709色域		 // pixel shader中的定义
		color_range = CR_Full;		// 默认为FULL RANGE   // pixel shader中的定义
		color_gamma = CG_SDR;		// 默认为SDR
		color_type = ETT_Invalid;	// 默认为无效格式,着色器按rgba读取
	}

	Video_ClrMetedata(int nColorType)
	{
		color_bit = 8;
		color_space = CS_709;
		color_range = CR_Full;
		color_gamma = CG_SDR;
		color_type = nColorType;
	}

	Video_ClrMetedata& operator=(const Video_ClrMetedata& other)
	{
		//memcpy(this, &other, sizeof(Video_ClrMetedata));
		color_bit = other.color_bit;
		color_space = other.color_space;
		color_range = other.color_range;
		color_gamma = other.color_gamma;
		color_type = other.color_type;
		return *this;
	}
};

struct STexNode
{
	ComPtr<ID3D11Texture2D>	pTex = nullptr;
	HANDLE texHandle = nullptr;
	D3D11_TEXTURE2D_DESC texDesc;

	STexNode& operator=(const STexNode& other)
	{
		Reset();
		if (other.pTex)
			other.pTex.CopyTo(pTex.ReleaseAndGetAddressOf());
		texHandle = other.texHandle;
		texDesc = other.texDesc;

		return *this;
	}

	void Reset()
	{
		pTex.Reset();
		texHandle = nullptr;
	}

	bool operator==(const STexNode& other)
	{
		return (pTex == other.pTex);
	}

	bool UpdateTexPty()
	{
		if (!pTex)
			return false;
		ZeroMemory(&texDesc, sizeof(texDesc));
		pTex->GetDesc(&texDesc);

		texHandle = nullptr;
		ComPtr<IDXGIResource> dxgi_resource;
		pTex->QueryInterface(__uuidof(IDXGIResource), reinterpret_cast<void**>(dxgi_resource.GetAddressOf()));
		auto hr = dxgi_resource->GetSharedHandle(&texHandle);
		if (!SUCCEEDED(hr))
		{
			assert(0);
			return false;
		}
		dxgi_resource.Reset();

		return true;
	}
};

// 纹理数组(硬解码一般是一组，软解根据类型有不同组)
struct STexDatas
{
	// 三层纹理(全局device创建，解码线程更新)

	STexNode texNode0;
	STexNode texNode1;
	STexNode texNode2;

	STexDatas()
	{
	}

	STexDatas(const STexDatas& data)
	{
		*this = data;
	}

	~STexDatas()
	{
		
		Reset();
	}

	STexDatas& operator=(const STexDatas& other)
	{
		texNode0 = other.texNode0;
		texNode1 = other.texNode1;
		texNode2 = other.texNode2;

		return *this;
	}

	void Reset()
	{
		texNode0.Reset();
		texNode1.Reset();
		texNode2.Reset();
	}

	bool operator==(const STexDatas& other)
	{
		return ((texNode0 == other.texNode0) && (texNode1 == other.texNode1) && (texNode2 == other.texNode2));
	}
};
typedef std::vector<STexDatas> TEXTURES;	// 纹理数组

// 每帧数据对应的纹理
struct STextureFrame
{
	// 当前帧时间 pts
	microseconds m_pts = -1us;
	int			 m_serial = 0;
	double		 m_duration = 0.0f;
	bool		 m_bScrollState = false;	//纹理滚动状态


	// 纹理类型
	EVideoTextType textType = EVideoTextType::No;
	Video_ClrMetedata	m_TexColorPty;		// 纹理格式属性

	// 纹理数组
	TEXTURES m_texs;

	bool m_bFrameChange = false;
	int nFrameIndex = -1;

	STextureFrame()
	{

	}

	void Reset()
	{
		m_pts = -1us;
		textType = EVideoTextType::No;
		m_bScrollState = false;
		while (!m_texs.empty())
		{
			m_texs.back().Reset();
			m_texs.pop_back();
		}
		m_texs.shrink_to_fit();
		m_bFrameChange = false;
	}

	void Resave()
	{
		m_pts = -1us;
		m_bFrameChange = false;
		m_bScrollState = false;
	}

	STextureFrame& operator=(const STextureFrame& data)
	{
		m_pts = data.m_pts;
		m_duration = data.m_duration;

		//while (!m_texs.empty())
		//{
		//	m_texs.back().Reset();
		//	m_texs.pop_back();
		//}
		//m_texs.shrink_to_fit();
		//m_texs.clear();
		//m_texs = data.m_texs;
		m_texs.assign(data.m_texs.begin(), data.m_texs.end());
		textType = data.textType;
		m_TexColorPty = data.m_TexColorPty;
		m_bFrameChange = data.m_bFrameChange;
		m_bScrollState = data.m_bScrollState;
		nFrameIndex = data.nFrameIndex;

		return *this;
	}

	~STextureFrame()
	{
		Reset();
	}
};

struct SDrawTexture
{
	STransInfo transFxData;
	STextureFrame textureFrame;

	SDrawTexture()
	{
		transFxData = STransInfo();
		textureFrame = STextureFrame();
	}

	SDrawTexture(STextureFrame text)
	{
		transFxData = STransInfo();
		textureFrame = text;
	}

	SDrawTexture(STextureFrame text, STransInfo fxData)
	{
		textureFrame = text;
		transFxData = fxData;
	}

	~SDrawTexture()
	{
		Reset();
	}

	void Reset()
	{
		transFxData.Reset();
		textureFrame.Reset();
	}
};

struct SDrawTextureData
{
	std::vector<SDrawTexture> vecTextureFrame;
	bool bReadyFinish = true;

	SDrawTextureData()
	{
		Reset();
	}

	SDrawTextureData(const SDrawTextureData& other)
	{
		Reset();
		//vecTextureFrame = other.vecTextureFrame;
		vecTextureFrame.assign(other.vecTextureFrame.begin(), other.vecTextureFrame.end());
		bReadyFinish = other.bReadyFinish;
	}

	~SDrawTextureData()
	{
		Reset();
	}

	SDrawTextureData(std::vector<SDrawTexture> vec)
	{
		//vecTextureFrame = vec;
		vecTextureFrame.assign(vec.begin(), vec.end());
	}

	void Reset()
	{
		vecTextureFrame.clear();
		vecTextureFrame.shrink_to_fit();
	}
};

/** @brief 表示 RenderItem 渲染的一个纹理分段 */
struct SRenderItemSegmentTex
{
	HANDLE texHandle = nullptr; //纹理句柄
	D2D1_RECT_D rcTex;	// 纹理位置

	void SaveDataToRapidJson(rapidjson::Value& obj, rapidjson::Document::AllocatorType& allocator) const
	{
		obj.SetObject();
		const std::string handleStr = std::to_string(reinterpret_cast<uintptr_t>(texHandle));
		obj.AddMember(
			rapidjson::StringRef("texHandle"),
			rapidjson::Value(handleStr.c_str(), static_cast<rapidjson::SizeType>(handleStr.length()), allocator).Move(),
			allocator);
		obj.AddMember(rapidjson::StringRef("left"), rcTex.left, allocator);
		obj.AddMember(rapidjson::StringRef("top"), rcTex.top, allocator);
		obj.AddMember(rapidjson::StringRef("right"), rcTex.right, allocator);
		obj.AddMember(rapidjson::StringRef("bottom"), rcTex.bottom, allocator);
	}

	bool LoadDataFromRapidJson(const rapidjson::Value& obj)
	{
		if (!obj.IsObject())
			return false;

		if (obj.HasMember("texHandle"))
		{
			const rapidjson::Value& h = obj["texHandle"];
			if (h.IsString())
			{
				texHandle = reinterpret_cast<HANDLE>(static_cast<uintptr_t>(
					std::strtoull(h.GetString(), nullptr, 10)));
			}
		}

		rcTex.InitRect();

		if (obj.HasMember("left"))
			rcTex.left = obj["left"].GetDouble();
		if (obj.HasMember("top"))
			rcTex.top = obj["top"].GetDouble();
		if (obj.HasMember("right"))
			rcTex.right = obj["right"].GetDouble();
		if (obj.HasMember("bottom"))
			rcTex.bottom = obj["bottom"].GetDouble();

		return true;
	}
};

/** @brief 表示 RenderItem 渲染的纹理集合 */
struct SRenderItemTex
{
	std::vector<SRenderItemSegmentTex> texs;		//纹理分段集合
	float uvWidth = 0.f;							//整体uv宽度
	float uvHeight = 0.f;							//整体uv高度

	void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) const
	{
		root.SetObject();
		root.AddMember(rapidjson::StringRef("uvWidth"), static_cast<double>(uvWidth), allocator);
		root.AddMember(rapidjson::StringRef("uvHeight"), static_cast<double>(uvHeight), allocator);
		rapidjson::Value arr(rapidjson::kArrayType);
		for (const SRenderItemSegmentTex& seg : texs)
		{
			rapidjson::Value segObj(rapidjson::kObjectType);
			seg.SaveDataToRapidJson(segObj, allocator);
			arr.PushBack(segObj, allocator);
		}
		root.AddMember(rapidjson::StringRef("texs"), arr, allocator);
	}

	bool LoadDataFromRapidJson(const rapidjson::Value& obj)
	{
		if (!obj.IsObject())
			return false;

		if (obj.HasMember("uvWidth") && obj["uvWidth"].IsNumber())
			uvWidth = static_cast<float>(obj["uvWidth"].GetDouble());
		if (obj.HasMember("uvHeight") && obj["uvHeight"].IsNumber())
			uvHeight = static_cast<float>(obj["uvHeight"].GetDouble());

		texs.clear();
		if (!obj.HasMember("texs") || !obj["texs"].IsArray())
			return true;

		const rapidjson::Value& arr = obj["texs"];
		texs.reserve(static_cast<size_t>(arr.Size()));
		for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
		{
			SRenderItemSegmentTex seg;
			if (!seg.LoadDataFromRapidJson(arr[i]))
				return false;
			texs.push_back(seg);
		}
		return true;
	}
};
