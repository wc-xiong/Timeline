#include "MMDecoder.h"

#include <initguid.h>
#include <ppl.h>

#include "Common/MediaCommon.h"

#include <QImage>
#include <QDebug>

extern "C"
{
#include <libavutil/eval.h>
#include <libavutil/display.h>
#include <libavutil/opt.h>
}

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

#define MIN_PACKET 10
#define MIN_PACKET_DURATION 0.5
#define VIDEO_FRAME_LIST_SIZE 3
#define AUDIO_FRAME_LIST_SIZE 10

#define VIDEO_ERROR_TIMES 10

AVPixelFormat hw_pix_fmt;
AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
	const enum AVPixelFormat *p;

	for (p = pix_fmts; *p != -1; p++) {
		if (*p == hw_pix_fmt)
		{
			/*{
				AVCodecContext* pCodecCtx = ctx;
				AVBufferRef* new_frames_ctx = nullptr;
				if (avcodec_get_hw_frames_parameters(pCodecCtx, pCodecCtx->hw_device_ctx, AV_PIX_FMT_D3D11, &new_frames_ctx) < 0)
				{
					break;
				}

				auto fctx = (AVHWFramesContext*)new_frames_ctx->data;
				fctx->initial_pool_size = 32;
				fctx->width = FFALIGN(ctx->coded_width, 32);
				fctx->height = FFALIGN(ctx->coded_height, 32);
				auto d3d11HFrameCtx = static_cast<AVD3D11VAFramesContext*>(fctx->hwctx);
				d3d11HFrameCtx->BindFlags |= D3D11_BIND_SHADER_RESOURCE;
				d3d11HFrameCtx->MiscFlags |= D3D11_RESOURCE_MISC_SHARED;

				auto ret = av_hwframe_ctx_init(new_frames_ctx);
				if (ret < 0)
				{
					char buffer[AV_ERROR_MAX_STRING_SIZE] = { 0 };
					av_strerror(ret, buffer, AV_ERROR_MAX_STRING_SIZE);
				}
				pCodecCtx->hw_frames_ctx = new_frames_ctx;
			}*/
			return *p;
		}
	}

	return AV_PIX_FMT_NONE;
}

int DecodeInterruptCallback(LPVOID p)
{
	CMMDecoder * pThis;

	pThis = (CMMDecoder*)p;
	return pThis->m_bEnd ? 1 : 0;
}

CMMDecoder::CMMDecoder()
	: m_bEnd(false)
	, m_pAVFormatContext(NULL)
	, m_nVideoStreamIndex(-1)
	, m_nAudioStreamIndex(-1)
	, m_pVideoCodecContext(nullptr)
	, m_pAudioCodecContext(nullptr)
	, m_bMP3(FALSE)
	, m_pixformat(AV_PIX_FMT_NONE)
	, m_colorSpace(AVCOL_SPC_RGB)
	, m_colorRange(AVCOL_RANGE_UNSPECIFIED)
	, m_colorTrc(AVCOL_TRC_RESERVED0)
	, m_videoPacketDuration(0)
	, m_audioPacketDuration(0)
	, m_bReadEOF(TRUE)
	, m_bVideoDecoderEOF(true)
	, m_bAudioDecoderEOF(true)
	, m_hRead(nullptr)
	, m_hVideoDecoder(nullptr)
	, m_hAudioDecoder(nullptr)
	, m_bSeekingForVideo(false)
	, m_bKeyFrameValid(false)
	, m_bSeekingForAudio(false)
	, m_bTryFilter(false)
	, m_pGraph(nullptr)
	, m_pInContext(nullptr)
	, m_pContext(nullptr)
	, m_pOutContext(nullptr)
	//, m_bUseHap(FALSE)
	, m_pDropJudge(NULL)
	//, m_pNumaAllocator(pNumaAllocator)
	, m_nFilterType(FILETER_NONE)
{
}

CMMDecoder::~CMMDecoder()
{
	if (m_pDecoder)
	{
		m_pDecoder->close();
		SafeDelete(m_pDecoder);
	}

	if (m_pAVFormatContext)
	{
		if (m_pVideoCodecContext && m_pVideoCodecContext->hw_device_ctx)
		{
			av_buffer_unref(&m_pVideoCodecContext->hw_device_ctx);
		}

		if (m_pVideoCodecContext)
		{
			avcodec_free_context(&m_pVideoCodecContext);
		}

		if (m_pAudioCodecContext)
		{
			avcodec_free_context(&m_pAudioCodecContext);
		}

		if (m_pAVFormatContext->pb)
		{
			av_freep(&(m_pAVFormatContext->pb->buffer));
			avio_closep(&m_pAVFormatContext->pb);
			avio_context_free(&m_pAVFormatContext->pb);
		}

		avformat_close_input(&m_pAVFormatContext);
		avformat_free_context(m_pAVFormatContext);

		if (m_pDropJudge)
		{
			delete m_pDropJudge;
			m_pDropJudge = NULL;
		}
	}

}

bool CMMDecoder::Init(
	const char * pFile, 
	bool bNeedVideo, 
	bool bDiscardAudio, 
	EDecoderType emType, 
	UINT uAdapter, 
	bool tryHap, 
	IDECCallBacker* pDECCallback,
	bool bLocate,
	std::chrono::milliseconds position,
	UINT gpuIndex)
{
	m_emDecoderType = emType;
	m_gpuIndex = gpuIndex;
	AVDictionary* opts = NULL;
	int nResult = av_dict_set(&opts, "probesize", "20000000", AV_DICT_MATCH_CASE);
	if (nResult < 0)
		goto failed;

	nResult = av_dict_set(&opts, "seek2any", "1", AV_DICT_MATCH_CASE);
	if (nResult < 0)
		goto failed;

	//nResult = av_dict_set(&opts, "analyzeduration", "100000", AV_DICT_MATCH_CASE);
	//if (nResult < 0)
	//	goto failed;

	nResult = avformat_open_input(&m_pAVFormatContext, pFile, NULL, &opts);
	if (nResult < 0)
		goto failed;

	av_format_inject_global_side_data(m_pAVFormatContext);

	// 由于 avformat_open_input 读取出来的 duration 不准确，因此先将其设置为无效值，由avformat_find_stream_info进行赋值
	nResult = avformat_find_stream_info(m_pAVFormatContext, NULL);
	if (nResult < 0)
		goto failed;


	//int64_t filesize = 0ll;
	//if ((m_pAVFormatContext->bit_rate == 0 || m_pAVFormatContext->bit_rate == AV_NOPTS_VALUE)
	//	&& m_pAVFormatContext->pb && (filesize = avio_size(m_pAVFormatContext->pb)) > 0 && m_duration > 0)
	//{
	//	double bitrate = (double)filesize * 8.0 * AV_TIME_BASE / (double)m_duration;
	//	if (bitrate >= 0 && bitrate <= INT64_MAX)
	//		m_pAVFormatContext->bit_rate = bitrate;
	//}

	if (bNeedVideo)
		m_nVideoStreamIndex = av_find_best_stream(m_pAVFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (!bDiscardAudio)
		m_nAudioStreamIndex = av_find_best_stream(m_pAVFormatContext, AVMEDIA_TYPE_AUDIO, -1, m_nVideoStreamIndex, NULL, 0);

	if (m_nVideoStreamIndex < 0 && m_nAudioStreamIndex < 0)
		goto failed;

	//printf("av_version_info = %s.\n", av_version_info());
	//printf("avcodec_version = %d.\n", avcodec_version());
	//emType = EDT_SOFTWARW;
	if (m_nVideoStreamIndex >= 0)
	{
		bool bResult = false;
		bool isHap = m_pAVFormatContext->streams[m_nVideoStreamIndex]->codecpar->codec_id == AV_CODEC_ID_HAP;
		m_nRotate = GetRotateAngle();
		bool isRotate = (m_nRotate != 0);	//旋转的视频统一软解，硬解无法使用fliter

		if (!isHap && !isRotate && (EDT_HARDWARD == emType || EDT_AUTO == emType)) // HAP 必须使用软解
		{
			bResult = OpenVideoStreamHardWare(uAdapter);
			if (bResult)
				m_emDecoderType = EDT_HARDWARD;
			else if (EDT_HARDWARD == emType)
				goto failed;
		}

		if (!bResult)
		{
			if (!OpenVideoStreamSoftWare())
				goto failed;
			m_emDecoderType = EDT_SOFTWARW;
		}

		//printf("Decoder Type = %s.\n", m_emDecoderType == EDT_HARDWARD ? "HW" : "SW");

		GetVideoSizeAndRate();
		if (m_width == 0 || m_height == 0)
			m_emDecoderType = EDT_FAILED;

		//获取颜色相关数据
		m_pixformat = m_pVideoCodecContext->pix_fmt;
		m_colorSpace = m_pVideoCodecContext->colorspace;
		m_colorRange = m_pVideoCodecContext->color_range;
		m_colorTrc = m_pVideoCodecContext->color_trc;

		if (m_pAVFormatContext->streams[m_nVideoStreamIndex]->disposition & AV_DISPOSITION_ATTACHED_PIC)
		{
			m_bMP3 = TRUE;
			AVPacket* pPacket = NULL;
			pPacket = av_packet_alloc();
			if (!pPacket)
				goto failed;
			nResult = av_packet_ref(pPacket, &m_pAVFormatContext->streams[m_nVideoStreamIndex]->attached_pic);
			if (nResult < 0)
			{
				av_packet_unref(pPacket);
				av_packet_free(&pPacket);
				goto failed;
			}

			m_videoPacketList.push_back(pPacket);
			m_videoPacketDuration += pPacket->duration;

			pPacket = av_packet_alloc();
			if (!pPacket)
				goto failed;
			pPacket->data = NULL;
			pPacket->size = 0;
			pPacket->stream_index = m_nVideoStreamIndex;
			m_videoPacketList.push_back(pPacket);
		}
	}

	if (m_nAudioStreamIndex >= 0)
	{
		if (!OpenAudioStream())
			goto failed;
	}

	m_bReadEOF = FALSE;

	if (bLocate)
	{
		Seek(position.count(), true);
	}

	m_hRead = CreateThread(NULL, 0, ReadProc, this, 0, NULL);

	if (m_pVideoCodecContext && (m_width > 0) && (m_height > 0))
	{
		m_bVideoDecoderEOF = false;
		m_hVideoDecoder = CreateThread(NULL, 0, VideoDecoderProc, this, 0, NULL);

		if (m_pVideoCodecContext->codec_id == AV_CODEC_ID_HAP
			|| m_pVideoCodecContext->codec_id == AV_CODEC_ID_PRORES)
		{
			m_pDropJudge = new CDropJudgment();
			m_pDropJudge->SetCalculateInteval(microseconds(1000000));
			m_pDECCallback = pDECCallback;
			std::function<microseconds()> getRenderTime = [this]()->microseconds { return m_pDECCallback->GetRenderVideoPts(); };
			m_pDropJudge->SetGetRenderTimeFunction(getRenderTime);
		}

	}
	if (m_pAudioCodecContext)
	{
		m_bAudioDecoderEOF = false;
		m_hAudioDecoder = CreateThread(NULL, 0, AudioDecoderProc, this, 0, NULL);
	}
	av_dict_free(&opts);

	return true;

failed:
	av_dict_free(&opts);
	UnInit();
	return false;
}

void CMMDecoder::UnInit()
{
	m_bEnd = true;

	//if (m_pNumaAllocator && m_threadAffinityMask)
	//{
	//	// 释放 NUMA 节点
	//	m_pNumaAllocator->FreeNumaNode(m_threadAffinityMask, m_decodeWeight);
	//	m_threadAffinityMask = 0;
	//}

	if (m_pAVFormatContext)
	{
		if (m_nVideoStreamIndex >= 0)
		{
			m_pAVFormatContext->streams[m_nVideoStreamIndex]->discard = AVDISCARD_ALL;
		}

		if (m_nAudioStreamIndex >= 0)
		{
			m_pAVFormatContext->streams[m_nAudioStreamIndex]->discard = AVDISCARD_ALL;
		}

		if (m_hRead)
		{
			WaitForSingleObject(m_hRead, INFINITE);
			CloseHandle(m_hRead);
			m_hRead = NULL;
		}

		if (m_hVideoDecoder)
		{
			WaitForSingleObject(m_hVideoDecoder, INFINITE);
			CloseHandle(m_hVideoDecoder);
			m_hVideoDecoder = NULL;
		}

		if (m_hAudioDecoder)
		{
			WaitForSingleObject(m_hAudioDecoder, INFINITE);
			CloseHandle(m_hAudioDecoder);
			m_hAudioDecoder = NULL;
		}

		FlushPackets();
		FlushFrames();

		if (m_pGraph)
		{
			avfilter_free(m_pInContext);
			m_pInContext = NULL;
			avfilter_free(m_pContext);
			m_pContext = NULL;
			avfilter_free(m_pOutContext);
			m_pOutContext = NULL;
			avfilter_graph_free(&m_pGraph);
		}
	}
}

AVFrame * CMMDecoder::GetVideoFrame()
{
	AVFrame * pFrame;

	QWriteLocker lock(&m_csVideoFrameListLock);
	if (m_videoFrameList.empty())
	{
		return nullptr;
	}

	pFrame = m_videoFrameList.front();
	m_videoFrameList.pop_front();

	return pFrame;
}

int64_t CMMDecoder::GetVideoBitrate()
{
	int64_t rate = 0;
	if (m_pVideoCodecContext)
	{
		rate = m_pAVFormatContext->bit_rate;
	}
	return rate;
}

std::string CMMDecoder::GetVideoCodecName()
{
	std::string name;
	if (m_pVideoCodecContext)
	{
		name = avcodec_get_name(m_pVideoCodecContext->codec_id);
	}
	return name;
}

std::string CMMDecoder::GetContainerFormat()
{
	std::string fmt;
	if (m_pAVFormatContext)
	{
		fmt = m_pAVFormatContext->iformat->long_name;
	}
	return fmt;
}

int CMMDecoder::GetAudioChannels()
{
	int c = 0;
	if (m_pAudioCodecContext)
	{
		c = m_pAudioCodecContext->ch_layout.nb_channels;
	}
	return c;
}

int64_t CMMDecoder::GetAudioSampleRate()
{
	int64_t rate = 0;
	if (m_pAudioCodecContext)
	{
		rate = m_pAudioCodecContext->sample_rate;
	}
	return rate;
}

std::string CMMDecoder::GetAudioCodecName()
{
	std::string name;
	if (m_pAudioCodecContext)
	{
		name = avcodec_get_name(m_pAudioCodecContext->codec_id);
	}
	return name;
}

int CMMDecoder::GetVideoPackSize()
{
	QReadLocker lock(&m_csVideoPacketListLock);
	return (int)m_videoPacketList.size();
}

int CMMDecoder::GetVideoFrameSize()
{
	QReadLocker lock(&m_csVideoFrameListLock);
	return (int)m_videoFrameList.size();
}

AVFrame * CMMDecoder::GetAudioFrame()
{
	AVFrame * pFrame = nullptr;

	QWriteLocker lock(&m_csAudioFrameListLock);
	if (m_audioFrameList.empty())
		return pFrame;

	pFrame = m_audioFrameList.front();
	m_audioFrameList.pop_front();

	return pFrame;
}

void CMMDecoder::Seek(LONGLONG pts, bool bExact)
{
	if (!m_pAVFormatContext)
		return;

	m_bSeeking = true;

	LONGLONG ts = pts * 1000;
	if (ts && AV_NOPTS_VALUE != m_pAVFormatContext->start_time)
		ts += m_pAVFormatContext->start_time;

	CAutoLock autoLock(&m_csReadLock);

	int nFlag = AVSEEK_FLAG_ANY;
	if (bExact)
		nFlag = AVSEEK_FLAG_BACKWARD/* | AVSEEK_FLAG_FRAME*/;

	int ret = av_seek_frame(m_pAVFormatContext, -1, ts, nFlag);
	if (ret < 0)
	{
		m_bSeeking = false;
		return;
	}

	FlushPackets();
	if (m_pVideoCodecContext)
	{
		QWriteLocker lock(&m_csVideoDecoderLock);
		avcodec_flush_buffers(m_pVideoCodecContext);
		m_bVideoDecoderEOF = false;

		if (m_pGraph)
		{
			avfilter_free(m_pInContext);
			m_pInContext = nullptr;
			avfilter_free(m_pContext);
			m_pContext = nullptr;
			avfilter_free(m_pOutContext);
			m_pOutContext = nullptr;
			avfilter_graph_free(&m_pGraph);
			m_bTryFilter = false;
		}
	}

	if (m_pAudioCodecContext)
	{
		QWriteLocker lock(&m_csAudioDecoderLock);
		avcodec_flush_buffers(m_pAudioCodecContext);
		m_bAudioDecoderEOF = false;
	}
	FlushFrames();

	if (m_pVideoCodecContext &&
		(m_pAVFormatContext->streams[m_nVideoStreamIndex]->disposition & AV_DISPOSITION_ATTACHED_PIC))
	{
		AVPacket* pPacket = av_packet_alloc();
		if (!pPacket)
		{
			m_bSeeking = false;
			return;
		}

		ret = av_packet_ref(pPacket, &m_pAVFormatContext->streams[m_nVideoStreamIndex]->attached_pic);
		if (ret < 0)
		{
			av_packet_unref(pPacket);
			av_packet_free(&pPacket);
			m_bSeeking = false;
			return;
		}

		QWriteLocker lock(&m_csVideoPacketListLock);
		m_videoPacketList.push_back(pPacket);
		m_videoPacketDuration += pPacket->duration;

		pPacket = av_packet_alloc();
		if (!pPacket)
		{
			m_bSeeking = false;
			return;
		}
		pPacket->data = NULL;
		pPacket->size = 0;
		pPacket->stream_index = m_nVideoStreamIndex;
		m_videoPacketList.push_back(pPacket);
	}

	m_bNeedKeyframe = bExact;

	m_bKeyFrameValid = bExact;
	m_seekingPTS = pts;

	if (m_pVideoCodecContext)
		m_bSeekingForVideo = true;
	if (m_pAudioCodecContext)
		m_bSeekingForAudio = true;

	m_bReadEOF = false;

	// Seek 成功后保持 m_bSeeking，由 FinishSeek() 在首帧就绪后清除
	// 读包/解码线程不再因 m_bSeeking 停转（见 Read/Send/Receive），靠锁与 Seek 临界区互斥
}

void CMMDecoder::FinishSeek()
{
	m_bSeeking = false;
	m_seekingPTS = 0;
}

bool CMMDecoder::IsComplete()
{
	if (!m_bReadEOF ||
		!m_bVideoDecoderEOF ||
		!m_bAudioDecoderEOF)
	{
		return false;
	}

	if (m_pVideoCodecContext)
	{
		QReadLocker lock(&m_csVideoFrameListLock);
		if (!m_videoFrameList.empty())
		{
			return false;
		}
	}

	if (m_pAudioCodecContext)
	{
		QReadLocker lock(&m_csAudioFrameListLock);
		if (!m_audioFrameList.empty())
		{
			return false;
		}
	}

	return true;
}

int CMMDecoder::GetRotateAngle()
{
	AVDictionaryEntry * tag = av_dict_get(m_pAVFormatContext->streams[m_nVideoStreamIndex]->metadata, "rotate", NULL, 0);
	uint8_t* displaymatrix = av_stream_get_side_data(m_pAVFormatContext->streams[m_nVideoStreamIndex], AV_PKT_DATA_DISPLAYMATRIX, NULL);

	double theta = 0;
	if (tag && *tag->value && strcmp(tag->value, 0))
	{
		char *tail;
		theta = av_strtod(tag->value, &tail);
		if (*tail)
			theta = 0;
	}

	if (displaymatrix && !theta)
		theta = -av_display_rotation_get((int32_t*)displaymatrix);

	theta -= 360 * floor(theta / 360 + 0.9 / 360);

	int angle = 0;
	if (fabs(theta - 90) < 1.0)
		angle = 90;
	else if (fabs(theta - 180) < 1.0 || fabs(theta + 180) < 1.0)
		angle = 180;
	else if (fabs(theta - 270) < 1.0 || fabs(theta + 90) < 1.0)
		angle = 270;
	
	return angle;
}

bool CMMDecoder::HasVideo()
{
	if ((!m_bMP3) && m_pVideoCodecContext && (m_width != 0) && (m_height != 0))
		return TRUE;

	return FALSE;
}

bool CMMDecoder::HasAudio()
{
	if (m_pAudioCodecContext)
		return TRUE;

	return FALSE;
}

__int64 CMMDecoder::GetDuration()
{
	if (m_pAVFormatContext)
		return m_pAVFormatContext->duration / 1000;

	return 0;
}

void CMMDecoder::Read()
{
	AVPacket * pPacket = nullptr;
	int nResult;
	LONGLONG pts, duration;

	while (!m_bEnd)
	{
		if (m_bReadEOF)
		{
			Sleep(5);
			continue;
		}

		if (IsVideoPacketsEnough() &&
			IsAudioPacketsEnough())
		{
			Sleep(5);
			continue;
		}

		// Seek 期间仍可读包：m_bSeeking 仅表示尚未 FinishSeek；与 Seek() 临界区靠 m_csReadLock 互斥
		{
			CAutoLock autoLock(&m_csReadLock);
			pPacket = av_packet_alloc();
			if (!pPacket)
				continue;
			nResult = av_read_frame(m_pAVFormatContext, pPacket);
			if (nResult < 0)
			{
				av_packet_unref(pPacket);
				av_packet_free(&pPacket);
				if ((AVERROR_EOF == nResult ||
					avio_feof(m_pAVFormatContext->pb)) &&
					!m_bReadEOF)
				{
					if (m_pVideoCodecContext)
					{
						pPacket = av_packet_alloc();
						if (!pPacket)
							continue;
						pPacket->data = NULL;
						pPacket->size = 0;
						pPacket->stream_index = m_nVideoStreamIndex;

						QWriteLocker lock(&m_csVideoPacketListLock);
						m_videoPacketList.push_back(pPacket);
					}

					if (m_pAudioCodecContext)
					{
						pPacket = av_packet_alloc();
						if (!pPacket)
							continue;
						pPacket->data = NULL;
						pPacket->size = 0;
						pPacket->stream_index = m_nAudioStreamIndex;

						QWriteLocker lock(&m_csAudioPacketListLock);
						m_audioPacketList.push_back(pPacket);
					}
					m_bReadEOF = true;
				}

				if (m_pAVFormatContext->pb &&
					m_pAVFormatContext->pb->error)
				{
					break;
				}
			}
			else
			{
				if (m_nVideoStreamIndex == pPacket->stream_index)
				{
					// 部分视频Seek闪花丢包解决方案，后期现场需要再考虑放开
					//if (m_bNeedKeyframe.load())
					//{
					//	if ((pPacket->flags & AV_PKT_FLAG_KEY))
					//		m_bNeedKeyframe = false;
					//	else
					//	{
					//		av_packet_unref(pPacket);
					//		av_packet_free(&pPacket);
					//		pPacket = nullptr;
					//	}
					//}

					if (pPacket)
					{
						QWriteLocker lock(&m_csVideoPacketListLock);
						m_videoPacketList.push_back(pPacket);

						m_videoPacketDuration += pPacket->duration;
						//LOGA() << (void*)this << " ---- push video packet: " << m_videoPacketList.size();
					}
				}
				else if (m_nAudioStreamIndex == pPacket->stream_index)
				{
					if (m_bKeyFrameValid &&
						m_bSeekingForAudio &&
						AV_NOPTS_VALUE != pPacket->pts &&
						pPacket->duration)
					{
						pts = (int64_t)(pPacket->pts * av_q2d(m_pAVFormatContext->streams[m_nAudioStreamIndex]->time_base) * 1000);
						duration = (int64_t)(pPacket->duration * av_q2d(m_pAVFormatContext->streams[m_nAudioStreamIndex]->time_base) * 1000);
						if (pts + duration < m_seekingPTS)
						{
							av_packet_unref(pPacket);
							av_packet_free(&pPacket);
							pPacket = nullptr;
						}
					}

					if (pPacket)
					{
						QWriteLocker lock(&m_csAudioPacketListLock);
						m_audioPacketList.push_back(pPacket);

						m_audioPacketDuration += pPacket->duration;
						//LOGA() << (void*)this << " ---- push audio packet: " << m_audioPacketList.size();
					}
				}
				else
				{
					av_packet_unref(pPacket);
					av_packet_free(&pPacket);
					pPacket = nullptr;
				}

				continue;
			}
		}

		Sleep(5);
	}
}

void CMMDecoder::VideoDecoder()
{
	while (!m_bEnd)
	{
		{
			QReadLocker lock(&m_csVideoDecoderLock);
			if (ReceiveVideoFrame() || SendVideoPacket())
			{
				continue;
			}
		}

		Sleep(2);
	}
}

void CMMDecoder::AudioDecoder()
{
	while (!m_bEnd)
	{
		{
			QReadLocker lock(&m_csAudioDecoderLock);
			if (ReceiveAudioFrame() ||
				SendAudioPacket())
			{
				continue;
			}
		}

		Sleep(5);
	}
}

DWORD WINAPI CMMDecoder::ReadProc(LPVOID p)
{
	CMMDecoder * pThis;

	pThis = (CMMDecoder*)p;
	pThis->Read();
	return 0;
}

DWORD WINAPI CMMDecoder::VideoDecoderProc(LPVOID p)
{
	CMMDecoder * pThis;

	pThis = (CMMDecoder*)p;
	pThis->VideoDecoder();
	return 0;
}

DWORD WINAPI CMMDecoder::AudioDecoderProc(LPVOID p)
{
	CMMDecoder * pThis;

	pThis = (CMMDecoder*)p;
	pThis->AudioDecoder();
	return 0;
}

//bool CMMDecoder::OpenVideoStreamHardWare(UINT uAdapter)
//{
//	int nResult = 0;
//	enum AVHWDeviceType type = AV_HWDEVICE_TYPE_D3D11VA;
//	const AVCodec* decoder = NULL;
//	int nIndex = av_find_best_stream(m_pAVFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
//	if (nIndex != m_nVideoStreamIndex)
//		return FALSE;
//
//	if (decoder->id == AV_CODEC_ID_AV1)
//	{
//		const AVCodec* av1Decoder = avcodec_find_decoder_by_name("av1");
//		if (av1Decoder != nullptr)
//			decoder = av1Decoder;
//	}
//
//	//
//	for (int i = 0;; i++)
//	{
//		const AVCodecHWConfig* config = avcodec_get_hw_config(decoder, i);
//		if (!config)
//			return FALSE;
//
//		if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == type)
//		{
//			hw_pix_fmt = config->pix_fmt;
//			break;
//		}
//	}
//
//	if (!(m_pVideoCodecContext = avcodec_alloc_context3(decoder)))
//		return FALSE;
//
//	if (avcodec_parameters_to_context(m_pVideoCodecContext, m_pAVFormatContext->streams[m_nVideoStreamIndex]->codecpar) < 0)
//		return FALSE;
//
//	m_pVideoCodecContext->get_format = get_hw_format;
//	m_pVideoCodecContext->opaque = this;
//
//	int err = 0;
//	char sz[16];
//	sprintf_s(sz, "%d", uAdapter);
//
//	AVBufferRef* hw_device_ctx = NULL;
//	if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type, sz, NULL, 0)) < 0)
//	{
//		return FALSE;
//	}
//	m_pVideoCodecContext->hw_device_ctx = av_buffer_ref(hw_device_ctx);
//	av_buffer_unref(&hw_device_ctx);
//
//	AVDictionary* opts = NULL;
//	av_dict_set(&opts, "threads", "auto", 0);
//	av_dict_set(&opts, "refcounted_frames", "1", 0);
//
//	if ((nResult = avcodec_open2(m_pVideoCodecContext, decoder, NULL)) < 0)
//		return FALSE;
//
//	av_dict_free(&opts);
//
//	m_pAVFormatContext->streams[m_nVideoStreamIndex]->discard = AVDISCARD_DEFAULT;
//
//	return TRUE;
//}

bool CMMDecoder::OpenVideoStreamHardWare(UINT uAdapter)
{
	//// d3d11指定解码屏蔽
	
	AVDictionary * opts = NULL;
	const AVCodec* decoder = NULL;
	int nIndex = av_find_best_stream(m_pAVFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
	if (nIndex != m_nVideoStreamIndex)
		return false;

	if (!(m_pVideoCodecContext = avcodec_alloc_context3(decoder)))
		return false;

	if (avcodec_parameters_to_context(m_pVideoCodecContext, m_pAVFormatContext->streams[m_nVideoStreamIndex]->codecpar) < 0)
	{
		avcodec_free_context(&m_pVideoCodecContext);
		return false;
	}

	m_pVideoCodecContext->pkt_timebase = m_pAVFormatContext->streams[m_nVideoStreamIndex]->time_base;
	
	if(m_pVideoCodecContext->codec_id == AV_CODEC_ID_H264)
	{
		m_pVideoCodecContext->thread_count = 1;
	}
	else
	{
		if(!av_dict_get(opts, "threads", nullptr, 0))
			av_dict_set(&opts, "threads", "auto", 0);
	}


	if (m_pVideoCodecContext->lowres > decoder->max_lowres)
		m_pVideoCodecContext->lowres = decoder->max_lowres;

	if (!m_pDecoder)
	{
		m_pDecoder = PlayCore::CVideoDecoder::create(PlayCore::VideoDecoderId_D3D11);
		m_pDecoder->setCodec(decoder);
		m_pDecoder->setCodecContext(m_pVideoCodecContext);
		m_pDecoder->setGpuIndex(m_gpuIndex);
	}

	if (!m_pDecoder->open())
	{
		avcodec_free_context(&m_pVideoCodecContext);
		av_dict_free(&opts);

		m_pDecoder->close();
		SafeDelete(m_pDecoder);
		return false;
	}

	m_pAVFormatContext->streams[m_nVideoStreamIndex]->discard = AVDISCARD_DEFAULT;
	av_dict_free(&opts);
	return true;
}

bool CMMDecoder::OpenVideoStreamSoftWare()
{
	int nResult, stream_lowres = 0;
	//AVCodec * pCodec;
	AVDictionary * opts = NULL;

	m_pVideoCodecContext = avcodec_alloc_context3(NULL);
	if (!m_pVideoCodecContext)
		return false;

	nResult = avcodec_parameters_to_context(m_pVideoCodecContext, m_pAVFormatContext->streams[m_nVideoStreamIndex]->codecpar);
	if (nResult < 0)
	{
		avcodec_free_context(&m_pVideoCodecContext);
		return false;
	}

	m_pVideoCodecContext->pkt_timebase = m_pAVFormatContext->streams[m_nVideoStreamIndex]->time_base;
	const AVCodec * pCodec = avcodec_find_decoder(m_pVideoCodecContext->codec_id);
	if (!pCodec)
	{
		avcodec_free_context(&m_pVideoCodecContext);
		return false;
	}

	m_pVideoCodecContext->codec_id = pCodec->id;
	if (stream_lowres > pCodec->max_lowres)
	{
		stream_lowres = pCodec->max_lowres;
	}
	m_pVideoCodecContext->lowres = stream_lowres;

	if (stream_lowres)
	{
		av_dict_set_int(&opts, "lowres", stream_lowres, 0);
	}

	av_dict_set(&opts, "refcounted_frames", "1", 0);

//#ifndef DISABLE_MULTI_CPU
//	if (!SetupDecodeThreads())
//#endif
	{
		av_dict_set(&opts, "threads", "auto", 0);
	}

	nResult = avcodec_open2(m_pVideoCodecContext, pCodec, &opts);
	if (nResult < 0)
	{
		avcodec_free_context(&m_pVideoCodecContext);
		av_dict_free(&opts);
		return false;
	}

	av_dict_free(&opts);
	m_pAVFormatContext->streams[m_nVideoStreamIndex]->discard = AVDISCARD_DEFAULT;

	return true;
}

bool CMMDecoder::OpenAudioStream()
{
	int nResult, stream_lowres = 0;
	//AVCodec * pCodec;
	AVDictionary * opts = NULL;

	m_pAudioCodecContext = avcodec_alloc_context3(NULL);
	if (!m_pAudioCodecContext)
	{
		return false;
	}

	nResult = avcodec_parameters_to_context(m_pAudioCodecContext, m_pAVFormatContext->streams[m_nAudioStreamIndex]->codecpar);
	if (nResult < 0)
	{
		avcodec_free_context(&m_pAudioCodecContext);
		return false;
	}

	m_pAudioCodecContext->pkt_timebase = m_pAVFormatContext->streams[m_nAudioStreamIndex]->time_base;
	if (m_pAudioCodecContext->channel_layout == 0) // 某些文件没有设置通道类型，使用默认的
	{
		m_pAudioCodecContext->channel_layout = av_get_default_channel_layout(m_pAudioCodecContext->channels);
	}
	const AVCodec *pCodec = avcodec_find_decoder(m_pAudioCodecContext->codec_id);
	if (!pCodec)
	{
		avcodec_free_context(&m_pAudioCodecContext);
		return false;
	}

	m_pAudioCodecContext->codec_id = pCodec->id;
	if (stream_lowres > pCodec->max_lowres)
	{
		stream_lowres = pCodec->max_lowres;
	}
	m_pAudioCodecContext->lowres = stream_lowres;

	av_dict_set(&opts, "threads", "auto", 0);
	if (stream_lowres)
	{
		av_dict_set_int(&opts, "lowres", stream_lowres, 0);
	}
	av_dict_set(&opts, "refcounted_frames", "1", 0);

	nResult = avcodec_open2(m_pAudioCodecContext, pCodec, &opts);
	if (nResult < 0)
	{
		avcodec_free_context(&m_pAudioCodecContext);
		av_dict_free(&opts);
		return false;
	}

	av_dict_free(&opts);
	m_pAVFormatContext->streams[m_nAudioStreamIndex]->discard = AVDISCARD_DEFAULT;

	return true;
}

void CMMDecoder::GetVideoSizeAndRate()
{
	if (m_nRotate == 90 || m_nRotate == 270)
	{
		m_width = m_pAVFormatContext->streams[m_nVideoStreamIndex]->codecpar->height;
		m_height = m_pAVFormatContext->streams[m_nVideoStreamIndex]->codecpar->width;
	}
	else
	{
		m_width = m_pAVFormatContext->streams[m_nVideoStreamIndex]->codecpar->width;
		m_height = m_pAVFormatContext->streams[m_nVideoStreamIndex]->codecpar->height;
	}

	if (m_pAVFormatContext->streams[m_nVideoStreamIndex]->avg_frame_rate.num != 0)
		m_frameTime = 1000000LL * m_pAVFormatContext->streams[m_nVideoStreamIndex]->avg_frame_rate.den / m_pAVFormatContext->streams[m_nVideoStreamIndex]->avg_frame_rate.num;
	else
		m_frameTime = 1000000LL * m_pAVFormatContext->streams[m_nVideoStreamIndex]->r_frame_rate.den / m_pAVFormatContext->streams[m_nVideoStreamIndex]->r_frame_rate.num;
}

void CMMDecoder::FlushPackets()
{
	AVPacket * pPacket;

	if (m_pVideoCodecContext)
	{
		QWriteLocker lock(&m_csVideoPacketListLock);
		while (!m_videoPacketList.empty())
		{
			pPacket = m_videoPacketList.front();
			m_videoPacketList.pop_front();
			av_packet_unref(pPacket);
			av_packet_free(&pPacket);
		}
		m_videoPacketDuration = 0;
	}

	if (m_pAudioCodecContext)
	{
		QWriteLocker lock(&m_csAudioPacketListLock);
		while (!m_audioPacketList.empty())
		{
			pPacket = m_audioPacketList.front();
			m_audioPacketList.pop_front();
			av_packet_unref(pPacket);
			av_packet_free(&pPacket);
		}
		m_audioPacketDuration = 0;
	}
}

void CMMDecoder::FlushFrames()
{
	if (m_pVideoCodecContext)
	{
		QWriteLocker lock(&m_csVideoFrameListLock);
		while (!m_videoFrameList.empty())
		{
			auto *pFrame = m_videoFrameList.front();
			m_videoFrameList.pop_front();
			av_frame_unref(pFrame);
			av_frame_free(&pFrame);
		}
	}

	if (m_pAudioCodecContext)
	{
		QWriteLocker lock(&m_csAudioFrameListLock);
		while (!m_audioFrameList.empty())
		{
			auto *pFrame = m_audioFrameList.front();
			m_audioFrameList.pop_front();
			av_frame_unref(pFrame);
			av_frame_free(&pFrame);
		}
	}
}

bool CMMDecoder::IsVideoPacketsEnough()
{
	if (m_bEnd ||
		!m_pVideoCodecContext ||
		(m_pAVFormatContext->streams[m_nVideoStreamIndex]->disposition & AV_DISPOSITION_ATTACHED_PIC))
	{
		return true;
	}

	QReadLocker lock(&m_csVideoPacketListLock);
	if (m_videoPacketDuration * av_q2d(m_pAVFormatContext->streams[m_nVideoStreamIndex]->time_base) > MIN_PACKET_DURATION ||
		m_videoPacketList.size() > MIN_PACKET)
	{
		return true;
	}

	return false;
}

bool CMMDecoder::IsAudioPacketsEnough()
{
	if (m_bEnd ||
		!m_pAudioCodecContext ||
		(m_pAVFormatContext->streams[m_nAudioStreamIndex]->disposition & AV_DISPOSITION_ATTACHED_PIC))
	{
		return true;
	}

	QReadLocker lock(&m_csAudioPacketListLock);
	if (m_audioPacketDuration * av_q2d(m_pAVFormatContext->streams[m_nAudioStreamIndex]->time_base) > MIN_PACKET_DURATION ||
		m_audioPacketList.size() > MIN_PACKET)
	{
		return true;
	}
	return false;
}

bool CMMDecoder::ReceiveVideoFrame()
{
	AVFrame * pFrame = NULL;
	int nResult;

	// 不再因 m_bSeeking 拒收：Seek 后需继续解码直至首帧，再由上层 FinishSeek
	if (m_bVideoDecoderEOF)
		return false;

	if (m_pGraph)
	{
		pFrame = av_frame_alloc();
		nResult = av_buffersink_get_frame(m_pOutContext, pFrame);
		if (AVERROR(EAGAIN) == nResult ||
			AVERROR_EOF == nResult)
		{
			av_frame_unref(pFrame);
			av_frame_free(&pFrame);
		}
		else if (nResult < 0)
		{
			av_frame_unref(pFrame);
			av_frame_free(&pFrame);

			avfilter_free(m_pInContext);
			m_pInContext = NULL;
			avfilter_free(m_pContext);
			m_pContext = NULL;
			avfilter_free(m_pOutContext);
			m_pOutContext = NULL;
			avfilter_graph_free(&m_pGraph);
		}
	}

	if (!pFrame)
	{
		pFrame = av_frame_alloc();
		nResult = avcodec_receive_frame(m_pVideoCodecContext, pFrame);
		if (AVERROR_EOF == nResult)
		{
			av_frame_unref(pFrame);
			av_frame_free(&pFrame);
			m_bVideoDecoderEOF = true;

			return false;
		}
		else if (AVERROR(EAGAIN) == nResult)
		{
			av_frame_unref(pFrame);
			av_frame_free(&pFrame);
			return false;
		}
		else if (nResult < 0)
		{
			av_frame_unref(pFrame);
			av_frame_free(&pFrame);
			return false;
		}

		if (pFrame->interlaced_frame)
			m_nFilterType |= FILTER_INTERLACED;

		if (m_nRotate == 90 || m_nRotate == 270 || m_nRotate == 180)
			m_nFilterType |= FILTER_ROTATE;

		if (!m_bTryFilter && m_nFilterType != FILETER_NONE)
		{
			InitFilter();
			m_bTryFilter = true;
		}

		if (m_pGraph)
		{
			nResult = av_buffersrc_add_frame_flags(m_pInContext, pFrame, AV_BUFFERSRC_FLAG_KEEP_REF);
			if (nResult < 0)
			{
				avfilter_free(m_pInContext);
				m_pInContext = NULL;
				avfilter_free(m_pContext);
				m_pContext = NULL;
				avfilter_free(m_pOutContext);
				m_pOutContext = NULL;
				avfilter_graph_free(&m_pGraph);
			}
			else
			{
				av_frame_unref(pFrame);
				av_frame_free(&pFrame);
				return true;
			}
		}
	}

	if (AV_NOPTS_VALUE == pFrame->best_effort_timestamp)
	{
		pFrame->pts = (int64_t)NAN;
		if (!m_videoFrameList.empty())
		{
			pFrame->pts = m_videoFrameList.back()->pts + av_q2d(m_pAVFormatContext->streams[m_nVideoStreamIndex]->time_base) * 1000ll;
		}
		else
		{
			av_frame_unref(pFrame);
			av_frame_free(&pFrame);
			return false;
		}
	}
	else
	{
		pFrame->pts = (int64_t)((pFrame->best_effort_timestamp - (AV_NOPTS_VALUE == m_pAVFormatContext->streams[m_nVideoStreamIndex]->start_time ? 0 : m_pAVFormatContext->streams[m_nVideoStreamIndex]->start_time)) * av_q2d(m_pAVFormatContext->streams[m_nVideoStreamIndex]->time_base) * 1000);
	}

	if (m_bSeekingForVideo)
	{
		if (m_bKeyFrameValid && ((m_frameTime / 2.0) < (m_seekingPTS - pFrame->pts) * 1000))
		{
			//printf("seeking free frame pts = %lld.\n", pFrame->pts);
			av_frame_unref(pFrame);
			av_frame_free(&pFrame);
			return false;
		}

		m_bSeekingForVideo = false;
	}

	{
		QWriteLocker lock(&m_csVideoFrameListLock);
		m_videoFrameList.push_back(pFrame);
	}
	//std::cout << "--receive frame-- " << pFrame->pts*1000L << std::endl;

	return true;
}

bool CMMDecoder::SendVideoPacket()
{
	AVPacket * pPacket;
	int nResult;

	// 不再因 m_bSeeking 拒发：Seek 后需继续送包解码
	if (m_bVideoDecoderEOF)
	{
		return false;
	}

	{
		QReadLocker lock(&m_csVideoFrameListLock);
		if (m_videoFrameList.size() >= VIDEO_FRAME_LIST_SIZE)
		{
			return false;
		}
	}

	QWriteLocker lock(&m_csVideoPacketListLock);
	if (m_videoPacketList.empty())
		return false;

	pPacket = m_videoPacketList.front();
	m_videoPacketList.pop_front();

	if (nullptr != m_pDropJudge && m_pDropJudge->IsNeedDropSample(microseconds(av_rescale(pPacket->dts * 1000000,
		m_pAVFormatContext->streams[pPacket->stream_index]->time_base.num,
		m_pAVFormatContext->streams[pPacket->stream_index]->time_base.den)), microseconds(av_gettime()), 1.0f))
	{
		printf("CMMDecoder::SendVideoPacket ---- Dropping one packet\n");
	}
	else
	{
	
		nResult = avcodec_send_packet(m_pVideoCodecContext, pPacket);
		if (AVERROR(EAGAIN) == nResult)
		{
			return false;
		}
	}

	m_videoPacketDuration -= pPacket->duration;
	av_packet_unref(pPacket);
	av_packet_free(&pPacket);

	return true;
}

bool CMMDecoder::ReceiveAudioFrame()
{
	AVFrame * pFrame;
	int nResult;
	LONGLONG duration;

	if (m_bAudioDecoderEOF)
	{
		return false;
	}

	pFrame = av_frame_alloc();
	nResult = avcodec_receive_frame(m_pAudioCodecContext, pFrame);
	if (AVERROR_EOF == nResult)
	{
		av_frame_unref(pFrame);
		av_frame_free(&pFrame);
		m_bAudioDecoderEOF = TRUE;
		return false;
	}

	if (nResult < 0)
	{
#ifdef _DEBUG
		char buf[512];
		av_make_error_string(buf, 512, nResult);
		//printf("Receive Audio Frame error : %s\n", buf);
#endif // _DEBUG

		av_frame_unref(pFrame);
		av_frame_free(&pFrame);
		return false;
	}

	if (AV_NOPTS_VALUE != pFrame->best_effort_timestamp)
	{
		pFrame->pts = (int64_t)((pFrame->best_effort_timestamp - 
			(AV_NOPTS_VALUE == m_pAVFormatContext->streams[m_nAudioStreamIndex]->start_time ? 0 : m_pAVFormatContext->streams[m_nAudioStreamIndex]->start_time)) * av_q2d(m_pAVFormatContext->streams[m_nAudioStreamIndex]->time_base) * 1000);
	}

	if (m_bSeekingForAudio)
	{
		duration = (LONGLONG)(pFrame->nb_samples / (double)pFrame->sample_rate * 1000);
		if (m_bKeyFrameValid && (pFrame->pts + duration < m_seekingPTS))
		{
			av_frame_unref(pFrame);
			av_frame_free(&pFrame);
			return false;
		}
		m_bSeekingForAudio = false;
	}

	QWriteLocker lock(&m_csAudioFrameListLock);
	m_audioFrameList.push_back(pFrame);

	return true;
}

bool CMMDecoder::SendAudioPacket()
{
	AVPacket * pPacket;
	int nResult;

	// 不再因 m_bSeeking 拒发：Seek 后需继续送包解码
	if (m_bAudioDecoderEOF)
	{
		return false;
	}

	{
		QReadLocker lock(&m_csAudioFrameListLock);
		if (m_audioFrameList.size() >= AUDIO_FRAME_LIST_SIZE)
		{
			return false;
		}
	}

	QWriteLocker lock(&m_csAudioPacketListLock);
	if (m_audioPacketList.empty())
	{
		return false;
	}

	pPacket = m_audioPacketList.front();
	nResult = avcodec_send_packet(m_pAudioCodecContext, pPacket);
	if (AVERROR(EAGAIN) == nResult)
	{
		return false;
	}

	m_audioPacketList.pop_front();

	m_audioPacketDuration -= pPacket->duration;
	av_packet_unref(pPacket);
	av_packet_free(&pPacket);

	return true;
}

void CMMDecoder::InitFilter()
{
	char sz[256];
	int nResult;
	AVPixelFormat pix_fmts[] = { m_pVideoCodecContext->pix_fmt, AV_PIX_FMT_NONE };

	do
	{
		m_pGraph = avfilter_graph_alloc();
		if (!m_pGraph)
		{
			break;
		}

		sprintf_s(
			sz,
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d",
			m_pVideoCodecContext->width,
			m_pVideoCodecContext->height,
			m_pVideoCodecContext->pix_fmt,
			m_pAVFormatContext->streams[m_nVideoStreamIndex]->time_base.num,
			m_pAVFormatContext->streams[m_nVideoStreamIndex]->time_base.den);

		const AVFilter *src_filter = avfilter_get_by_name("buffer");
		nResult = avfilter_graph_create_filter(
			&m_pInContext,
			src_filter,
			"in",
			sz,
			NULL,
			m_pGraph);
		if (nResult < 0)
		{
			char sz[256];
			av_strerror(nResult, sz, 256);
			qDebug() << nResult << sz;
			break;
		}

		nResult = avfilter_graph_create_filter(
			&m_pOutContext,
			avfilter_get_by_name("buffersink"),
			"ou",
			NULL,
			NULL,
			m_pGraph);
		if (nResult < 0)
		{
			char sz[256];
			av_strerror(nResult, sz, 256);
			qDebug() << nResult << sz;
			break;
		}

		nResult = av_opt_set_int_list(
			m_pOutContext,
			"pix_fmts",
			pix_fmts,
			AV_PIX_FMT_NONE,
			AV_OPT_SEARCH_CHILDREN);
		if (nResult < 0)
		{
			char sz[256];
			av_strerror(nResult, sz, 256);
			qDebug() << nResult << sz;
			break;
		}

		AVFilterInOut *inputs = avfilter_inout_alloc();
		AVFilterInOut *ouputs = avfilter_inout_alloc();

		inputs->name = av_strdup("out");
		inputs->filter_ctx = m_pOutContext;
		inputs->pad_idx = 0;
		inputs->next = NULL;

		ouputs->name = av_strdup("in");
		ouputs->filter_ctx = m_pInContext;
		ouputs->pad_idx = 0;
		ouputs->next = NULL;

		std::string strfilter_desc;
		if (m_nFilterType & FILTER_INTERLACED)
			strfilter_desc = "yadif=mode=send_frame:parity=auto:deint=interlaced";

		if (m_nFilterType & FILTER_ROTATE)
		{
			std::string strFilterRotate;
			if (m_nRotate == 270)
				strFilterRotate = "transpose=dir=cclock";
			else if (m_nRotate == 90)
				strFilterRotate = "transpose=dir=clock";
			else if (m_nRotate == 180)
				strFilterRotate = "transpose=1,transpose=1";

			if (strfilter_desc.empty())
				strfilter_desc = strFilterRotate;
			else
				strfilter_desc = strfilter_desc + "," + strFilterRotate;
		}

		nResult = avfilter_graph_parse_ptr(m_pGraph, strfilter_desc.c_str(), &inputs, &ouputs, NULL);
		if (nResult < 0)
		{
			char sz[256];
			av_strerror(nResult, sz, 256);
			qDebug() << nResult << sz;
			break;
		}

		nResult = avfilter_graph_config(m_pGraph, NULL);
		if (nResult < 0)
		{
			char sz[256];
			av_strerror(nResult, sz, 256);
			qDebug() << nResult << sz;
			break;
		}

		return;
	} while (FALSE);


	avfilter_free(m_pInContext);
	m_pInContext = NULL;

	avfilter_free(m_pContext);
	m_pContext = NULL;

	avfilter_free(m_pOutContext);
	m_pOutContext = NULL;

	avfilter_graph_free(&m_pGraph);
}


static inline void SetThreadAffinity(DWORD_PTR affinity)
{
	// 标记线程是否已经修改
	thread_local int threadModified = 0;
	if (!threadModified)
	{
		threadModified = true;
		SetThreadAffinityMask(GetCurrentThread(), affinity);
	}
}