#include "GifPlayer.h"
#include "Common/CommonFunctionFileOp.h"
#include "Common/ResourceDefine.h"

#include <d2d1.h>
#include <QDebug>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

//#ifdef Q_CC_MSVC
//#pragma execution_character_set("utf-8")
//#endif

struct SGifInfo
{
	// 图像数量
	unsigned int mFrameCount = 0;

	unsigned int mWidth = 0;
	unsigned int mHeight = 0;
	unsigned int mWidthPixel = 0;
	unsigned int mHeightPixel = 0;

	bool mHasColor = false;
	DWORD mDwColor;
	unsigned int mLoopCount = 0;

	struct SFramePt
	{
		unsigned int mFrameIndex = 0;
		float mLeft = 0.0f;
		float mTop = 0.0f;
		float mWidth = 0.0f;
		float mHeight = 0.0f;
		unsigned int mFrameDelay = 0;

		enum class EDisposalMethods : unsigned int
		{
			DM_UNDEFINED = 0,
			DM_NONE = 1,
			DM_BACKGROUND = 2,
			DM_PREVIOUS = 3
		};
		EDisposalMethods mDisposal = EDisposalMethods::DM_UNDEFINED;

		unsigned long long mFramePts = 0;
	};
	std::vector<SFramePt> mFramePts;
};
using EDisposalType = SGifInfo::SFramePt::EDisposalMethods;

CGifPlayer::CGifPlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx, const QString& path/*, std::shared_ptr<IShareProperty> pSharedProperty*/)
	: CBasePlayer(pResBase, ctx)
	, m_strFilePath(path)
	//, m_pSharedProperty(pSharedProperty)
{
	InitThread();
}

CGifPlayer::~CGifPlayer()
{
	UnInitThread();
	m_p2dfactory.Reset();
	m_pWICfactory.Reset();
	m_pWICdecoder.Reset();
	m_pComposeRT.Reset();
	m_pComposeTexture.Reset();
	m_pRawFrame.Reset();
}

void CGifPlayer::InitThread()
{
	m_bRun = true;
	if (m_pUpdateTrd == nullptr)
		m_pUpdateTrd = new std::thread(&CGifPlayer::UpdateImageInfo, this);
}

void CGifPlayer::UnInitThread()
{
	m_bRun = false;

	if (m_pUpdateTrd && m_pUpdateTrd->joinable())
		m_pUpdateTrd->join();

	if (m_pUpdateTrd)
	{
		delete m_pUpdateTrd;
		m_pUpdateTrd = nullptr;
	}
}

bool CGifPlayer::InitD2DResource()
{
	//D2D相关
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_p2dfactory.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return false;

	// 创建纹理
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = m_pGifInfo->mWidth;
	texDesc.Height = m_pGifInfo->mHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
	hr = m_pD3DDevice->CreateTexture2D(&texDesc, nullptr, m_pComposeTexture.GetAddressOf());
	if (!SUCCEEDED(hr))
		return false;

	hr = m_pD3DDevice->CreateTexture2D(&texDesc, nullptr, m_sTexs.texNode0.pTex.GetAddressOf());
	if (!SUCCEEDED(hr))
		return false;

	if (!m_sTexs.texNode0.UpdateTexPty())
		return false;

	// 创建D2D
	ComPtr<IDXGISurface> pDXGISurface = nullptr;
	hr = m_pComposeTexture.As(&pDXGISurface);
	if (!SUCCEEDED(hr))
		return false;
	D2D1_RENDER_TARGET_PROPERTIES props;
	props.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
	props.dpiX = 0;
	props.dpiY = 0;
	props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	props.pixelFormat.format = DXGI_FORMAT_R8G8B8A8_UNORM;
	props.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
	props.usage = D2D1_RENDER_TARGET_USAGE_NONE;
	hr = m_p2dfactory->CreateDxgiSurfaceRenderTarget(pDXGISurface.Get(), props, m_pComposeRT.GetAddressOf());
	if (!SUCCEEDED(hr))
		return false;

	return true;
}

bool CGifPlayer::Start(const microseconds& curTime)
{
	
	return CBasePlayer::Start(curTime);
}

bool CGifPlayer::Pause(const microseconds& curTime)
{
	
	return CBasePlayer::Pause(curTime);
}

bool CGifPlayer::Stop()
{
	
	return CBasePlayer::Stop();
}

bool CGifPlayer::Prepare()
{
	// WIC 解码初始化
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWICfactory));
	if (!SUCCEEDED(hr))
		return false;
	hr = m_pWICfactory->CreateDecoderFromFilename(m_strFilePath.toStdWString().c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_pWICdecoder);
	if (!SUCCEEDED(hr))
		return false;

	// 获取GIF图信息
	m_pGifInfo = std::make_shared<SGifInfo>();
	LoadGifInfo(m_pGifInfo.get());
	qDebug() << "Frame Count: " << m_pGifInfo->mFrameCount;
	if (m_pGifInfo->mFrameCount == 0)
		return false;

	if (m_pGifInfo->mHasColor)
	{
		// TODO: HDR 黑白 gif
		FLOAT alpha = (m_pGifInfo->mDwColor >> 24) / 255.f;
		m_gifDecoderCtx.m_BackgroundColor = D2D1::ColorF(m_pGifInfo->mDwColor, alpha);
	}
	m_gifDecoderCtx.m_duration = duration_cast<microseconds>(milliseconds(m_pGifInfo->mFramePts.back().mFramePts + m_pGifInfo->mFramePts.back().mFrameDelay));

	if(!InitD2DResource())
		return false;

	m_bPrepared = true;
	return CBasePlayer::Prepare();
}

bool CGifPlayer::GetTextView(const microseconds & curTime, SDrawTextureData & videoData)
{
	if (m_gifDecoderCtx.m_duration.count() == 0)
		return false;
	// 循环需从头帧开始
	microseconds curFrameTime = curTime % m_gifDecoderCtx.m_duration;

	// 获取当前期望的帧
	long long expectFrame = GetExpectIndex(curFrameTime);
	//qDebug() << "expectFrame: " << expectFrame;
	// 告诉解码器期望的下一帧是什么
	std::lock_guard<std::mutex> lk(m_NextFrameIndexMutex);
	m_NextFrameIndex = (expectFrame /*+ 1*/ )% m_pGifInfo->mFrameCount;
	

	return CBasePlayer::GetTextView(curTime, videoData);
}

void CGifPlayer::UpdateImageInfo()
{
	std::chrono::milliseconds waitMs(10ms);
	while (m_bRun)
	{
		waitMs = WorkImp();

		std::this_thread::sleep_for(waitMs);
	}
}

std::chrono::milliseconds CGifPlayer::WorkImp()
{
	std::chrono::milliseconds waitMs(10ms);

	if (m_bPrepared)
	{
		DecodeGifInLoop();
	}

	return waitMs;
}

void CGifPlayer::DecodeGifInLoop()
{
	std::lock_guard<std::mutex> guard(m_NextFrameIndexMutex);

	if (-1 == m_NextFrameIndex)
		return;
	if (m_NowFrameIndex == m_NextFrameIndex)
		return;

	// 从头开始渲染
	if (m_NowFrameIndex > m_NextFrameIndex)
	{
		m_NowFrameIndex = 0;
		RenderOnD2DTarget();
	}

	// 渲染
	while (m_NowFrameIndex < m_NextFrameIndex)
	{
		m_NowFrameIndex++;
		RenderOnD2DTarget();
	}

	{
		AutoSectionLocker locker(m_pD3DCSContext);
		m_pD3DDeviceContext->CopyResource(m_sTexs.texNode0.pTex.Get(), m_pComposeTexture.Get());
		m_pD3DDeviceContext->Flush();
	}

	{
		AutoSectionLocker locker(m_pD3DCSContext);
		m_pD3DDeviceContext->Flush();
	}

	{
		std::lock_guard<std::mutex> lock(m_mtxVideoData);

		m_videoData.Reset();

		SDrawTexture drawTex;

		drawTex.textureFrame.m_bFrameChange = true;
		drawTex.textureFrame.m_texs.push_back(m_sTexs);

		m_videoData.vecTextureFrame.push_back(drawTex);

		m_bFrameChange = true;
	}
}

void CGifPlayer::LoadGifInfo(SGifInfo* pInfo) const
{
	if (!pInfo)
		return;

	constexpr UINT DEFAULT_DELAY = 90;
	HRESULT hr = S_OK;

	//
	hr = m_pWICdecoder->GetFrameCount(&pInfo->mFrameCount);
	if (!SUCCEEDED(hr))
		return;

	//
	ComPtr<IWICMetadataQueryReader> gifMetaReader = nullptr;
	hr = m_pWICdecoder->GetMetadataQueryReader(gifMetaReader.GetAddressOf());
	if (!SUCCEEDED(hr))
		return;

	//
	hr = GetBackgroundColor(gifMetaReader, pInfo->mDwColor);
	if (SUCCEEDED(hr))
		pInfo->mHasColor = true;

	//
	PROPVARIANT propValue;
	PropVariantInit(&propValue);

	
	hr = gifMetaReader->GetMetadataByName(L"/logscrdesc/Width", &propValue);
	if (SUCCEEDED(hr))
	{
		hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
		if (SUCCEEDED(hr))
			pInfo->mWidth = propValue.uiVal;
		PropVariantClear(&propValue);
	}

	//
	hr = gifMetaReader->GetMetadataByName(L"/logscrdesc/Height", &propValue);
	if (SUCCEEDED(hr))
	{
		hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
		if (SUCCEEDED(hr))
			pInfo->mHeight = propValue.uiVal;
		PropVariantClear(&propValue);
	}

	//
	hr = gifMetaReader->GetMetadataByName(L"/logscrdesc/PixelAspectRatio", &propValue);
	if (SUCCEEDED(hr))
	{
		hr = (propValue.vt == VT_UI1 ? S_OK : E_FAIL);
		if (SUCCEEDED(hr))
		{
			UINT uPixelAspRatio = propValue.bVal;

			if (uPixelAspRatio != 0)
			{
				// Need to calculate the ratio. The value in uPixelAspRatio 
				// allows specifying widest pixel 4:1 to the tallest pixel of 
				// 1:4 in increments of 1/64th
				FLOAT pixelAspRatio = (uPixelAspRatio + 15.f) / 64.f;

				// Calculate the image width and height in pixel based on the
				// pixel aspect ratio. Only shrink the image.
				if (pixelAspRatio > 1.f)
				{
					pInfo->mWidthPixel = pInfo->mWidth;
					pInfo->mHeightPixel = static_cast<UINT>(pInfo->mHeight / pixelAspRatio);
				}
				else
				{
					pInfo->mWidthPixel = static_cast<UINT>(pInfo->mWidth * pixelAspRatio);
					pInfo->mHeightPixel = pInfo->mHeight;
				}
			}
			else
			{
				// The value is 0, so its ratio is 1
				pInfo->mWidthPixel = pInfo->mWidth;
				pInfo->mHeightPixel = pInfo->mHeight;
			}
		}
		PropVariantClear(&propValue);
	}

	// Get looping information
	if (SUCCEEDED(hr))
	{
		// First check to see if the application block in the Application Extension
		// contains "NETSCAPE2.0" and "ANIMEXTS1.0", which indicates the gif animation
		// has looping information associated with it.
		// 
		// If we fail to get the looping information, loop the animation infinitely.
		if (SUCCEEDED(gifMetaReader->GetMetadataByName(
			L"/appext/application",
			&propValue)) &&
			propValue.vt == (VT_UI1 | VT_VECTOR) &&
			propValue.caub.cElems == 11 &&  // Length of the application block
			(!memcmp(propValue.caub.pElems, "NETSCAPE2.0", propValue.caub.cElems) ||
				!memcmp(propValue.caub.pElems, "ANIMEXTS1.0", propValue.caub.cElems)))
		{
			PropVariantClear(&propValue);

			hr = gifMetaReader->GetMetadataByName(L"/appext/data", &propValue);
			if (SUCCEEDED(hr))
			{
				//  The data is in the following format:
				//  byte 0: extsize (must be > 1)
				//  byte 1: loopType (1 == animated gif)
				//  byte 2: loop count (least significant byte)
				//  byte 3: loop count (most significant byte)
				//  byte 4: set to zero
				if (propValue.vt == (VT_UI1 | VT_VECTOR) &&
					propValue.caub.cElems >= 4 &&
					propValue.caub.pElems[0] > 0 &&
					propValue.caub.pElems[1] == 1)
				{
					pInfo->mLoopCount = MAKEWORD(propValue.caub.pElems[2], propValue.caub.pElems[3]);
				}
			}
		}

	}

	PropVariantClear(&propValue);

	unsigned long long ptsCurrent = 0;
	for (UINT index = 0; index < pInfo->mFrameCount; ++index)
	{
		ComPtr<IWICBitmapFrameDecode> frame = nullptr;
		hr = m_pWICdecoder->GetFrame(index, frame.GetAddressOf());
		if (FAILED(hr) || frame == nullptr)
			return;

		ComPtr<IWICMetadataQueryReader> frameMetaReader = nullptr;
		hr = frame->GetMetadataQueryReader(frameMetaReader.GetAddressOf());
		if (FAILED(hr) || frameMetaReader == nullptr)
			return;

		SGifInfo::SFramePt framePt;

		// left
		hr = frameMetaReader->GetMetadataByName(L"/imgdesc/Left", &propValue);
		if (SUCCEEDED(hr))
		{
			hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
			if (SUCCEEDED(hr))
				framePt.mLeft = static_cast<float>(propValue.uiVal);
			PropVariantClear(&propValue);
		}

		// top
		hr = frameMetaReader->GetMetadataByName(L"/imgdesc/Top", &propValue);
		if (SUCCEEDED(hr))
		{
			hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
			if (SUCCEEDED(hr))
				framePt.mTop = static_cast<float>(propValue.uiVal);
			PropVariantClear(&propValue);
		}

		// width
		hr = frameMetaReader->GetMetadataByName(L"/imgdesc/Width", &propValue);
		if (SUCCEEDED(hr))
		{
			hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
			if (SUCCEEDED(hr))
				framePt.mWidth = static_cast<float>(propValue.uiVal);
			PropVariantClear(&propValue);
		}

		// height
		hr = frameMetaReader->GetMetadataByName(L"/imgdesc/Height", &propValue);
		if (SUCCEEDED(hr))
		{
			hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
			if (SUCCEEDED(hr))
				framePt.mHeight = static_cast<float>(propValue.uiVal);
			PropVariantClear(&propValue);
		}

		// delay
		UINT delay = DEFAULT_DELAY;
		hr = frameMetaReader->GetMetadataByName(L"/grctlext/Delay", &propValue);
		if (SUCCEEDED(hr))
		{
			hr = (propValue.vt == VT_UI2 ? S_OK : E_FAIL);
			if (SUCCEEDED(hr))
				UIntMult(propValue.uiVal, 10, &delay);
			delay = delay == 0 ? DEFAULT_DELAY : delay;
			framePt.mFrameDelay = delay;
			PropVariantClear(&propValue);
		}

		// disposal
		if (SUCCEEDED(frameMetaReader->GetMetadataByName(L"/grctlext/Disposal", &propValue)))
		{
			hr = (propValue.vt == VT_UI1) ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
				framePt.mDisposal = (EDisposalType)propValue.bVal;
			PropVariantClear(&propValue);
		}

		//
		framePt.mFrameIndex = index;
		framePt.mFramePts = ptsCurrent;
		//qDebug() << QString("%1 Frame PTS: %2, Delay: %3").arg(index).arg(ptsCurrent).arg(delay);
		ptsCurrent += delay;
		pInfo->mFramePts.push_back(std::move(framePt));
		
		PropVariantClear(&propValue);
	}

}

HRESULT CGifPlayer::GetBackgroundColor(Microsoft::WRL::ComPtr<IWICMetadataQueryReader> pMetadataQueryReader, DWORD & dwBGColor) const
{
	BYTE backgroundIndex = 0;
	WICColor rgColors[256];
	UINT cColorsCopied = 0;
	PROPVARIANT propVariant;
	PropVariantInit(&propVariant);
	IWICPalette *pWicPalette = NULL;

	// If we have a global palette, get the palette and background color
	HRESULT hr = pMetadataQueryReader->GetMetadataByName(
		L"/logscrdesc/GlobalColorTableFlag",
		&propVariant);
	if (SUCCEEDED(hr))
	{
		hr = (propVariant.vt != VT_BOOL || !propVariant.boolVal) ? E_FAIL : S_OK;
		PropVariantClear(&propVariant);
	}

	if (SUCCEEDED(hr))
	{
		// Background color index
		hr = pMetadataQueryReader->GetMetadataByName(
			L"/logscrdesc/BackgroundColorIndex",
			&propVariant);
		if (SUCCEEDED(hr))
		{
			hr = (propVariant.vt != VT_UI1) ? E_FAIL : S_OK;
			if (SUCCEEDED(hr))
			{
				backgroundIndex = propVariant.bVal;
			}
			PropVariantClear(&propVariant);
		}
	}

	// Get the color from the palette
	if (SUCCEEDED(hr))
	{
		hr = m_pWICfactory->CreatePalette(&pWicPalette);
	}

	if (SUCCEEDED(hr))
	{
		// Get the global palette
		hr = m_pWICdecoder->CopyPalette(pWicPalette);
	}

	if (SUCCEEDED(hr))
	{
		hr = pWicPalette->GetColors(
			ARRAYSIZE(rgColors),
			rgColors,
			&cColorsCopied);
	}

	if (SUCCEEDED(hr))
	{
		// Check whether background color is outside range 
		hr = (backgroundIndex >= cColorsCopied) ? E_FAIL : S_OK;
	}

	if (SUCCEEDED(hr))
	{
		// Get the color in ARGB format
		dwBGColor = rgColors[backgroundIndex];

		// The background color is in ARGB format, and we want to 
		// extract the alpha value and convert it in FLOAT
		FLOAT alpha = (dwBGColor >> 24) / 255.f;
	}

	if (pWicPalette != nullptr)
	{
		pWicPalette->Release();
		pWicPalette = nullptr;
	}

	return hr;
}



HRESULT CGifPlayer::LoadRawFrame()
{
	HRESULT hr = E_FAIL;

	// 取gif帧
	ComPtr<IWICBitmapFrameDecode> frame = nullptr;
	hr = m_pWICdecoder->GetFrame(m_NowFrameIndex, frame.GetAddressOf());
	if (!SUCCEEDED(hr))
		return false;
	ComPtr< IWICFormatConverter> converter = nullptr;
	hr = m_pWICfactory->CreateFormatConverter(converter.GetAddressOf());
	if (!SUCCEEDED(hr))
		return false;
	hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeCustom);
	if (!SUCCEEDED(hr))
		return false;
	hr = m_pComposeRT->CreateBitmapFromWicBitmap(converter.Get(), NULL, m_pRawFrame.ReleaseAndGetAddressOf());
	if (!SUCCEEDED(hr))
		return false;

	return hr;
}

HRESULT CGifPlayer::ClearOnD2DTarget()
{
	HRESULT hr = E_FAIL;

	if (m_pComposeRT == nullptr)
		return E_FAIL;

	AutoSectionLocker locker(m_pD3DCSContext);
	m_pComposeRT->BeginDraw();
	m_pComposeRT->PushAxisAlignedClip(&m_gifDecoderCtx.m_FramePosition, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	if (m_pGifInfo->mHasColor)
		m_pComposeRT->Clear(m_gifDecoderCtx.m_BackgroundColor);
	else
		m_pComposeRT->Clear();
	m_pComposeRT->PopAxisAlignedClip();
	hr = m_pComposeRT->EndDraw();

	return hr;
}

HRESULT CGifPlayer::RenderOnD2DTarget()
{
	HRESULT hr = E_FAIL;

	if (m_pComposeRT == nullptr || m_NowFrameIndex >= m_pGifInfo->mFrameCount)
		return E_FAIL;

	auto& frameInfo = m_pGifInfo->mFramePts[m_NowFrameIndex];

	// 保持前一帧渲染结果
	if (frameInfo.mDisposal == EDisposalType::DM_PREVIOUS)
		return S_OK;

	// 获取渲染位置
	m_gifDecoderCtx.m_FramePosition.left = frameInfo.mLeft;
	m_gifDecoderCtx.m_FramePosition.top = frameInfo.mTop;
	m_gifDecoderCtx.m_FramePosition.right = frameInfo.mLeft + frameInfo.mWidth;
	m_gifDecoderCtx.m_FramePosition.bottom = frameInfo.mTop + frameInfo.mHeight;

	// 取Raw帧
	hr = LoadRawFrame();
	if (!SUCCEEDED(hr))
		return false;

	// 首帧、背景帧需要清理背景
	if (m_NowFrameIndex == 0 || frameInfo.mDisposal == EDisposalType::DM_BACKGROUND)
		hr = ClearOnD2DTarget();
	if (!SUCCEEDED(hr))
		return false;

	// 绘制
	AutoSectionLocker locker(m_pD3DCSContext);
	m_pComposeRT->BeginDraw();
	m_pComposeRT->DrawBitmap(m_pRawFrame.Get(), m_gifDecoderCtx.m_FramePosition);
	hr = m_pComposeRT->EndDraw();

	return hr;
}

long long CGifPlayer::GetExpectIndex(const std::chrono::microseconds & expectTime) const
{
	auto& framePts = m_pGifInfo->mFramePts;
	UINT frameCount = m_pGifInfo->mFrameCount;

	// Convert expectTime to milliseconds to match mFramePts unit
	auto expectTimeMs = duration_cast<milliseconds>(expectTime).count();

	// 获取期望的索引
	auto expectLower = std::lower_bound(framePts.begin(), framePts.end(), expectTimeMs,
		[](const SGifInfo::SFramePt& pt, const unsigned long long& curTimeMs)
		{
			return pt.mFramePts < curTimeMs;
		});

	// 未找到则设置为最大索引
	long long nExpectIndex = 0;
	if (expectLower != framePts.end())
	{
		nExpectIndex = expectLower->mFrameIndex;

		long long nLastEndTime = framePts[nExpectIndex].mFramePts;	//上一帧结束时间

		//int nNextIndex = std::min(UINT(nExpectIndex+1), frameCount - 1);
		//long long nNextEndTime = framePts[nNextIndex].mFramePts;	//帧结束时间

		if (expectTimeMs < nLastEndTime)
			nExpectIndex = std::max((long long)0, nExpectIndex - 1);
	}
	else
		nExpectIndex = frameCount - 1;

	// 取最近的帧为期望帧
	if (nExpectIndex > 0 && nExpectIndex <= frameCount - 1)
	{
		long long expectUpper = nExpectIndex;
		long long expectLowerIdx = nExpectIndex - 1;
		long long FramePtLower = framePts[expectLowerIdx].mFramePts;
		long long FramePtUpper = framePts[expectUpper].mFramePts;

		//if(expectTimeMs < FramePtUpper && expectTimeMs>FramePtLower)
		//	nExpectIndex = expectUpper;
		if (FramePtUpper - expectTimeMs < expectTimeMs - FramePtLower)
			nExpectIndex = expectUpper;
		else
			nExpectIndex = expectLowerIdx;
	}

	nExpectIndex = std::min(UINT(nExpectIndex), frameCount - 1);

	// Debug information
	/*qDebug() << "CurTime: " << expectTime.count() << " CurTimeMs: " << expectTimeMs
		<< " ExpectIndex: " << nExpectIndex;*/

	return nExpectIndex;
}

