#include "PreviewGpuPipeline.h"

#include "Play/PlayCore/MMDecoder.h"
#include "Play/PlayCore/D3D11VideoProcessor.h"
#include "GSMedia/RenderDevice/DeviceResource.h"
#include "GSMedia/RenderBase2D/BasicEffect.h"
#include "Common/Common/AVCommon.h"

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext_d3d11va.h>
#include <libswscale/swscale.h>
}

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <windows.h>
#include <QByteArray>
#include <algorithm>

namespace PreviewGpuDetail
{
	struct SGpuSlot
	{
		QUuid clipId;
		QString path;
		CMMDecoder* decoder = nullptr;
		std::unique_ptr<CD3D11VideoProcessor> vp;		// 渲染设备 VP（同设备硬解时用）
		std::unique_ptr<CD3D11VideoProcessor> hwVp;	// 解码器设备 VP（跨设备时用）
		Microsoft::WRL::ComPtr<ID3D11Device> hwVpDevice;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> rgbaTex;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rgbaSrv;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingUpload;
		// 渲染侧 rgba 在解码器设备上打开的共享视图（VP 写这里）
		Microsoft::WRL::ComPtr<ID3D11Texture2D> hwOpenedRgba;
		Microsoft::WRL::ComPtr<ID3D11Query> hwFlushQuery;
		SwsContext* sws = nullptr;
		int rgbaW = 0;
		int rgbaH = 0;
		int width = 0;
		int height = 0;
		qint64 lastPtsMs = -1;
		qint64 seekTargetMs = -1; // Seek 目标，Pump/Prime 用
		bool bPlaying = false;
		/** 已从解码器取出、但 pts 超过当前时钟的一帧。时钟追上后再上传，避免暂停 Seek 画出目标右侧。 */
		AVFrame* pending = nullptr;
	};

	void ReleasePendingFrame(SGpuSlot& slot)
	{
		if (slot.pending)
			av_frame_free(&slot.pending);
	}

	void CloseGpuSlot(SGpuSlot& slot)
	{
		if (slot.decoder)
		{
			slot.decoder->UnInit();
			delete slot.decoder;
			slot.decoder = nullptr;
		}
		if (slot.sws)
		{
			sws_freeContext(slot.sws);
			slot.sws = nullptr;
		}
		slot.hwVp.reset();
		slot.hwVpDevice.Reset();
		slot.hwOpenedRgba.Reset();
		slot.hwFlushQuery.Reset();
		slot.vp.reset();
		slot.rgbaSrv.Reset();
		slot.rgbaTex.Reset();
		slot.stagingUpload.Reset();
		slot.rgbaW = slot.rgbaH = 0;
		slot.width = slot.height = 0;
		slot.lastPtsMs = -1;
		slot.seekTargetMs = -1;
		ReleasePendingFrame(slot);
		slot.bPlaying = false;
		slot.path.clear();
		slot.clipId = QUuid();
	}

	void WaitGpuIdle(ID3D11Device* device, ID3D11DeviceContext* ctx,
		Microsoft::WRL::ComPtr<ID3D11Query>& query)
	{
		if (!device || !ctx)
			return;
		if (!query)
		{
			D3D11_QUERY_DESC qd = {};
			qd.Query = D3D11_QUERY_EVENT;
			device->CreateQuery(&qd, query.ReleaseAndGetAddressOf());
		}
		if (!query)
		{
			ctx->Flush();
			return;
		}
		ctx->End(query.Get());
		ctx->Flush();
		for (int i = 0; i < 64; ++i)
		{
			const HRESULT hr = ctx->GetData(query.Get(), nullptr, 0, 0);
			if (hr != S_FALSE)
				break;
			Sleep(0);
		}
	}

	// 硬解纹理与渲染设备不同：在解码器设备上 VP，结果写到共享 rgba
	bool ProcessHwFrame(SGpuSlot& slot, ID3D11Device* renderDevice,
		ID3D11Texture2D* hwTexture, UINT arraySlice)
	{
		if (!renderDevice || !hwTexture || !slot.rgbaTex)
			return false;

		Microsoft::WRL::ComPtr<ID3D11Device> hwDevice;
		hwTexture->GetDevice(hwDevice.GetAddressOf());
		if (!hwDevice)
			return false;

		if (hwDevice.Get() == renderDevice)
		{
			if (!slot.vp)
				slot.vp = std::make_unique<CD3D11VideoProcessor>(renderDevice);
			slot.vp->SetOutput(slot.rgbaTex.Get(),
				static_cast<UINT>(slot.rgbaW), static_cast<UINT>(slot.rgbaH));
			return slot.vp->Process(hwTexture, static_cast<int>(arraySlice));
		}

		if (!slot.hwVp || slot.hwVpDevice.Get() != hwDevice.Get())
		{
			slot.hwVp = std::make_unique<CD3D11VideoProcessor>(hwDevice);
			slot.hwVpDevice = hwDevice;
			slot.hwOpenedRgba.Reset();
			slot.hwFlushQuery.Reset();
		}

		if (!slot.hwOpenedRgba)
		{
			Microsoft::WRL::ComPtr<IDXGIResource> dxgiRes;
			if (FAILED(slot.rgbaTex.As(&dxgiRes)))
				return false;
			HANDLE sharedHandle = nullptr;
			if (FAILED(dxgiRes->GetSharedHandle(&sharedHandle)) || !sharedHandle)
				return false;
			if (FAILED(hwDevice->OpenSharedResource(sharedHandle, __uuidof(ID3D11Texture2D),
				reinterpret_cast<void**>(slot.hwOpenedRgba.GetAddressOf()))))
				return false;
		}

		slot.hwVp->SetOutput(slot.hwOpenedRgba.Get(),
			static_cast<UINT>(slot.rgbaW), static_cast<UINT>(slot.rgbaH));
		if (!slot.hwVp->Process(hwTexture, static_cast<int>(arraySlice)))
			return false;

		Microsoft::WRL::ComPtr<ID3D11DeviceContext> hwCtx;
		hwDevice->GetImmediateContext(hwCtx.GetAddressOf());
		WaitGpuIdle(hwDevice.Get(), hwCtx.Get(), slot.hwFlushQuery);
		return true;
	}

	bool EnsureRgba(SGpuSlot& slot, ID3D11Device* device, int w, int h)
	{
		if (w <= 0 || h <= 0 || !device)
			return false;
		if (slot.rgbaTex && slot.rgbaW == w && slot.rgbaH == h)
			return true;

		slot.rgbaSrv.Reset();
		slot.rgbaTex.Reset();
		slot.stagingUpload.Reset();
		slot.hwOpenedRgba.Reset();

		D3D11_TEXTURE2D_DESC td = {};
		td.Width = static_cast<UINT>(w);
		td.Height = static_cast<UINT>(h);
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.Usage = D3D11_USAGE_DEFAULT;
		// SHARED：供解码器设备上的 VideoProcessor 写入
		td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		td.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
		if (FAILED(device->CreateTexture2D(&td, nullptr, &slot.rgbaTex)))
			return false;
		if (FAILED(device->CreateShaderResourceView(slot.rgbaTex.Get(), nullptr, &slot.rgbaSrv)))
			return false;

		td.MiscFlags = 0;
		td.Usage = D3D11_USAGE_DYNAMIC;
		td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		device->CreateTexture2D(&td, nullptr, &slot.stagingUpload);

		slot.rgbaW = w;
		slot.rgbaH = h;
		if (!slot.vp)
			slot.vp = std::make_unique<CD3D11VideoProcessor>(device);
		slot.vp->SetOutput(slot.rgbaTex.Get(), static_cast<UINT>(w), static_cast<UINT>(h));
		return true;
	}

	bool OpenGpuSlot(SGpuSlot& slot, const QString& path, bool bPreferHard, ID3D11Device* device)
	{
		CloseGpuSlot(slot);
		slot.decoder = new CMMDecoder();
		const QByteArray utf8 = path.toUtf8();
		// 非硬解优先时直接软解，避免 EDT_AUTO 再次打开硬解却无法上屏
		const EDecoderType type = bPreferHard ? EDT_HARDWARD : EDT_SOFTWARW;
		const UINT adapterId = DeviceResource_GetAdapterId(0);
		bool ok = slot.decoder->Init(utf8.constData(), true, true, type, adapterId, false, nullptr,
			false, std::chrono::milliseconds(0), 0);
		if (!ok && bPreferHard)
		{
			delete slot.decoder;
			slot.decoder = new CMMDecoder();
			ok = slot.decoder->Init(utf8.constData(), true, true, EDT_SOFTWARW, adapterId, false, nullptr);
		}
		if (!ok)
		{
			delete slot.decoder;
			slot.decoder = nullptr;
			return false;
		}
		slot.width = static_cast<int>(slot.decoder->GetWidth());
		slot.height = static_cast<int>(slot.decoder->GetHeight());
		slot.path = path;
		slot.lastPtsMs = -1;
		EnsureRgba(slot, device, slot.width, slot.height);
		return true;
	}

	bool PumpGpuSlot(SGpuSlot& slot, qint64 clockMs, ID3D11DeviceContext* ctx,
		ID3D11ShaderResourceView** outSrv, ID3D11Texture2D** outTex,
		int& outW, int& outH, qint64& outPtsMs)
	{
		if (outSrv) *outSrv = nullptr;
		if (outTex) *outTex = nullptr;
		outW = outH = 0;
		outPtsMs = -1;
		if (!slot.decoder)
			return false;

		AVFrame* pFrame = nullptr;
		const bool bSeeking = slot.decoder->IsSeeking();
		const qint64 seekTarget = (bSeeking && slot.seekTargetMs >= 0) ? slot.seekTargetMs : clockMs;
		// 约 2 帧容差：未到目标的关键帧继续丢，直到接近 seek 点才 FinishSeek/Present
		const qint64 kSeekSlackMs = 80;
		const int maxPull = bSeeking ? 64 : 8;
		for (int i = 0; i < maxPull; ++i)
		{
			AVFrame* f = nullptr;
			if (slot.pending)
			{
				f = slot.pending;
				slot.pending = nullptr;
			}
			else
			{
				f = slot.decoder->GetVideoFrame();
			}
			if (!f)
				break;
			// 关键帧/过早帧：丢弃，继续解到接近目标（否则暂停 Seek 会停在错误画面）
			if (bSeeking && f->pts + kSeekSlackMs < seekTarget)
			{
				av_frame_free(&f);
				continue;
			}
			// 超过时钟：留到时钟追上再画。暂停时时钟不动，所以不会播出目标右侧。
			if (f->pts > clockMs)
			{
				ReleasePendingFrame(slot);
				slot.pending = f;
				break;
			}
			if (pFrame)
				av_frame_free(&pFrame);
			pFrame = f;
		}
		// 解码器会丢掉目标前的帧，空白处第一次 Seek 的首帧常落在时钟右侧一帧内。
		// 尚无不超过时钟的帧时，把这帧当作命中帧上传；播放中已有画面则仍等时钟追上。
		if (!pFrame && slot.pending)
		{
			const bool bNoPictureYet = slot.lastPtsMs < 0;
			if ((bSeeking || bNoPictureYet) && slot.pending->pts <= clockMs + kSeekSlackMs)
			{
				pFrame = slot.pending;
				slot.pending = nullptr;
			}
		}
		if (!pFrame)
		{
			// Prime 已上传、本轮只剩下更晚的帧：把已有纹理交回，避免窗口 PresentBlack
			if (slot.lastPtsMs >= 0 && slot.rgbaSrv)
			{
				if (outSrv) *outSrv = slot.rgbaSrv.Get();
				if (outTex) *outTex = slot.rgbaTex.Get();
				outW = slot.rgbaW;
				outH = slot.rgbaH;
				outPtsMs = slot.lastPtsMs;
			}
			return false;
		}

		outPtsMs = pFrame->pts;
		if (outPtsMs == slot.lastPtsMs)
		{
			av_frame_free(&pFrame);
			if (slot.rgbaSrv)
			{
				if (outSrv) *outSrv = slot.rgbaSrv.Get();
				if (outTex) *outTex = slot.rgbaTex.Get();
				outW = slot.rgbaW;
				outH = slot.rgbaH;
			}
			return false;
		}

		const int fw = slot.width > 0 ? slot.width : pFrame->width;
		const int fh = slot.height > 0 ? slot.height : pFrame->height;
		ID3D11Device* device = nullptr;
		if (ctx)
			ctx->GetDevice(&device);
		Microsoft::WRL::ComPtr<ID3D11Device> deviceHold(device);
		if (device)
			device->Release();
		if (!EnsureRgba(slot, deviceHold.Get(), fw, fh))
		{
			av_frame_free(&pFrame);
			return false;
		}

		const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(static_cast<AVPixelFormat>(pFrame->format));
		bool bOk = false;
		bool bHwFrame = desc && (desc->flags & AV_PIX_FMT_FLAG_HWACCEL) && pFrame->data[3];
		if (bHwFrame)
		{
			auto* srcSurface = reinterpret_cast<ID3D11VideoDecoderOutputView*>(
				reinterpret_cast<uintptr_t>(pFrame->data[3]));
			D3D11_VIDEO_DECODER_OUTPUT_VIEW_DESC viewDesc = {};
			srcSurface->GetDesc(&viewDesc);
			Microsoft::WRL::ComPtr<ID3D11Texture2D> pHWTexture;
			srcSurface->GetResource(reinterpret_cast<ID3D11Resource**>(pHWTexture.GetAddressOf()));
			if (pHWTexture && deviceHold)
			{
				bOk = ProcessHwFrame(slot, deviceHold.Get(), pHWTexture.Get(),
					viewDesc.Texture2D.ArraySlice);
			}

			// 硬解帧无法转换到预览纹理 → 改软解重开，避免一直黑屏
			if (!bOk && deviceHold)
			{
				av_frame_free(&pFrame);
				const QString path = slot.path;
				const QUuid clipId = slot.clipId;
				if (!OpenGpuSlot(slot, path, false, deviceHold.Get()))
					return false;
				slot.clipId = clipId;
				return false;
			}
		}
		else if (pFrame->data[0] && slot.stagingUpload && ctx)
		{
			slot.sws = sws_getCachedContext(slot.sws,
				pFrame->width, pFrame->height, static_cast<AVPixelFormat>(pFrame->format),
				slot.rgbaW, slot.rgbaH, AV_PIX_FMT_BGRA,
				SWS_BILINEAR, nullptr, nullptr, nullptr);
			std::vector<uint8_t> bgra(static_cast<size_t>(slot.rgbaW) * slot.rgbaH * 4);
			uint8_t* dstSlice[4] = { bgra.data(), nullptr, nullptr, nullptr };
			int dstStride[4] = { slot.rgbaW * 4, 0, 0, 0 };
			if (slot.sws && sws_scale(slot.sws, pFrame->data, pFrame->linesize, 0, pFrame->height,
				dstSlice, dstStride) > 0)
			{
				D3D11_MAPPED_SUBRESOURCE mapped = {};
				if (SUCCEEDED(ctx->Map(slot.stagingUpload.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
				{
					for (int y = 0; y < slot.rgbaH; ++y)
						memcpy(static_cast<uint8_t*>(mapped.pData) + y * mapped.RowPitch,
							bgra.data() + static_cast<size_t>(y) * slot.rgbaW * 4,
							static_cast<size_t>(slot.rgbaW) * 4);
					ctx->Unmap(slot.stagingUpload.Get(), 0);
					ctx->CopyResource(slot.rgbaTex.Get(), slot.stagingUpload.Get());
					bOk = true;
				}
			}
		}

		av_frame_free(&pFrame);
		if (!bOk)
			return false;

		slot.lastPtsMs = outPtsMs;
		// 接近 seek 目标的帧上屏后再 FinishSeek
		if (slot.decoder && slot.decoder->IsSeeking())
		{
			const qint64 target = slot.seekTargetMs >= 0 ? slot.seekTargetMs : outPtsMs;
			if (outPtsMs + 80 >= target)
				slot.decoder->FinishSeek();
		}
		if (outSrv) *outSrv = slot.rgbaSrv.Get();
		if (outTex) *outTex = slot.rgbaTex.Get();
		outW = slot.rgbaW;
		outH = slot.rgbaH;
		return true;
	}

	SGpuSlot* FindSlot(std::vector<SGpuSlot>& gpuSlots, const QUuid& clipId)
	{
		for (auto& s : gpuSlots)
		{
			if (s.clipId == clipId)
				return &s;
		}
		return nullptr;
	}
}

using PreviewGpuDetail::SGpuSlot;
using PreviewGpuDetail::CloseGpuSlot;
using PreviewGpuDetail::OpenGpuSlot;
using PreviewGpuDetail::PumpGpuSlot;
using PreviewGpuDetail::FindSlot;
using PreviewGpuDetail::ReleasePendingFrame;

struct CPreviewGpuPipeline::Impl
{
	std::vector<PreviewGpuDetail::SGpuSlot> gpuSlots;
	std::unique_ptr<BasicEffect> basicEffect;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> ctx;
};

CPreviewGpuPipeline& CPreviewGpuPipeline::Instance()
{
	static CPreviewGpuPipeline s;
	return s;
}

CPreviewGpuPipeline::CPreviewGpuPipeline()
	: m_pImpl(new Impl)
{
}

CPreviewGpuPipeline::~CPreviewGpuPipeline()
{
	CloseLayers();
	UninitDevice();
	delete m_pImpl;
	m_pImpl = nullptr;
}

bool CPreviewGpuPipeline::InitDevice()
{
	if (m_bDeviceReady)
		return true;

	SDeviceResourceInitPrefer prefer;
	UINT gpuIndex = 0;
	DXGI_ADAPTER_DESC2 adapterDesc = {};
	if (DeviceResource_GetBestAdapterForRendering(gpuIndex, &adapterDesc))
	{
		prefer.bHasLuid = true;
		prefer.luid = adapterDesc.AdapterLuid;
		prefer.deviceId = adapterDesc.DeviceId;
	}
	if (FAILED(DeviceResource_InitPrefer(prefer)))
		return false;

	m_bDeviceReady = (GetDevice() != nullptr);
	if (m_bDeviceReady && GetDevice())
	{
		m_pImpl->basicEffect = std::make_unique<BasicEffect>();
		if (!m_pImpl->basicEffect->InitAll(GetDevice()))
		{
			OutputDebugStringW(L"[PreviewGpu] BasicEffect::InitAll failed (HLSL missing?)\n");
			m_pImpl->basicEffect.reset();
		}
	}
	return m_bDeviceReady;
}

void CPreviewGpuPipeline::UninitDevice()
{
	CloseLayers();
	m_pImpl->basicEffect.reset();
	m_pImpl->ctx.Reset();
	if (m_bDeviceReady)
	{
		DeviceResource_UnInit();
		m_bDeviceReady = false;
	}
}

BasicEffect* CPreviewGpuPipeline::GetBasicEffect() const
{
	return m_pImpl ? m_pImpl->basicEffect.get() : nullptr;
}

ID3D11Device* CPreviewGpuPipeline::GetDevice() const
{
	return DeviceResource_GetD3DDevice(0, Device_Decode);
}

ID3D11DeviceContext* CPreviewGpuPipeline::GetContext()
{
	if (m_pImpl->ctx)
		return m_pImpl->ctx.Get();
	ID3D11Device* dev = GetDevice();
	if (!dev)
		return nullptr;
	dev->GetImmediateContext(&m_pImpl->ctx);
	return m_pImpl->ctx.Get();
}

bool CPreviewGpuPipeline::Open(const QString& path, bool bPreferHard)
{
	SPreviewGpuLayerDesc d;
	d.path = path;
	return SyncLayers({ d }, bPreferHard);
}

void CPreviewGpuPipeline::Close()
{
	CloseLayers();
}

bool CPreviewGpuPipeline::IsOpen() const
{
	std::lock_guard<std::mutex> lock(m_mtx);
	return !m_pImpl->gpuSlots.empty() && m_pImpl->gpuSlots.front().decoder != nullptr;
}

bool CPreviewGpuPipeline::SeekMs(qint64 ms)
{
	std::lock_guard<std::mutex> lock(m_mtx);
	if (m_pImpl->gpuSlots.empty() || !m_pImpl->gpuSlots.front().decoder)
		return false;
	m_pImpl->gpuSlots.front().decoder->Seek(ms < 0 ? 0 : ms, true);
	m_pImpl->gpuSlots.front().lastPtsMs = -1;
	m_pImpl->gpuSlots.front().seekTargetMs = ms < 0 ? 0 : ms;
	ReleasePendingFrame(m_pImpl->gpuSlots.front());
	return true;
}

void CPreviewGpuPipeline::SetPlaying(bool playing)
{
	std::lock_guard<std::mutex> lock(m_mtx);
	for (auto& s : m_pImpl->gpuSlots)
		s.bPlaying = playing;
}

int CPreviewGpuPipeline::Width() const
{
	std::lock_guard<std::mutex> lock(m_mtx);
	return m_pImpl->gpuSlots.empty() ? 0 : m_pImpl->gpuSlots.front().width;
}

int CPreviewGpuPipeline::Height() const
{
	std::lock_guard<std::mutex> lock(m_mtx);
	return m_pImpl->gpuSlots.empty() ? 0 : m_pImpl->gpuSlots.front().height;
}

QString CPreviewGpuPipeline::Path() const
{
	std::lock_guard<std::mutex> lock(m_mtx);
	return m_pImpl->gpuSlots.empty() ? QString() : m_pImpl->gpuSlots.front().path;
}

ID3D11Texture2D* CPreviewGpuPipeline::GetRgbaTexture() const
{
	std::lock_guard<std::mutex> lock(m_mtx);
	return m_pImpl->gpuSlots.empty() ? nullptr : m_pImpl->gpuSlots.front().rgbaTex.Get();
}

void CPreviewGpuPipeline::CloseLayers()
{
	std::lock_guard<std::mutex> lock(m_mtx);
	for (auto& s : m_pImpl->gpuSlots)
		CloseGpuSlot(s);
	m_pImpl->gpuSlots.clear();
}

int CPreviewGpuPipeline::LayerCount() const
{
	std::lock_guard<std::mutex> lock(m_mtx);
	return static_cast<int>(m_pImpl->gpuSlots.size());
}

bool CPreviewGpuPipeline::SyncLayers(const std::vector<SPreviewGpuLayerDesc>& layers, bool bPreferHard)
{
	std::lock_guard<std::mutex> lock(m_mtx);
	if (!m_bDeviceReady && !InitDevice())
		return false;
	ID3D11Device* device = GetDevice();
	if (!device)
		return false;

	std::vector<SPreviewGpuLayerDesc> want = layers;
	if (static_cast<int>(want.size()) > kMaxSlots)
		want.erase(want.begin(), want.end() - kMaxSlots);

	std::vector<SGpuSlot> oldSlots = std::move(m_pImpl->gpuSlots);
	m_pImpl->gpuSlots.clear();
	std::vector<SGpuSlot> next;
	next.reserve(want.size());
	std::vector<char> taken(oldSlots.size(), 0);

	for (const auto& d : want)
	{
		int reuseIdx = -1;
		for (size_t i = 0; i < oldSlots.size(); ++i)
		{
			if (taken[i] || !oldSlots[i].decoder)
				continue;
			if (oldSlots[i].clipId == d.clipId && oldSlots[i].path == d.path)
			{
				reuseIdx = static_cast<int>(i);
				break;
			}
		}
		if (reuseIdx >= 0)
		{
			taken[reuseIdx] = 1;
			next.push_back(std::move(oldSlots[reuseIdx]));
			if (next.back().decoder)
			{
				next.back().decoder->Seek(d.seekMs < 0 ? 0 : d.seekMs, true);
				next.back().lastPtsMs = -1;
				next.back().seekTargetMs = d.seekMs < 0 ? 0 : d.seekMs;
				ReleasePendingFrame(next.back());
			}
		}
		else
		{
			SGpuSlot s;
			s.clipId = d.clipId;
			if (!OpenGpuSlot(s, d.path, bPreferHard, device))
				continue;
			if (d.seekMs > 0 && s.decoder)
			{
				s.decoder->Seek(d.seekMs, true);
				s.lastPtsMs = -1;
				s.seekTargetMs = d.seekMs;
				ReleasePendingFrame(s);
			}
			next.push_back(std::move(s));
		}
	}

	for (size_t i = 0; i < oldSlots.size(); ++i)
	{
		if (!taken[i])
			CloseGpuSlot(oldSlots[i]);
	}
	m_pImpl->gpuSlots = std::move(next);
	return !m_pImpl->gpuSlots.empty();
}

bool CPreviewGpuPipeline::SeekLayer(const QUuid& clipId, qint64 ms)
{
	std::lock_guard<std::mutex> lock(m_mtx);
	SGpuSlot* s = FindSlot(m_pImpl->gpuSlots, clipId);
	if (!s && !m_pImpl->gpuSlots.empty() && m_pImpl->gpuSlots.front().clipId.isNull())
		s = &m_pImpl->gpuSlots.front();
	if (!s || !s->decoder)
		return false;
	s->decoder->Seek(ms < 0 ? 0 : ms, true);
	s->lastPtsMs = -1;
	s->seekTargetMs = ms < 0 ? 0 : ms;
	ReleasePendingFrame(*s);
	return true;
}

void CPreviewGpuPipeline::FinishSeekLayer(const QUuid& clipId)
{
	std::lock_guard<std::mutex> lock(m_mtx);
	SGpuSlot* s = FindSlot(m_pImpl->gpuSlots, clipId);
	if (!s && !m_pImpl->gpuSlots.empty() && m_pImpl->gpuSlots.front().clipId.isNull())
		s = &m_pImpl->gpuSlots.front();
	if (s && s->decoder && s->decoder->IsSeeking())
		s->decoder->FinishSeek();
}

void CPreviewGpuPipeline::FinishSeekAll()
{
	std::lock_guard<std::mutex> lock(m_mtx);
	for (auto& s : m_pImpl->gpuSlots)
	{
		if (s.decoder && s.decoder->IsSeeking())
			s.decoder->FinishSeek();
	}
}

bool CPreviewGpuPipeline::IsAnySeeking() const
{
	std::lock_guard<std::mutex> lock(m_mtx);
	for (const auto& s : m_pImpl->gpuSlots)
	{
		if (s.decoder && s.decoder->IsSeeking())
			return true;
	}
	return false;
}

bool CPreviewGpuPipeline::PrimeSeekFrames(int maxAttempts, int sleepMs)
{
	bool bAnyNew = false;
	for (int attempt = 0; attempt < maxAttempts; ++attempt)
	{
		bool bStillSeeking = false;
		{
			std::lock_guard<std::mutex> lock(m_mtx);
			ID3D11DeviceContext* ctx = GetContext();
			for (auto& s : m_pImpl->gpuSlots)
			{
				if (!s.decoder)
					continue;
				ID3D11ShaderResourceView* srv = nullptr;
				int fw = 0, fh = 0;
				qint64 pts = -1;
				// 时钟固定在 Seek 目标，不往 lastPts+1 垫下一帧
				qint64 clock = 0;
				if (s.seekTargetMs >= 0)
					clock = s.seekTargetMs;
				else if (s.lastPtsMs >= 0)
					clock = s.lastPtsMs;
				if (PumpGpuSlot(s, clock, ctx, &srv, nullptr, fw, fh, pts))
					bAnyNew = true;
				if (s.decoder->IsSeeking())
					bStillSeeking = true;
			}
		}
		if (!bStillSeeking)
			break;
		if (sleepMs > 0)
			Sleep(static_cast<DWORD>(sleepMs));
	}
	return bAnyNew;
}

void CPreviewGpuPipeline::SetLayersPlaying(bool playing)
{
	SetPlaying(playing);
}

bool CPreviewGpuPipeline::PumpFrame(qint64 clockMs,
	ID3D11ShaderResourceView** outSrv, int& outW, int& outH, qint64& outPtsMs)
{
	std::lock_guard<std::mutex> lock(m_mtx);
	if (m_pImpl->gpuSlots.empty())
	{
		if (outSrv) *outSrv = nullptr;
		outW = outH = 0;
		outPtsMs = -1;
		return false;
	}
	return PumpGpuSlot(m_pImpl->gpuSlots.front(), clockMs, GetContext(),
		outSrv, nullptr, outW, outH, outPtsMs);
}

bool CPreviewGpuPipeline::PumpLayer(const QUuid& clipId, qint64 clockMs,
	ID3D11ShaderResourceView** outSrv, ID3D11Texture2D** outTex,
	int& outW, int& outH, qint64& outPtsMs)
{
	std::lock_guard<std::mutex> lock(m_mtx);
	SGpuSlot* s = FindSlot(m_pImpl->gpuSlots, clipId);
	if (!s && !m_pImpl->gpuSlots.empty() && m_pImpl->gpuSlots.front().clipId.isNull())
		s = &m_pImpl->gpuSlots.front();
	if (!s)
	{
		if (outSrv) *outSrv = nullptr;
		if (outTex) *outTex = nullptr;
		outW = outH = 0;
		outPtsMs = -1;
		return false;
	}
	return PumpGpuSlot(*s, clockMs, GetContext(), outSrv, outTex, outW, outH, outPtsMs);
}

ID3D11Texture2D* CPreviewGpuPipeline::GetLayerTexture(const QUuid& clipId) const
{
	std::lock_guard<std::mutex> lock(m_mtx);
	for (const auto& s : m_pImpl->gpuSlots)
	{
		if (s.clipId == clipId)
			return s.rgbaTex.Get();
	}
	if (!m_pImpl->gpuSlots.empty() && m_pImpl->gpuSlots.front().clipId.isNull())
		return m_pImpl->gpuSlots.front().rgbaTex.Get();
	return nullptr;
}
