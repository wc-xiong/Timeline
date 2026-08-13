#include "Player.h"
#include "GSMedia/RenderCommon/CommonFunction.h"
#include "GSMedia/RenderDevice/DeviceResource.h"
#include "Common/IResourceInterface.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CBasePlayer::CBasePlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx)
	: m_playState(EDPS_Invalid)
	, m_ctx(ctx)
	, m_bReady(true)
{
	m_pResBase = pResBase;
	assert(m_pResBase);

	m_strDecodePath = m_pResBase->GetPath();

	InitD3DResource(Device_Decode, ctx.gpuIndex);
}

CBasePlayer::~CBasePlayer()
{
	{
		std::unique_lock<std::mutex> lock(m_mtxVideoData);
		m_videoData.Reset();
	}
}

int CBasePlayer::GetPlayerType()
{
	if (m_pResBase)
		return m_pResBase->GetType();
	return -1;
}

bool CBasePlayer::GetTextView(const microseconds& curTime, SDrawTextureData& videoData)
{
	std::unique_lock<std::mutex> lock(m_mtxVideoData);

	if (m_playState == EDPS_Stop)
	{
		videoData.Reset();
		return false;
	}

	videoData = m_videoData;

	for (auto &it : videoData.vecTextureFrame)
	{
		it.textureFrame.m_pts = curTime;
		it.textureFrame.m_bScrollState = (m_playState == EDPS_Play);
		it.textureFrame.m_bFrameChange = m_bFrameChange;
	}

	m_bFrameChange = false;

	return !videoData.vecTextureFrame.empty();
}

bool CBasePlayer::GetAudioPty(SAudioPlayerProp & pty)
{
	return false;
}

bool CBasePlayer::GetAudioData(const microseconds & curTs, BYTE * pBYTE, int nLen)
{
	return false;
}

bool CBasePlayer::Start(const microseconds& curTime)
{
	m_playState = EDPS_Play;
	return true;
}

bool CBasePlayer::Stop()
{
	m_playState = EDPS_Stop;
	return true;
}

void CBasePlayer::UpdateTexToData()
{
	std::lock_guard<std::mutex> lock(m_mtxVideoData);
	m_videoData.Reset();

	STextureFrame textureFrame;
	textureFrame.m_texs.push_back(m_sTexs);

	m_videoData.vecTextureFrame.push_back(textureFrame);

	m_bFrameChange = true;
}

bool CBasePlayer::UpdateImageToData(QImage& img)
{
	m_width = img.width();
	m_height = img.height();

	bool bReset = false;
	if (!m_sTexs.texNode0.pTex)
	{
		bReset = true;
	}
	else
	{
		bReset = (m_sTexs.texNode0.texDesc.Width != m_width || m_sTexs.texNode0.texDesc.Height != m_height);
	}

	if (bReset)
	{
		HRESULT hr = CreateD3DTextureFromMemory(m_pD3DDevice.Get(), nullptr, img.width(), img.height(),
			DXGI_FORMAT_R8G8B8A8_UNORM, img.bytesPerLine(), m_sTexs.texNode0.pTex.ReleaseAndGetAddressOf(), true);
		if (FAILED(hr))
			return false;

		if (!m_sTexs.texNode0.UpdateTexPty())
			return false;
	}

	m_pD3DDeviceContext->UpdateSubresource(m_sTexs.texNode0.pTex.Get(), 0, nullptr, img.bits(), img.bytesPerLine(), 0);
	m_pD3DDeviceContext->Flush();

	UpdateTexToData();

	return true;
}

QString CBasePlayer::GetResPath() 
{
	return m_strDecodePath;
}
