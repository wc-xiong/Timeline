#include "VideoPlayer.h"

#include "Play/PlayCore/VABaseDecoder.h"
#include "Common/IResourceInterface.h"
#include "Model/ResourceModel/ResourceModel.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Common/CommonFunctionFileOp.h"
#include "Common/PerformanceDefine.h"
#include <iostream>
#include <QImage>
#include <QDebug>
#include "Common/ResourceDefine.h"
#include "Model/GlobalModel/GlobalModel.h"
#include "GSMedia/RenderCommon/CommonFunction.h"
#include "Common/AVCommon.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

//////////////////////////////////////////////////////////////////////

static constexpr int VIDEO_QUEUE_SIZE = 5;
static constexpr int HALF_VIDEO_QUEUE_SIZE = VIDEO_QUEUE_SIZE / 2;

//////////////////////////////////////////////////////////////////////

CVideoPlayer::CVideoPlayer(std::shared_ptr<IResourceBase> pResBase,
	const BasePlayerContext& ctx,
	const milliseconds& starttime, const milliseconds& playtime, const milliseconds& mediaPlayTime)
	: QObject(nullptr), CBasePlayer(pResBase, ctx)
	, m_bPrepared(false), m_clipStartTime(starttime), m_clipCropPlayTime(playtime), m_mediaPlayTime(mediaPlayTime)
{
	m_firstPts = microseconds(0);
	m_nLastVideoDecodePerSecond = 0;

	// 如果媒体播放时长大于媒体素材截取时长则需要内部自动循环
	if (m_mediaPlayTime > m_clipCropPlayTime)
	{
		m_bCirclePlay = true;
	}

	if (pResBase->GetType() == ERT_Video)
		m_pVideoRes = std::dynamic_pointer_cast<IVideoResource>(pResBase);
	else if (pResBase->GetType() == ERT_Audio)
		m_pAudioRes = std::dynamic_pointer_cast<IAudioResource>(pResBase);
	else
		assert(0);
}

CVideoPlayer::~CVideoPlayer()
{
	Stop();
}

bool CVideoPlayer::Start(const microseconds& curTime)
{
	if (!m_bPrepared)
	{
		if (!Prepare())
		{
			emit signal_ReadyFinish(m_ctx.resId);
			return false;
		}
	}

	CBasePlayer::Start(curTime);

	emit signal_ReadyFinish(m_ctx.resId);

	return true;
}

bool CVideoPlayer::Prepare()
{
	m_filePath = m_pResBase->GetPath().toStdString();			// 路径
		
	bool ret = PrepareVideo();

	m_pResBase->SetHWDEC(m_nDecodeType == EDT_HARDWARD);
	return ret;
}

bool CVideoPlayer::Seek(LONGLONG dwTime)
{
	// 如果没有视频，则默认为已准备
	m_bReady = !CVAPlayer::HasVideo();
	{
		QReadLocker lock(&m_videoMutex);

		// Seek位置与视频缓存队列中数据差值在一帧之内，默认为已准备
		if (!m_queVideoData.empty() && abs(dwTime * 1000L - m_queVideoData[0]->m_pts.count()) <= m_videoFrameTime.count())
		{
			m_bReady = true;
			return CBasePlayer::Seek(dwTime);
		}
	}

	dwTime += m_clipStartTime.count();
	if (dwTime < m_firstPts.count() / 1000L && m_firstPts <= m_clipStartTime + m_clipCropPlayTime)
		dwTime = m_firstPts.count() / 1000L;

	if(m_pVAPlayer)
		SeekDecoder(milliseconds(dwTime), true, true);

	return CBasePlayer::Seek(dwTime);
}

bool CVideoPlayer::Stop()
{
	m_bPrepared = false;
	CBasePlayer::Stop();
	return CVAPlayer::Stop();
}

bool CVideoPlayer::Restart(const microseconds& curTime)
{
	if (Seek(0))
	{
		Start(curTime);
	}

	return CBasePlayer::Restart(curTime);
}

bool CVideoPlayer::SetPlayTime(const milliseconds & starttime, const milliseconds & playtime)
{
	m_clipStartTime		= starttime;
	m_clipCropPlayTime	= playtime;

	return true;
}

bool CVideoPlayer::IsSeekReady()
{
	if (m_videoPtsOfLastQuery == m_firstPts)
		return true;

	return false;
}

void CVideoPlayer::SeekToReadyPosition()
{
	// 默认情况，媒体素材获取的播放数据从头部开始（建议换成m_clipStartTime，待测试）
	m_videoPtsOfLastQuery = m_audioPtsOfLastQuery = m_firstPts;
}

void CVideoPlayer::SeekToReady()
{
	Seek(0);
}

EDecoderType CVideoPlayer::GetDecodeType()
{
	return EDecoderType(m_nDecodeType);
}

bool CVideoPlayer::IsMediaEnough()
{
	return CVAPlayer::IsVideoEnough(m_bCirclePlay);
}

bool CVideoPlayer::GetAudioData(const microseconds& curTime64, BYTE* pBYTE, int nLen)
{
	if (m_playState == EDPS_Stop || m_playState == EDPS_Invalid)
		return false;

	m_audioPtsOfLastQuery = curTime64;

	if (curTime64 >= microseconds(m_clipCropPlayTime)) // 截断时不播放后续内容
		return false;

	microseconds curPts = microseconds(m_clipStartTime) + curTime64;

	return CVAPlayer::GetAudioData(curPts, pBYTE, nLen);
}

bool CVideoPlayer::GetAudioPty(SAudioPlayerProp& pty)
{
	return CVAPlayer::GetAudioPty(pty);
}

bool CVideoPlayer::IsSupportAudio()
{
	return CVAPlayer::IsSupportAudio();
}

bool CVideoPlayer::GetTextView(const microseconds& curTime, SDrawTextureData& videoData)
{
	m_videoPtsOfLastQuery = curTime;

	//printf("GetTexView pts = %lld.\n", curTime.count());
#if 0
	if (m_playState == EDPS_Seek)
		return !videoData.vecTextureFrame.empty();

	if (m_playState == EDPS_Stop || m_playState == EDPS_Invalid || m_playState == EDPS_Prepare)
	{
		videoData.Reset();
		return false;
	}
#endif

	// 音频直接返回缩略图
	if (GetPlayerType() == ERT_Audio)
	{
		videoData.Reset();
		return CBasePlayer::GetTextView(curTime, videoData);
	}

	//if (m_clipPlayTime <= microseconds(0) || curTime > microseconds(m_clipCropPlayTime)) // 截断时不播放后续内容
	//	return false;

	microseconds realPlayTime = curTime + m_clipStartTime;
	if (realPlayTime <= m_firstPts)
		realPlayTime = m_firstPts;
	
	STextureFrame frameData;
	bool ret = CVAPlayer::GetVideoData(realPlayTime, frameData);

	int nIndex = -1;
	if (!videoData.vecTextureFrame.empty())
		nIndex = videoData.vecTextureFrame[0].textureFrame.nFrameIndex;

	if (ret)
	{
		if(m_pVideoRes)
			frameData.m_TexColorPty.color_range = m_pVideoRes->GetColorRange();

		/*if (nIndex == frameData.nFrameIndex)
			printf("------Get Error Frame :%d----------:time:%lld, lastTs:%lld\n", nIndex, realPlayTime.count(), lstGetTs);*/

		videoData.Reset();
		videoData.vecTextureFrame.push_back(SDrawTexture(frameData));

		__int64 nCurDrawTime = frameData.m_pts.count();
		if (m_nLastDrawTime != nCurDrawTime && nCurDrawTime != -1)
		{
			m_nLastDrawTime = nCurDrawTime;
			m_nLastVideoDrawPerSecond++;

			//qDebug() << "--------------------------: cur Draw Speed is:" << m_nLastVideoDrawPerSecond.load();
		}
	}

	return ret && !videoData.vecTextureFrame.empty();
}

bool CVideoPlayer::PrepareReady()
{
	if ((m_playState == EDPS_Prepare || m_playState == EDPS_Seek) && !CVAPlayer::IsVideoEmpty())
	{
		m_bReady = true;
		return true;
	}

	return false;
}

bool CVideoPlayer::IsEOF()
{
	return CVAPlayer::IsComplete();
}

bool CVideoPlayer::PresentTexture(STextureFrame* pVideoData)
{
	if (!pVideoData)
	{
		PrepareReady();
		return false;
	}

	CVAPlayer::PresentTexture(pVideoData);

	if (pVideoData && !m_bReady && m_playState == EDPS_Seek && CVAPlayer::IsVideoEnough(m_bCirclePlay))
	{
		m_bReady = true;
	}

	if (m_strVideoFrameType.isEmpty())
		m_strVideoFrameType = GetVideoFrameType(pVideoData);

	m_nLastVideoDecodePerSecond++;
	return true;
}

void CVideoPlayer::VideoComplete()
{
	if (m_bCirclePlay)
	{
		MediaLoopPlayback();
	}
	else
	{
		CVAPlayer::VideoComplete();
	}
}

bool CVideoPlayer::HasAudio()
{
	return CVAPlayer::HasAudio();
}

std::pair<QUuid, SVideoDecodeData> CVideoPlayer::GetVideoDecodeData()
{
	if (!m_pVAPlayer)
		return { m_ctx.resId , SVideoDecodeData {} };

	int nVideoDecodeSpeed = std::min(m_nLastVideoDecodePerSecond.load(), 70);
	m_nLastVideoDecodePerSecond = 0;

	int nVideoDrawSpeed = std::min(m_nLastVideoDrawPerSecond.load(), 70);
	m_nLastVideoDrawPerSecond = 0;
	
	SVideoDecodeData node = SVideoDecodeData(nVideoDecodeSpeed, 
		m_pVAPlayer->GetVideoDecodePacketSize(), 
		m_pVAPlayer->GetVideoDecodeFrameSize(),
		nVideoDrawSpeed);
	return { m_ctx.resId,  node };
}

QString CVideoPlayer::GetVideoFrameFormat()
{
	return m_strVideoFrameType;
}

bool CVideoPlayer::PrepareVideo()
{
	SPlayerResource pr;
	pr.idxAdapter = m_adapterDesc.DeviceId;
	pr.idxDevice = Device_Decode;
	pr.gpuIndex = m_gpuIndex;

	pr.pCriticalSection = m_pD3DCSContext;
	pr.pD3DDevice = m_pD3DDevice;

	// 软解复用状态：仅写入 pr；共享/独立分流由 InitPlayer → SoftShareSession::TryAttachIfEligible
	// 门控命中后失败不降级（Init 失败 + SoftShare NoFallback 日志）
	pr.bMultiGpusMode = m_ctx.bMultiGpusMode;
	pr.shareKey =
		m_ctx.layerId.toString().toStdString() + "|" +
		m_ctx.mediaId.toString().toStdString() + "|" +
		m_filePath;

	m_nDecodeType = ToDecoderType(m_ctx.decodingMode);

	// 如果素材截取起点位置不是0ms，则在创建解码器时直接从指定位置开始解码
	bool bLocate	= (m_clipStartTime != 0ms);
	bool bNeedVideo = (GetPlayerType() == ERT_Video);
	// 独立路径音频亲和预填；共享路径注册资格以 IsSupportAudio（CreateOrBind 后）为准
	bool bShouldDecodeAudio = m_ctx.bShouldDecodeAudio;

	bool bInitOk	= CVAPlayer::InitPlayer(pr, m_filePath, static_cast<EDecoderType>(m_nDecodeType),
		bNeedVideo, bShouldDecodeAudio, bLocate, m_clipStartTime);

	if (!bInitOk)
		return false;

	m_pVAPlayer->GetDecType(m_nDecodeType);

	if (GetPlayerType() == ERT_Audio)
	{
		QImage img;
		if (img.load(":/dark/images/ResourceWnd/Thumbmail/Normal/Audio.png"))
		{
			img = img.convertToFormat(QImage::Format_RGBA8888);
		}
		 
		
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

				if(texNode.UpdateTexPty())
				{
					STexDatas data;
					data.texNode0 = texNode;

					STextureFrame textureData;
					textureData.m_texs.push_back(data);
					textureData.m_bFrameChange = true;
					{
						std::lock_guard<std::mutex> lock(m_mtxVideoData);
						m_videoData.Reset();
						m_videoData.vecTextureFrame.push_back(SDrawTexture(textureData));
					}
				}
			}
		}
	}
	else
	{
		microseconds pta(0), ptb(0);
		pta = microseconds(QueryHighTickCountUS());

		while (true && (m_nDecodeType != -1))
		{
			// wait,确定第一帧帧数值
			{
				QReadLocker lock(&m_videoMutex);
				if (!m_queVideoData.empty())
				{
					m_firstPts = m_queVideoData.front()->m_pts;
					break;
				}
			}

			ptb = microseconds(QueryHighTickCountUS());
			if ((ptb - pta).count() >= 100000LL)
				break;
			Sleep(1);
		}
	}

#if 0
	if (m_clipStartTime.count() != 0)
		m_pVAPlayer->Seek(0, m_clipStartTime, true, true);
#endif

	m_bPrepared = true;

	return CBasePlayer::Prepare();
}

QString CVideoPlayer::GetVideoFrameType(STextureFrame* texFrame)
{
	QString strVideoType = "Unknown";

	if (texFrame && texFrame->m_texs.size() && texFrame->m_texs[0].texNode0.pTex)
	{
		D3D11_TEXTURE2D_DESC desc = texFrame->m_texs[0].texNode0.texDesc;
		switch (desc.Format)
		{
		case DXGI_FORMAT_R8_UNORM:	//yuv420
			strVideoType = "YUV420";
			break;
		case DXGI_FORMAT_NV12:	//NV12
			strVideoType = "NV420";
			break;
		case DXGI_FORMAT_P010:	//P010
			strVideoType = "P010";
			break;
		case DXGI_FORMAT_R8G8_B8G8_UNORM:	//RGB
			strVideoType = "RGB";
			break;
		case DXGI_FORMAT_B8G8R8A8_UNORM:		//RGBA
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:	//RGBA
		case DXGI_FORMAT_R32G32B32A32_FLOAT:	//RGBA
		case DXGI_FORMAT_R10G10B10A2_UNORM:		//RGBA
			strVideoType = "RGBA";
			break;
		default:
			break;
		}
	}

	return strVideoType;
}

void CVideoPlayer::MediaLoopPlayback()
{
	if (m_pVAPlayer)
		SeekDecoder(m_clipStartTime, true, false);
}

