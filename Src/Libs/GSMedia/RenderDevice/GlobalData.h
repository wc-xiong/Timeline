#pragma once
#include "D3DWrapper.h"
#include "DeviceResourcePrefer.h"
#include "Common/Common/AdapterModeDefine.h"
#include <map>
#include <vector>
#include <windows.h>
#include "RenderDeviceGlobal.h"

class CDeviceResourceManager
{
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	// 仅按已选适配器创建 D3D，不夹杂选卡业务
	explicit CDeviceResourceManager(const SDeviceResourcePrefer& prefer);
	~CDeviceResourceManager();
	CDeviceResourceManager(const CDeviceResourceManager& other) = delete;
	CDeviceResourceManager& operator=(const CDeviceResourceManager& other) = delete;

	CD3D11Wrapper* GetD3DWrapper(UINT index);
	/** 在指定物理适配器（m_adapterDescs 下标）上取/创建逻辑设备角色的 Wrapper */
	CD3D11Wrapper* GetD3DWrapperOnAdapter(UINT adapterDescIndex, UINT deviceRole);

	UINT GetAdapterCount() const { return (UINT)m_adapterDescs.size(); }
	DXGI_ADAPTER_DESC2 GetAdapterDesc(UINT index) const;
	/** 物理槽 → DXGI 枚举下标（SDeviceResource::idxAdapter） */
	UINT GetSystemAdapterIndex(UINT gpuIndex) const;
	/** 物理槽 → 已绑定 IDXGIAdapter2（按值返回 ComPtr；越界/空返回空） */
	ComPtr<IDXGIAdapter2> GetAdapter(UINT gpuIndex) const;
	/** 主卡在 m_adapterDescs 中的下标（通常为 0） */
	UINT GetMasterAdapterDescIndex() const { return m_masterAdapterDescIndex; }

	// ---- 静态探测 / 模式（无需 Init）----
	static bool MultiGpusEnabled();
	/** Nv Bus 排序后首个可渲染卡；若无则回落 index=0 */
	static BOOL GetMasterAdapterForRendering(UINT& index, DXGI_ADAPTER_DESC2* pDesc = nullptr);
	/** 与 Master 相同优先可渲染；无可渲染时按 PerformenceRate 取最高 */
	static BOOL GetBestAdapterForRendering(UINT& index, DXGI_ADAPTER_DESC2* pDesc = nullptr);
	static HRESULT SetGpusMode(EAdapterMode mode);
	static bool IsMultiGpusMode();

private:
	struct SDeviceResource
	{
		// 适配器索引, 此为系统的索引
		UINT idxAdapter = 0;
		ComPtr<IDXGIAdapter2> adapter;
		DXGI_ADAPTER_DESC2 desc{};
		std::map<int, CD3D11Wrapper*> mapd3dWraper;
		~SDeviceResource()
		{
			for (auto& d3dWraper : mapd3dWraper)
			{
				if (d3dWraper.second)
				{
					d3dWraper.second->UnInitD3D();
					delete d3dWraper.second;
					d3dWraper.second = nullptr;
				}
			}
		}
	};
	struct SAdapter
	{
		UINT idxAdapter = 0;
		DXGI_ADAPTER_DESC2 desc{};
		ComPtr<IDXGIAdapter2> adapter = nullptr;
	};

private:
	static std::vector<SAdapter> GetAllAdapters(ComPtr<IDXGIFactory1> factory);
	static void SortNvGpusByBusID(std::vector<SAdapter>& adapters);
	static bool DetectMultiGpusCapability(std::vector<SAdapter> const& adapters);
	static std::vector<DXGI_ADAPTER_DESC2> ProbeSortedAdapterDescs();
	static bool LuidEqual(const LUID& a, const LUID& b);

	SAdapter GetHighestPerformingAdapter(std::vector<SAdapter> const& all);
	SAdapter GetAdapterByPrefer(ComPtr<IDXGIFactory1> factory, const SDeviceResourcePrefer& prefer);
	void AddDeviceResource(const SAdapter& adapter);
	void CreateSiblingAdapters(const SAdapter& master);

private:
	ComPtr<IDXGIFactory1> m_factory;
	std::vector<SDeviceResource*> m_vecDeviceResource;
	SAdapter m_sAdapter;
	std::vector<DXGI_ADAPTER_DESC2> m_adapterDescs;
	UINT m_masterAdapterDescIndex = 0;

	static EAdapterMode s_gpusMode;
};

extern CDeviceResourceManager* gDeviceResourceManager;
