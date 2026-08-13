#include "pch.h"
#include "Dx11SwapChain.h"
#include <winrt\Windows.Devices.Display.h>
#include <winrt\Windows.Devices.Enumeration.h>
#include <QDebug>

#pragma comment(lib, "windowsapp")

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

namespace
{
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	// 调用方已用 IScreenItem::GetDXGIindex() 指定本 Device Adapter 上的输出序号
	HRESULT FindOutputByIndex(ID3D11Device* pDevice, UINT outputIndex, IDXGIOutput** ppOutput)
	{
		if (!pDevice || !ppOutput)
			return E_INVALIDARG;

		*ppOutput = nullptr;

		ComPtr<IDXGIDevice> dxgiDevice;
		HRESULT hr = pDevice->QueryInterface(IID_PPV_ARGS(dxgiDevice.GetAddressOf()));
		if (FAILED(hr) || !dxgiDevice)
			return hr;

		ComPtr<IDXGIAdapter> adapter;
		hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
		if (FAILED(hr) || !adapter)
			return hr;

		return adapter->EnumOutputs(outputIndex, ppOutput);
	}
}
CDx11SwapChain::CDx11SwapChain(UINT nDeviceIndex, int options, UINT gpuIndex)
{
	// 多卡出屏：必须与 HWND 所属 Adapter / owner RD 的 gpuIndex 一致，禁止默认落主卡
	InitD3DResource(nDeviceIndex, gpuIndex);
	m_options = options;

	m_pBasicEffect = new BasicEffect;
	m_pBasicEffect->InitAll(m_pD3DDevice.Get());
}

CDx11SwapChain::~CDx11SwapChain()
{
	EnterCriticalSection(m_pD3DCSContext);

	FreeGroupBarries();

	if (m_pSwapChain && HasSwapChainOption(m_options, ESwapChainOption::ESCO_FullScreen))
		m_pSwapChain->SetFullscreenState(FALSE, nullptr);

	m_pRenderTargetView.Reset();
	m_renderTarget.Reset();
	m_pd2dRenderTarget.Reset();
	m_pSwapChain.Reset();

	delete m_pBasicEffect;
	m_pBasicEffect = nullptr;

	LeaveCriticalSection(m_pD3DCSContext);
}

void CDx11SwapChain::OnDeviceLost()
{
	FreeGroupBarries();

	// 不用进入临界区 ：EnterCriticalSection
	m_pRenderTargetView.Reset();
	m_renderTarget.Reset();
	m_pd2dRenderTarget.Reset();
	m_pSwapChain.Reset();

}

void CDx11SwapChain::OnDeviceRestore()
{
	// 重新获取新的device
	CreateSwapChain(m_hSwapWnd);
	UpdateBufffer(m_bufferWidth, m_bufferHeight);
}

UINT CDx11SwapChain::BufferCountFromOptions() const
{
	if (HasSwapChainOption(m_options, ESwapChainOption::ESCO_FlipPresent))
	{
		// FRAME_LATENCY_WAITABLE：BufferCount >= MaxFrameLatency + 1
		if (HasSwapChainOption(m_options, ESwapChainOption::ESCO_FrameLatency))
		{
			return 3;
		}
		return 2;
	}
	return 1;
}

DXGI_SWAP_EFFECT CDx11SwapChain::SwapEffectFromOptions() const
{
	if (HasSwapChainOption(m_options, ESwapChainOption::ESCO_FlipPresent))
	{
		if (HasSwapChainOption(m_options, ESwapChainOption::ESCO_EnableDiscard))
			return DXGI_SWAP_EFFECT_FLIP_DISCARD;
		else
			return DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	}
	else
	{
		if (HasSwapChainOption(m_options, ESwapChainOption::ESCO_EnableDiscard))
			return DXGI_SWAP_EFFECT_DISCARD;
		else
			return DXGI_SWAP_EFFECT_SEQUENTIAL;
	}
}

UINT CDx11SwapChain::DxgiSwapChainFlagsFromOptions() const
{
	UINT flag = 0u;
	if (HasSwapChainOption(m_options, ESwapChainOption::ESCO_FrameLatency))
		flag |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
	if (HasSwapChainOption(m_options, ESwapChainOption::ESCO_FullScreen))
		flag |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	return flag;
}

HRESULT CDx11SwapChain::CreateSwapChain(HWND hWnd)
{
	m_hSwapWnd = hWnd;
	CheckOption();

	// Create a descriptor for the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = 1;
	swapChainDesc.Height = 1;
	swapChainDesc.Format = m_backBufferFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = BufferCountFromOptions();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SwapEffect = SwapEffectFromOptions();
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	// 独占全屏用 STRETCH；窗口 + NONE 在尺寸不完全匹配时会强制 DWM blit
	if (HasSwapChainOption(m_options, ESwapChainOption::ESCO_FlipPresent)
		&& !HasSwapChainOption(m_options, ESwapChainOption::ESCO_FullScreen))
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
	else
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;

	swapChainDesc.Flags = DxgiSwapChainFlagsFromOptions();

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
	fsSwapChainDesc.Windowed = TRUE;
	fsSwapChainDesc.RefreshRate.Numerator = (UINT)0;
	fsSwapChainDesc.RefreshRate.Denominator = 1;
	fsSwapChainDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	fsSwapChainDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	ComPtr<IDXGISwapChain1> pSwapChain1 = nullptr;
	HRESULT hr = m_pDXGIFactory->CreateSwapChainForHwnd(
		m_pD3DDevice.Get(),
		hWnd,
		&swapChainDesc,
		&fsSwapChainDesc,
		nullptr, pSwapChain1.ReleaseAndGetAddressOf()
	);
	if (hr != S_OK)
		return hr;

	HR(pSwapChain1.As(&m_pSwapChain));

	// 独占全屏时需允许 DXGI 调整窗口；否则 SetFullscreenState 无法真正离开 DWM 合成
	UINT assocFlags = DXGI_MWA_NO_ALT_ENTER;
	if (!HasSwapChainOption(m_options, ESwapChainOption::ESCO_FullScreen))
		assocFlags |= DXGI_MWA_NO_WINDOW_CHANGES;
	m_pDXGIFactory->MakeWindowAssociation(hWnd, assocFlags);
	DXGISetDebugObjectName(m_pSwapChain.Get(), "DX11SwapChain");

	// 此时 buffer 多为 1x1；独占全屏由屏端在摆好 HWND 后调用 EnterExclusiveFullscreen

	if (HasSwapChainOption(m_options, ESwapChainOption::ESCO_FrameLatency))
	{
		// 2：允许 CPU 提交与 GPU/显示退役重叠，避免 MaxFrameLatency=1 时 Present 串行卡住
		m_pSwapChain->SetMaximumFrameLatency(2);

		m_frameLatencyWaitableObject = m_pSwapChain->GetFrameLatencyWaitableObject();

		WaitOnSwapChain();
	}

	InitGroupBarries();

	return S_OK;
}

HRESULT CDx11SwapChain::Create2DTarget()
{
	ComPtr<IDXGISurface> surface;
	HR(m_pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(surface.GetAddressOf())));

	D2D1_RENDER_TARGET_PROPERTIES props;
	props.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
	props.dpiX = 0;
	props.dpiY = 0;
	props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	props.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
	props.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
	props.usage = D2D1_RENDER_TARGET_USAGE_NONE;

	HR(m_pD2DFactory->CreateDxgiSurfaceRenderTarget(surface.Get(), &props, m_pd2dRenderTarget.ReleaseAndGetAddressOf()));

	return S_OK;
}

HRESULT CDx11SwapChain::UpdateBufffer(int width, int height)
{
	if (nullptr == m_pSwapChain)
		return E_FAIL;

	m_pd2dRenderTarget.Reset();
	m_pRenderTargetView.Reset();
	m_renderTarget.Reset();

	m_bufferWidth = std::max<UINT>(width, 1);
	m_bufferHeight = std::max<UINT>(height, 1);
	m_OutputViewPort = CD3D11_VIEWPORT(0.0f , 0.0f, m_bufferWidth * 1.0f, m_bufferHeight* 1.0f);

	CheckHDRSupport();

	UINT flag = DxgiSwapChainFlagsFromOptions();

	HRESULT hr = m_pSwapChain->ResizeBuffers(BufferCountFromOptions(), m_bufferWidth, m_bufferHeight, m_backBufferFormat, flag);
	if (!SUCCEEDED(hr))
		return hr;

	hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(m_renderTarget.ReleaseAndGetAddressOf()));
	if (!SUCCEEDED(hr))
		return hr;

	CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2D, m_backBufferFormat);
	m_pD3DDevice->CreateRenderTargetView(m_renderTarget.Get(), &renderTargetViewDesc, m_pRenderTargetView.ReleaseAndGetAddressOf());

	D3D11SetDebugObjectName(m_renderTarget.Get(), "Dx11_BackBuffer");
	D3D11SetDebugObjectName(m_pRenderTargetView.Get(), "DX11_BackBufferRTV");

	UpdateColorSpace();
	Create2DTarget();

	if (HasSwapChainOption(m_options, ESwapChainOption::ESCO_EnableMSAA))
		m_msaaHelper->SizeResources(m_bufferWidth, m_bufferHeight, m_backBufferFormat);

	return S_OK;
}

// 调用方须先按 IScreenItem::GetPhysicalRect() 摆好 HWND，并 UpdateBufffer 到同尺寸；
// dxgiOutputIndex 来自 IScreenItem::GetDXGIindex()。
HRESULT CDx11SwapChain::EnterExclusiveFullscreen(int dxgiOutputIndex)
{
	if (!m_pSwapChain || !m_hSwapWnd)
		return E_FAIL;
	if (!HasSwapChainOption(m_options, ESwapChainOption::ESCO_FullScreen))
		return S_FALSE;
	if (dxgiOutputIndex < 0)
		return E_INVALIDARG;

	BOOL bFullscreen = FALSE;
	if (SUCCEEDED(m_pSwapChain->GetFullscreenState(&bFullscreen, nullptr)) && bFullscreen)
		return S_FALSE;

	ComPtr<IDXGIOutput> pOutput;
	const HRESULT hrOut = FindOutputByIndex(m_pD3DDevice.Get(), (UINT)dxgiOutputIndex, pOutput.GetAddressOf());
	if (FAILED(hrOut) || !pOutput)
		return FAILED(hrOut) ? hrOut : E_FAIL;

	DXGI_MODE_DESC mode = {};
	mode.Width = m_bufferWidth;
	mode.Height = m_bufferHeight;
	mode.Format = m_backBufferFormat;
	mode.RefreshRate.Numerator = 0;
	mode.RefreshRate.Denominator = 0;
	mode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	mode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	HRESULT hr = m_pSwapChain->ResizeTarget(&mode);
	if (FAILED(hr))
		return hr;

	hr = m_pSwapChain->SetFullscreenState(TRUE, pOutput.Get());
	if (FAILED(hr))
		return hr;

	mode.RefreshRate.Numerator = 0;
	mode.RefreshRate.Denominator = 0;
	m_pSwapChain->ResizeTarget(&mode);

	// flip 全屏后重建 RTV
	return UpdateBufffer((int)m_bufferWidth, (int)m_bufferHeight);
}

HRESULT CDx11SwapChain::ExitExclusiveFullscreen()
{
	if (!m_pSwapChain)
		return E_FAIL;
	if (!HasSwapChainOption(m_options, ESwapChainOption::ESCO_FullScreen))
		return S_FALSE;

	BOOL bFullscreen = FALSE;
	if (FAILED(m_pSwapChain->GetFullscreenState(&bFullscreen, nullptr)) || !bFullscreen)
		return S_FALSE;

	return m_pSwapChain->SetFullscreenState(FALSE, nullptr);
}

static const UINT32 sc_redShift = 0;
static const UINT32 sc_greenShift = 8;
static const UINT32 sc_blueShift = 16;
static const UINT32 sc_alphaShift = 24;

static const UINT32 sc_redMask = 0xff << sc_redShift;
static const UINT32 sc_greenMask = 0xff << sc_greenShift;
static const UINT32 sc_blueMask = 0xff << sc_blueShift;
static const UINT32 sc_alphaMask = 0xff << sc_alphaShift;

HRESULT CDx11SwapChain::BeginComposite(DWORD abgr)
{
	float r = static_cast<FLOAT>((abgr & sc_redMask) >> sc_redShift) / 255.f;
	float g = static_cast<FLOAT>((abgr & sc_greenMask) >> sc_greenShift) / 255.f;
	float b = static_cast<FLOAT>((abgr & sc_blueMask) >> sc_blueShift) / 255.f;
	float a = static_cast<FLOAT>((abgr & sc_alphaMask) >> sc_alphaShift) / 255.f;

	float bkClr[4] = {r, g, b, a};

	if (HasSwapChainOption(m_options, ESwapChainOption::ESCO_EnableMSAA))
	{
		auto renderTarget = m_msaaHelper->GetMSAARenderTargetView();
		auto depthStencil = m_msaaHelper->GetMSAADepthStencilView();

		m_pD3DDeviceContext->OMSetRenderTargets(1, &renderTarget, depthStencil);
		m_pD3DDeviceContext->ClearRenderTargetView(renderTarget, bkClr);
		m_pD3DDeviceContext->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_pD3DDeviceContext->RSSetViewports(1, &m_OutputViewPort);
	}
	else
	{
		m_pD3DDeviceContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), nullptr);
		m_pD3DDeviceContext->ClearRenderTargetView(m_pRenderTargetView.Get(), bkClr);
		m_pD3DDeviceContext->RSSetViewports(1, &m_OutputViewPort);
	}

	return S_OK;
}

void CDx11SwapChain::SetPresentSyncInterval(UINT syncInterval)
{
	m_presentSyncInterval = syncInterval;
}

HRESULT CDx11SwapChain::EndComposite(std::function<void()>&& func)
{
	if (HasSwapChainOption(m_options, ESwapChainOption::ESCO_EnableMSAA))
		m_msaaHelper->Resolve(m_pD3DDeviceContext.Get(), m_renderTarget.Get());

	if (func)
		func();

	HRESULT hr;
	const UINT syncInterval = m_presentSyncInterval;

	if (m_GroupId > 0)
	{
		NvAPI_Status status = NVAPI_OK;
		status = NvAPI_D3D1x_Present(m_pD3DDevice.Get(), m_pSwapChain.Get(), syncInterval, 0);
		hr = (status == NVAPI_OK) ? S_OK : E_FAIL;
	}
	else
	{
		hr = m_pSwapChain->Present(syncInterval, 0);
	}

	//WaitOnSwapChain();

	hr = m_pD3DDevice->GetDeviceRemovedReason();

	return hr;
}

HRESULT CDx11SwapChain::ReSetRenderRect(int nX, int nY, int nWidth, int nHeight)
{
	m_OutputViewPort = CD3D11_VIEWPORT(nX * 1.0f, nY * 1.0f, nWidth * 1.0f, nHeight * 1.0f);
	m_pD3DDeviceContext->RSSetViewports(1, &m_OutputViewPort);
	return S_OK;
}

void CDx11SwapChain::lock()
{
	EnterCriticalSection(m_pD3DCSContext);
}

void CDx11SwapChain::unlock()
{
	LeaveCriticalSection(m_pD3DCSContext);
}

void CDx11SwapChain::SetHDR(BOOL bEnable)
{
	if (!m_bDisplayHDR10)
		return;

	if (bEnable)
		m_options |= ESwapChainOption::ESCO_EnableHDR;
	else
		m_options &= ~ESwapChainOption::ESCO_EnableHDR;
	
	CheckHDRSupport();
}

void CDx11SwapChain::SetMSAA(BOOL bEnable)
{
	if (bEnable)
		m_options |= ESwapChainOption::ESCO_EnableMSAA;
	else
		m_options &= ~ESwapChainOption::ESCO_EnableMSAA;

	if (m_msaaHelper && (m_msaaHelper->GetSampleCount() < 2))
		m_options &= ~ESwapChainOption::ESCO_EnableMSAA;
}

HRESULT CDx11SwapChain::SetHDRMetaData(DXGI_HDR_METADATA_TYPE Type, UINT Size, void *pMetaData)
{
	ComPtr<IDXGISwapChain4> pSwapChain4;
	m_pSwapChain.As(&pSwapChain4);
	if (pSwapChain4)
	{
		return pSwapChain4->SetHDRMetaData(Type, Size, pMetaData);
	}
	return E_FAIL;
}

void CDx11SwapChain::CheckHDRSupport()
{
	m_bDisplayHDR10 = FALSE;
#if defined(NTDDI_WIN10_RS2)
	if (m_pSwapChain)
	{
		ComPtr<IDXGIOutput> output;
		if (SUCCEEDED(m_pSwapChain->GetContainingOutput(output.GetAddressOf())))
		{
			ComPtr<IDXGIOutput6> output6;
			if (SUCCEEDED(output.As(&output6)))
			{
				DXGI_OUTPUT_DESC1 desc;
				HR(output6->GetDesc1(&desc));

				if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020)
					m_bDisplayHDR10 = true;
			}
		}
	}
#endif

// 	m_backBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	if (m_bDisplayHDR10 && HasSwapChainOption(m_options, ESwapChainOption::ESCO_EnableHDR))
		m_backBufferFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	else
		m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	/*if (m_pSwapChain)
	{
		using namespace winrt::Windows::Devices;
		using namespace winrt::Windows::Devices::Display;
		using namespace winrt::Windows::Devices::Enumeration;

		ComPtr<IDXGIOutput> pOutput;
		ComPtr<IDXGIOutput6> pOutput6;
		m_pSwapChain->GetContainingOutput(&pOutput);
		if (pOutput && SUCCEEDED(pOutput.As(&pOutput6)))
		{
			if (pOutput6 && SUCCEEDED(pOutput6->GetDesc1(&m_outputDesc)))
			{
				DISPLAY_DEVICE device = {};
				device.cb = sizeof(device);
				DisplayMonitor monitor{ nullptr };
				for (UINT deviceIndex = 0; EnumDisplayDevices(m_outputDesc.DeviceName, deviceIndex, &device, EDD_GET_DEVICE_INTERFACE_NAME); ++deviceIndex)
				{
					if (device.StateFlags & DISPLAY_DEVICE_ACTIVE)
						monitor = DisplayMonitor::FromInterfaceIdAsync(device.DeviceID).get();
					if (monitor)
					{
						m_rawOutDesc.MaxLuminance = monitor.MaxLuminanceInNits();
						m_rawOutDesc.MaxFullFrameLuminance = monitor.MaxAverageFullFrameLuminanceInNits();
						m_rawOutDesc.MinLuminance = monitor.MinLuminanceInNits();
						break;
					}
				}
			}
		}
	}*/
}

void CDx11SwapChain::UpdateColorSpace()
{
	DXGI_COLOR_SPACE_TYPE colorSpace = (m_backBufferFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) ? DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709 : DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

	ComPtr<IDXGISwapChain3> swapChain3;
	if (SUCCEEDED(m_pSwapChain.As(&swapChain3)))
	{
		UINT colorSpaceSupport = 0;
		if (SUCCEEDED(swapChain3->CheckColorSpaceSupport(colorSpace, &colorSpaceSupport)))
		{
			if (colorSpaceSupport & DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT)
				HR(swapChain3->SetColorSpace1(colorSpace));
		}
	}
}

void CDx11SwapChain::CheckOption()
{
	ComPtr<IDXGIFactory2> pDXGIFactory = m_pDXGIFactory;

	ComPtr<IDXGIFactory4> factory4;
	if (FAILED(pDXGIFactory.As(&factory4)))
		m_options &= ~ESwapChainOption::ESCO_FlipPresent;

	ComPtr<IDXGIFactory5> factory5;
	HRESULT hr = pDXGIFactory.As(&factory5);
	if (FAILED(hr))
		m_options &= ~ESwapChainOption::ESCO_EnableHDR;

	m_msaaHelper = std::make_unique<DX::MSAAHelper>(m_backBufferFormat, DXGI_FORMAT_D32_FLOAT, 4);
	m_msaaHelper->SetDevice(m_pD3DDevice.Get());

	if (m_msaaHelper->GetSampleCount() < 2)
		m_options &= ~ESwapChainOption::ESCO_EnableMSAA;
}

void CDx11SwapChain::WaitOnSwapChain()
{
	if (nullptr != m_frameLatencyWaitableObject)
		WaitForSingleObjectEx(m_frameLatencyWaitableObject, 1000, true);
}

bool CDx11SwapChain::InitGroupBarries()
{
	if (!HasSwapChainOption(m_options, ESwapChainOption::ESCO_TryNvSync))
	{
		m_GroupId = 0;
		m_BarrierId = 0;
		return false;
	}

	NvAPI_Status status = NVAPI_OK;
	NvU32 gSyncSwapGroups = 0;
	NvU32 gSyncBarriers = 0;
	status = NvAPI_D3D1x_QueryMaxSwapGroup(m_pD3DDevice.Get(), &gSyncSwapGroups, &gSyncBarriers);

	if ((NVAPI_OK == status) && (gSyncSwapGroups > 0) && (gSyncBarriers > 0))
	{
		assert(1 == gSyncSwapGroups);
		status = NvAPI_D3D1x_JoinSwapGroup(m_pD3DDevice.Get(), m_pSwapChain.Get(), 1,
			HasSwapChainOption(m_options, ESwapChainOption::ESCO_TryNvMaster));
		if (status != NVAPI_OK)
		{
			m_GroupId = 0;
			m_BarrierId = 0;
			qWarning() << "CDx11SwapChain::InitGroupBarries JoinSwapGroup failed, status=" << status
				<< ", fallback to DXGI Present";
			return false;
		}

		assert(1 == gSyncBarriers);

		if (HasSwapChainOption(m_options, ESwapChainOption::ESCO_TryNvMaster))
			status = NvAPI_D3D1x_ResetFrameCount(m_pD3DDevice.Get());

		status = NvAPI_D3D1x_BindSwapBarrier(m_pD3DDevice.Get(), 1, 1);
		if (status != NVAPI_OK)
		{
			m_BarrierId = 0;
			qWarning() << "CDx11SwapChain::InitGroupBarries BindSwapBarrier failed, status=" << status
				<< ", fallback to DXGI Present";
			return false;
		}
		
		status = NvAPI_D3D1x_QuerySwapGroup(m_pD3DDevice.Get(), m_pSwapChain.Get(), &m_GroupId, &m_BarrierId);
	}
	else if (m_GroupId > 0)
	{
		m_GroupId = 0;
		m_BarrierId = 0;
		qWarning() << "CDx11SwapChain::InitGroupBarries QueryMaxSwapGroup unavailable, status=" << status
			<< " groups=" << gSyncSwapGroups << " barriers=" << gSyncBarriers
			<< ", fallback to DXGI Present";
		return false;
	}

	if (status != NVAPI_OK)
	{
		qWarning() << "CDx11SwapChain::InitGroupBarries failed, status=" << status
			<< ", fallback to DXGI Present";
		return false;
	}

	return true;
}

void CDx11SwapChain::FreeGroupBarries()
{
	NvAPI_Status status;

	if (m_GroupId > 0)
	{
		if (m_BarrierId > 0)
		{
			if (NVAPI_OK == (status = NvAPI_D3D1x_BindSwapBarrier(m_pD3DDevice.Get(), m_GroupId, 0)))
				m_BarrierId = 0;
		}

		if (NVAPI_OK == (status = NvAPI_D3D1x_JoinSwapGroup(m_pD3DDevice.Get(), m_pSwapChain.Get(), 0, 0)))
			m_GroupId = 0;
	}
}
