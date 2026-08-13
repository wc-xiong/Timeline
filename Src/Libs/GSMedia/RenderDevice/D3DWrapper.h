#pragma once

#include <list>
#include <atlcomcli.h>

class CD3D11Wrapper
{
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
public:
	CD3D11Wrapper();
	~CD3D11Wrapper();
	CD3D11Wrapper(const CD3D11Wrapper& other) = delete;
	CD3D11Wrapper& operator=(const CD3D11Wrapper& other) = delete;

	std::wstring GetDirectXVersion(); // 获取DirectX版本
	bool InitD3D(ComPtr<IDXGIAdapter2> adapter);
	void UnInitD3D();

protected:
	bool Create3DResource(ComPtr<IDXGIAdapter2> adapter); // Direct3D初始化
	bool Create2DResource();
	void ClearImmediateContextBindings();

	void CheckMaxSupportedFeatureLevel(); // 检查D3D11支持项

public:
	DXGI_ADAPTER_DESC2 m_adapterDesc;
	UINT m_debugDeviceIndex = 0;

	CRITICAL_SECTION m_csDeviceContext; // 关键段：用于互斥访问DeviceContext
	ComPtr<IDXGIFactory2> m_dxgiFactory = nullptr;
	ComPtr<ID3D11Device> m_pd3dDevice = nullptr; // D3D11设备
	ComPtr<ID3D11DeviceContext> m_pd3dImmediateContext = nullptr; // D3D11设备上下文
	ComPtr<ID3D11Query>	m_pQuery = nullptr;
	ComPtr<ID3DUserDefinedAnnotation> m_d3dAnnotation = nullptr;

	ComPtr<ID2D1Factory> m_pd2dFactory = nullptr; // D2D工厂
	ComPtr<IDWriteFactory> m_pdwriteFactory = nullptr; // DWrite工厂

protected:
	D3D_FEATURE_LEVEL m_MaxSupportedFeatureLevel; // 最大支持特性等级
};
