#include "pch.h"
#include "D3DWrapper.h"

#include "GSMedia/RenderCommon/DXTrace.h"
#include "GSMedia/RenderCommon/d3dUtil.h"

using namespace std;

namespace
{
#if defined(_DEBUG)
	void OutputDeviceRefLog(const wchar_t* stage, ID3D11Device* pDevice)
	{
		if (pDevice == nullptr)
		{
			return;
		}

		const ULONG refCountAfterAddRef = pDevice->AddRef();
		pDevice->Release();
		wchar_t buffer[256] = {};
		swprintf_s(
			buffer,
			L"[D3DWrapper] %s device=%p ref=%lu\n",
			stage,
			pDevice,
			static_cast<unsigned long>(refCountAfterAddRef - 1));
		OutputDebugStringW(buffer);
	}

	ULONG QueryDeviceRefCount(ID3D11Device* pDevice)
	{
		if (pDevice == nullptr)
		{
			return 0;
		}

		const ULONG refCountAfterAddRef = pDevice->AddRef();
		pDevice->Release();
		return refCountAfterAddRef - 1;
	}

	void OutputDeviceRefLogWithIndex(const wchar_t* stage, UINT deviceIndex, ID3D11Device* pDevice)
	{
		if (pDevice == nullptr)
		{
			return;
		}

		const ULONG refCount = QueryDeviceRefCount(pDevice);
		wchar_t buffer[320] = {};
		swprintf_s(
			buffer,
			L"[D3DWrapper] %s idx=%u device=%p ref=%lu\n",
			stage,
			deviceIndex,
			pDevice,
			static_cast<unsigned long>(refCount));
		OutputDebugStringW(buffer);
	}
#endif
}

CD3D11Wrapper::CD3D11Wrapper()
{
	m_MaxSupportedFeatureLevel = D3D_FEATURE_LEVEL_9_1;
}

CD3D11Wrapper::~CD3D11Wrapper()
{
}

bool CD3D11Wrapper::InitD3D(ComPtr<IDXGIAdapter2> adapter)
{
	CheckMaxSupportedFeatureLevel();
	InitializeCriticalSection(&m_csDeviceContext);
	if (m_MaxSupportedFeatureLevel < D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, _T("不支持DX11.\n"), 0, 0);
		return false;
	}

	if (!Create3DResource(adapter))
		return false;

	// query
	D3D11_QUERY_DESC queryDesc = CD3D11_QUERY_DESC(D3D11_QUERY_EVENT, 0);
	m_pd3dDevice->CreateQuery(&queryDesc, m_pQuery.ReleaseAndGetAddressOf());

	if (!Create2DResource())
		return false;

	adapter->GetDesc2(&m_adapterDesc);

#if defined(_DEBUG)
	OutputDeviceRefLogWithIndex(L"InitD3D.Done", m_debugDeviceIndex, m_pd3dDevice.Get());
#endif

	return true;
}

void CD3D11Wrapper::UnInitD3D()
{
	if (m_pd3dDevice == nullptr)
		return;

#if defined(_DEBUG)
	OutputDeviceRefLogWithIndex(L"UnInitD3D.Begin", m_debugDeviceIndex, m_pd3dDevice.Get());
#endif

	//EnterCriticalSection(&m_csDeviceContext);
	//
	////// 恢复所有默认设定
	////if (m_pd3dImmediateContext)
	////{
	////	ClearImmediateContextBindings();
	////	m_pd3dImmediateContext->ClearState();
	////	m_pd3dImmediateContext->Flush();
	////}
	//LeaveCriticalSection(&m_csDeviceContext);

	m_pQuery.Reset();
	m_pd3dImmediateContext.Reset();

	m_pdwriteFactory.Reset();
	m_pd2dFactory.Reset();

#ifdef _DEBUG
	OutputDeviceRefLogWithIndex(L"UnInitD3D.BeforeReport", m_debugDeviceIndex, m_pd3dDevice.Get());
	const ULONG deviceRefBeforeReport = QueryDeviceRefCount(m_pd3dDevice.Get());
	if (deviceRefBeforeReport > 1)
	{
		ID3D11Debug *d3dDebug = nullptr;
		HRESULT hr = m_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
		if (SUCCEEDED(hr))
		{
			hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
		}
		if (d3dDebug != nullptr)
		{
			d3dDebug->Release();
		}
	}
	else
	{
		OutputDebugStringW(L"[D3DWrapper] Skip live-object report because ref<=1\n");
	}
#endif

	m_pd3dDevice.Reset();

	DeleteCriticalSection(&m_csDeviceContext);
}

void CD3D11Wrapper::ClearImmediateContextBindings()
{
	if (!m_pd3dImmediateContext)
	{
		return;
	}

	ID3D11DeviceContext* pContext = m_pd3dImmediateContext.Get();

	ID3D11RenderTargetView* pRtvs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = {};
	pContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, pRtvs, nullptr);
	pContext->IASetInputLayout(nullptr);

	ID3D11Buffer* pNullBuffer[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {};
	ID3D11ShaderResourceView* pNullSrv[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {};
	ID3D11SamplerState* pNullSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT] = {};
	ID3D11UnorderedAccessView* pNullUavs[D3D11_PS_CS_UAV_REGISTER_COUNT] = {};
	UINT pUavInitialCounts[D3D11_PS_CS_UAV_REGISTER_COUNT] = {};
	for (UINT i = 0; i < D3D11_PS_CS_UAV_REGISTER_COUNT; ++i)
	{
		pUavInitialCounts[i] = UINT(-1);
	}

	pContext->VSSetShader(nullptr, nullptr, 0);
	pContext->PSSetShader(nullptr, nullptr, 0);
	pContext->GSSetShader(nullptr, nullptr, 0);
	pContext->HSSetShader(nullptr, nullptr, 0);
	pContext->DSSetShader(nullptr, nullptr, 0);
	pContext->CSSetShader(nullptr, nullptr, 0);

	pContext->VSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, pNullBuffer);
	pContext->PSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, pNullBuffer);
	pContext->GSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, pNullBuffer);
	pContext->HSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, pNullBuffer);
	pContext->DSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, pNullBuffer);
	pContext->CSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, pNullBuffer);

	pContext->VSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pNullSrv);
	pContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pNullSrv);
	pContext->GSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pNullSrv);
	pContext->HSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pNullSrv);
	pContext->DSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pNullSrv);
	pContext->CSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, pNullSrv);

	pContext->VSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, pNullSamplers);
	pContext->PSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, pNullSamplers);
	pContext->GSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, pNullSamplers);
	pContext->HSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, pNullSamplers);
	pContext->DSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, pNullSamplers);
	pContext->CSSetSamplers(0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, pNullSamplers);

	pContext->OMSetRenderTargetsAndUnorderedAccessViews(
		0,
		nullptr,
		nullptr,
		0,
		D3D11_PS_CS_UAV_REGISTER_COUNT,
		pNullUavs,
		pUavInitialCounts);
	pContext->CSSetUnorderedAccessViews(0, D3D11_PS_CS_UAV_REGISTER_COUNT, pNullUavs, pUavInitialCounts);
}

bool CD3D11Wrapper::Create3DResource(ComPtr<IDXGIAdapter2> adapter)
{
	HRESULT hr = E_FAIL;

	// 创建D3D设备 和 D3D设备上下文
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT; // Direct2D需要支持BGRA格式

#if defined(DEBUG) || defined(_DEBUG)
	hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_NULL, // There is no need to create a real hardware device.
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG, // Check for the SDK layers.
		nullptr, // Any feature level will do.
		0,
		D3D11_SDK_VERSION,
		nullptr, // No need to keep the D3D device reference.
		nullptr, // No need to know the feature level.
		nullptr // No need to keep the D3D device context reference.
	);

	if (SUCCEEDED(hr))
	{
		// If the project is in a debug build, enable debugging via SDK Layers with this flag.
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
	else
	{
		OutputDebugString(L"WARNING: Direct3D Debug Device is not available\n");
	}
#endif

	// 驱动类型数组
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	// 特性等级数组
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);
	D3D_FEATURE_LEVEL featureLevel;

	hr = D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
		D3D11_SDK_VERSION, m_pd3dDevice.GetAddressOf(), &featureLevel, m_pd3dImmediateContext.GetAddressOf());

	if (hr == E_INVALIDARG)
	{
		// Direct3D 11.0 的API不承认D3D_FEATURE_LEVEL_11_1，所以我们需要尝试特性等级11.0以及以下的版本
		hr = D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
			D3D11_SDK_VERSION, m_pd3dDevice.GetAddressOf(), &featureLevel, m_pd3dImmediateContext.GetAddressOf());
	}

	// 设置调试对象名
	D3D11SetDebugObjectName(m_pd3dImmediateContext.Get(), "ImmediateContext");

	// 启用多线程
	ComPtr<ID3D10Multithread> pMultiThread;
	m_pd3dImmediateContext.As(&pMultiThread);
	pMultiThread->SetMultithreadProtected(TRUE);
	
	// 	//启用低延迟
// 	ComPtr<IDXGIDevice1> pDXGIDevice;
// 	hr = m_pd3dDevice.As(&pDXGIDevice);
// 	HR(pDXGIDevice->SetMaximumFrameLatency(1));

	return true;
}

bool CD3D11Wrapper::Create2DResource()
{
	HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, m_pd2dFactory.GetAddressOf()));
	HR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(m_pdwriteFactory.GetAddressOf())));

	return true;
}

void CD3D11Wrapper::CheckMaxSupportedFeatureLevel()
{
	D3D_FEATURE_LEVEL FeatureLevels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	HR(D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, FeatureLevels, ARRAYSIZE(FeatureLevels),
		D3D11_SDK_VERSION, NULL, &m_MaxSupportedFeatureLevel, NULL));
}

std::wstring CD3D11Wrapper::GetDirectXVersion()
{
	switch (m_MaxSupportedFeatureLevel)
	{
	case D3D_FEATURE_LEVEL_9_1:
		return L"DirectX 9.1";
	case D3D_FEATURE_LEVEL_9_2:
		return L"DirectX 9.2";
	case D3D_FEATURE_LEVEL_9_3:
		return L"DirectX 9.3";
	case D3D_FEATURE_LEVEL_10_0:
		return L"DirectX 10.0";
	case D3D_FEATURE_LEVEL_10_1:
		return L"DirectX 10.1";
	case D3D_FEATURE_LEVEL_11_0:
		return L"DirectX 11.0";
	case D3D_FEATURE_LEVEL_11_1:
		return L"DirectX 11.1";
	case D3D_FEATURE_LEVEL_12_0:
		return L"DirectX 12.0";
	case D3D_FEATURE_LEVEL_12_1:
		return L"DirectX 12.1";
	default:
		break;
	}

	assert(0);

	return L"DirectX Unknown.";
}
