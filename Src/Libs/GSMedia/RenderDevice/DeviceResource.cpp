#include "pch.h"
#include "DeviceResource.h"
#include "D3DWrapper.h"
#include "GlobalData.h"

CDeviceResourceManager* gDeviceResourceManager = nullptr;

RENDERDEVICE_API HRESULT DeviceResource_InitPrefer(const SDeviceResourceInitPrefer& prefer)
{
	if (gDeviceResourceManager)
	{
		delete gDeviceResourceManager;
		gDeviceResourceManager = nullptr;
	}
	gDeviceResourceManager = new CDeviceResourceManager(prefer);
	return S_OK;
}

RENDERDEVICE_API HRESULT DeviceResource_UnInit()
{
	delete gDeviceResourceManager;
	gDeviceResourceManager = nullptr;
	return S_OK;
}

RENDERDEVICE_API bool DeviceResource_MultiGpusEnabled()
{
	return CDeviceResourceManager::MultiGpusEnabled();
}

RENDERDEVICE_API HRESULT DeviceResource_SetGpusMode(EAdapterMode mode)
{
	return CDeviceResourceManager::SetGpusMode(mode);
}

RENDERDEVICE_API bool DeviceResource_IsMultiGpusMode()
{
	return CDeviceResourceManager::IsMultiGpusMode();
}

RENDERDEVICE_API BOOL DeviceResource_GetMasterAdapterForRendering(UINT& index, DXGI_ADAPTER_DESC2* pDesc)
{
	return CDeviceResourceManager::GetMasterAdapterForRendering(index, pDesc);
}

RENDERDEVICE_API BOOL DeviceResource_GetBestAdapterForRendering(UINT& index, DXGI_ADAPTER_DESC2* pDesc)
{
	return CDeviceResourceManager::GetBestAdapterForRendering(index, pDesc);
}

RENDERDEVICE_API UINT DeviceResource_GetAdapterCount()
{
	if (!gDeviceResourceManager)
		return 0;
	return gDeviceResourceManager->GetAdapterCount();
}

RENDERDEVICE_API DXGI_ADAPTER_DESC2 DeviceResource_GetAdapterDesc(UINT index)
{
	if (!gDeviceResourceManager)
		return {};
	return gDeviceResourceManager->GetAdapterDesc(index);
}

RENDERDEVICE_API UINT DeviceResource_GetAdapterId(UINT gpuIndex)
{
	if (!gDeviceResourceManager)
		return 0;
	return gDeviceResourceManager->GetSystemAdapterIndex(gpuIndex);
}

RENDERDEVICE_API ComPtr<IDXGIAdapter2> DeviceResource_GetAdapter(UINT gpuIndex)
{
	if (!gDeviceResourceManager)
		return nullptr;
	return gDeviceResourceManager->GetAdapter(gpuIndex);
}

RENDERDEVICE_API ID3D11Device* DeviceResource_GetD3DDevice(UINT gpuIndex, UINT role)
{
	if (!gDeviceResourceManager)
		return nullptr;
	CD3D11Wrapper* pWrapper = gDeviceResourceManager->GetD3DWrapperOnAdapter(gpuIndex, role);
	if (!pWrapper)
		return nullptr;
	return pWrapper->m_pd3dDevice.Get();
}

void CD3DResourceChild::InitD3DResource(UINT role, UINT gpuIndex)
{
	m_idxDevice = role;
	m_gpuIndex = gpuIndex;

	if (!gDeviceResourceManager)
	{
		assert(0);
		return;
	}

	CD3D11Wrapper* pWrapper = gDeviceResourceManager->GetD3DWrapperOnAdapter(gpuIndex, role);
	if (!pWrapper)
	{
		assert(0);
		return;
	}

	m_pD3DDevice = pWrapper->m_pd3dDevice.Get();
	m_pD3DDeviceContext = pWrapper->m_pd3dImmediateContext.Get();
	m_pD3DQuery = pWrapper->m_pQuery.Get();
	m_pD3DCSContext = &(pWrapper->m_csDeviceContext);
	m_pD2DFactory = pWrapper->m_pd2dFactory.Get();
	m_pDXGIFactory = pWrapper->m_dxgiFactory.Get();
	m_pD2DWriteFactory = pWrapper->m_pdwriteFactory.Get();

	m_adapterDesc = pWrapper->m_adapterDesc;

#if defined(_DEBUG)
	char buffer[320] = {};
	sprintf_s(
		buffer,
		"[DeviceResource] ChildInit this=%p role=%u gpu=%u wrapperRole=%u device=%p\n",
		this,
		role,
		gpuIndex,
		pWrapper->m_debugDeviceIndex,
		m_pD3DDevice.Get());
	OutputDebugStringA(buffer);
#endif
}

void CD3DResourceChild::WaitGPU()
{
	int nOutTime = 0;	// 最多等20毫秒
	EnterCriticalSection(m_pD3DCSContext);

	if (!m_pD3DQuery)
	{
		LeaveCriticalSection(m_pD3DCSContext);
		return;
	}

	// 如果上一次查询还没取回，先尝试取回，避免重复End同一个Query触发D3D警告。
	while (m_bGpuQueryPending)
	{
		BOOL bCompleted = FALSE;
		HRESULT hr = m_pD3DDeviceContext->GetData(m_pD3DQuery.Get(), &bCompleted, sizeof(BOOL), 0);
		if (hr == S_OK && bCompleted)
		{
			m_bGpuQueryPending = false;
			break;
		}
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET || (FAILED(hr) && hr != S_FALSE))
		{
			m_bGpuQueryPending = false;
			LeaveCriticalSection(m_pD3DCSContext);
			return;
		}
		if (nOutTime >= 20)
		{
			LeaveCriticalSection(m_pD3DCSContext);
			return;
		}
		nOutTime++;
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}

	nOutTime = 0;
	m_pD3DDeviceContext->Flush();
	m_pD3DDeviceContext->End(m_pD3DQuery.Get());
	m_bGpuQueryPending = true;

	while (m_bGpuQueryPending)
	{
		BOOL bCompleted = FALSE;
		// 临时移除DONOTFLUSH，让驱动刷新命令队列
		HRESULT hr = m_pD3DDeviceContext->GetData(m_pD3DQuery.Get(), &bCompleted, sizeof(BOOL), 0);
		if (hr == S_OK && bCompleted)
		{
			m_bGpuQueryPending = false;
			break;
		}
		// 1. 检查设备异常
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			m_bGpuQueryPending = false;
			break;
		}
		else if (FAILED(hr) && hr != S_FALSE) // 2. 检查GetData其他错误
		{
			m_bGpuQueryPending = false;
			break;
		}
		if (nOutTime >= 20)
		{
			break;
		}
		nOutTime++;
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}

	LeaveCriticalSection(m_pD3DCSContext);
}