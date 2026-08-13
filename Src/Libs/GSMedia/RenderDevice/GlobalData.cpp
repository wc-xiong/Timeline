#include "pch.h"
#include "GlobalData.h"
#include "Common/Common/DxgiAdapterEnum.h"
#include <map>

EAdapterMode CDeviceResourceManager::s_gpusMode = EAM_One;

bool CDeviceResourceManager::LuidEqual(const LUID& a, const LUID& b)
{
	return a.LowPart == b.LowPart && a.HighPart == b.HighPart;
}

CDeviceResourceManager::CDeviceResourceManager(const SDeviceResourcePrefer& prefer)
{
	// 仅创建 D3D 资源，不做多卡资格 / Master / Best 决策
	CreateDXGIFactory1(IID_PPV_ARGS(&m_factory));

	m_sAdapter = GetAdapterByPrefer(m_factory, prefer);
	if (m_sAdapter.adapter)
	{
		AddDeviceResource(m_sAdapter);
		m_masterAdapterDescIndex = 0;

		if (prefer.bCreateSiblingAdapters && IsMultiGpusMode())
		{
			CreateSiblingAdapters(m_sAdapter);
			// 预热从卡播放端角色：Decode=0, SampleTex=6（Screen 按窗口创建时再建）
			for (UINT i = 1; i < (UINT)m_vecDeviceResource.size(); ++i)
			{
				GetD3DWrapperOnAdapter(i, 0); // Device_Decode
				GetD3DWrapperOnAdapter(i, 6); // Device_RenderSampleTex
			}
		}
	}
}

void CDeviceResourceManager::AddDeviceResource(const SAdapter& adapter)
{
	auto pDR = new SDeviceResource();
	pDR->idxAdapter = adapter.idxAdapter;
	pDR->adapter = adapter.adapter;
	pDR->desc = adapter.desc;
	m_vecDeviceResource.push_back(pDR);
	m_adapterDescs.push_back(adapter.desc);
}

void CDeviceResourceManager::CreateSiblingAdapters(const SAdapter& master)
{
	auto adapters = GetAllAdapters(m_factory);
	for (const auto& a : adapters)
	{
		if (a.desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;
		if (LuidEqual(a.desc.AdapterLuid, master.desc.AdapterLuid))
			continue;
		if (a.desc.DeviceId != master.desc.DeviceId)
			continue;
		if (!PciVendor::IsAdapterForRendering(a.desc))
			continue;
		AddDeviceResource(a);
	}
}

CDeviceResourceManager::~CDeviceResourceManager()
{
	for (auto& dr : m_vecDeviceResource)
	{
		if (dr)
		{
			delete dr;
			dr = nullptr;
		}
	}
	m_vecDeviceResource.clear();
}

CD3D11Wrapper* CDeviceResourceManager::GetD3DWrapper(UINT index)
{
	return GetD3DWrapperOnAdapter(m_masterAdapterDescIndex, index);
}

CD3D11Wrapper* CDeviceResourceManager::GetD3DWrapperOnAdapter(UINT adapterDescIndex, UINT deviceRole)
{
	if (adapterDescIndex >= m_vecDeviceResource.size())
	{
		assert(0);
		return nullptr;
	}

	SDeviceResource* dr = m_vecDeviceResource[adapterDescIndex];
	if (!dr || !dr->adapter)
	{
		assert(0);
		return nullptr;
	}

	auto iter = dr->mapd3dWraper.find(static_cast<int>(deviceRole));
	if (iter != dr->mapd3dWraper.end())
		return iter->second;

	auto pWrapper = new CD3D11Wrapper;
	pWrapper->m_debugDeviceIndex = deviceRole;
	m_factory.As(&pWrapper->m_dxgiFactory);
	pWrapper->InitD3D(dr->adapter.Get());
	dr->mapd3dWraper.insert({ static_cast<int>(deviceRole), pWrapper });
	return pWrapper;
}

DXGI_ADAPTER_DESC2 CDeviceResourceManager::GetAdapterDesc(UINT index) const
{
	if (index >= m_adapterDescs.size())
		return {};
	return m_adapterDescs[index];
}

UINT CDeviceResourceManager::GetSystemAdapterIndex(UINT gpuIndex) const
{
	if (gpuIndex >= m_vecDeviceResource.size() || !m_vecDeviceResource[gpuIndex])
		return 0;
	return m_vecDeviceResource[gpuIndex]->idxAdapter;
}

CDeviceResourceManager::ComPtr<IDXGIAdapter2> CDeviceResourceManager::GetAdapter(UINT gpuIndex) const
{
	if (gpuIndex >= m_vecDeviceResource.size() || !m_vecDeviceResource[gpuIndex])
		return nullptr;
	return m_vecDeviceResource[gpuIndex]->adapter;
}

HRESULT CDeviceResourceManager::SetGpusMode(EAdapterMode mode)
{
	if (mode == EAM_Multi && !MultiGpusEnabled())
	{
		s_gpusMode = EAM_One;
		OutputDebugStringA("[DeviceResource] SetGpusMode(EAM_Multi) rejected: MultiGpus not enabled, fallback to EAM_One\n");
		return E_FAIL;
	}
	s_gpusMode = mode;
	return S_OK;
}

bool CDeviceResourceManager::IsMultiGpusMode()
{
	return s_gpusMode == EAM_Multi;
}

bool CDeviceResourceManager::MultiGpusEnabled()
{
	ComPtr<IDXGIFactory1> factory;
	CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	auto adapters = GetAllAdapters(factory);
	return DetectMultiGpusCapability(adapters);
}

BOOL CDeviceResourceManager::GetMasterAdapterForRendering(UINT& index, DXGI_ADAPTER_DESC2* pDesc)
{
	// Master：Nv Bus 排序后首个可渲染卡；多卡模式默认主卡
	auto descs = ProbeSortedAdapterDescs();
	if (descs.empty())
		return FALSE;

	for (UINT i = 0; i < descs.size(); ++i)
	{
		if (PciVendor::IsAdapterForRendering(descs[i]))
		{
			index = i;
			if (pDesc)
				*pDesc = descs[i];
			return TRUE;
		}
	}
	index = 0;
	if (pDesc)
		*pDesc = descs[0];
	return TRUE;
}

BOOL CDeviceResourceManager::GetBestAdapterForRendering(UINT& index, DXGI_ADAPTER_DESC2* pDesc)
{
	// Best：与 Master 相同优先可渲染；仅当无可渲染卡时按 PerformenceRate 取最高（单卡回落）
	auto descs = ProbeSortedAdapterDescs();
	if (descs.empty())
		return FALSE;

	for (UINT i = 0; i < descs.size(); ++i)
	{
		if (PciVendor::IsAdapterForRendering(descs[i]))
		{
			index = i;
			if (pDesc)
				*pDesc = descs[i];
			return TRUE;
		}
	}

	// 回退：最高性能可用卡
	auto rateHighest = PciVendor::PerformenceRate::Unspecified;
	UINT best = 0;
	for (UINT i = 0; i < descs.size(); ++i)
	{
		auto rate = PciVendor::GetPerformenceRate(descs[i]);
		if (rate > rateHighest)
		{
			rateHighest = rate;
			best = i;
		}
	}
	index = best;
	if (pDesc)
		*pDesc = descs[best];
	return TRUE;
}

std::vector<DXGI_ADAPTER_DESC2> CDeviceResourceManager::ProbeSortedAdapterDescs()
{
	ComPtr<IDXGIFactory1> factory;
	CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	auto adapters = GetAllAdapters(factory);
	SortNvGpusByBusID(adapters);

	std::vector<DXGI_ADAPTER_DESC2> descs;
	for (auto const& a : adapters)
	{
		if (a.desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;
		descs.push_back(a.desc);
	}
	return descs;
}

bool CDeviceResourceManager::DetectMultiGpusCapability(std::vector<SAdapter> const& adapters)
{
	// 同一可渲染 DeviceId 出现次数 >= 2 → 具备多卡资格
	std::map<UINT, int> deviceIdCount;
	std::map<UINT, bool> deviceIdRenderable;
	for (auto const& adapter : adapters)
	{
		if (adapter.desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;
		deviceIdCount[adapter.desc.DeviceId]++;
		deviceIdRenderable[adapter.desc.DeviceId] = PciVendor::IsAdapterForRendering(adapter.desc);
	}
	for (auto const& kv : deviceIdCount)
	{
		if (kv.second >= 2 && deviceIdRenderable[kv.first])
			return true;
	}
	return false;
}

void CDeviceResourceManager::SortNvGpusByBusID(std::vector<SAdapter>& adapters)
{
	std::vector<SAdapter> nvGpuAdapters, otherAdapters;
	for (auto const& a : adapters)
	{
		if (a.desc.VendorId == PciVendor::Nvidia::VID)
			nvGpuAdapters.push_back(a);
		else
			otherAdapters.push_back(a);
	}

	if (!nvGpuAdapters.empty())
	{
		// BusID 升序（越小越靠近 CPU）；按完整 LUID 匹配 DXGI Adapter
		struct SNvBusInfo
		{
			uint32_t BusID = 0;
			LUID idAdapter{};
		};
		std::map<uint32_t, SNvBusInfo> nvGpusSorted;

		auto res = NvAPI_Initialize();
		if (res == NVAPI_OK)
		{
			NvU32 count = 0;
			NvPhysicalGpuHandle handles[NVAPI_MAX_PHYSICAL_GPUS] = {};
			if (NvAPI_EnumPhysicalGPUs(handles, &count) == NVAPI_OK && count)
			{
				for (NvU32 i = 0; i < count; ++i)
				{
					SNvBusInfo info;
					NvAPI_GPU_GetBusId(handles[i], (NvU32*)&info.BusID);
					NvAPI_GPU_GetAdapterIdFromPhysicalGpu(handles[i], &info.idAdapter);
					nvGpusSorted[info.BusID] = info;
				}
			}
			NvAPI_Unload();
		}

		std::vector<SAdapter> sorted;
		std::vector<bool> used(nvGpuAdapters.size(), false);
		for (auto const& kv : nvGpusSorted)
		{
			for (size_t i = 0; i < nvGpuAdapters.size(); ++i)
			{
				if (used[i])
					continue;
				if (LuidEqual(nvGpuAdapters[i].desc.AdapterLuid, kv.second.idAdapter))
				{
					sorted.push_back(nvGpuAdapters[i]);
					used[i] = true;
					break;
				}
			}
		}
		for (size_t i = 0; i < nvGpuAdapters.size(); ++i)
		{
			if (!used[i])
				sorted.push_back(nvGpuAdapters[i]);
		}
		nvGpuAdapters = std::move(sorted);
	}

	adapters = nvGpuAdapters;
	adapters.insert(adapters.end(), otherAdapters.begin(), otherAdapters.end());
}

std::vector<CDeviceResourceManager::SAdapter> CDeviceResourceManager::GetAllAdapters(ComPtr<IDXGIFactory1> factory)
{
	std::vector<SAdapter> adapters;
	if (!factory)
		return adapters;

	DxgiAdapterEnum::ForEachAdapterDesc2(factory.Get(),
		[&](UINT iAdapterNum, IDXGIAdapter2* adapter2, const DXGI_ADAPTER_DESC2& desc2) -> bool
		{
			if (DxgiAdapterFilter::ShouldSkipAdapter(desc2))
			{
				if (DxgiAdapterFilter::IsVirtualPreemptionAdapter(
					desc2.GraphicsPreemptionGranularity,
					desc2.ComputePreemptionGranularity))
				{
					char buffer[512] = {};
					sprintf_s(buffer, "[DeviceResource] Filter Virtual GPU Adapter: %ls\n", desc2.Description);
					OutputDebugStringA(buffer);
				}
				return true;
			}

			ComPtr<IDXGIAdapter2> hold(adapter2);
			adapters.push_back({ iAdapterNum, desc2, hold });
			return true;
		});
	return adapters;
}

CDeviceResourceManager::SAdapter CDeviceResourceManager::GetHighestPerformingAdapter(std::vector<CDeviceResourceManager::SAdapter> const& all)
{
	auto rateHighest = PciVendor::PerformenceRate::Unspecified;
	SAdapter adapterHighest;
	for (auto& adapter : all)
	{
		auto rate = PciVendor::GetPerformenceRate(adapter.desc);
		if (rate > rateHighest)
		{
			rateHighest = rate;
			adapterHighest = adapter;
		}
	}
	return adapterHighest;
}

CDeviceResourceManager::SAdapter CDeviceResourceManager::GetAdapterByPrefer(
	ComPtr<IDXGIFactory1> factory, const SDeviceResourcePrefer& prefer)
{
	auto adapters = GetAllAdapters(factory);
	assert(adapters.size());

	if (prefer.bHasLuid)
	{
		for (auto& adapter : adapters)
		{
			if (adapter.desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;
			if (LuidEqual(adapter.desc.AdapterLuid, prefer.luid))
				return adapter;
		}
		OutputDebugStringA("[DeviceResource] GetAdapterByPrefer: LUID not found, fallback\n");
	}

	if (0 != prefer.deviceId)
	{
		for (auto& adapter : adapters)
		{
			if (adapter.desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;
			if (prefer.deviceId == adapter.desc.DeviceId)
				return adapter;
		}
	}

	// 没有找到或未指定显卡时选择最优的显卡
	auto adapter = GetHighestPerformingAdapter(adapters);
	if (adapter.adapter)
		return adapter;

	if (adapters.size())
		return adapters[0];
	return {};
}
