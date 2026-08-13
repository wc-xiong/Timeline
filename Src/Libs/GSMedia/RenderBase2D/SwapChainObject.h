#pragma once
#include <memory>
#include <type_traits>
#include "SwapChainImpl.h"
#include "RenderBase2DExport.h"

enum ESwapChainOption : unsigned int
{
	ESCO_None          = 0,
	ESCO_FlipPresent   = 0x01,
	ESCO_EnableHDR     = 0x02,
	ESCO_FrameLatency  = 0x04,
	ESCO_FullScreen    = 0x08,
	ESCO_EnableMSAA    = 0x10,
	ESCO_EnableDiscard = 0x20,
	ESCO_TryNvSync     = 0x40,
	ESCO_TryNvMaster   = 0x80,


	// 对齐旧构造默认 bFilp=TRUE、bLowLatency=TRUE
	ESCO_DefaultSwapChainOptions = ESCO_FlipPresent | ESCO_FrameLatency,
};

inline  bool HasSwapChainOption(int options, int flag)
{
	return (options & flag) != ESCO_None;
}

class RENDERBASE2D_API CSwapChainObject
{
public:
	CSwapChainObject(UINT nDeviceIndex
		, UINT gpuIndex = 0
		, int options = ESCO_DefaultSwapChainOptions);
	virtual ~CSwapChainObject();

	virtual void Initial(HWND hWnd) = 0;
	virtual void OnSizeChange(int nWidth, int nHeight) = 0;
	virtual HRESULT Composite(const std::chrono::microseconds& ts, UINT index) = 0;
	virtual void SetWaterMark(std::wstring str) = 0;
	virtual bool IsInit() { return m_bInit; }

	virtual DXGI_OUTPUT_DESC1	GetOutputDesc() { return m_pSwapChainImpl->GetOutputDesc(); }
	virtual rawOutputDesc		GetRawOutputDesc() { return m_pSwapChainImpl->GetRawOutputDesc(); }
	virtual DXGI_FORMAT			GetBackBufferFormat() { return m_pSwapChainImpl->GetBackBufferFormat(); }

	virtual HRESULT SetHDRMetaData(DXGI_HDR_METADATA_TYPE Type, UINT Size, void *pMetaData)
	{
		return m_pSwapChainImpl->SetHDRMetaData(Type, Size, pMetaData);
	}

protected:
	std::unique_ptr<CSwapChainImpl> m_pSwapChainImpl;
	HWND m_hWnd = nullptr;
	int m_nWidth = 1;
	int m_nHeight = 1;
	GUID m_guid = GUID_NULL;

	bool m_bInit = false;
	BasicEffect*					m_pBasicEffect = nullptr;
};
