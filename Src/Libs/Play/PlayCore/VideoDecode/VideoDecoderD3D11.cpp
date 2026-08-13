#include <windows.h>
#include <cguid.h>
#include <initguid.h>
#include <dxgi1_2.h>
#include "VideoDecoderD3D11.h"

#include <d3d11_1.h>
#include "GSMedia/RenderDevice/DeviceResource.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

namespace PlayCore
{
	struct dxgi_fcc {
		int fourcc;
		DXGI_FORMAT dxgi;
	} dxgi_formats[] = {
		{ MAKEFOURCC('P','0','1','0'), DXGI_FORMAT_P010 },
		{ MAKEFOURCC('N','V','1','2'), DXGI_FORMAT_NV12 },
		{ MAKEFOURCC('P','0','1','6'), DXGI_FORMAT_P016 },
	};

	DXGI_FORMAT fourccToDXGI(int fourcc)
	{
		for (size_t i = 0; i < sizeof(dxgi_formats) / sizeof(dxgi_formats[0]); ++i)
		{
			if (dxgi_formats[i].fourcc == fourcc)
			{
				return dxgi_formats[i].dxgi;
			}
		}
		return DXGI_FORMAT_UNKNOWN;
	}

	int fourccFromDXGI(DXGI_FORMAT fmt)
	{
		for (const dxgi_fcc* f = dxgi_formats;
			f < dxgi_formats + sizeof(dxgi_formats) / sizeof(dxgi_formats[0]); ++f)
		{
			if (f->dxgi == fmt)
			{
				return f->fourcc;
			}
		}
		return 0;
	}

	std::string WChar2Ansi(LPCWSTR pwszSrc)
	{
		int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);
		if (nLen <= 0) return std::string("");
		char* pszDst = new char[nLen];
		if (NULL == pszDst) return std::string("");
		WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
		pszDst[nLen - 1] = 0;
		std::string strTemp(pszDst);
		delete[] pszDst;
		return strTemp;
	}

	VideoDecoderId CVideoDecoderD3D11::id() const
	{
		return VideoDecoderId_D3D11;
	}

	std::string CVideoDecoderD3D11::getDescription() const
	{
		if (!sD3D11Description.empty())
		{
			return sD3D11Description;
		}
		return "D3D11 Video Acceleration";
	}

	CVideoDecoderD3D11::CVideoDecoderD3D11()
		: m_hModule(0)
		, m_device(nullptr)
		, m_deviceContext(nullptr)
		, m_videoDevice(nullptr)
		, m_videoContext(nullptr)
		, m_videoDecoder(nullptr)
		//, m_eglInteropResource(nullptr)
		, m_adapter(nullptr)
	{
		memset(&m_vaContext, 0, sizeof(AVD3D11VAContext));
		memset(&m_videoDecoderConifg, 0x00, sizeof(D3D11_VIDEO_DECODER_CONFIG));

		copy_mode = CVideoDecoderHW::ZeroCopy;
		m_hModule = LoadLibrary(TEXT("d3d11.dll"));
		available = !!m_hModule;
	}

	CVideoDecoderD3D11::~CVideoDecoderD3D11()
	{
		closeInternal();
		if (m_hModule)
		{
			FreeLibrary(m_hModule);
		}
		//if (m_eglInteropResource)
		//{
		//	delete m_eglInteropResource;
		//	m_eglInteropResource = nullptr;
		//}
		m_adapter.Reset();
	}

	void CVideoDecoderD3D11::findAdapter()
	{
		// 同卡硬约束：按 DecoderManager/Player 的 gpuIndex 取 DeviceResource 已绑定 adapter
		m_adapter = DeviceResource_GetAdapter(m_gpuIndex);
	}

	bool CVideoDecoderD3D11::createDevice()
	{
		//创建一个代表显示适配器的设备
		findAdapter();

		if (!m_adapter)
			return false;

		UINT createDeviceFlags = 0;
		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,   //硬件驱动
			D3D_DRIVER_TYPE_REFERENCE,  //软件驱动
		};
		UINT numDriverTypes = ARRAYSIZE(driverTypes);
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);
		//D3D_FEATURE_LEVEL featureLevel;
		D3D_DRIVER_TYPE d3dDriverType = D3D_DRIVER_TYPE_UNKNOWN;
		HRESULT hr = D3D11CreateDevice(m_adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL,
			D3D11_CREATE_DEVICE_VIDEO_SUPPORT, NULL, 0,
			D3D11_SDK_VERSION, m_device.ReleaseAndGetAddressOf(), NULL,
			m_deviceContext.ReleaseAndGetAddressOf());
		if(!SUCCEEDED(hr) || !m_device || !m_deviceContext)
		{
			return false;
		}

		ComPtr<ID3D10Multithread> multiThread;
		hr = m_device.As(&multiThread);
		if (!SUCCEEDED(hr))
		{
			return false;
		}
		multiThread->SetMultithreadProtected(true);


		hr = m_deviceContext.As(&m_videoContext);
		hr = m_device.As(&m_videoDevice);
		if (!SUCCEEDED(hr) ||! m_videoContext|| !m_videoDevice)
		{
			return false;
		}

		return true;
	}

	void CVideoDecoderD3D11::destroyDevice()
	{
		m_videoDevice.Reset();
		m_videoContext.Reset();
		if (m_deviceContext)
			m_deviceContext->ClearState();
		m_deviceContext.Reset();
		m_device.Reset();
	}

	int CVideoDecoderD3D11::fourccFor(const GUID* guid) const
	{
		for (size_t i = 0; i < sizeof(dxgi_formats) / sizeof(dxgi_formats[i]); ++i)
		{
			const dxgi_fcc& f = dxgi_formats[i];
			BOOL isSupported = FALSE;
			if (m_videoDevice != nullptr)
			{
				//Given a profile, checks whether the driver supports a specified output format.
				m_videoDevice->CheckVideoDecoderFormat(guid, f.dxgi, &isSupported);
				if (isSupported)
				{
					return f.fourcc;
				}
			}
		}
		return 0;
	}

	std::vector<GUID> CVideoDecoderD3D11::getSupportedCodecs() const
	{
		if (!m_videoDevice)
		{
			return std::vector<GUID>();
		}
		UINT profileCount = m_videoDevice->GetVideoDecoderProfileCount();//Gets the number of profiles that are supported by the driver.
		std::vector<GUID> guids(profileCount);
		HRESULT hr = S_OK;
		for (UINT i = 0; i < profileCount; ++i)
		{
			hr = m_videoDevice->GetVideoDecoderProfile(i, &guids[i]);
			//if(FAILED(hr))
			//	std::vector<GUID>();//Gets a profile that is supported by the driver.
		}
		return guids;
	}

	bool CVideoDecoderD3D11::getDecoderConfig()
	{
		D3D11_VIDEO_DECODER_DESC decoderDesc;
		ZeroMemory(&decoderDesc, sizeof(decoderDesc));
		decoderDesc.Guid = codec_guid;//解码配置文件，要获取设备支持的配置文件列表GetVideoDecoderProfile
		decoderDesc.SampleWidth = codedWidth(codec_ctx)/*codec_ctx->coded_width*/;//视频帧的宽度，以像素为单位
		decoderDesc.SampleHeight = codedHeight(codec_ctx)/*codec_ctx->coded_height*/;//视频帧的高度，以像素为单位
		decoderDesc.OutputFormat = fourccToDXGI(format_fcc);//输出表面格式，指定为DXGI_FORMAT值

		UINT configCount = 0;
		HRESULT hr = m_videoDevice->GetVideoDecoderConfigCount(&decoderDesc, &configCount);
		if (FAILED(hr))
			return false;

		//D3D11_VIDEO_DECODER_CONFIG *cfg_list = NULL;
		//cfg_list = av_malloc_array(cfg_count, sizeof(D3D11_VIDEO_DECODER_CONFIG));
		//if (cfg_list == NULL)
		//	return false;

		//for (i = 0; i < cfg_count; i++) {
		//	hr = ID3D11VideoDevice_GetVideoDecoderConfig(video_device, desc, i, &cfg_list[i]);
		//	if (FAILED(hr)) {
		//		av_log(avctx, AV_LOG_ERROR, "Unable to retrieve decoder configurations. (hr=0x%lX)\n", hr);
		//		av_free(cfg_list);
		//		return AVERROR(EINVAL);
		//	}
		//}

		//ret = dxva_get_decoder_configuration(avctx, cfg_list, cfg_count);
		//if (ret >= 0)
		//	*m_videoDecoderConifg = cfg_list[ret];
		//av_free(cfg_list);

		return configCount > 0 ? true : false;
	}

	bool CVideoDecoderD3D11::isValidTexture()
	{
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width = aligned(codedWidth(codec_ctx));
		texDesc.Height = aligned(codedHeight(codec_ctx));
		texDesc.MipLevels = 1;
		texDesc.Format = fourccToDXGI(format_fcc);
		texDesc.SampleDesc.Count = 1;
		texDesc.MiscFlags = 0;
		texDesc.ArraySize = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DECODER;
		texDesc.CPUAccessFlags = 0;

		ComPtr<ID3D11Texture2D> tex;
		HRESULT hr = m_device->CreateTexture2D(&texDesc, NULL, tex.GetAddressOf());
		if (FAILED(hr))
		{
			tex.Reset();
			return false;
		}
		tex.Reset();
		return true;
	}

	bool CVideoDecoderD3D11::createDecoder(AVCodecID codec_id, int w, int h, std::vector<va_surface_t*>& surf)
	{
		const int surfaceCount = (int)surf.size();
		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width = aligned(w);
		texDesc.Height = aligned(h);
		texDesc.MipLevels = 1;
		texDesc.Format = fourccToDXGI(format_fcc);
		texDesc.SampleDesc.Count = 1;
		texDesc.MiscFlags = 0;
		texDesc.ArraySize = surfaceCount;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_DECODER;
		texDesc.CPUAccessFlags = 0;
		ComPtr<ID3D11Texture2D> tex;
		HRESULT hr = m_device->CreateTexture2D(&texDesc, NULL, tex.GetAddressOf());
		if (FAILED(hr))
		{
			printf("CreateTexture2D failed!");
			tex.Reset();
			return false;
		}

		D3D11_VIDEO_DECODER_OUTPUT_VIEW_DESC viewDesc;
		ZeroMemory(&viewDesc, sizeof(viewDesc));
		viewDesc.DecodeProfile = codec_guid;
		viewDesc.ViewDimension = D3D11_VDOV_DIMENSION_TEXTURE2D;

		for (int i = 0; i < surfaceCount; ++i)
		{
			viewDesc.Texture2D.ArraySlice = i;
			ComPtr<ID3D11VideoDecoderOutputView> view;
			hr = m_videoDevice->CreateVideoDecoderOutputView(tex.Get(), &viewDesc, &view);
			if (FAILED(hr))
			{
				printf("CreateVideoDecoderOutputView failed!");
				tex.Reset();
				return false;
			}

			d3d11_surface_t* s = new d3d11_surface_t();
			s->setSurface(view.Get());
			surf[i] = s;
		}
		//printf("ID3D11VideoDecoderOutputView %d,surfaces:(%d*%d)", surfaceCount, aligned(w), aligned(h));

		tex.Reset();

		D3D11_VIDEO_DECODER_DESC decoderDesc;
		ZeroMemory(&decoderDesc, sizeof(decoderDesc));
		decoderDesc.Guid = codec_guid;
		decoderDesc.SampleWidth = w;
		decoderDesc.SampleHeight = h;
		decoderDesc.OutputFormat = fourccToDXGI(format_fcc);

		UINT cfgCount;
		hr = m_videoDevice->GetVideoDecoderConfigCount(&decoderDesc, &cfgCount);
		if (FAILED(hr))
			return false;

		//std::vector<D3D11_VIDEO_DECODER_CONFIG> cfg_list(cfgCount);
		D3D11_VIDEO_DECODER_CONFIG *cfg_list = nullptr;
		cfg_list = (D3D11_VIDEO_DECODER_CONFIG*)av_malloc_array(cfgCount, sizeof(D3D11_VIDEO_DECODER_CONFIG));
		if (!cfg_list)
			return false;

		for (unsigned i = 0; i < cfgCount; i++)
		{
			hr = m_videoDevice->GetVideoDecoderConfig(&decoderDesc, i, &cfg_list[i]);
			if (FAILED(hr))
			{
				av_free(cfg_list);
				return false;
			}
		}

		if (SelectConfig(codec_id, cfg_list, cfgCount, &m_videoDecoderConifg) <= 0)
		{
			av_free(cfg_list);
			return false;
		}
		// not associated with surfaces, so surfaces can be dynamicall added?
		hr = m_videoDevice->CreateVideoDecoder(&decoderDesc, &m_videoDecoderConifg, &m_videoDecoder);
		av_free(cfg_list);
		return !FAILED(hr);
	}

	void CVideoDecoderD3D11::destroyDecoder()
	{
		m_videoDecoder.Reset();
	} 

	void* CVideoDecoderD3D11::setupAVVAContext()
	{
		// TODO: FF_DXVA2_WORKAROUND_SCALING_LIST_ZIGZAG
		if (isIntelClearVideo(&codec_guid))
		{
#ifdef FF_DXVA2_WORKAROUND_INTEL_CLEARVIDEO //2014-03-07 - 8b2a130 - lavc 55.50.0 / 55.53.100 - dxva2.h
			qDebug("FF_DXVA2_WORKAROUND_INTEL_CLEARVIDEO");
			m_vaContext.workaround |= FF_DXVA2_WORKAROUND_INTEL_CLEARVIDEO;
#endif
		}
		else
		{
			m_vaContext.workaround = 0;
		}
		m_vaContext.video_context = m_videoContext.Get();
		m_vaContext.decoder = m_videoDecoder.Get();
		m_vaContext.cfg = &m_videoDecoderConifg;
		m_vaContext.surface_count = (unsigned int)hw_surfaces.size();
		m_vaContext.surface = (ID3D11VideoDecoderOutputView**)hw_surfaces.data();
		m_vaContext.context_mutex = INVALID_HANDLE_VALUE;
		return &m_vaContext;
	}

} //namespace PlayCore
