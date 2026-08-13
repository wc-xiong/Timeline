#include "VADecoder.h"
#include <assert.h>
#include <algorithm>

#include <iostream>
#include "MMDecoder.h"
#include "Common/CommonNonPlatform.h"
#include "Common/AVCommonFunction.h"
#include "Common/VideoDataCommon.h"

#include "boost/thread/condition_variable.hpp"

#pragma comment(lib, "libboost_thread-vc141-mt-x64-1_69.lib")
#pragma comment(lib, "libboost_chrono-vc141-mt-x64-1_69.lib")

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

#define	MAX_TRANS_THREAD_SIZE	10	// 最大转换线程个数

CVADecoder::CVADecoder()
	: m_pDecoder(new CMMDecoder())
	, m_pVideoSwrctx(nullptr)
	, m_pAudioSwrctx(nullptr)
	, m_bFirstTimeDealFrame(true)
	, m_videoDealThrd(nullptr)
	, m_audioDealThrd(nullptr)
	, m_nTexWidth(0)
	, m_nTexHeight(0)
	, m_llAudioClk(0ll)
{
}

CVADecoder::~CVADecoder()
{
	if (m_pDecoder)
	{
		delete m_pDecoder;
		m_pDecoder = nullptr;
	}
}

bool CVADecoder::Init(
	const char *pFile, 
	EDecoderType emDecType, 
	SPlayerResource playerResource,
	bool bNeedVideo, 
	bool needAudio,
	bool bLocate,
	std::chrono::milliseconds position
	)
{
	{
		AddPlayerResource(playerResource);
	}

	// sticky 创建者：仅该 gpuIndex 有资格 Seek 底层解码器（与音频先来后到归属解耦）
	m_creatorGpuIndex = playerResource.gpuIndex;

	if (!(m_pDecoder->Init(pFile, bNeedVideo, !needAudio, (EDecoderType)emDecType, playerResource.idxAdapter, TRUE, playerResource.pDECCallback, bLocate, position,
		playerResource.gpuIndex)))
		return false;

	m_bEnd.store(false);

	if (m_pDecoder->HasAudio())
	{
		bool bResult = CAudioChecker::GetInstance()->CheckAudioProperty(m_pDecoder->m_pAudioCodecContext->ch_layout,
			m_pDecoder->m_pAudioCodecContext->sample_rate,
			m_sAudioProperty);
		if (!bResult)
		{
#ifdef _DEBUG
			MessageBox(NULL, L"音频初始化失败", L"GrandShow", MB_OK);
#endif // DEBUG
		}
	}

	Thread();

	if (!m_pDecoder->IsDropDecEnable())
	{
		m_pDropJudge = new CDropJudgment();
		m_pDropJudge->SetCalculateInteval(microseconds(1000000));
		m_pDECCallback = playerResource.pDECCallback;
		std::function<microseconds()> getRenderTime = [this]()->microseconds { return m_pDECCallback->GetRenderVideoPts(); };
		m_pDropJudge->SetGetRenderTimeFunction(getRenderTime);
	}

	return true;
}

int CVADecoder::GetVideoDecodePacketSize()
{
	return m_pDecoder->GetVideoPackSize();
}

int CVADecoder::GetVideoDecodeFrameSize()
{
	return m_pDecoder->GetVideoFrameSize();
}

bool CVADecoder::GetDecType(int& pType)
{
	pType = m_pDecoder->GetDecoderType();
	return true;
}

bool CVADecoder::GetAudioPty(int gpuIndex, SAudioPlayerProp& pty)
{
	// 纯校验：音频归属 = 先来后到（到达序号最小的 PR）
	if (gpuIndex < 0 || (UINT)gpuIndex != GetAudioOwnerKey())
		return false;

	if (!m_pDecoder->HasAudio())
		return false;

	pty.channels	= m_sAudioProperty.m_channels;
	pty.sampleRate	= m_sAudioProperty.m_sampleRate;
	pty.nSamples	= m_sAudioProperty.m_nSample;

	return true;
}

std::chrono::microseconds CVADecoder::GetFrameTime()
{
	return microseconds(m_pDecoder->GetFrameTime());
}

std::chrono::microseconds CVADecoder::GetAudioWaitTime()
{
	return m_sAudioProperty.m_waitTime;
}

int CVADecoder::HasVideo()
{
	return m_pDecoder->HasVideo();
}

int CVADecoder::HasAudio()
{
	return m_pDecoder->HasAudio();
}

bool CVADecoder::IsComplete()
{
	return m_pDecoder->IsComplete();
}

bool CVADecoder::UnInit()
{
	//m_bEnd = TRUE;
	m_bEnd.store(true);
	if (m_audioDealThrd && m_audioDealThrd->joinable())
		m_audioDealThrd->join();
	if (m_videoDealThrd && m_videoDealThrd->joinable())
		m_videoDealThrd->join();

	//if (m_hPlayerThread)
	//{
	//	WaitForSingleObject(m_hPlayerThread, INFINITE);
	//	CloseHandle(m_hPlayerThread);
	//	m_hPlayerThread = NULL;
	//}

	if (m_pVideoSwrctx)
	{
		sws_freeContext(m_pVideoSwrctx);
		m_pVideoSwrctx = nullptr;
	}

	if (m_pDropJudge)
	{
		delete m_pDropJudge;
		m_pDropJudge = NULL;
	}

	if (m_pAudioSwrctx)
		swr_free(&m_pAudioSwrctx);

	m_pHWDeviceContex.Reset();
	m_pHWDevice.Reset();
	m_pFlushQuery.Reset();
	m_pDecoder->UnInit();

	ClearPlayerResources();

	return true;
}

bool CVADecoder::Seek(int gpuIndex, milliseconds position, bool bExact, bool bReset)
{
	if (m_bEnd)
		return false;

	// 共享场景（PR 表 >1）单写者门控：仅建播创建者执行；创建者退出后 Seek 不移交
	{
		int prCount = 0;
		{
			QReadLocker lock(&m_mtxPlayerResource);
			prCount = (int)m_playerResources.size();
		}
		if (prCount > 1 && gpuIndex != (int)m_creatorGpuIndex)
			return true;
	}

	m_bSeeking = true;
	CAutoLock autoLock(&m_csVideoLock);
	CAutoLock autoLock2(&m_csAudioLock);
	m_pDecoder->Seek(position.count(), bExact);
	if (bReset)
	{
		ClearAVData();
	}

	m_llAudioClk = position.count();

	m_bSeeking = false;
	return true;
}

void CVADecoder::Thread()
{
	m_audioDealThrd = std::make_unique<std::thread>([&]()
	{
		while (!m_bEnd.load())
		{
			if (m_pDecoder->HasAudio())
			{
				CAutoLock autoLock(&m_csAudioLock);
				const UINT audioOwner = GetAudioOwnerKey();
				if (m_pDecoder->IsAudioComplete())
				{
					QReadLocker lock(&m_mtxPlayerResource);
					if (m_playerResources.count(audioOwner))
					{
						m_playerResources[audioOwner].second.pVACallbacker->AudioComplete();
					}
				}

				//处理音频数据
				bool dealAudioData = false;
				{
					QReadLocker lock(&m_mtxPlayerResource);
					dealAudioData = m_playerResources.count(audioOwner) && !m_playerResources[audioOwner].second.pVACallbacker->IsAudioEnough(false);
				}

				if (dealAudioData)
				{
					AVFrame *pFrame = m_pDecoder->GetAudioFrame();
					if (pFrame)
					{
						//LOG_TIME_ELAPSED("DealAudioData", [&]() {
						DealAudioData(pFrame);
						av_frame_unref(pFrame);
						av_frame_free(&pFrame);
						//});

					}
				}
			}
			boost::this_thread::sleep(boost::chrono::milliseconds(2));	//部分视频音频帧长度只有5ms,Sleep(5)会导致无法获取到音频数据
		}
	});

	m_videoDealThrd = std::make_unique<std::thread>([&]()
	{
		while (!m_bEnd.load())
		{
			if (m_pDecoder->HasVideo())
			{
				//处理视频纹理
				CAutoLock autoLock(&m_csVideoLock);
				if (m_pDecoder->IsVideoComplete())
				{
					QReadLocker lock(&m_mtxPlayerResource);
					for (auto &pr : m_playerResources)
					{
						pr.second.second.pVACallbacker->VideoComplete();
					}
				}

				{
					if (!IsVideoEnough())
					{
						AVFrame *pFrame = m_pDecoder->GetVideoFrame();
						if (pFrame)
						{
							if (nullptr != m_pDropJudge && m_pDropJudge->IsNeedDropSample(microseconds(pFrame->pts * 1000ll), microseconds(av_gettime()), m_fSpeed))
							{
								std::cout << "CVADecoder::Thread ---- Dropping one frame" <<std::endl;
							}
							else
							{
								DealVideoData(pFrame);
							}

							av_frame_unref(pFrame);
							av_frame_free(&pFrame);
						}
					}
				}
			}

			//Sleep(2);	//部分视频音频帧长度只有5ms,Sleep(5)会导致无法获取到音频数据
			boost::this_thread::sleep(boost::chrono::milliseconds(2));
		}
	});
}

void CVADecoder::DealAudioData(AVFrame * pFrame)
{
	const UINT audioOwner = GetAudioOwnerKey();
	{
		QReadLocker lock(&m_mtxPlayerResource);
		if (!m_playerResources.count(audioOwner))
			return;
	}

	//异常判断，部分视频读取错误会导致frame属性错误
	if ((pFrame->ch_layout.nb_channels != m_pDecoder->m_pAudioCodecContext->ch_layout.nb_channels)
		|| (pFrame->sample_rate != m_pDecoder->m_pAudioCodecContext->sample_rate))
	{
		std::cout << "音频帧属性错误, 丢弃!" <<std::endl;
		return;
	}
	
	try
	{
		if (!m_pAudioSwrctx)
		{

			int errorMsg = swr_alloc_set_opts2(				// 设置重采样参数
				&m_pAudioSwrctx,												// ==null 新建一个SwrContext
				&m_sAudioProperty.m_channelLayout,					// 输出声道
				AV_SAMPLE_FMT_S16,			// 输出格式
				m_sAudioProperty.m_sampleRate,							// 输出采样率
				&pFrame->ch_layout,									// 输入声道
				(AVSampleFormat)pFrame->format,						// 输入格式
				pFrame->sample_rate,								// 输入采样率
				0,													// log日志相关
				NULL);												// log日志相关

			if ((!m_pAudioSwrctx) || (swr_init(m_pAudioSwrctx) < 0))			// 设置好后参数后初始化
			{
				swr_free(&m_pAudioSwrctx);
				return;
			}
		}

		int out_count = (int64_t)pFrame->nb_samples * m_sAudioProperty.m_sampleRate / pFrame->sample_rate + 256;		// nb_sample：单通道样本数量
		int out_size = av_samples_get_buffer_size(NULL, m_sAudioProperty.m_channels, out_count, AV_SAMPLE_FMT_S16, 0);
		if (out_size < 0)
			return;

		BYTE* pBuf = new BYTE[out_size];

		int size = swr_convert(							// 返回值为每个通道的采样数
			m_pAudioSwrctx,
			(uint8_t**)(&pBuf),							// 输出缓冲区:packet 都存在out[0], planer: out[0]存L, out[1]存R
			out_count,									// 输出缓冲区每通道样本数据数量(对于音频,每个通道数据长度都相同),注意这里不是以字节为单位.
			(const uint8_t **)pFrame->extended_data,	// 输入缓冲区数据
			pFrame->nb_samples);						// 音频通道数

		if (size < 0) // 转换失败
		{
			delete[] pBuf;
			return;
		}

		if (size == out_count)
		{
			av_log(NULL, AV_LOG_WARNING, "audio buffer is probably too small\n");

			if (swr_init(m_pAudioSwrctx) < 0)
				swr_free(&m_pAudioSwrctx);
		}

		// 接收重采样后的数据
		int nDataLen = av_samples_get_buffer_size(NULL, m_sAudioProperty.m_channels, size, AV_SAMPLE_FMT_S16, 1);

		AUDIO_DATA* pAudioData = new AUDIO_DATA;
		pAudioData->pBuf = new BYTE[nDataLen];
		pAudioData->pSeekTo = pAudioData->pBuf;

		//qDebug() << "frame pts " << pFrame->pts;

		if (pFrame->pts == AV_NOPTS_VALUE)
		{
			// 更新音频时钟，根据buffer长度计算时长
			m_llAudioClk += (__int64)(size / ((double)pFrame->sample_rate) * 1000ll);
			pAudioData->pts = microseconds((long long)m_llAudioClk * 1000ll);
		}
		else
		{
			pAudioData->pts = microseconds(pFrame->pts * 1000ll + 1000000ll * pFrame->nb_samples / pFrame->sample_rate);

			LONGLONG c_pts = pAudioData->pts.count() / 1000ll;
			// 避免如1080P accepted 这样的视频中，音频时间戳一部分是 AV_NOPTS_VALUE，但也有有pts的帧
			// 判断一下自己计算的pts与帧的pts对比，如果小了多余一帧数据，那么就需要补上差的数据
			if (m_llAudioClk < c_pts)
			{
				LONGLONG samples_need = (c_pts - m_llAudioClk) / 1000ll * (LONGLONG)pFrame->sample_rate;
				//LONGLONG len_need = samples_need * m_sAudioProperty.m_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
				LONGLONG len_need = av_samples_get_buffer_size(NULL, m_sAudioProperty.m_channels, samples_need, AV_SAMPLE_FMT_S16, 1);
				if (len_need >= nDataLen)
				{
					// 多于nDataLen一半的数据也加多一帧数据
					// 保留成nDataLen的倍数的数据长度，这样比较符合音频完整播放一帧数据
					LONGLONG nNeedDataLen = (len_need + nDataLen / 2) / nDataLen * nDataLen;
					AUDIO_DATA* pNeedData = new AUDIO_DATA;
					pNeedData->pBuf = new BYTE[nNeedDataLen];
					pNeedData->pSeekTo = pNeedData->pBuf;
					pNeedData->nDataLen = nNeedDataLen;
					pNeedData->newpts = pNeedData->pts = microseconds(pAudioData->pts.count() - 1000000ll * pFrame->nb_samples / pFrame->sample_rate);
					memset(pNeedData->pBuf, 0, nNeedDataLen);

					if (m_playerResources.count(audioOwner))
						m_playerResources[audioOwner].second.pVAPresent->PresentAudioData(pNeedData);
					else
						SafeDelete(pNeedData);
				}
			}
		}

		pAudioData->newpts = pAudioData->pts;
		m_llAudioClk = pAudioData->pts.count() / 1000ll;	// 更新自己计算的pts，保持与有效pts的音频保持一致
		pAudioData->nDataLen = nDataLen;
		memcpy(pAudioData->pBuf, pBuf, nDataLen);

		//qDebug() << "pts " << pAudioData->newpts.count() << m_llAudioClk;

		{
			QReadLocker lock(&m_mtxPlayerResource);
			if (m_playerResources.count(audioOwner))
				m_playerResources[audioOwner].second.pVAPresent->PresentAudioData(pAudioData);
			else
				SafeDelete(pAudioData);
		}


		delete[] pBuf;
	}
	catch (...)
	{
		assert(0);
	}
}

void CVADecoder::DealVideoData(AVFrame * pFrame)
{
	{
		QReadLocker lock(&m_mtxPlayerResource);
		if (m_playerResources.empty())
			return;
	}

	LONG nWidth = m_pDecoder->GetWidth();
	LONG nHeight = m_pDecoder->GetHeight();

	const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get((AVPixelFormat)pFrame->format);

	if ((desc->flags & AV_PIX_FMT_FLAG_HWACCEL))	//硬解码纹理
	{
		ID3D11VideoDecoderOutputView* srcSurface = (ID3D11VideoDecoderOutputView*)(uintptr_t)pFrame->data[3];
		if (!srcSurface)
			return;

		D3D11_VIDEO_DECODER_OUTPUT_VIEW_DESC viewDesc;
		srcSurface->GetDesc(&viewDesc);

		ComPtr<ID3D11Texture2D> pHWTexture;
		srcSurface->GetResource((ID3D11Resource**)pHWTexture.GetAddressOf());

		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		pHWTexture->GetDesc(&texDesc);

		if (m_bFirstTimeDealFrame)
		{
			pHWTexture->GetDevice(m_pHWDevice.ReleaseAndGetAddressOf());
			m_pHWDevice->GetImmediateContext(m_pHWDeviceContex.ReleaseAndGetAddressOf());
			D3D11_QUERY_DESC queryDesc = CD3D11_QUERY_DESC(D3D11_QUERY_EVENT, 0);
			m_pHWDevice->CreateQuery(&queryDesc, m_pFlushQuery.ReleaseAndGetAddressOf());

			m_bFirstTimeDealFrame = false;

			m_nTexWidth = nWidth;
			m_nTexHeight = nHeight;

			HRESULT hr = CreateTexture(nullptr, 0, nWidth, nHeight, texDesc.Format, m_pVideoTexture0.ReleaseAndGetAddressOf(), m_playerResources.begin()->second.second.pD3DDevice);
			if (!SUCCEEDED(hr))
			{
				hr = CreateTexture(nullptr, 0, texDesc.Width, texDesc.Height, texDesc.Format, m_pVideoTexture0.ReleaseAndGetAddressOf(), m_playerResources.begin()->second.second.pD3DDevice);
				if (!SUCCEEDED(hr))
					return;

				m_nTexWidth = texDesc.Width;
				m_nTexHeight = texDesc.Height;
			}
		}

		STextureFrame* pVideoData = nullptr;
		pVideoData = m_playerResources.begin()->second.second.pVAPresent->FetchVideoData();
		if (!pVideoData)
		{
			pVideoData = new STextureFrame;
			pVideoData->textType = EVideoTextType::Src_Normal;
			GetVideoProperty(pFrame, EDT_HARDWARD, pVideoData->m_TexColorPty);	// 获取纹理颜色属性
		}

		pVideoData->m_pts = microseconds(pFrame->pts * 1000ll);

		if (pVideoData->m_texs.empty())
		{
			STexDatas tex{};
	
			HRESULT hr = CreateTexture(nullptr, 0, m_nTexWidth, m_nTexHeight, texDesc.Format, tex.texNode0.pTex.ReleaseAndGetAddressOf(), m_playerResources.begin()->second.second.pD3DDevice);
			if (!SUCCEEDED(hr))
			{
				SafeDelete(pVideoData);
				return;
			}

			if (!tex.texNode0.UpdateTexPty())
			{
				SafeDelete(pVideoData);
				return;
			}

			pVideoData->m_texs.push_back(tex);
		}

		ComPtr<ID3D11Texture2D> sharedTexture;
		HRESULT hr = m_pHWDevice->OpenSharedResource(pVideoData->m_texs[0].texNode0.texHandle, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(sharedTexture.GetAddressOf()));
		if (!SUCCEEDED(hr))
		{
			SafeDelete(pVideoData);
			return;
		}

		m_pHWDeviceContex->CopySubresourceRegion(sharedTexture.Get(), 0U, 0U, 0U, 0U, pHWTexture.Get(), viewDesc.Texture2D.ArraySlice, nullptr);
		m_pHWDeviceContex->Flush();
		m_pHWDeviceContex->End(m_pFlushQuery.Get());

		pHWTexture.Reset();
		hr = S_OK;
		
		auto ts = QueryHighTickCountMS();
		while (!m_bEnd.load())
		{
			hr = m_pHWDeviceContex->GetData(m_pFlushQuery.Get(), NULL, 0U, 0U);
			if (hr != S_FALSE)
			{
				if (hr == DXGI_ERROR_DEVICE_REMOVED)
				{
					printf("The GPU device instance has been suspended. Use GetDeviceRemovedReason to determine the appropriate action.\n");
					hr = m_pHWDevice->GetDeviceRemovedReason();
				}
				break;
			}

			if (m_bSeeking || (QueryHighTickCountMS() - ts >= 1000))
			{
				hr = E_FAIL;
				printf("Copy Resource Fail to break.\n");
				break;
			}
		}

		if (!SUCCEEDED(hr))
		{
			SafeDelete(pVideoData);
			return;
		}

		{
			QReadLocker lock(&m_mtxPlayerResource);
			if (!m_playerResources.empty())
				m_playerResources.begin()->second.second.pVAPresent->PresentTexture(pVideoData); //硬解码只有一个 pr
		}

		if (!TryReadStereo(pFrame, pVideoData))
		{
			SafeDelete(pVideoData);
		}
	}
	else
	{
		std::map<int, STextureFrame*> videoDatas = FetchVideoData(microseconds(pFrame->pts * 1000ll));

		auto itor = PIX_TEX.find((AVPixelFormat)pFrame->format);
		if (PIX_TEX.end() != itor)
		{
			int uvHeight = nHeight >> desc->log2_chroma_h;
			int nByte = pFrame->linesize[0] / nWidth;

			assert(nHeight < D3D11_REQ_TEXTURECUBE_DIMENSION);

			for (int yX = 0, uvX = 0, nIndex = 0; yX < nWidth; nIndex++)
			{
				int yWidth = std::min(D3D11_REQ_TEXTURECUBE_DIMENSION, (int)nWidth - yX);
				int uvWidth = yWidth >> desc->log2_chroma_w;

				QReadLocker lock(&m_mtxPlayerResource);
				// 并行创建每个显卡的纹理
				std::async_for_each(m_playerResources.begin(), m_playerResources.end(), true, [&](auto &pr)
				{
					HRESULT hr = S_OK;
					if (videoDatas[pr.first]->m_texs.size() <= nIndex)
					{
						STexDatas tex;
						if (pFrame->data[0])
						{
							hr = CreateTexture(yX * nByte + pFrame->data[0], pFrame->linesize[0],
								yWidth, nHeight, itor->second.dxgiFormat, tex.texNode0.pTex.GetAddressOf(), pr.second.second.pD3DDevice);
							if (!SUCCEEDED(hr))
								return hr;

							if (!tex.texNode0.UpdateTexPty())
								return E_FAIL;
						}

						if (pFrame->data[1])
						{
							hr = CreateTexture(uvX * nByte + pFrame->data[1], pFrame->linesize[1],
								uvWidth, uvHeight, itor->second.dxgiFormat, tex.texNode1.pTex.GetAddressOf(), pr.second.second.pD3DDevice);
							if (!SUCCEEDED(hr))
								return hr;

							if (!tex.texNode1.UpdateTexPty())
								return E_FAIL;
						}

						if (pFrame->data[2])
						{
							hr = CreateTexture(uvX * nByte + pFrame->data[2], pFrame->linesize[2],
								uvWidth, uvHeight, itor->second.dxgiFormat, tex.texNode2.pTex.GetAddressOf(), pr.second.second.pD3DDevice);
							if (!SUCCEEDED(hr))
								return hr;

							if (!tex.texNode2.UpdateTexPty())
								return E_FAIL;
						}

						ComPtr<ID3D11DeviceContext> pDeviceContex;
						pr.second.second.pD3DDevice->GetImmediateContext(pDeviceContex.ReleaseAndGetAddressOf());
						pDeviceContex->Flush();

						videoDatas[pr.first]->textType = EVideoTextType::Src_Normal;
						GetVideoProperty(pFrame, EDT_SOFTWARW, videoDatas[pr.first]->m_TexColorPty);	// 获取纹理颜色属性

						videoDatas[pr.first]->m_texs.push_back(tex);
					}
					else
					{
						EnterCriticalSection(pr.second.second.pCriticalSection);

						ComPtr<ID3D11DeviceContext> pDeviceContex;
						pr.second.second.pD3DDevice->GetImmediateContext(pDeviceContex.ReleaseAndGetAddressOf());
						STexDatas& srcTex = videoDatas[pr.first]->m_texs[nIndex];
						if (pFrame->data[0])
							pDeviceContex->UpdateSubresource(srcTex.texNode0.pTex.Get(), 0, nullptr, yX * nByte + pFrame->data[0], pFrame->linesize[0], 0);
						if (pFrame->data[1])
							pDeviceContex->UpdateSubresource(srcTex.texNode1.pTex.Get(), 0, nullptr, uvX * nByte + pFrame->data[1], pFrame->linesize[1], 0);
						if (pFrame->data[2])
							pDeviceContex->UpdateSubresource(srcTex.texNode2.pTex.Get(), 0, nullptr, uvX * nByte + pFrame->data[2], pFrame->linesize[2], 0);

						pDeviceContex->Flush();
						LeaveCriticalSection(pr.second.second.pCriticalSection);
					}
					return hr;
				});

				yX += yWidth;
				uvX += uvWidth;
			}
		}
		else
		{
			int nAfterWidht = pFrame->width;
			int nAfterHeight = pFrame->height;

			if ((nAfterWidht > D3D11_REQ_TEXTURECUBE_DIMENSION) || (nAfterHeight > D3D11_REQ_TEXTURECUBE_DIMENSION))
			{
				int nRatio = ((nAfterWidht / D3D11_REQ_TEXTURECUBE_DIMENSION) > (nAfterHeight / D3D11_REQ_TEXTURECUBE_DIMENSION) ? (nAfterWidht / D3D11_REQ_TEXTURECUBE_DIMENSION) : (nAfterHeight / D3D11_REQ_TEXTURECUBE_DIMENSION)) + 1;
				nAfterWidht /= nRatio;
				nAfterHeight /= nRatio;

				if ((nAfterWidht % 2) != 0)
					nAfterWidht--;

				if ((nAfterHeight % 2) != 0)
					nAfterHeight--;
			}

			if (!m_pVideoSwrctx)
			{
				//强制转换为709色域下的 full Range yuv420 
				m_pVideoSwrctx = sws_getContext(pFrame->width,
					pFrame->height,
					(enum AVPixelFormat)pFrame->format,
					nAfterWidht,
					nAfterHeight,
					AV_PIX_FMT_YUV420P,
					SWS_BICUBIC,
					NULL, NULL, NULL);

				// 				//软解码还做会颜色转换比较耗时，对于一些比较大的视频会出现卡顿
				// 				int srcRange = (AVCOL_RANGE_JPEG == pFrame->color_range) ? 1 : 0;
				// 				AVColorSpace colorspace = pFrame->colorspace;
				// 				if (colorspace < 1 || colorspace > 10 || colorspace == 8)
				// 					colorspace = AVCOL_SPC_BT470BG;
				// 				int ret = sws_setColorspaceDetails(m_pVideoSwrctx, sws_getCoefficients(colorspace), srcRange, sws_getCoefficients(SWS_CS_ITU709), 1, 0, 1 << 16, 1 << 16);
			}
			int nUVWidth = nAfterWidht / 2;
			int nUVHeight = nAfterHeight / 2;

			uint8_t* data = new uint8_t[nAfterWidht * nAfterHeight * 2];
			uint8_t* dest[4] = { data, data + nAfterWidht * nAfterHeight, data + nAfterWidht * nAfterHeight + nUVWidth * nUVHeight, 0 };
			int linesize[4] = { nAfterWidht, nUVWidth, nUVWidth, 0 };

			sws_scale(m_pVideoSwrctx,
				(const uint8_t* const*)pFrame->data,
				pFrame->linesize,
				0,
				pFrame->height,
				dest,
				linesize);

			QReadLocker lock(&m_mtxPlayerResource);
			// 并行创建每个显卡的纹理
			std::async_for_each(m_playerResources.begin(), m_playerResources.end(), true, [&](auto &pr)
			{
				HRESULT hr = S_OK;
				if (videoDatas[pr.first]->m_texs.empty())
				{
					STexDatas tex;

					{
						hr = CreateTexture(dest[0], linesize[0], nAfterWidht, nAfterHeight, DXGI_FORMAT_R8_UNORM, tex.texNode0.pTex.GetAddressOf(), pr.second.second.pD3DDevice);
						if (!SUCCEEDED(hr))
							return hr;

						tex.texNode0.UpdateTexPty();
					}

					{
						hr = CreateTexture(dest[1], linesize[1], nUVWidth, nUVHeight, DXGI_FORMAT_R8_UNORM, tex.texNode1.pTex.GetAddressOf(), pr.second.second.pD3DDevice);
						if (!SUCCEEDED(hr))
							return hr;
						tex.texNode1.UpdateTexPty();
					}

					{
						hr = CreateTexture(dest[2], linesize[2], nUVWidth, nUVHeight, DXGI_FORMAT_R8_UNORM, tex.texNode2.pTex.GetAddressOf(), pr.second.second.pD3DDevice);
						if (!SUCCEEDED(hr))
							return hr;
						tex.texNode2.UpdateTexPty();
					}

					ComPtr<ID3D11DeviceContext> pDeviceContex;
					pr.second.second.pD3DDevice->GetImmediateContext(pDeviceContex.ReleaseAndGetAddressOf());
					pDeviceContex->Flush();

					videoDatas[pr.first]->textType = EVideoTextType::Src_Normal;
					GetVideoProperty(pFrame, EDT_SOFTWARW, videoDatas[pr.first]->m_TexColorPty);	// 获取纹理颜色属性

					videoDatas[pr.first]->m_texs.push_back(tex);
				}
				else
				{
					EnterCriticalSection(pr.second.second.pCriticalSection);

					ComPtr<ID3D11DeviceContext> pDeviceContex;
					pr.second.second.pD3DDevice->GetImmediateContext(pDeviceContex.ReleaseAndGetAddressOf());
					STexDatas& srcTex = videoDatas[pr.first]->m_texs[0];
					pDeviceContex->UpdateSubresource(srcTex.texNode0.pTex.Get(), 0, nullptr, dest[0], linesize[0], 0);
					pDeviceContex->UpdateSubresource(srcTex.texNode1.pTex.Get(), 0, nullptr, dest[1], linesize[1], 0);
					pDeviceContex->UpdateSubresource(srcTex.texNode2.pTex.Get(), 0, nullptr, dest[2], linesize[2], 0);

					pDeviceContex->Flush();
					LeaveCriticalSection(pr.second.second.pCriticalSection);

				}
				return hr;
			}
			);

			delete[] data;
		}

		PresentTextures(videoDatas);
		videoDatas.clear();
	}
}

bool CVADecoder::TryReadStereo(AVFrame * pFrame, STextureFrame * pVideoData)
{
	if (!pFrame || !pVideoData)
		return false;

	// try to detect Stereo Video Frame
	AVFrameSideData  *pSideData = av_frame_get_side_data(pFrame, AV_FRAME_DATA_STEREO3D);
	if (pSideData != nullptr)
	{
		AVStereo3D *stereo3d = (AVStereo3D *)pSideData->data;
		switch (stereo3d->view) {
		case AV_STEREO3D_VIEW_LEFT:
			pVideoData->textType = EVideoTextType::Src_StereoLeft;
			return true;
		case AV_STEREO3D_VIEW_RIGHT:
			pVideoData->textType = EVideoTextType::Src_StereoRight;
			return true;
		default:
			break;
		}
	}
	return true;
}
