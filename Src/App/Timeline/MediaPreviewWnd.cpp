#include "MediaPreviewWnd.h"
#include "PreviewDecodeSession.h"
#include "PreviewGpuPipeline.h"
#include "PreviewFxPass.h"
#include "TimelinePreviewBridge.h"
#include "GSMedia/RenderDevice/DeviceResource.h"
#include "GSMedia/RenderCommon/Vertex.h"
#include "Controll/PlayerController/PlayerControllerExport.h"

#include <QShowEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QPalette>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <vector>

#ifdef _MSC_VER
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#endif

// DeviceResource.h 已 using ComPtr = Microsoft::WRL::ComPtr

namespace
{
	// 预览色调：亮度/对比度/色相/饱和度 + RGBA
	struct alignas(16) SFxCb
	{
		float lumin, contrast, hue, saturability;
		float r, g, b, a;
	};

	struct SVertex
	{
		float x, y;
		float u, v;
	};

	static const char* kShaderSrc = R"(
cbuffer FxCb : register(b0)
{
	float lumin;
	float contrast;
	float hue;
	float saturability;
	float4 rgba;
};
Texture2D tex : register(t0);
SamplerState samp : register(s0);
struct VSIn { float2 pos : POSITION; float2 uv : TEXCOORD0; };
struct VSOut { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };
VSOut VSMain(VSIn i)
{
	VSOut o;
	o.pos = float4(i.pos, 0.0, 1.0);
	o.uv = i.uv;
	return o;
}
float3 rgb2hsl(float3 rgb)
{
	float r = rgb.r, g = rgb.g, b = rgb.b;
	float K = 0.f;
	if (g < b) { float tmp = g; g = b; b = tmp; K = -1.f; }
	if (r < g) { float tmp = r; r = g; g = tmp; K = -2.f / 6.f - K; }
	const float chroma = r - (g < b ? g : b);
	return float3(360.0f * abs(K + (g - b) / (6.f * chroma + 1e-20f)), chroma / (r + 1e-20f), r);
}
float3 hsl2rgb(float3 hsl)
{
	float h = hsl.x / 360.0f, s = hsl.y, v = hsl.z;
	if (s == 0.0f) return float3(v, v, v);
	h = fmod(h, 1.0f) / (60.0f / 360.0f);
	int i = (int)h;
	float f = h - (float)i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - s * f);
	float t = v * (1.0f - s * (1.0f - f));
	float out_r = 0, out_g = 0, out_b = 0;
	switch (i)
	{
	case 0: out_r = v; out_g = t; out_b = p; break;
	case 1: out_r = q; out_g = v; out_b = p; break;
	case 2: out_r = p; out_g = v; out_b = t; break;
	case 3: out_r = p; out_g = q; out_b = v; break;
	case 4: out_r = t; out_g = p; out_b = v; break;
	default: out_r = v; out_g = p; out_b = q; break;
	}
	return float3(out_r, out_g, out_b);
}
float4 rgbaAdj(float4 colorRGBA, float4 scaleRgba)
{
	float4x4 adj = float4x4(
		scaleRgba.r, 0, 0, 0,
		0, scaleRgba.g, 0, 0,
		0, 0, scaleRgba.b, 0,
		0, 0, 0, scaleRgba.a);
	return mul(colorRGBA, adj);
}
float3 contrastAdj(float3 colorRGB, float c)
{
	const float3 addv = float3(0.5f, 0.5f, 0.5f);
	return pow(abs(colorRGB + addv), (c + 1)) - addv;
}
float3 hsvAdj(float3 rgb, float hueAmt, float satAmt, float luminAmt)
{
	float maxRGB = max(max(rgb.r, max(rgb.g, rgb.b)), 1.0f);
	float3 hsl = rgb2hsl(rgb);
	float H = fmod(hsl.x + 180.0 * hueAmt + 360.0, 360.0);
	float S = saturate(hsl.y + hsl.y * satAmt);
	float L = saturate(hsl.z + hsl.z * luminAmt);
	return hsl2rgb(float3(H, S, L)) * maxRGB;
}
float4 colorPtyAdjUnit(float4 cellRGBA, float luminAmt, float hueAmt, float satAmt, float4 mulRgba, float contrastAmt)
{
	float4 ret = rgbaAdj(cellRGBA, mulRgba);
	ret.rgb = contrastAdj(ret.rgb, contrastAmt);
	ret.rgb = hsvAdj(ret.rgb, hueAmt, satAmt, luminAmt);
	return ret;
}
float4 PSMain(VSOut i) : SV_Target
{
	float4 c = tex.Sample(samp, i.uv);
	return colorPtyAdjUnit(c, lumin, hue, saturability, rgba, contrast);
}
)";
}

struct CMediaPreviewWnd::DxState
{
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> ctx;
	ComPtr<IDXGISwapChain> swapChain;
	ComPtr<ID3D11RenderTargetView> rtv;

	ComPtr<ID3D11Texture2D> staging;
	ComPtr<ID3D11Texture2D> frameTex;
	ComPtr<ID3D11ShaderResourceView> srv;
	ID3D11ShaderResourceView* extSrv = nullptr; // 硬解管线持有，勿 Release
	int texW = 0;
	int texH = 0;

	ComPtr<ID3D11VertexShader> vs;
	ComPtr<ID3D11PixelShader> ps;
	ComPtr<ID3D11InputLayout> layout;
	ComPtr<ID3D11Buffer> vb;
	ComPtr<ID3D11Buffer> ib;
	ComPtr<ID3D11Buffer> cb;
	ComPtr<ID3D11SamplerState> samp;
	ComPtr<ID3D11BlendState> blend;
	ComPtr<ID3D11RasterizerState> rs;
	UINT ibCapacity = 0;
	bool bPipeline = false;
};

CMediaPreviewWnd::CMediaPreviewWnd(QWidget* parent)
	: QWidget(parent)
	, m_pSession(new CPreviewDecodeSession)
{
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NativeWindow);
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setMinimumSize(160, 90);

	QPalette pal = palette();
	const QColor bg(13, 13, 15);
	pal.setColor(QPalette::Window, bg);
	pal.setColor(QPalette::Base, bg);
	setPalette(pal);
	setAutoFillBackground(true);

	m_timer.setTimerType(Qt::PreciseTimer);
	connect(&m_timer, &QTimer::timeout, this, &CMediaPreviewWnd::OnTick);

	m_primeRetryTimer.setSingleShot(true);
	connect(&m_primeRetryTimer, &QTimer::timeout, this, [this]() {
		if (m_bPlaying || m_primeRetryLeft <= 0)
			return;
		// 已有更新的 Seek：丢弃本轮 retry，避免画出上一目标帧
		if (m_retryEpoch != m_seekEpoch)
			return;
		--m_primeRetryLeft;
		bool bGot = false;
		if (m_bUseHwPipeline)
			bGot = PullLayersAtClock();
		else
		{
			const qint64 prev = m_lastPresentedPts;
			m_lastPresentedPts = -1;
			bGot = PullFrameAtClock(m_previewClockMs);
			if (!bGot)
				m_lastPresentedPts = prev;
		}
		if (m_retryEpoch != m_seekEpoch)
			return;

		constexpr qint64 kNearMs = 80;
		const bool bNearTarget = bGot && m_lastPresentedPts >= 0
			&& std::llabs(m_lastPresentedPts - m_previewClockMs) <= kNearMs;

		if (bGot && bNearTarget)
			PresentLastFrame();

		const bool bStillSeeking = m_bUseHwPipeline
			? CPreviewGpuPipeline::Instance().IsAnySeeking()
			: (m_pSession && m_pSession->IsSeeking());
		// 必须当前目标帧就绪才停；上一 Seek 的帧即使 bGot 也继续等
		if (bNearTarget && !bStillSeeking)
			return;
		if (m_primeRetryLeft <= 0)
		{
			if (m_retryEpoch != m_seekEpoch)
				return;
			if (m_bUseHwPipeline && CPreviewGpuPipeline::Instance().IsAnySeeking())
				CPreviewGpuPipeline::Instance().FinishSeekAll();
			else if (m_pSession && m_pSession->IsSeeking())
				m_pSession->FinishSeek();
			if (bGot)
				PresentLastFrame();
			return;
		}
		m_primeRetryTimer.start(16);
	});
}

CMediaPreviewWnd::~CMediaPreviewWnd()
{
	m_primeRetryTimer.stop();
	m_timer.stop();
	delete m_pSession;
	m_pSession = nullptr;
	delete m_pDx;
	m_pDx = nullptr;
}

void CMediaPreviewWnd::SetPreviewBridge(CTimelinePreviewBridge* bridge)
{
	m_pBridge = bridge;
}

bool CMediaPreviewWnd::EnsurePipeline()
{
	if (!m_pDx || !m_pDx->device)
		return false;
	if (m_pDx->bPipeline)
		return true;

	ComPtr<ID3DBlob> vsBlob, psBlob, err;
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
#endif
	if (FAILED(D3DCompile(kShaderSrc, strlen(kShaderSrc), nullptr, nullptr, nullptr,
		"VSMain", "vs_4_0", flags, 0, &vsBlob, &err)))
		return false;
	if (FAILED(D3DCompile(kShaderSrc, strlen(kShaderSrc), nullptr, nullptr, nullptr,
		"PSMain", "ps_4_0", flags, 0, &psBlob, &err)))
		return false;

	if (FAILED(m_pDx->device->CreateVertexShader(vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(), nullptr, &m_pDx->vs)))
		return false;
	if (FAILED(m_pDx->device->CreatePixelShader(psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(), nullptr, &m_pDx->ps)))
		return false;

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	if (FAILED(m_pDx->device->CreateInputLayout(layoutDesc, 2, vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(), &m_pDx->layout)))
		return false;

	D3D11_BUFFER_DESC vbd = {};
	vbd.ByteWidth = sizeof(SVertex) * 4096;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(m_pDx->device->CreateBuffer(&vbd, nullptr, &m_pDx->vb)))
		return false;

	D3D11_BUFFER_DESC ibd = {};
	ibd.ByteWidth = sizeof(UINT) * 8192;
	ibd.Usage = D3D11_USAGE_DYNAMIC;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(m_pDx->device->CreateBuffer(&ibd, nullptr, &m_pDx->ib)))
		return false;
	m_pDx->ibCapacity = 8192;

	D3D11_BUFFER_DESC cbd = {};
	cbd.ByteWidth = sizeof(SFxCb);
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(m_pDx->device->CreateBuffer(&cbd, nullptr, &m_pDx->cb)))
		return false;

	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	if (FAILED(m_pDx->device->CreateSamplerState(&sd, &m_pDx->samp)))
		return false;

	D3D11_BLEND_DESC bl = {};
	bl.RenderTarget[0].BlendEnable = TRUE;
	bl.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bl.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bl.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bl.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bl.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bl.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bl.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	if (FAILED(m_pDx->device->CreateBlendState(&bl, &m_pDx->blend)))
		return false;

	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_NONE;
	rd.DepthClipEnable = TRUE;
	if (FAILED(m_pDx->device->CreateRasterizerState(&rd, &m_pDx->rs)))
		return false;

	m_pDx->bPipeline = true;
	return true;
}

bool CMediaPreviewWnd::EnsureDx()
{
	if (m_pDx && m_pDx->swapChain)
		return EnsurePipeline();

	winId();
	HWND hwnd = reinterpret_cast<HWND>(winId());
	if (!hwnd)
		return false;

	if (!m_pDx)
		m_pDx = new DxState;

	// 优先与硬解共用 DeviceResource 设备，否则无法绑定 D3D11VA 输出纹理
	ID3D11Device* sharedDev = CPreviewGpuPipeline::Instance().GetDevice();
	HRESULT hr = E_FAIL;
	if (sharedDev)
	{
		ComPtr<IDXGIDevice> dxgiDev;
		hr = sharedDev->QueryInterface(IID_PPV_ARGS(&dxgiDev));
		ComPtr<IDXGIAdapter> adapter;
		if (SUCCEEDED(hr))
			hr = dxgiDev->GetAdapter(&adapter);
		ComPtr<IDXGIFactory> factory;
		if (SUCCEEDED(hr))
			hr = adapter->GetParent(IID_PPV_ARGS(&factory));

		DXGI_SWAP_CHAIN_DESC scd = {};
		scd.BufferCount = 2;
		scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		scd.BufferDesc.Width = static_cast<UINT>(std::max(1, width()));
		scd.BufferDesc.Height = static_cast<UINT>(std::max(1, height()));
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow = hwnd;
		scd.SampleDesc.Count = 1;
		scd.Windowed = TRUE;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		if (SUCCEEDED(hr) && factory)
			hr = factory->CreateSwapChain(sharedDev, &scd, &m_pDx->swapChain);
		if (SUCCEEDED(hr))
		{
			m_pDx->device = sharedDev;
			sharedDev->GetImmediateContext(&m_pDx->ctx);
		}
	}

	if (!m_pDx->swapChain)
	{
		DXGI_SWAP_CHAIN_DESC scd = {};
		scd.BufferCount = 2;
		scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		scd.BufferDesc.Width = static_cast<UINT>(std::max(1, width()));
		scd.BufferDesc.Height = static_cast<UINT>(std::max(1, height()));
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow = hwnd;
		scd.SampleDesc.Count = 1;
		scd.Windowed = TRUE;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		D3D_FEATURE_LEVEL featureLevel;
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
			D3D11_SDK_VERSION, &scd, &m_pDx->swapChain, &m_pDx->device, &featureLevel, &m_pDx->ctx);
		if (FAILED(hr))
			return false;
	}

	ComPtr<ID3D11Texture2D> backBuffer;
	m_pDx->swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	m_pDx->device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_pDx->rtv);
	return EnsurePipeline();
}

void CMediaPreviewWnd::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	EnsureDx();
	PresentLastFrame();
}

void CMediaPreviewWnd::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	RefreshSurface();
}

void CMediaPreviewWnd::wheelEvent(QWheelEvent* event)
{
	if (event->modifiers() & Qt::ControlModifier)
	{
		const int delta = event->angleDelta().y();
		if (delta != 0)
		{
			const float step = (delta > 0) ? 1.1f : (1.0f / 1.1f);
			m_fZoom = std::clamp(m_fZoom * step, 0.25f, 8.0f);
			m_bFxDirty = true; // 仅重画 letterbox，不重新解码
			PresentLastFrame();
		}
		event->accept();
		return;
	}
	QWidget::wheelEvent(event);
}

void CMediaPreviewWnd::RefreshSurface()
{
	if (!m_pDx || !m_pDx->swapChain)
	{
		EnsureDx();
		PresentLastFrame();
		return;
	}

	m_pDx->ctx->OMSetRenderTargets(0, nullptr, nullptr);
	m_pDx->rtv.Reset();
	m_pDx->swapChain->ResizeBuffers(0,
		static_cast<UINT>(std::max(1, width())),
		static_cast<UINT>(std::max(1, height())),
		DXGI_FORMAT_UNKNOWN, 0);

	ComPtr<ID3D11Texture2D> backBuffer;
	m_pDx->swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	m_pDx->device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_pDx->rtv);

	// Resize 后只需 GPU 重画，禁止 CPU 缩放
	m_bFxDirty = true;
	PresentLastFrame();
}

void CMediaPreviewWnd::paintEvent(QPaintEvent*)
{
	// DISCARD swapchain：expose 时用已上传 GPU 纹理重画，禁止 CPU 逐像素缩放
	if (m_bGpuHasFrame)
	{
		DrawFrameToBackBuffer(false);
		if (m_pDx && m_pDx->swapChain)
			m_pDx->swapChain->Present(0, 0);
	}
	else
	{
		PresentBlack();
	}
}

void CMediaPreviewWnd::EnsureAudioSidecar(const QString& path, qint64 seekMs, bool bPlaying)
{
	if (path.isEmpty())
	{
		if (m_pSession->IsAudioOnly())
			m_pSession->Close();
		m_audioPath.clear();
		m_lastAudioSeekMs = -1;
		return;
	}

	const qint64 targetMs = seekMs < 0 ? 0 : seekMs;
	bool bOpenedNew = false;
	if (m_audioPath != path || !m_pSession->IsOpen() || !m_pSession->IsAudioOnly())
	{
		// 软解视频占用 session 时不要抢
		if (m_pSession->IsOpen() && !m_pSession->IsAudioOnly() && !m_bUseHwPipeline)
			return;
		m_pSession->Close();
		m_lastAudioSeekMs = -1;
		if (!m_pSession->OpenAudioOnly(path))
		{
			m_audioPath.clear();
			return;
		}
		m_audioPath = path;
		bOpenedNew = true;
	}

	// 播放中大跳才 Seek；暂停 scrub 节流，避免每像素同步 Seek 卡顿
	const bool bNeedSeek = bOpenedNew
		|| m_lastAudioSeekMs < 0
		|| std::llabs(targetMs - m_lastAudioSeekMs) > (bPlaying ? 180 : 120);
	if (bNeedSeek)
		m_pSession->SeekMs(targetMs);
	m_lastAudioSeekMs = targetMs;
	m_pSession->SetPlaying(bPlaying);
}

void CMediaPreviewWnd::OpenFile(const QString& path)
{
	// 时间线播放中不打断预览（素材库选中会触发本接口，否则会 Stop 后黑屏）
	if (g_pPlayerController && g_pPlayerController->IsPlaying())
		return;

	Stop();
	EnsureDx();
	m_bUseHwPipeline = false;
	m_bMultiLayer = false;
	m_layerInfos.clear();
	m_layerFrames.clear();
	if (CPreviewGpuPipeline::Instance().IsDeviceReady()
		&& CPreviewGpuPipeline::Instance().Open(path, true))
	{
		m_bUseHwPipeline = true;
		m_openedPath = path;
		EnsureAudioSidecar(path, 0, false);
		CPreviewGpuPipeline::Instance().SeekMs(0);
		m_lastSeekMs = 0;
		m_previewClockMs = 0;
		PullFrameAtClock(0);
		PresentLastFrame();
		return;
	}

	// 无视频轨（mp3 等）：OpenAudioOnly；否则软解音视频
	if (m_pSession->OpenAudioOnly(path) || m_pSession->Open(path))
	{
		m_openedPath = m_pSession->IsAudioOnly() ? QString() : path;
		if (m_pSession->IsAudioOnly())
			m_audioPath = path;
		m_lastSeekMs = 0;
		m_lastAudioSeekMs = 0;
		m_previewClockMs = 0;
		m_pSession->SeekMs(0);
		if (m_pSession->IsAudioOnly())
			PresentBlack();
		else
		{
			PullFrameAtClock(0);
			PresentLastFrame();
		}
	}
}

void CMediaPreviewWnd::SetPreviewClockMs(qint64 clipLocalMs)
{
	m_previewClockMs = clipLocalMs < 0 ? 0 : clipLocalMs;
}

void CMediaPreviewWnd::SyncTimelineMedia(const QString& path, qint64 clipLocalMs, bool bPlaying)
{
	SPreviewLayerInfo layer;
	layer.path = path;
	layer.clipLocalMs = clipLocalMs;
	layer.color = m_previewColor;
	SyncTimelineLayers(path.isEmpty() ? std::vector<SPreviewLayerInfo>{}
		: std::vector<SPreviewLayerInfo>{ layer }, bPlaying, QUuid());
}

void CMediaPreviewWnd::SyncTimelineLayers(const std::vector<SPreviewLayerInfo>& layers, bool bPlaying,
	const QUuid& audioFocusId, bool bForceSeek)
{
	if (layers.empty())
	{
		Stop();
		return;
	}

	EnsureDx();

	std::vector<SPreviewLayerInfo> visualLayers;
	visualLayers.reserve(layers.size());
	for (const auto& L : layers)
	{
		if (L.bVisual)
			visualLayers.push_back(L);
	}

	auto sameVisualSet = [](const std::vector<SPreviewLayerInfo>& a,
		const std::vector<SPreviewLayerInfo>& b) {
		if (a.size() != b.size())
			return false;
		for (size_t i = 0; i < a.size(); ++i)
		{
			if (a[i].clipId != b[i].clipId || a[i].path != b[i].path)
				return false;
		}
		return true;
	};

	std::vector<SPreviewLayerInfo> prevVisual;
	for (const auto& L : m_layerInfos)
	{
		if (L.bVisual)
			prevVisual.push_back(L);
	}

	const bool bHwSameSet = m_bUseHwPipeline && sameVisualSet(prevVisual, visualLayers);

	QString audioPath;
	qint64 audioSeek = 0;
	for (const auto& L : layers)
	{
		if (!audioFocusId.isNull() && L.clipId == audioFocusId)
		{
			audioPath = L.path;
			audioSeek = L.clipLocalMs < 0 ? 0 : L.clipLocalMs;
			break;
		}
	}
	if (audioPath.isEmpty() && !visualLayers.empty())
	{
		audioPath = visualLayers.back().path;
		audioSeek = visualLayers.back().clipLocalMs < 0 ? 0 : visualLayers.back().clipLocalMs;
	}
	else if (audioPath.isEmpty())
	{
		audioPath = layers.back().path;
		audioSeek = layers.back().clipLocalMs < 0 ? 0 : layers.back().clipLocalMs;
	}

	m_layerInfos = layers;
	m_audioFocusId = audioFocusId;
	m_bMultiLayer = (visualLayers.size() > 1);

	const qint64 topLocal = !visualLayers.empty()
		? (visualLayers.back().clipLocalMs < 0 ? 0 : visualLayers.back().clipLocalMs)
		: audioSeek;
	m_previewClockMs = topLocal;

	// 纯音频时间线（仅 mp3 等）：关 GPU，走 sidecar / OpenAudioOnly
	if (visualLayers.empty())
	{
		CPreviewGpuPipeline::Instance().CloseLayers();
		m_bUseHwPipeline = false;
		m_bMultiLayer = false;
		m_layerFrames.clear();
		m_openedPath.clear();
		if (m_pDx)
			m_pDx->extSrv = nullptr;
		EnsureAudioSidecar(audioPath, audioSeek, bPlaying);
		PresentBlack();
		if (bPlaying)
			Play();
		else
			Pause();
		return;
	}

	const QString topPath = visualLayers.back().path;
	const bool bSoftSameSet = !m_bUseHwPipeline
		&& m_pSession->IsOpen()
		&& !m_pSession->IsAudioOnly()
		&& !topPath.isEmpty()
		&& topPath == m_openedPath;

	if (bHwSameSet)
	{
		constexpr qint64 kPlaySeekJumpMs = 180;
		bool bDidSeek = false;
		for (const auto& L : visualLayers)
		{
			const qint64 local = L.clipLocalMs < 0 ? 0 : L.clipLocalMs;
			const qint64 delta = std::llabs(local - m_lastSeekMs);
			// 暂停 + 游标强制：位置变化就 Seek，保证画面跟上
			const bool bDoSeek = (m_lastSeekMs < 0)
				|| (bForceSeek && !bPlaying && delta > 0)
				|| (bPlaying && delta >= kPlaySeekJumpMs)
				|| (!bPlaying && !bForceSeek && delta >= 40);
			if (bDoSeek)
			{
				CPreviewGpuPipeline::Instance().SeekLayer(L.clipId, local);
				bDidSeek = true;
			}
		}
		m_lastSeekMs = topLocal;
		if (bDidSeek)
		{
			++m_seekEpoch;
			m_lastPresentedPts = -1; // 允许软/硬路径收下 Seek 后首帧
		}
		EnsureAudioSidecar(audioPath, audioSeek, bPlaying);
		if (bPlaying)
		{
			if (bDidSeek)
				PrimeAndFinishSeekFrames();
			else
				PullLayersAtClock();
			Play();
		}
		else
		{
			Pause();
			// 拖拽中不阻塞；仅当新帧接近当前目标才 Present，避免画出上一 Seek
			const bool bGot = PullLayersAtClock();
			const bool bNear = bGot && m_lastPresentedPts >= 0
				&& std::llabs(m_lastPresentedPts - m_previewClockMs) <= 80;
			if (bNear)
				PresentLastFrame();
			if (bDidSeek)
				SchedulePausedFrameRetry();
			else if (!bGot)
				PresentLastFrame();
		}
		return;
	}

	if (bSoftSameSet)
	{
		constexpr qint64 kPlaySeekJumpMs = 180;
		const qint64 delta = std::llabs(topLocal - m_lastSeekMs);
		const bool bNeedSeek = (m_lastSeekMs < 0)
			|| (bForceSeek && !bPlaying && delta > 0)
			|| (bPlaying && delta >= kPlaySeekJumpMs)
			|| (!bPlaying && !bForceSeek && delta >= 40);
		if (bNeedSeek)
		{
			++m_seekEpoch;
			m_pSession->SeekMs(topLocal);
			m_lastPresentedPts = -1; // Seek 后必收环内首帧（避免同 PTS/超 clock 拒收）
		}
		m_lastSeekMs = topLocal;
		if (bPlaying)
		{
			if (bNeedSeek)
				PrimeAndFinishSeekFrames();
			Play();
		}
		else
		{
			Pause();
			const bool bGot = PullFrameAtClock(topLocal);
			const bool bNear = bGot && m_lastPresentedPts >= 0
				&& std::llabs(m_lastPresentedPts - m_previewClockMs) <= 80;
			if (bNear)
				PresentLastFrame();
			if (bNeedSeek)
				SchedulePausedFrameRetry();
			else if (!bGot)
				PresentLastFrame();
		}
		return;
	}

	std::vector<SPreviewGpuLayerDesc> descs;
	descs.reserve(visualLayers.size());
	for (const auto& L : visualLayers)
	{
		SPreviewGpuLayerDesc d;
		d.clipId = L.clipId;
		d.path = L.path;
		d.seekMs = L.clipLocalMs < 0 ? 0 : L.clipLocalMs;
		descs.push_back(d);
	}

	bool bHwOk = false;
	if (CPreviewGpuPipeline::Instance().IsDeviceReady())
		bHwOk = CPreviewGpuPipeline::Instance().SyncLayers(descs, true);

	if (bHwOk)
	{
		m_bUseHwPipeline = true;
		m_openedPath = topPath;
		m_lastSeekMs = topLocal;
		m_lastPresentedPts = -1;
		EnsureAudioSidecar(audioPath, audioSeek, bPlaying);
		// 开槽后 Seek 已下发：短等首帧 FinishSeek
		CPreviewGpuPipeline::Instance().PrimeSeekFrames(32, 2);
		PullLayersAtClock();
		PresentLastFrame();
		if (!bPlaying)
			SchedulePausedFrameRetry();
	}
	else
	{
		m_bUseHwPipeline = false;
		m_bMultiLayer = false;
		CPreviewGpuPipeline::Instance().CloseLayers();
		const auto& top = visualLayers.back();
		const bool bPathChanged = (top.path != m_openedPath) || !m_pSession->IsOpen() || m_pSession->IsAudioOnly();
		if (bPathChanged)
		{
			m_pSession->Close();
			m_lastAudioSeekMs = -1;
			if (!m_pSession->Open(top.path))
			{
				m_openedPath.clear();
				PresentBlack();
				return;
			}
			m_openedPath = top.path;
			m_audioPath.clear();
			m_pSession->SeekMs(topLocal);
		}
		else
		{
			constexpr qint64 kPlaySeekJumpMs = 180;
			const qint64 delta = std::llabs(topLocal - m_lastSeekMs);
			const bool bNeedSeek = (m_lastSeekMs < 0)
				|| (bForceSeek && !bPlaying && delta > 0)
				|| (bPlaying && delta >= kPlaySeekJumpMs)
				|| (!bPlaying && !bForceSeek && delta >= 40);
			if (bNeedSeek)
				m_pSession->SeekMs(topLocal);
		}
		m_lastSeekMs = topLocal;
		PullFrameAtClock(topLocal);
		PresentLastFrame();
		if (!bPlaying)
			SchedulePausedFrameRetry();
	}

	if (bPlaying)
		Play();
	else
		Pause();
}

void CMediaPreviewWnd::Play()
{
	const bool bGpuOpen = m_bUseHwPipeline && CPreviewGpuPipeline::Instance().IsOpen();
	const bool bSessionOpen = m_pSession->IsOpen();
	if (!bGpuOpen && !bSessionOpen)
		return;
	m_bPlaying = true;
	if (bGpuOpen)
		CPreviewGpuPipeline::Instance().SetLayersPlaying(true);
	if (bSessionOpen)
		m_pSession->SetPlaying(true);
	if (!m_timer.isActive())
		m_timer.start(16);
}

void CMediaPreviewWnd::Pause()
{
	m_bPlaying = false;
	if (m_bUseHwPipeline)
		CPreviewGpuPipeline::Instance().SetLayersPlaying(false);
	m_pSession->SetPlaying(false);
	m_timer.stop();
}

void CMediaPreviewWnd::Stop()
{
	Pause();
	m_pSession->Close();
	CPreviewGpuPipeline::Instance().CloseLayers();
	m_bUseHwPipeline = false;
	m_bMultiLayer = false;
	m_layerInfos.clear();
	m_layerFrames.clear();
	m_audioPath.clear();
	if (m_pDx)
		m_pDx->extSrv = nullptr;
	m_openedPath.clear();
	m_lastSeekMs = -1;
	m_lastAudioSeekMs = -1;
	m_lastPresentedPts = -1;
	m_previewClockMs = 0;
	ClearFrameCache();
	PresentBlack();
}

void CMediaPreviewWnd::ClearFrameCache()
{
	m_lastBgra.clear();
	m_lastFw = 0;
	m_lastFh = 0;
	m_bFrameDirty = false;
	m_bGpuHasFrame = false;
	m_layerFrames.clear();
}

void CMediaPreviewWnd::PresentLastFrame()
{
	if (!EnsureDx() || !m_pDx->rtv)
		return;
	const bool bHasLayers = m_bUseHwPipeline && !m_layerFrames.empty();
	const bool bHasHw = m_bUseHwPipeline && m_pDx->extSrv && m_lastFw > 0;
	const bool bHasSw = m_lastFw > 0 && m_lastFh > 0 && !m_lastBgra.empty();
	if (!bHasLayers && !bHasHw && !bHasSw)
	{
		PresentBlack();
		return;
	}
	DrawFrameToBackBuffer(!bHasHw && !bHasLayers && (m_bFrameDirty || !m_bGpuHasFrame));
	m_pDx->swapChain->Present(0, 0);
	m_bFrameDirty = false;
	m_bFxDirty = false;
}

void CMediaPreviewWnd::SetVolume(double volume)
{
	m_pSession->SetVolume(volume);
}

void CMediaPreviewWnd::SetPreviewColor(const SColorPty& color)
{
	if (m_previewColor == color)
		return;
	m_previewColor = color;
	m_bFxDirty = true;
	if (!m_bPlaying && m_bGpuHasFrame)
		PresentLastFrame();
}

void CMediaPreviewWnd::SetPreviewFx(float alpha, float rMul, float gMul, float bMul)
{
	SColorPty c = m_previewColor;
	c.alpha = std::clamp(alpha, 0.f, 1.f);
	c.red = std::max(0.f, rMul);
	c.green = std::max(0.f, gMul);
	c.blue = std::max(0.f, bMul);
	SetPreviewColor(c);
}

void CMediaPreviewWnd::InvalidatePreviewFx()
{
	m_bFxDirty = true;
	if (!m_bPlaying && m_bGpuHasFrame)
		PresentLastFrame();
}

void CMediaPreviewWnd::PrimeAndFinishSeekFrames()
{
	if (m_bUseHwPipeline)
	{
		// Seek → 有限次 Pump → 首帧自动 FinishSeek（见 PumpGpuSlot）
		// 暂停 scrub 需要足够等待硬解出帧（约 0.5s）
		CPreviewGpuPipeline::Instance().PrimeSeekFrames(80, 4);
		PullLayersAtClock();
		if (CPreviewGpuPipeline::Instance().IsAnySeeking())
			CPreviewGpuPipeline::Instance().FinishSeekAll();
	}
	else if (m_pSession && m_pSession->IsOpen() && !m_pSession->IsAudioOnly())
	{
		// 软解 SeekMs 内已 prime；强制用 lastPresentedPts=-1 再取一次
		if (m_pSession->IsSeeking())
			m_pSession->FinishSeek();
		const qint64 prevPts = m_lastPresentedPts;
		m_lastPresentedPts = -1;
		if (!PullFrameAtClock(m_previewClockMs))
			m_lastPresentedPts = prevPts;
	}
}

void CMediaPreviewWnd::SchedulePausedFrameRetry()
{
	if (m_bPlaying)
		return;
	m_retryEpoch = m_seekEpoch;
	// 长 GOP：从关键帧解到目标可能需 >1s
	m_primeRetryLeft = 90;
	m_primeRetryTimer.start(16);
}

void CMediaPreviewWnd::FinishSeekPreview()
{
	PrimeAndFinishSeekFrames();
	if (!m_bPlaying)
	{
		PresentLastFrame();
		SchedulePausedFrameRetry();
	}
}

bool CMediaPreviewWnd::PullLayersAtClock()
{
	if (!m_bUseHwPipeline || m_layerInfos.empty())
		return false;

	std::vector<SLayerGpuFrame> nextFrames;
	bool bAnyNew = false;
	for (const auto& info : m_layerInfos)
	{
		if (!info.bVisual)
			continue;
		ID3D11ShaderResourceView* srv = nullptr;
		ID3D11Texture2D* tex = nullptr;
		int fw = 0, fh = 0;
		qint64 pts = -1;
		const qint64 clock = info.clipLocalMs < 0 ? 0 : info.clipLocalMs;
		const bool bNew = CPreviewGpuPipeline::Instance().PumpLayer(
			info.clipId, clock, &srv, &tex, fw, fh, pts);
		if (!srv || fw <= 0 || fh <= 0)
			continue;
		SLayerGpuFrame fr;
		fr.clipId = info.clipId;
		fr.srv = srv;
		fr.tex = tex ? tex : CPreviewGpuPipeline::Instance().GetLayerTexture(info.clipId);
		fr.w = fw;
		fr.h = fh;
		fr.pts = pts;
		fr.color = info.color;
		fr.trans = info.trans;
		nextFrames.push_back(fr);
		if (bNew)
			bAnyNew = true;
	}
	// seek 后解码器可能短暂无帧：保留上一幅，避免 PresentBlack
	if (nextFrames.empty())
		return false;

	m_layerFrames = std::move(nextFrames);
	m_pDx->extSrv = m_layerFrames.back().srv;
	m_lastFw = m_layerFrames.back().w;
	m_lastFh = m_layerFrames.back().h;
	m_lastPresentedPts = m_layerFrames.back().pts;
	m_bGpuHasFrame = true;
	if (bAnyNew)
		m_bFrameDirty = true;
	return bAnyNew;
}

bool CMediaPreviewWnd::PullFrameAtClock(qint64 clockMs)
{
	if (m_bUseHwPipeline && !m_layerInfos.empty())
	{
		int nVisual = 0;
		size_t iVisual = 0;
		for (size_t i = 0; i < m_layerInfos.size(); ++i)
		{
			if (!m_layerInfos[i].bVisual)
				continue;
			iVisual = i;
			++nVisual;
		}
		if (nVisual == 1)
			m_layerInfos[iVisual].clipLocalMs = clockMs;
		return PullLayersAtClock();
	}

	if (m_bUseHwPipeline)
	{
		ID3D11ShaderResourceView* srv = nullptr;
		int fw = 0, fh = 0;
		qint64 pts = -1;
		const bool bNew = CPreviewGpuPipeline::Instance().PumpFrame(clockMs, &srv, fw, fh, pts);
		if (srv && fw > 0 && fh > 0)
		{
			m_pDx->extSrv = srv;
			m_lastFw = fw;
			m_lastFh = fh;
			m_bGpuHasFrame = true;
			if (bNew)
			{
				m_lastPresentedPts = pts;
				m_bFrameDirty = true;
			}
			return bNew;
		}
		return false;
	}

	if (m_pSession->IsAudioOnly())
		return false;

	std::vector<uint8_t> bgra;
	int fw = 0, fh = 0;
	qint64 pts = -1;
	if (!m_pSession->TryGetFrame(clockMs, m_lastPresentedPts, bgra, fw, fh, pts)
		|| fw <= 0 || fh <= 0)
		return false;
	m_lastBgra = std::move(bgra);
	m_lastFw = fw;
	m_lastFh = fh;
	m_lastPresentedPts = pts;
	m_bFrameDirty = true;
	return true;
}

void CMediaPreviewWnd::OnTick()
{
	if (m_pBridge)
		m_pBridge->OnPlayTick();

	if (!m_bPlaying)
		return;

	if (m_bUseHwPipeline && !m_layerInfos.empty())
		PullLayersAtClock();
	else
		PullFrameAtClock(m_previewClockMs);
	if (m_bFrameDirty || m_bFxDirty)
		PresentLastFrame();
}

void CMediaPreviewWnd::PresentBlack()
{
	if (!EnsureDx() || !m_pDx->rtv)
		return;
	const float clear[4] = { 0.08f, 0.08f, 0.1f, 1.0f };
	m_pDx->ctx->OMSetRenderTargets(1, m_pDx->rtv.GetAddressOf(), nullptr);
	m_pDx->ctx->ClearRenderTargetView(m_pDx->rtv.Get(), clear);
	m_pDx->swapChain->Present(0, 0);
}

void CMediaPreviewWnd::DrawFrameToBackBuffer(bool bUploadPixels)
{
	if (!m_pDx || !m_pDx->rtv || !m_pDx->swapChain || !m_pDx->bPipeline)
		return;

	// 阶段5：多层合成（底→顶，SRC_ALPHA 混合，仅首层 Clear）
	if (m_bUseHwPipeline && !m_layerFrames.empty())
	{
		ComPtr<ID3D11Texture2D> backBuffer;
		m_pDx->swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		if (!backBuffer)
			return;
		D3D11_TEXTURE2D_DESC bd = {};
		backBuffer->GetDesc(&bd);
		const int bw = static_cast<int>(bd.Width);
		const int bh = static_cast<int>(bd.Height);
		if (bw <= 0 || bh <= 0)
			return;

		m_pDx->ctx->OMSetRenderTargets(1, m_pDx->rtv.GetAddressOf(), nullptr);
		const float clear[4] = { 0.05f, 0.05f, 0.06f, 1.0f };
		m_pDx->ctx->ClearRenderTargetView(m_pDx->rtv.Get(), clear);

		D3D11_VIEWPORT vp = {};
		vp.Width = static_cast<float>(bw);
		vp.Height = static_cast<float>(bh);
		vp.MaxDepth = 1.f;
		m_pDx->ctx->RSSetViewports(1, &vp);
		m_pDx->ctx->RSSetState(m_pDx->rs.Get());
		float blendFactor[4] = { 0, 0, 0, 0 };
		m_pDx->ctx->OMSetBlendState(m_pDx->blend.Get(), blendFactor, 0xffffffff);
		m_pDx->ctx->IASetInputLayout(m_pDx->layout.Get());
		m_pDx->ctx->VSSetShader(m_pDx->vs.Get(), nullptr, 0);
		m_pDx->ctx->PSSetShader(m_pDx->ps.Get(), nullptr, 0);
		m_pDx->ctx->PSSetSamplers(0, 1, m_pDx->samp.GetAddressOf());

		for (const auto& L : m_layerFrames)
		{
			if (!L.srv || L.w <= 0 || L.h <= 0)
				continue;
			CPreviewFxPass::Instance().BindClip(L.clipId);
			CPreviewFxPass::Instance().SetTransInfo(L.trans);

			ID3D11ShaderResourceView* srcSrv = L.srv;
			ID3D11Texture2D* srcTex = L.tex
				? L.tex
				: CPreviewGpuPipeline::Instance().GetLayerTexture(L.clipId);
			const int64_t ptsUs = (L.pts >= 0) ? static_cast<int64_t>(L.pts) * 1000 : 0;
			if (auto* fx = CPreviewFxPass::Instance().Apply(srcSrv, srcTex, L.w, L.h, ptsUs))
				srcSrv = fx;

			float u0 = 0.f, v0 = 0.f, u1 = 1.f, v1 = 1.f;
			CPreviewFxPass::Instance().GetCropUv(u0, v0, u1, v1);
			float ox = 0.f, oy = 0.f, sx = 1.f, sy = 1.f, rotZ = 0.f;
			CPreviewFxPass::Instance().GetPreviewTransform(ox, oy, sx, sy, rotZ);

			const float fit = std::min(bw / float(L.w), bh / float(L.h)) * m_fZoom;
			const int dw = std::max(1, int(std::lround(L.w * fit)));
			const int dh = std::max(1, int(std::lround(L.h * fit)));
			const int dx = (bw - dw) / 2;
			const int dy = (bh - dh) / 2;
			float x0 = (dx / float(bw)) * 2.f - 1.f;
			float x1 = ((dx + dw) / float(bw)) * 2.f - 1.f;
			float y0 = 1.f - (dy / float(bh)) * 2.f;
			float y1 = 1.f - ((dy + dh) / float(bh)) * 2.f;
			const float cx = (x0 + x1) * 0.5f;
			const float cy = (y0 + y1) * 0.5f;
			const float hw = (x1 - x0) * 0.5f * sx;
			const float hh = (y0 - y1) * 0.5f * sy;
			const float rad = rotZ * 3.14159265f / 180.f;
			const float cr = std::cos(rad), sr = std::sin(rad);
			auto mapLocal = [&](float lx, float ly) {
				return std::pair<float, float>(cx + ox + lx * cr - ly * sr, cy + oy + lx * sr + ly * cr);
			};

			std::vector<VertexPosTex> transVerts;
			std::vector<UINT> transIdx;
			const bool bTransMesh = CPreviewFxPass::Instance().BuildTransVertices(transVerts, transIdx);
			std::vector<SVertex> drawVerts;
			std::vector<UINT> drawIdx;
			if (bTransMesh)
			{
				for (const auto& tv : transVerts)
				{
					const auto p = mapLocal(tv.pos.x * hw, tv.pos.y * hh);
					drawVerts.push_back({ p.first, p.second,
						u0 + tv.tex.x * (u1 - u0), v0 + tv.tex.y * (v1 - v0) });
				}
				drawIdx = transIdx;
			}
			else
			{
				const auto p00 = mapLocal(-hw, hh);
				const auto p10 = mapLocal(hw, hh);
				const auto p01 = mapLocal(-hw, -hh);
				const auto p11 = mapLocal(hw, -hh);
				drawVerts = {
					{ p00.first, p00.second, u0, v0 },
					{ p10.first, p10.second, u1, v0 },
					{ p01.first, p01.second, u0, v1 },
					{ p11.first, p11.second, u1, v1 },
				};
			}
			if (drawVerts.size() > 4096) drawVerts.resize(4096);
			if (drawIdx.size() > m_pDx->ibCapacity) drawIdx.resize(m_pDx->ibCapacity);

			D3D11_MAPPED_SUBRESOURCE mappedVb = {};
			if (SUCCEEDED(m_pDx->ctx->Map(m_pDx->vb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVb)))
			{
				memcpy(mappedVb.pData, drawVerts.data(), sizeof(SVertex) * drawVerts.size());
				m_pDx->ctx->Unmap(m_pDx->vb.Get(), 0);
			}
			D3D11_MAPPED_SUBRESOURCE mappedCb = {};
			if (SUCCEEDED(m_pDx->ctx->Map(m_pDx->cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCb)))
			{
				auto* cb = static_cast<SFxCb*>(mappedCb.pData);
				cb->lumin = L.color.luminance;
				cb->contrast = L.color.contrast;
				cb->hue = L.color.hue;
				cb->saturability = L.color.saturability;
				cb->r = L.color.red;
				cb->g = L.color.green;
				cb->b = L.color.blue;
				cb->a = L.color.alpha;
				m_pDx->ctx->Unmap(m_pDx->cb.Get(), 0);
			}

			UINT stride = sizeof(SVertex), offset = 0;
			m_pDx->ctx->IASetVertexBuffers(0, 1, m_pDx->vb.GetAddressOf(), &stride, &offset);
			m_pDx->ctx->PSSetConstantBuffers(0, 1, m_pDx->cb.GetAddressOf());
			m_pDx->ctx->PSSetShaderResources(0, 1, &srcSrv);
			if (bTransMesh && !drawIdx.empty() && m_pDx->ib)
			{
				D3D11_MAPPED_SUBRESOURCE mappedIb = {};
				if (SUCCEEDED(m_pDx->ctx->Map(m_pDx->ib.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedIb)))
				{
					memcpy(mappedIb.pData, drawIdx.data(), sizeof(UINT) * drawIdx.size());
					m_pDx->ctx->Unmap(m_pDx->ib.Get(), 0);
				}
				m_pDx->ctx->IASetIndexBuffer(m_pDx->ib.Get(), DXGI_FORMAT_R32_UINT, 0);
				m_pDx->ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				m_pDx->ctx->DrawIndexed(static_cast<UINT>(drawIdx.size()), 0, 0);
			}
			else
			{
				m_pDx->ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				m_pDx->ctx->Draw(static_cast<UINT>(drawVerts.size()), 0);
			}
		}
		ID3D11ShaderResourceView* nullSrv = nullptr;
		m_pDx->ctx->PSSetShaderResources(0, 1, &nullSrv);
		return;
	}

	if (m_lastFw <= 0 || m_lastFh <= 0)
		return;

	const int fw = m_lastFw;
	const int fh = m_lastFh;
	const bool bExt = (m_bUseHwPipeline && m_pDx->extSrv);
	if (!bExt && m_lastBgra.empty())
		return;

	// 动态纹理直接作 SRV：每帧 Map 上传原生分辨率，GPU 负责 letterbox/FX
	if (!bExt && (bUploadPixels || !m_pDx->staging || m_pDx->texW != fw || m_pDx->texH != fh))
	{
		if (!m_pDx->staging || m_pDx->texW != fw || m_pDx->texH != fh)
		{
			m_pDx->staging.Reset();
			m_pDx->frameTex.Reset();
			m_pDx->srv.Reset();

			D3D11_TEXTURE2D_DESC td = {};
			td.Width = static_cast<UINT>(fw);
			td.Height = static_cast<UINT>(fh);
			td.MipLevels = 1;
			td.ArraySize = 1;
			td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			td.SampleDesc.Count = 1;
			td.Usage = D3D11_USAGE_DYNAMIC;
			td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			if (FAILED(m_pDx->device->CreateTexture2D(&td, nullptr, &m_pDx->staging)))
				return;
			if (FAILED(m_pDx->device->CreateShaderResourceView(m_pDx->staging.Get(), nullptr, &m_pDx->srv)))
				return;
			m_pDx->texW = fw;
			m_pDx->texH = fh;
		}

		if (bUploadPixels || !m_bGpuHasFrame)
		{
			D3D11_MAPPED_SUBRESOURCE mapped = {};
			if (FAILED(m_pDx->ctx->Map(m_pDx->staging.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
				return;
			uint8_t* dst = static_cast<uint8_t*>(mapped.pData);
			const int rowBytes = fw * 4;
			for (int y = 0; y < fh; ++y)
				memcpy(dst + y * mapped.RowPitch, m_lastBgra.data() + static_cast<size_t>(y) * rowBytes, rowBytes);
			m_pDx->ctx->Unmap(m_pDx->staging.Get(), 0);
			m_bGpuHasFrame = true;
		}
	}

	ComPtr<ID3D11Texture2D> backBuffer;
	m_pDx->swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	if (!backBuffer)
		return;

	D3D11_TEXTURE2D_DESC bd = {};
	backBuffer->GetDesc(&bd);
	const int bw = static_cast<int>(bd.Width);
	const int bh = static_cast<int>(bd.Height);
	if (bw <= 0 || bh <= 0)
		return;

	const float fit = std::min(bw / float(fw), bh / float(fh)) * m_fZoom;
	const int dw = std::max(1, int(std::lround(fw * fit)));
	const int dh = std::max(1, int(std::lround(fh * fit)));
	const int dx = (bw - dw) / 2;
	const int dy = (bh - dh) / 2;

	// 阶段3：ClipFx 像素特效（Keying/Feather/Blur…）
	ID3D11ShaderResourceView* srcSrv = bExt ? m_pDx->extSrv : m_pDx->srv.Get();
	ID3D11Texture2D* srcTex = bExt
		? CPreviewGpuPipeline::Instance().GetRgbaTexture()
		: m_pDx->staging.Get();
	const int64_t ptsUs = (m_lastPresentedPts >= 0)
		? static_cast<int64_t>(m_lastPresentedPts) * 1000
		: 0;
	ID3D11ShaderResourceView* fxSrv = CPreviewFxPass::Instance().Apply(
		srcSrv, srcTex, fw, fh, ptsUs);
	if (fxSrv)
		srcSrv = fxSrv;

	float u0 = 0.f, v0 = 0.f, u1 = 1.f, v1 = 1.f;
	CPreviewFxPass::Instance().GetCropUv(u0, v0, u1, v1);

	float ox = 0.f, oy = 0.f, sx = 1.f, sy = 1.f, rotZ = 0.f;
	CPreviewFxPass::Instance().GetPreviewTransform(ox, oy, sx, sy, rotZ);

	// letterbox 中心与半宽高（NDC，Y 向上）
	float x0 = (dx / float(bw)) * 2.f - 1.f;
	float x1 = ((dx + dw) / float(bw)) * 2.f - 1.f;
	float y0 = 1.f - (dy / float(bh)) * 2.f;
	float y1 = 1.f - ((dy + dh) / float(bh)) * 2.f;
	const float cx = (x0 + x1) * 0.5f;
	const float cy = (y0 + y1) * 0.5f;
	const float hw = (x1 - x0) * 0.5f * sx;
	const float hh = (y0 - y1) * 0.5f * sy;
	const float rad = rotZ * 3.14159265f / 180.f;
	const float cr = std::cos(rad);
	const float sr = std::sin(rad);
	auto mapLocal = [&](float lx, float ly) {
		const float rx = lx * cr - ly * sr;
		const float ry = lx * sr + ly * cr;
		return std::pair<float, float>(cx + ox + rx, cy + oy + ry);
	};
	// 转场顶点在全屏 NDC[-1,1]；映射到 letterbox：ndc * (hw,hh) + center
	auto mapNdc = [&](float nx, float ny) {
		return mapLocal(nx * hw, ny * hh);
	};

	std::vector<VertexPosTex> transVerts;
	std::vector<UINT> transIdx;
	const bool bTransMesh = CPreviewFxPass::Instance().BuildTransVertices(transVerts, transIdx);

	std::vector<SVertex> drawVerts;
	std::vector<UINT> drawIdx;
	if (bTransMesh)
	{
		drawVerts.reserve(transVerts.size());
		for (const auto& tv : transVerts)
		{
			const auto p = mapNdc(tv.pos.x, tv.pos.y);
			// 转场 UV 与裁剪 UV 合成
			const float u = u0 + tv.tex.x * (u1 - u0);
			const float v = v0 + tv.tex.y * (v1 - v0);
			drawVerts.push_back({ p.first, p.second, u, v });
		}
		drawIdx = transIdx;
	}
	else
	{
		const auto p00 = mapLocal(-hw, hh);
		const auto p10 = mapLocal(hw, hh);
		const auto p01 = mapLocal(-hw, -hh);
		const auto p11 = mapLocal(hw, -hh);
		drawVerts = {
			{ p00.first, p00.second, u0, v0 },
			{ p10.first, p10.second, u1, v0 },
			{ p01.first, p01.second, u0, v1 },
			{ p11.first, p11.second, u1, v1 },
		};
	}

	const size_t maxV = 4096;
	const size_t maxI = m_pDx->ibCapacity;
	if (drawVerts.size() > maxV)
		drawVerts.resize(maxV);
	if (drawIdx.size() > maxI)
		drawIdx.resize(maxI);

	D3D11_MAPPED_SUBRESOURCE mappedVb = {};
	if (SUCCEEDED(m_pDx->ctx->Map(m_pDx->vb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVb)))
	{
		memcpy(mappedVb.pData, drawVerts.data(), sizeof(SVertex) * drawVerts.size());
		m_pDx->ctx->Unmap(m_pDx->vb.Get(), 0);
	}

	D3D11_MAPPED_SUBRESOURCE mappedCb = {};
	if (SUCCEEDED(m_pDx->ctx->Map(m_pDx->cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCb)))
	{
		auto* cb = static_cast<SFxCb*>(mappedCb.pData);
		cb->lumin = m_previewColor.luminance;
		cb->contrast = m_previewColor.contrast;
		cb->hue = m_previewColor.hue;
		cb->saturability = m_previewColor.saturability;
		cb->r = m_previewColor.red;
		cb->g = m_previewColor.green;
		cb->b = m_previewColor.blue;
		cb->a = m_previewColor.alpha;
		m_pDx->ctx->Unmap(m_pDx->cb.Get(), 0);
	}

	const float clear[4] = { 0.05f, 0.05f, 0.06f, 1.0f };
	m_pDx->ctx->OMSetRenderTargets(1, m_pDx->rtv.GetAddressOf(), nullptr);
	m_pDx->ctx->ClearRenderTargetView(m_pDx->rtv.Get(), clear);

	D3D11_VIEWPORT vp = {};
	vp.Width = static_cast<float>(bw);
	vp.Height = static_cast<float>(bh);
	vp.MaxDepth = 1.f;
	m_pDx->ctx->RSSetViewports(1, &vp);
	m_pDx->ctx->RSSetState(m_pDx->rs.Get());

	float blendFactor[4] = { 0, 0, 0, 0 };
	m_pDx->ctx->OMSetBlendState(m_pDx->blend.Get(), blendFactor, 0xffffffff);
	m_pDx->ctx->IASetInputLayout(m_pDx->layout.Get());
	UINT stride = sizeof(SVertex), offset = 0;
	m_pDx->ctx->IASetVertexBuffers(0, 1, m_pDx->vb.GetAddressOf(), &stride, &offset);
	m_pDx->ctx->VSSetShader(m_pDx->vs.Get(), nullptr, 0);
	m_pDx->ctx->PSSetShader(m_pDx->ps.Get(), nullptr, 0);
	m_pDx->ctx->PSSetConstantBuffers(0, 1, m_pDx->cb.GetAddressOf());
	m_pDx->ctx->PSSetShaderResources(0, 1, &srcSrv);
	m_pDx->ctx->PSSetSamplers(0, 1, m_pDx->samp.GetAddressOf());

	if (bTransMesh && !drawIdx.empty() && m_pDx->ib)
	{
		D3D11_MAPPED_SUBRESOURCE mappedIb = {};
		if (SUCCEEDED(m_pDx->ctx->Map(m_pDx->ib.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedIb)))
		{
			memcpy(mappedIb.pData, drawIdx.data(), sizeof(UINT) * drawIdx.size());
			m_pDx->ctx->Unmap(m_pDx->ib.Get(), 0);
		}
		m_pDx->ctx->IASetIndexBuffer(m_pDx->ib.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_pDx->ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pDx->ctx->DrawIndexed(static_cast<UINT>(drawIdx.size()), 0, 0);
	}
	else
	{
		m_pDx->ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_pDx->ctx->Draw(static_cast<UINT>(drawVerts.size()), 0);
	}

	ID3D11ShaderResourceView* nullSrv = nullptr;
	m_pDx->ctx->PSSetShaderResources(0, 1, &nullSrv);
}
