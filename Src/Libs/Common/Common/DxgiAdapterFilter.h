#pragma once

/**
 * DXGI 适配器过滤（UI Identity / Init GetAllAdapters 共用）。
 * 软件 / Basic：两侧均跳过。
 * 虚拟卡（双 Preemption = DMA_BUFFER_BOUNDARY）：两侧均跳过，保证列表与 Init 候选一致。
 */
#include <dxgi1_2.h>

namespace DxgiAdapterFilter
{
	inline constexpr UINT kMicrosoftBasicVendorId = 5140u;
	inline constexpr UINT kMicrosoftBasicDeviceId = 140u;

	inline bool IsMicrosoftBasicAdapter(UINT vendorId, UINT deviceId)
	{
		return vendorId == kMicrosoftBasicVendorId && deviceId == kMicrosoftBasicDeviceId;
	}

	inline bool IsSoftwareOrBasicAdapter(UINT flags, UINT vendorId, UINT deviceId)
	{
		if (flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			return true;
		return IsMicrosoftBasicAdapter(vendorId, deviceId);
	}

	/** 三方虚拟显卡：Graphics/Compute Preemption 均为 DMA_BUFFER_BOUNDARY */
	inline bool IsVirtualPreemptionAdapter(
		DXGI_GRAPHICS_PREEMPTION_GRANULARITY graphics,
		DXGI_COMPUTE_PREEMPTION_GRANULARITY compute)
	{
		return graphics == DXGI_GRAPHICS_PREEMPTION_DMA_BUFFER_BOUNDARY
			&& compute == DXGI_COMPUTE_PREEMPTION_DMA_BUFFER_BOUNDARY;
	}

	inline bool IsVirtualPreemptionAdapter(int graphicsPreemption, int computePreemption)
	{
		return IsVirtualPreemptionAdapter(
			static_cast<DXGI_GRAPHICS_PREEMPTION_GRANULARITY>(graphicsPreemption),
			static_cast<DXGI_COMPUTE_PREEMPTION_GRANULARITY>(computePreemption));
	}

	/** UI 列表与 Init 枚举统一跳过规则（软件 / Basic / 虚拟 Preemption） */
	inline bool ShouldSkipAdapter(
		UINT flags,
		UINT vendorId,
		UINT deviceId,
		int graphicsPreemption,
		int computePreemption)
	{
		if (IsSoftwareOrBasicAdapter(flags, vendorId, deviceId))
			return true;
		return IsVirtualPreemptionAdapter(graphicsPreemption, computePreemption);
	}

	inline bool ShouldSkipAdapter(const DXGI_ADAPTER_DESC2& desc)
	{
		return ShouldSkipAdapter(
			desc.Flags,
			desc.VendorId,
			desc.DeviceId,
			static_cast<int>(desc.GraphicsPreemptionGranularity),
			static_cast<int>(desc.ComputePreemptionGranularity));
	}
}
