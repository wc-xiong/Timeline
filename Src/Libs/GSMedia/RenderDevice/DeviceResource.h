#pragma once
#include "GSMedia/RenderCommon/DXTrace.h"
#include "GSMedia/RenderCommon/d3dUtil.h"
#include "IDeviceResourceDefine.h"
#include "DeviceResourcePrefer.h"
#include "Common/Common/AdapterModeDefine.h"
#include <windows.h>
#include <cguid.h>
#include <atlcomcli.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>
#include "RenderDeviceGlobal.h"

template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

enum
{
	Device_Decode = 0,
	Device_RenderMedia,
	Device_RenderMain,
	Device_RenderInput,
	Device_RenderOutput,
	Device_Render3D,
	Device_RenderSampleTex,


	Device_RenderScreen = 100,
};

// 仅按已选适配器创建 D3D；决策阶段禁止调用
RENDERDEVICE_API HRESULT DeviceResource_InitPrefer(const SDeviceResourceInitPrefer& prefer);
RENDERDEVICE_API HRESULT DeviceResource_UnInit();

// 硬件资格（静态探测，无需 Init）
RENDERDEVICE_API bool DeviceResource_MultiGpusEnabled();

// 运行模式（进程级，Init 前可写可读）；无资格时 EAM_Multi 回落 EAM_One
RENDERDEVICE_API HRESULT DeviceResource_SetGpusMode(EAdapterMode mode);
RENDERDEVICE_API bool DeviceResource_IsMultiGpusMode();

// 主显卡 / 最佳卡（静态探测，无需 Init）；pDesc 可选输出适配器描述
// Master：Nv Bus 排序后首个可渲染卡；Best：同 Master，若无可渲染则按 PerformenceRate 取最高
RENDERDEVICE_API BOOL DeviceResource_GetMasterAdapterForRendering(UINT& index, DXGI_ADAPTER_DESC2* pDesc = nullptr);
RENDERDEVICE_API BOOL DeviceResource_GetBestAdapterForRendering(UINT& index, DXGI_ADAPTER_DESC2* pDesc = nullptr);

// Init 之后：当前已绑定资源的 Adapter 信息
RENDERDEVICE_API UINT DeviceResource_GetAdapterCount();
RENDERDEVICE_API DXGI_ADAPTER_DESC2 DeviceResource_GetAdapterDesc(UINT index);
/** 物理槽 gpuIndex → DXGI 系统适配器下标（idxAdapter）；越界返回 0 */
RENDERDEVICE_API UINT DeviceResource_GetAdapterId(UINT gpuIndex);
/** 物理槽 gpuIndex → 已绑定 IDXGIAdapter2（按值返回 ComPtr，已持有引用；越界/未 Init 返回空） */
RENDERDEVICE_API ComPtr<IDXGIAdapter2> DeviceResource_GetAdapter(UINT gpuIndex);
/** 按 (物理卡, 角色) 取 ID3D11Device；失败返回 nullptr */
RENDERDEVICE_API ID3D11Device* DeviceResource_GetD3DDevice(UINT gpuIndex, UINT role);

class RENDERDEVICE_API CD3DResourceChild
{
public:
	~CD3DResourceChild() = default;
	CD3DResourceChild() = default;
	/** 在指定物理卡上按角色绑定：(gpuIndex, Device_*)；默认 gpuIndex=0（主卡） */
	void InitD3DResource(UINT role, UINT gpuIndex = 0);

	void WaitGPU();

	UINT m_idxDevice = 0; // 逻辑角色 Device_*
	UINT m_gpuIndex = 0;  // 物理卡下标（Init 后主卡=0）
	DXGI_ADAPTER_DESC2 m_adapterDesc;	// 显卡信息
	PCRITICAL_SECTION m_pD3DCSContext = nullptr;

	ComPtr<ID3D11Device> m_pD3DDevice;
	ComPtr<ID2D1Factory> m_pD2DFactory;
	ComPtr<ID3D11DeviceContext> m_pD3DDeviceContext;
	ComPtr<ID3D11Query> m_pD3DQuery;
	ComPtr<IDXGIFactory2> m_pDXGIFactory;
	ComPtr<IDWriteFactory> m_pD2DWriteFactory;
	bool m_bGpuQueryPending = false;
};