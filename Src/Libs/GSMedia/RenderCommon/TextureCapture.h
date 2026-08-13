#pragma once
#include <d3d11_1.h>
#include <wrl/client.h>
#include <shared_mutex>
#include <vector>
#include "RenderCommonGlobal.h"
#include "SimpleMath.h"

class RENDERCOMMON_API CTextureCaptureColor
{
public:
	CTextureCaptureColor(uint32_t i);
	CTextureCaptureColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	DirectX::XMFLOAT4 ToDXColor();

	uint32_t m_i = 0;
};
bool RENDERCOMMON_API operator==(const CTextureCaptureColor& a, const CTextureCaptureColor& b);
bool RENDERCOMMON_API operator<(const CTextureCaptureColor& a, const CTextureCaptureColor& b);

class RENDERCOMMON_API TextureCapture
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	TextureCapture() = default;
	~TextureCapture() = default;
	// 不允许拷贝，允许移动
	TextureCapture(const TextureCapture&) = delete;
	TextureCapture& operator=(const TextureCapture&) = delete;
	TextureCapture(TextureCapture&&) = default;
	TextureCapture& operator=(TextureCapture&&) = default;

	HRESULT InitResource(ID3D11Device* device,
		int texWidth,
		int texHeight);

	void ResetResource();

	void CaptureTexture(ID3D11DeviceContext * deviceContext, ID3D11Texture2D* pTexture);

	CTextureCaptureColor GetColor(int x, int y);
	bool CopyImageData(std::vector<uint8_t>& outRgba);	// 拷贝已抓取的 RGBA 像素（与 GetColor 通道一致）

private:
	ComPtr<ID3D11Texture2D> m_pTexture = nullptr;

	std::shared_mutex m_mutex;
	std::vector<uint8_t> m_ImageData;
	int m_nWidth = 0;
	int m_nHeight = 0;
};