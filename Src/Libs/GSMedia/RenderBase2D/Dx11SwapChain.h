#pragma once
#include "SwapChainImpl.h"
#include "SwapChainObject.h"
#include "GSMedia/RenderDevice/DeviceResource.h"
#include "GSMedia/RenderCommon/MSAAHelper.h"
#include "BasicEffect.h"
#include <memory>

class CDx11SwapChain : public CSwapChainImpl, public CD3DResourceChild
{
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	CDx11SwapChain(UINT nDeviceIndex = 0
		, int options = ESCO_None
		, UINT gpuIndex = 0);
	virtual ~CDx11SwapChain();

	virtual void OnDeviceLost();
	virtual void OnDeviceRestore();

protected:
	HRESULT CreateSwapChain(HWND hWnd) final;
	HRESULT Create2DTarget() final;
	HRESULT UpdateBufffer(int width, int height) final;
	HRESULT EnterExclusiveFullscreen(int dxgiOutputIndex) final;
	HRESULT ExitExclusiveFullscreen() final;
	HRESULT BeginComposite(DWORD abgr) final;
	HRESULT EndComposite(std::function<void()>&& func) final;
	void SetPresentSyncInterval(UINT syncInterval) final;
	UINT GetPresentSyncInterval() const final { return m_presentSyncInterval; }
	HRESULT ReSetRenderRect(int nX, int nY, int nWidth, int nHeight) final;
	virtual void	lock() final;
	virtual void	unlock() final;
	void	SetHDR(BOOL bEnable) final;
	void	SetMSAA(BOOL bEnable) final;

	BOOL				IsEnableHDR()  const override { return (m_bDisplayHDR10 && HasSwapChainOption(m_options, ESwapChainOption::ESCO_EnableHDR)); }
	BOOL				IsEnableMSAA() const override { return HasSwapChainOption(m_options, ESwapChainOption::ESCO_EnableMSAA); }
	ID3D11Texture2D*	GetRenderTarget() const override { return m_renderTarget.Get(); };
	ID2D1RenderTarget*	GetD2DTarget() const override { return m_pd2dRenderTarget.Get(); }

	ID3D11Device*		GetD3DDevice() const override { return m_pD3DDevice.Get(); }
	IDWriteFactory*		GetDWriteFactory() const override { return m_pD2DWriteFactory.Get(); }
	BasicEffect*		GetBasicEffect() const override { return m_pBasicEffect; }

	DXGI_OUTPUT_DESC1	GetOutputDesc() override { return m_outputDesc; }
	rawOutputDesc		GetRawOutputDesc() override { return m_rawOutDesc; }
	DXGI_FORMAT			GetBackBufferFormat() override { return m_backBufferFormat; }

	HRESULT SetHDRMetaData(DXGI_HDR_METADATA_TYPE Type, UINT Size, void *pMetaData) override;

public:
	CD3D11_VIEWPORT					m_OutputViewPort = {};			// 输出所用的视口
	UINT							m_bufferWidth = 1;
	UINT							m_bufferHeight = 1;
	DXGI_FORMAT						m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;				// 颜色格式
	HWND							m_hSwapWnd = nullptr;

protected:
	ComPtr<IDXGISwapChain2>			m_pSwapChain = nullptr;			// 交换链
	ComPtr<ID3D11Texture2D>			m_renderTarget = nullptr;		// 渲染目标纹理
	ComPtr<ID3D11RenderTargetView>	m_pRenderTargetView = nullptr;	// 渲染目标

	ComPtr<ID2D1RenderTarget>		m_pd2dRenderTarget = nullptr;	// D2D绘制目标

	BasicEffect*					m_pBasicEffect = nullptr;

private:
	void	CheckHDRSupport();
	void	CheckOption();
	void	UpdateColorSpace();
	void	WaitOnSwapChain();

	UINT				BufferCountFromOptions() const;
	DXGI_SWAP_EFFECT	SwapEffectFromOptions() const;
	UINT				DxgiSwapChainFlagsFromOptions() const;

	bool	InitGroupBarries();
	void	FreeGroupBarries();

	int				m_options = ESwapChainOption::ESCO_None;	// 设备选项
	UINT							m_presentSyncInterval = 0;	// DXGI/NvAPI Present sync interval (1 = VSync)
	BOOL							m_bDisplayHDR10 = FALSE;
	std::unique_ptr<DX::MSAAHelper>	m_msaaHelper;					// 多重采样

	HANDLE							m_frameLatencyWaitableObject = nullptr;

	DXGI_OUTPUT_DESC1 m_outputDesc;
	rawOutputDesc m_rawOutDesc;

	NvU32	m_GroupId = 1;
	NvU32	m_BarrierId = 1;
};
