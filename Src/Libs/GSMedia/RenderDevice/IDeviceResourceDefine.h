#pragma once
#include <dxgi1_2.h>
#include <string>
#define RENDER_ZPLANE				(1.0f)

// PCI 设备厂商 ID
// https://devicehunt.com/all-pci-vendors
namespace PciVendor
{
	namespace Amd
	{
		enum { VID = 0x1002 };
		enum DID
		{
		};
	};
	namespace Intel
	{
		enum { VID = 0x8086 };
		enum DID
		{
		};
	};
	namespace Microsoft
	{
		enum { VID = 0x1414 };
		enum DID
		{
		};
	}
	namespace Nvidia
	{
		enum { VID = 0x10de };
		enum DID
		{
			GT620 = 0x0f02,
			T400 = 0x1ff2,
			T400_Mobile = 0x1fb2,

			T500 = 0x1fbb,

			T600 = 0x1fb1,
			T600_Mobile = 0x1fba,
			T600_Laptop = 0x1fb6,

			T1000 = 0x1ff0,
			T1000_Mobile1 = 0x1fb0,
			T1000_Mobile2 = 0x1fb9,
			T1000_Mobile3 = 0x1ff9,

			T2000 = 0x1fb8,
		};
	};

	// 是否可用
	static inline bool IsAdapterUseable(DXGI_ADAPTER_DESC2 const &desc)
	{
		switch (desc.VendorId)
		{
		case Amd::VID:
		case Intel::VID:
			return true;
		case Microsoft::VID:
			// 图形调试模式下的 GPU
			return desc.Description == std::wstring(L"Capture Adapter");
		case Nvidia::VID:
			return true;
		}
		return false;
	}
	// 多显卡模式下适合渲染的显卡
	static inline bool IsAdapterForRendering(DXGI_ADAPTER_DESC2 const &desc)
	{
		switch (desc.VendorId)
		{
		case Amd::VID:
			return false;
		case Intel::VID:
			return false;
		case Microsoft::VID:
			return false;
		case Nvidia::VID:
			switch (desc.DeviceId)
			{
			case Nvidia::GT620:
			case Nvidia::T400:
			case Nvidia::T400_Mobile:
			case Nvidia::T500:
			case Nvidia::T600:
			case Nvidia::T600_Mobile:
			case Nvidia::T600_Laptop:
			case Nvidia::T1000:
			case Nvidia::T1000_Mobile1:
			case Nvidia::T1000_Mobile2:
			case Nvidia::T1000_Mobile3:
			case Nvidia::T2000:
				return false;
			default:
				return true;
			}
		default:
			return false;
		}
	}
	enum class PerformenceRate
	{
		Unspecified, // 未指定
		Core, // 核显
		DedicatedLow, // 低性能独显
		DedicatedMid,
		DedicatedHigh, // 高性能独显
	};
	static inline PerformenceRate GetPerformenceRate(DXGI_ADAPTER_DESC2 const &desc)
	{
		switch (desc.VendorId)
		{
		case Amd::VID:
			return PerformenceRate::DedicatedLow;
		case Intel::VID:
			return PerformenceRate::Core;
		case Microsoft::VID:
			// 图形调试模式下的 GPU
			return desc.Description == std::wstring(L"Capture Adapter") ? PerformenceRate::Core : PerformenceRate::Unspecified;
		case Nvidia::VID:
			switch (desc.DeviceId)
			{
			case Nvidia::GT620:
			case Nvidia::T400:
			case Nvidia::T400_Mobile:
			case Nvidia::T500:
			case Nvidia::T600:
			case Nvidia::T600_Mobile:
			case Nvidia::T600_Laptop:
				return PerformenceRate::DedicatedLow;
			case Nvidia::T1000:
			case Nvidia::T1000_Mobile1:
			case Nvidia::T1000_Mobile2:
			case Nvidia::T1000_Mobile3:
			case Nvidia::T2000:
				return PerformenceRate::DedicatedMid;
			default:
				return PerformenceRate::DedicatedHigh;
			}
		default:
			return PerformenceRate::Unspecified;
		}
	}
};

static inline int64_t operator-(LUID const &l, LUID const &r)
{
	union { LUID id; int64_t _64 = 0; } _l{ l }, _r{ r };
	return _l._64 - _r._64;
}
