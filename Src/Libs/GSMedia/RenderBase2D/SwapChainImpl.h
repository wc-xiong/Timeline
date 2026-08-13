#pragma once
#include <dxgi1_6.h>
#include <functional>
#include "BasicEffect.h"

struct rawOutputDesc
{
	float MaxLuminance = 0.f;
	float MaxFullFrameLuminance = 0.f;
	float MinLuminance = 0.f;
};

class CSwapChainImpl
{
public:
	virtual ~CSwapChainImpl() {};

	virtual HRESULT CreateSwapChain(HWND hWnd) = 0;
	virtual HRESULT Create2DTarget() = 0;
	virtual HRESULT UpdateBufffer(int width, int height) = 0;
	// 屏端用 IScreenItem::GetDXGIindex() 指定输出；默认不支持
	virtual HRESULT EnterExclusiveFullscreen(int dxgiOutputIndex)
	{
		(void)dxgiOutputIndex;
		return E_NOTIMPL;
	}
	virtual HRESULT ExitExclusiveFullscreen()
	{
		return S_FALSE;
	}
	virtual HRESULT BeginComposite(DWORD abgr) = 0;
	virtual HRESULT EndComposite(std::function<void()>&& func) = 0;			//func: present 前回调函数，可以用于绘制水印或其他
	virtual void SetPresentSyncInterval(UINT syncInterval) { (void)syncInterval; }
	virtual UINT GetPresentSyncInterval() const { return 0; }
	virtual HRESULT ReSetRenderRect(int nX, int nY, int nWidth, int nHeight) = 0;
	//virtual void	lock() = 0;
	//virtual void	unlock() = 0;

	virtual void				SetHDR(BOOL bEnable) = 0;
	virtual BOOL				IsEnableHDR() const = 0;
	virtual void				SetMSAA(BOOL bEnable) = 0;
	virtual BOOL				IsEnableMSAA() const = 0;
	virtual ID3D11Texture2D*	GetRenderTarget() const = 0;
	virtual ID2D1RenderTarget*	GetD2DTarget() const = 0;
	virtual ID3D11Device*		GetD3DDevice() const = 0;
	virtual IDWriteFactory*	GetDWriteFactory() const = 0;
	virtual BasicEffect*		GetBasicEffect() const = 0;

	virtual DXGI_OUTPUT_DESC1	GetOutputDesc() = 0;
	virtual rawOutputDesc		GetRawOutputDesc() = 0;
	virtual DXGI_FORMAT			GetBackBufferFormat() = 0;

	virtual HRESULT SetHDRMetaData(DXGI_HDR_METADATA_TYPE Type, UINT Size, void *pMetaData) = 0;

protected:
	CSwapChainImpl() {};
};


