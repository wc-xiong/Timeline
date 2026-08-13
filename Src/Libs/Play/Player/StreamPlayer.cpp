#include "StreamPlayer.h"
#include "Common/CommonFunctionApi.h"
#include "Model/ResourceModel/ResourceModel.h"
#include "Common/IResourceInterface.h"
#include <thread>
#include <future>
#include <QDebug>
#include "Common/ResourceDefine.h"
#include "Common/AVCommon.h"
#include "Common/AudioCommon.h"
#include "Play/StreamMedia/StreamMediaExport.h"
#include "Play/StreamMedia/StreamMediaInterface.h"
#include "Play/StreamMedia/MediaStream.h"
#include "GSMedia/RenderDevice/DeviceResource.h"
#include "GSMedia/RenderCommon/CommonFunction.h"
#include "GSMedia/TimeServer/TimeServerExport.h"
#include "Controll/PlayStateCenter/PlayStateCenterExport.h"


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

CStreamPlayer::CStreamPlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx)
	: CBasePlayer(pResBase, ctx)
	, m_pStream(nullptr)
	, m_bHasVideo(false)
	, m_bHasAudio(false)
{
	m_streamPlayerId = QUuid::createUuid();
}

CStreamPlayer::~CStreamPlayer()
{
	Stop();

	{
		std::unique_lock<std::shared_timed_mutex> LockerStream(m_mtxStream);
		if (m_pStream)
		{
			StreamMedia_SetStreamBindResPlayId(GetPlayerType(), m_strStreamKey, m_streamPlayerId, false);
			StreamMedia_DestroyStream(GetPlayerType(), m_strStreamKey, false);
			m_pStream = nullptr;
		}
	}
}

void CStreamPlayer::BeforePlayerFree(bool bNormalScene)
{
	if (bNormalScene)
		Stop();
}

bool CStreamPlayer::IsStreamMedia()
{
	return true;
}

bool CStreamPlayer::Start(const microseconds& curTime)
{
	{
		std::shared_lock<std::shared_timed_mutex> LockerStream(m_mtxStream);
		if (m_pStream)
			m_pStream->Start();
	}

	return CBasePlayer::Start(curTime);
}

bool CStreamPlayer::Prepare()
{
	PreperStream(false);

	StreamMedia_SetStreamBindResPlayId(GetPlayerType(), m_strStreamKey, m_streamPlayerId, true);
	return true;
}

namespace
{
	bool IsValidStreamTextureFrame(const STextureFrame& frame)
	{
		if (frame.m_texs.empty())
			return false;
		const auto& node0 = frame.m_texs[0].texNode0;
		return node0.pTex != nullptr && node0.texHandle != nullptr;
	}

	bool SameStreamTextureHandles(const STextureFrame& a, const STextureFrame& b)
	{
		if (a.m_texs.empty() || b.m_texs.empty())
			return false;
		const auto& ta = a.m_texs[0];
		const auto& tb = b.m_texs[0];
		return ta.texNode0.texHandle == tb.texNode0.texHandle
			&& ta.texNode1.texHandle == tb.texNode1.texHandle
			&& ta.texNode2.texHandle == tb.texNode2.texHandle;
	}
}

bool CStreamPlayer::GetTextView(const microseconds & curTime, SDrawTextureData & videoData)
{
	if (!m_pStream)
		return false;

	microseconds curPts;
	TimeServer_GetCurTime(curPts);

	const auto tlRenderInfo = PlayStateCenter::GetStateManager()->GetTimelinePlayInfo(curPts, m_ctx.tlId);

	// 时间线暂停：提交上一有效帧（hold-last），避免空纹理导致闪绿
	if (tlRenderInfo.nPlayState != EnPlayState::EPS_PLAYING)
	{
		std::lock_guard<std::mutex> lock(m_mtxVideoData);
		if (!m_videoData.vecTextureFrame.empty())
		{
			videoData = m_videoData;
			return true;
		}
		return false;
	}

	if (GetPlayerType() == ERT_AudioCapture)
	{
		videoData.Reset();
		return CBasePlayer::GetTextView(curTime, videoData);
	}

	if (m_bHasAudio && !m_bAudioInitFinish)
	{
		{
			std::shared_lock<std::shared_timed_mutex> LockerStream(m_mtxStream);
			m_audioFormat = m_pStream->GetAudioFormat();
		}
		if (m_audioFormat.Channels > 0)
		{
			SAudioPlayerProp pty;
			pty.channels = m_audioFormat.Channels;
			pty.sampleRate = m_audioFormat.SampleRate;
			pty.bitsPerSample = m_audioFormat.Sample;
			pty.nSamples = m_audioFormat.GetSampleCount();
			m_addAudioRenderCallback(pty, m_ctx);
			m_bAudioInitFinish = true;
		}
	}

	if (m_bHasVideo)
	{
		int64_t msTex = 0;
		STextureFrame textureFrame;
		bool ret = false;
		{
			std::shared_lock<std::shared_timed_mutex> LockerStream(m_mtxStream);
			ret = m_pStream->GetVideoData(textureFrame, msTex);
		}

		std::lock_guard<std::mutex> lock(m_mtxVideoData);
		if (ret && IsValidStreamTextureFrame(textureFrame))
		{
			if (!m_videoData.vecTextureFrame.empty()
				&& SameStreamTextureHandles(textureFrame, m_videoData.vecTextureFrame[0].textureFrame))
			{
				// 同一帧 HANDLE：复用缓存，避免无意义 Reset
				videoData = m_videoData;
			}
			else
			{
				textureFrame.m_pts = duration_cast<milliseconds>(curTime);
				m_videoData.Reset();
				m_videoData.vecTextureFrame.emplace_back(textureFrame);
				videoData = m_videoData;
			}
		}
		else if (!m_videoData.vecTextureFrame.empty())
		{
			// 取帧失败或新帧不完整：保留上一有效帧
			videoData = m_videoData;
		}
	}
	return true;
}

bool CStreamPlayer::GetAudioPty(SAudioPlayerProp & pty)
{
	if (!m_bAudioInitFinish || !m_bHasAudio)
		return false;

	{
		std::shared_lock<std::shared_timed_mutex> LockerStream(m_mtxStream);
		if (!m_pStream)
			return false;
		else
			m_audioFormat = m_pStream->GetAudioFormat();
	}
	
	if (m_audioFormat.Channels <= 0)
		return false;

	pty.channels = m_audioFormat.Channels;
	pty.sampleRate = m_audioFormat.SampleRate;
	pty.bitsPerSample = m_audioFormat.Sample;
	pty.nSamples = m_audioFormat.GetSampleCount();

	return true;
}

bool CStreamPlayer::GetAudioData(const microseconds & curTs, BYTE * pBYTE, int nLen)
{
#if 0
	if (m_playState != EDPS_Play)
		return false;
#endif

	std::shared_lock<std::shared_timed_mutex> LockerStream(m_mtxStream);
	if (!m_bAudioInitFinish || !m_bHasAudio || !m_pStream)
		return false;
	
	std::chrono::microseconds tsStart = std::chrono::microseconds(0), tsEnd = std::chrono::microseconds(0);
	return m_pStream->GetAudio(m_audioFormat, pBYTE, nLen, tsStart, tsEnd);
}

void CStreamPlayer::InitAudioRenderCallback(AddResAudioRenderCallBack task)
{
	m_addAudioRenderCallback = task;
}

bool CStreamPlayer::Stop()
{
	{
		std::shared_lock<std::shared_timed_mutex> LockerStream(m_mtxStream);
		if (m_pStream)
			m_pStream->Stop();
	}

	if(m_pResBase->GetType() != ERT_AudioCapture)	// 音频采集卡不重置缩略图
	{
		std::lock_guard<std::mutex> lock(m_mtxVideoData);
		m_videoData.Reset();
	}
	return CBasePlayer::Stop();
}

bool CStreamPlayer::Pause(const microseconds & curTime)
{
	{
		std::shared_lock<std::shared_timed_mutex> LockerStream(m_mtxStream);
		if (m_pStream)
			m_pStream->Pause();
	}
	return  CBasePlayer::Pause(curTime);
}

bool CStreamPlayer::Restart(const microseconds& curTime)
{
	Start(curTime);

	return CBasePlayer::Restart(curTime);
}

bool CStreamPlayer::PreperStream(bool bReStart)
{
	m_strStreamKey = m_ctx.resId.toString();

	//创建流媒体
	{
		std::unique_lock<std::shared_timed_mutex> LockerStream(m_mtxStream);
		m_pStream = StreamMedia_CreateBindRes(m_pResBase, false);
		if (!m_pStream)
			return false;
	}

	switch (m_pResBase->GetType())
	{
	case EResourceType::ERT_AudioCapture:
	{
		auto pCapNode = std::dynamic_pointer_cast<IAudioCaptureResource>(m_pResBase);
		if (!pCapNode)
			return false;
		m_strStreamKey = m_pResBase->GetPath() + pCapNode->GetDeviceId();

		m_bHasVideo = false;
		m_bHasAudio = true;

		QImage img;
		if (img.load(":/dark/images/ResourceWnd/Thumbmail/Normal/AudioCapture.png"))
			img = img.convertToFormat(QImage::Format_RGBA8888);

		
		ComPtr<ID3D11Texture2D> pImageTex = nullptr;
		HRESULT hr = CreateD3DTextureFromMemory(m_pD3DDevice.Get(), img.bits(), img.width(), img.height(),
			DXGI_FORMAT_R8G8B8A8_UNORM, img.bytesPerLine(), pImageTex.ReleaseAndGetAddressOf(), true);

		if (SUCCEEDED(hr))
		{
			D3D11_TEXTURE2D_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			pImageTex->GetDesc(&desc);

			STexNode texNode;

			hr = CreateD3DTextureFromMemory(m_pD3DDevice.Get(), nullptr, img.width(), img.height(),
				DXGI_FORMAT_R8G8B8A8_UNORM, img.bytesPerLine(), texNode.pTex.ReleaseAndGetAddressOf(), true);
			if (SUCCEEDED(hr))
			{
				m_pD3DDeviceContext->CopySubresourceRegion(texNode.pTex.Get(), 0U, 0U, 0U, 0U, pImageTex.Get(), 0, nullptr);
				m_pD3DDeviceContext->Flush();

				if (texNode.UpdateTexPty())
				{
					STexDatas data;
					data.texNode0 = texNode;

					STextureFrame textureData;
					textureData.m_texs.push_back(data);
					{
						std::lock_guard<std::mutex> lock(m_mtxVideoData);
						m_videoData.Reset();
						m_videoData.vecTextureFrame.push_back(SDrawTexture(textureData));
					}
				}
			}
		}
	}
		break;
	case EResourceType::ERT_VideoCapture:
	{
		auto pCapNode = std::dynamic_pointer_cast<IVideoCaptureResource>(m_pResBase);
		if (!pCapNode)
			return false;
		m_strStreamKey = m_pResBase->GetPath() + pCapNode->GetDeviceId();

		m_bHasVideo = true;
		m_bHasAudio = false;

		break;
	}
	case EResourceType::ERT_WebMedia:
	{
		m_bHasVideo = true;
		m_bHasAudio = true;
		m_bAudioInitFinish = false;
		break;
	}
	case ERT_Ndi:
	{
		m_bHasVideo = true;
		m_bHasAudio = true;
	}
		break;
	}

	return CBasePlayer::Prepare();
}
