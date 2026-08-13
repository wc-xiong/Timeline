#pragma once

/**
 * 共享 DXGI Adapter2 / DESC2 枚举循环（RenderDevice GetAllAdapters 与 GpuEnumerator 复用）。
 * 调用方自行决定过滤（见 DxgiAdapterFilter）。
 */
#include "DxgiAdapterFilter.h"

#include <dxgi1_2.h>
#include <wrl/client.h>
#include <functional>

namespace DxgiAdapterEnum
{
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	/**
	 * 对每个成功 QI+GetDesc2 的适配器调用 visitor(index, adapter2, desc2)。
	 * visitor 返回 false 时提前结束枚举。
	 * QI/GetDesc2 失败：跳过该卡继续。
	 */
	inline void ForEachAdapterDesc2(
		IDXGIFactory1* factory,
		const std::function<bool(UINT index, IDXGIAdapter2* adapter2, const DXGI_ADAPTER_DESC2& desc)>& visitor)
	{
		if (!factory || !visitor)
			return;

		for (UINT iAdapterNum = 0; ; ++iAdapterNum)
		{
			ComPtr<IDXGIAdapter1> adapter1;
			HRESULT hr = factory->EnumAdapters1(iAdapterNum, &adapter1);
			if (DXGI_ERROR_NOT_FOUND == hr || !adapter1)
				break;

			ComPtr<IDXGIAdapter2> adapter2;
			hr = adapter1.As(&adapter2);
			if (FAILED(hr) || !adapter2)
				continue;

			DXGI_ADAPTER_DESC2 desc2 = {};
			hr = adapter2->GetDesc2(&desc2);
			if (FAILED(hr))
				continue;

			if (!visitor(iAdapterNum, adapter2.Get(), desc2))
				break;
		}
	}
}
