#pragma once

#include <initguid.h>
#include "VideoDecoderD3D.h"

#include <wrl/client.h>
template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

struct IDXGIAdapter2;


namespace PlayCore {
	static std::string sD3D11Description;

	class CVideoDecoderD3D11 : public CVideoDecoderD3D
	{
	public:
		CVideoDecoderD3D11();
		virtual ~CVideoDecoderD3D11();
		virtual bool getDecoderConfig();
		virtual bool isValidTexture();

		VideoDecoderId id() const Q_DECL_OVERRIDE;
		std::string getDescription() const Q_DECL_OVERRIDE;
		AVPixelFormat vaPixelFormat() const Q_DECL_OVERRIDE { return AV_PIX_FMT_D3D11VA_VLD; }
		//处理
		//bool updateSharedTexture(AVFrame* frame);
		//bool createSharedResource(int w, int h, DXGI_FORMAT format);
		//HANDLE getSharedHandle();

	private:
		void findAdapter();
		bool createDevice() Q_DECL_OVERRIDE;
		void destroyDevice() Q_DECL_OVERRIDE;
		bool createDecoder(AVCodecID codec_id, int w, int h, std::vector<va_surface_t*>& surf) Q_DECL_OVERRIDE;
		void destroyDecoder() Q_DECL_OVERRIDE;
		//bool setupSurfaceInterop() Q_DECL_OVERRIDE;
		void* setupAVVAContext() Q_DECL_OVERRIDE;
		std::vector<GUID> getSupportedCodecs() const Q_DECL_OVERRIDE;
		int fourccFor(const GUID *guid) const Q_DECL_OVERRIDE;

	public:
		HMODULE m_hModule;
		ComPtr<ID3D11Device> m_device;
		ComPtr<ID3D11DeviceContext> m_deviceContext;
		ComPtr<ID3D11VideoDevice> m_videoDevice;
		ComPtr<ID3D11VideoContext> m_videoContext;
		ComPtr<ID3D11VideoDecoder> m_videoDecoder;
		D3D11_VIDEO_DECODER_CONFIG m_videoDecoderConifg;
		struct AVD3D11VAContext m_vaContext;

		ComPtr<IDXGIAdapter2> m_adapter;
	};

	struct d3d11_surface_t : public va_surface_t
	{
		d3d11_surface_t() : va_surface_t(), view(0) {}
		void setSurface(IUnknown* s)
		{
			view = (ID3D11VideoDecoderOutputView*)s;
		}
		IUnknown* getSurface() const { return view.Get(); }

		virtual ~d3d11_surface_t() {
			view.Reset();
		}

	private:
		ComPtr<ID3D11VideoDecoderOutputView> view;
	};
}