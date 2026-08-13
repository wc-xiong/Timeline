
#include "VAPlayer.h"

#include "VADecoder.h"
#include <limits.h>
#include <sstream>
#include <fstream>
#include <windows.h>
#include "Common/AVCommonFunction.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

#define	VIDEO_TEXTUREVIEW_QUE_SIZE		4						// 纹理个数
#define	HALF_VIDEO_TEXTUREVIEW_QUE_SIZE	(VIDEO_TEXTUREVIEW_QUE_SIZE/2)						// 纹理个数
#define	AUDIO_DATA_LENGTH				milliseconds(100)		//音频时长

#define SafeDelete(p)		do{ if(p){delete p; p=nullptr;}}while(0)

inline __int64	Round64(double mvalue)
{
	if (mvalue >= 0)
		return __int64(mvalue + 0.5);
	else
		return __int64(mvalue - 0.5);
}

CVAPlayer::CVAPlayer()
{
	m_bVideoDecComplete = true;
	m_bAudioDecComplete = true;

	//m_maxIndex = -1;
	//m_minIndex = LLONG_MAX;

	m_videoFrameTime = microseconds(-1);
	m_videoFps = -1;

	m_lastVideoPts = microseconds(0);

	m_minAudioPts = LLONG_MAX;
	m_bSyncAudio = true;

	//#ifdef DEBUG
	//	AllocConsole();
	//	freopen("CONIN$", "r", stdin);
	//	freopen("CONOUT$", "w", stdout);
	//	freopen("CONOUT$", "w", stderr);
	//#endif

	m_bEnd.store(false);

	// 文件夹是否存在，作为打开取帧日志的开关
	DWORD attrs = GetFileAttributesW(L"D:\\VABasePlayer");
	if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY))
		m_bDebugLog = true;
}

CVAPlayer::~CVAPlayer()
{
	m_bEnd.store(true);
	//#ifdef DEBUG
	//	FreeConsole();
	//#endif
	ClearData();

	if (m_bDebugLog && !m_vecLogStr.empty())
	{
		std::ofstream ofs;
		ofs.open(m_debugLogName.c_str());
		if (ofs.is_open())
		{
			for (auto& e : m_vecLogStr)
				ofs.write(e.c_str(), (std::streamsize)e.length());
			ofs.close();
		}
		m_vecLogStr.clear();
	}
}

bool CVAPlayer::IsVideoEnough(bool bCicleDecode)
{
	int decType = -1;
	if (m_pVAPlayer)
	{
		m_pVAPlayer->GetDecType(decType);
		if (decType == -1)
			return true;
	}

	int size = 0;
	
	{
		QReadLocker lock(&m_videoMutex);
		size = (int)m_queVideoData.size();
	}
	if (bCicleDecode)
		return size >= VIDEO_TEXTUREVIEW_QUE_SIZE;
	else
		return (size >= VIDEO_TEXTUREVIEW_QUE_SIZE || m_bVideoDecComplete) ? true : false;
}

bool CVAPlayer::IsVideoEmpty()
{
	QReadLocker lock(&m_videoMutex);
	if (!m_queVideoData.empty())
		return false;
	else
		return !(m_bVideoDecComplete && m_bAudioDecComplete);
	//return m_queVideoData.empty();
}

bool CVAPlayer::IsAudioEnough(bool bCicleDecode)
{
	QReadLocker lock(&m_audioMutex);
	if (!m_queAudioData.empty())
	{
		microseconds beginTime = m_queAudioData.front()->pts;
		microseconds endTime = m_queAudioData.back()->pts;

		// 音频特殊处理，即无数据返回false
		return ((endTime - beginTime) > AUDIO_DATA_LENGTH);
	}

	return false;
}


void CVAPlayer::ClearVideoData()
{
	QWriteLocker lock(&m_videoMutex);
	while (!m_queVideoData.empty())
	{
		STextureFrame* pVVideoData = m_queVideoData.front();
		m_queVideoData.pop_front();
		ReleaseVideoData(pVVideoData);
	}
	m_bVideoDecComplete = false;
}

void CVAPlayer::ClearAudioData()
{
	QWriteLocker lock(&m_audioMutex);
	while (!m_queAudioData.empty())
	{
		AUDIO_DATA* pData = m_queAudioData.front();
		m_queAudioData.pop_front();
		SafeDelete(pData);
	}
	m_bAudioDecComplete = false;
}

void CVAPlayer::VideoComplete()
{
	m_bVideoDecComplete = true;
	PrepareReady();
}

void CVAPlayer::AudioComplete()
{
	m_bAudioDecComplete = true;
	PrepareReady();
}

void CVAPlayer::Complete()
{

}

bool CVAPlayer::IsComplete()
{
	if (m_pVAPlayer)
		return m_pVAPlayer->IsComplete();

	return false;
}

bool CVAPlayer::HasVideo()
{
	if (m_pVAPlayer)
		return m_pVAPlayer->HasVideo();

	return false;
}

bool CVAPlayer::HasAudio()
{
	if (m_pVAPlayer)
		return m_pVAPlayer->HasAudio();

	return false;
}

void CVAPlayer::BindAudioPolicy(EDecoderSession session)
{
	m_audioPolicy = std::make_unique<SessionAudioRegisterPolicy>(session, m_bShouldDecodeAudio);
}

bool CVAPlayer::IsSupportAudio()
{
	if (!m_pVAPlayer || !m_pVAPlayer->HasAudio())
		return false;

	if (!m_audioPolicy)
		return false;

	return m_audioPolicy->CanRegisterAudio();
}

// thread : MMEngine
bool CVAPlayer::PresentTexture(STextureFrame* pVideoData)
{
	if (!pVideoData || !m_pVAPlayer)
		return false;

	if (m_videoFrameTime < microseconds(0) || m_videoFps < 0)
	{
		m_videoFrameTime = m_pVAPlayer->GetFrameTime(); ///microseconds(Round64(m_pVAPlayer->GetFrameTime().count() / 1000.0f) * 1000);
		m_videoFps = Round64(1000000.0f / m_videoFrameTime.count());
	}

	if (!m_bInitdata)
	{
		D3D11_QUERY_DESC queryDesc = CD3D11_QUERY_DESC(D3D11_QUERY_EVENT, 0);
		m_pD3dDevice->CreateQuery(&queryDesc, m_pFlushQuery.ReleaseAndGetAddressOf());

		m_vecTexs.clear();

		for (auto& it : pVideoData->m_texs)
		{
			STexDatas texNode;

			if (it.texNode0.pTex)
			{
				CreateTexture(nullptr, 0, it.texNode0.texDesc.Width, it.texNode0.texDesc.Height,
					it.texNode0.texDesc.Format, texNode.texNode0.pTex.ReleaseAndGetAddressOf(), m_pD3dDevice);

				if (!texNode.texNode0.UpdateTexPty())
					return false;
			}

			if (it.texNode1.pTex)
			{
				CreateTexture(nullptr, 0, it.texNode1.texDesc.Width, it.texNode1.texDesc.Height,
					it.texNode1.texDesc.Format, texNode.texNode1.pTex.ReleaseAndGetAddressOf(), m_pD3dDevice);

				if (!texNode.texNode1.UpdateTexPty())
					return false;
			}

			if (it.texNode2.pTex)
			{
				CreateTexture(nullptr, 0, it.texNode2.texDesc.Width, it.texNode2.texDesc.Height,
					it.texNode2.texDesc.Format, texNode.texNode2.pTex.ReleaseAndGetAddressOf(), m_pD3dDevice);

				if (!texNode.texNode2.UpdateTexPty())
					return false;
			}

			m_vecTexs.push_back(texNode);
		}

		m_pD3dDevice->GetImmediateContext(m_pD3dDeviceContext.ReleaseAndGetAddressOf());
		if (!m_pD3dDeviceContext)
			return false;

		m_bInitdata = true;
	}
	//printf("PresentTex pts = %lld, idx = %lld.\n", pVideoData->m_pts.count(), pVVideoData->nFrameIndex);

	pVideoData->nFrameIndex = pVideoData->m_pts.count() / 1000000 * m_videoFps + Round64(pVideoData->m_pts.count() % 1000000 * 1.0f / m_videoFrameTime.count());

	{
		QWriteLocker lock(&m_videoMutex);
		
		if (!m_queVideoData.empty())
		{
			int nFrameIndex = m_queVideoData.back()->nFrameIndex;

			if (2 == pVideoData->nFrameIndex - nFrameIndex)
				pVideoData->nFrameIndex--;

			if (0 == pVideoData->nFrameIndex - nFrameIndex)
				pVideoData->nFrameIndex++;
		}

		//printf("--------------------------present video pts = %lld, index = %d.\n", pVideoData->m_pts.count(), pVideoData->nFrameIndex);
		m_queVideoData.push_back(pVideoData);
	}

	PrepareReady();

	return true;
}

bool CVAPlayer::PresentAudioData(AUDIO_DATA* pAudioData)
{
	//std::cout << "PresentAudioData pts = " << pAudioData->pts.count() / 1000ll << std::endl;
	if (!pAudioData)
		return false;

	m_minAudioPts = m_minAudioPts > pAudioData->pts.count() ? pAudioData->pts.count() : m_minAudioPts;

	{
		QWriteLocker lock(&m_audioMutex);
		if (!m_queAudioData.empty() && (*m_queAudioData.rbegin())->pts > pAudioData->pts)
			m_bSyncAudio = false;

		m_queAudioData.push_back(pAudioData);
	}

	PrepareReady();

	return true;
}

STextureFrame* CVAPlayer::FetchVideoData()
{
	STextureFrame* pVideoData = nullptr;
	{
		std::lock_guard<std::mutex> lock(m_resaveMutex);
		if (m_queResaveVideoData.empty() /*|| !m_bResaveFirstFinish*/)
			return nullptr;

		//printf("%d\n", (int)m_queResaveVideoData.size());

		pVideoData = m_queResaveVideoData.front();
		m_queResaveVideoData.pop();
	}

	return pVideoData;
}

bool CVAPlayer::Stop()
{	
	ClearData();
	return true;
}

bool CVAPlayer::InitPlayer(
	SPlayerResource pr, 
	std::string& strFilePath, 
	EDecoderType nDecodeType, 
	bool bNeedVideo,
	bool bShouldDecodeAudio,
	bool bLocate,
	std::chrono::milliseconds position)
{
	m_bVideoDecComplete = false;
	m_bAudioDecComplete = false;
	m_bShouldDecodeAudio = bShouldDecodeAudio;

	pr.pVACallbacker = this;
	pr.pVAPresent = this;
	pr.pDECCallback = this;

	m_pD3dDevice = pr.pD3DDevice;

	if (m_bDebugLog)
	{
		m_vecLogStr.clear();
		std::string fileName = strFilePath;
		size_t pos = fileName.find_last_of("\\/");
		if (pos != std::string::npos)
			fileName = fileName.substr(pos + 1);
		std::stringstream ss;
		ss << "D:\\VABasePlayer\\" << fileName << "_" << std::hex << (uintptr_t)this << ".log";
		m_debugLogName = ss.str();
	}

	// 软解复用：门控与 CreateOrBind 由 SoftShareSession 收拢；命中失败不降级
	const auto shareAttempt = m_softShare.TryAttachIfEligible(
		pr, strFilePath, nDecodeType, bNeedVideo, bLocate, position, m_pVAPlayer, m_decoderKey);

	if (shareAttempt == ESoftShareAttempt::Attached)
	{
		BindAudioPolicy(m_softShare.Session());
		return true;
	}
	if (shareAttempt == ESoftShareAttempt::Failed)
		return false; // 已打 SoftShare NoFallback 日志，禁止独立软解降级

	// Skipped：门控未命中 → Independent（硬解 / 单卡软解等）
	m_softShare.Reset();
	m_pVAPlayer = std::make_shared<CVADecoder>();
	m_decoderKey = pr.gpuIndex;
	BindAudioPolicy(EDecoderSession::Independent);
	return m_pVAPlayer->Init(strFilePath.c_str(), nDecodeType, pr, bNeedVideo, bShouldDecodeAudio, bLocate, position);
}

void CVAPlayer::ReleaseSharedDecoder()
{
	// 幂等：Stop/Finalize/析构任意组合重入即 no-op
	m_softShare.Release(m_pVAPlayer, m_decoderKey);
	m_audioPolicy.reset();
}

bool CVAPlayer::SeekDecoder(milliseconds position, bool bExact, bool bReset)
{
	if (!m_pVAPlayer)
		return false;

	// SharedBind：只清本卡呈现队列，不动底层 CVADecoder / CMMDecoder
	if (m_softShare.IsBindOnly())
	{
		ClearVideoData();
		ClearAudioData();
		return true;
	}

	// Independent 或 SharedCreator
	return m_pVAPlayer->Seek((int)m_decoderKey, position, bExact, bReset);
}

bool CVAPlayer::FinalizePlayer()
{
	if (m_softShare.IsActive())
	{
		ReleaseSharedDecoder();
		return true;
	}

	if (m_pVAPlayer)
	{
		if (!m_pVAPlayer->RemovePlayerResources((int)m_decoderKey))
		{
			m_pVAPlayer->UnInit();
		}

		m_pVAPlayer.reset();
		m_audioPolicy.reset();
		return true;
	}
	return false;
}

void CVAPlayer::ClearData()
{
	m_bVideoDecComplete = true;
	m_bAudioDecComplete = true;

	if (m_softShare.IsActive())
	{
		// 共享路径：摘本卡 PR + 计数释放（归零时 UnInit）；不影响其他持有者
		ReleaseSharedDecoder();
	}
	else if (m_pVAPlayer)
	{
		m_pVAPlayer->UnInit();
		m_pVAPlayer.reset();
		m_audioPolicy.reset();
	}

	{
		QWriteLocker lock(&m_videoMutex);
		while (!m_queVideoData.empty())
		{
			STextureFrame* pVVideoData = m_queVideoData.front();
			ReleaseVideoData(pVVideoData);
			m_queVideoData.pop_front();
		}
	}

	{
		std::lock_guard<std::mutex> lock(m_resaveMutex);
		while (!m_queResaveVideoData.empty())
		{
			auto pData = m_queResaveVideoData.front();
			m_queResaveVideoData.pop();

			SafeDelete(pData);
		}
	}

	{
		QWriteLocker lock(&m_audioMutex);
		while (!m_queAudioData.empty())
		{
			AUDIO_DATA* pAudioData = m_queAudioData.front();
			SafeDelete(pAudioData);
			m_queAudioData.pop_front();
		}
	}
}

// thread : OnRender
// 丢帧/取帧：先回绕丢帧 → 普通丢帧 → 匹配 → 按匹配下标 trim → CopyResource
// 详见 Doc/视频媒体素材播放器解码处理.md
bool CVAPlayer::GetVideoData(const microseconds curPts, STextureFrame& videoData)
{
	//printf("--------GetVideoData ps = %lld.\n", curPts.count());
	m_lastVideoPts = curPts;

	// 当纹理队列为空/正在跳转时，直接返回
	QWriteLocker lock(&m_videoMutex);
	if (m_queVideoData.empty())
		return false;

	__int64 nExpectIndex = curPts.count() / 1000000 * m_videoFps + Round64(curPts.count() % 1000000 * 1.0f / m_videoFrameTime.count());

	// A. 回绕丢帧：内部循环 Seek 后队列可能为「尾部大 index + 头部小 index」混排。
	// 渲染已进入头部进度时，丢弃远大于 expect 且后方存在更近帧的队首；仍在尾部进度时不误丢。
	while (m_queVideoData.size() > 1)
	{
		__int64 frontIndex = m_queVideoData.front()->nFrameIndex;
		if (frontIndex <= nExpectIndex + HALF_VIDEO_TEXTUREVIEW_QUE_SIZE)
			break;

		bool bHasNearer = false;
		long long frontDist = abs(curPts.count() - m_queVideoData.front()->m_pts.count());
		for (size_t i = 1; i < m_queVideoData.size(); ++i)
		{
			__int64 idx = m_queVideoData[i]->nFrameIndex;
			long long d = abs(curPts.count() - m_queVideoData[i]->m_pts.count());
			if (idx == nExpectIndex || d < frontDist || idx <= nExpectIndex + HALF_VIDEO_TEXTUREVIEW_QUE_SIZE)
			{
				bHasNearer = true;
				break;
			}
		}
		if (!bHasNearer)
			break;

		STextureFrame* pData = m_queVideoData.front();
		m_queVideoData.pop_front();
		ReleaseVideoData(pData);
	}

	// B. 普通丢帧：先丢后取
	while (m_queVideoData.size() > 1)
	{
		__int64 frameIndex = m_queVideoData.front()->nFrameIndex;
		if (frameIndex == nExpectIndex)
			break;

		if (frameIndex < nExpectIndex - HALF_VIDEO_TEXTUREVIEW_QUE_SIZE)
		{
			STextureFrame* pData = m_queVideoData.front();
			m_queVideoData.pop_front();
			ReleaseVideoData(pData);
		}
		else
		{
			break;
		}
	}

	// C. 查找与渲染目的帧邻近匹配的视频帧
	long long dpts = INT64_MAX;
	int nearestIndex = 0;
	STextureFrame *pVDNearest = nullptr;
	for (int i = 0; i < (int)m_queVideoData.size(); ++i)
	{
		auto &pFrame = m_queVideoData[i];
		__int64 frameIndex = pFrame->nFrameIndex;
		if (frameIndex == nExpectIndex)
		{
			pVDNearest = pFrame;
			nearestIndex = i;
			break;
		}

		auto d = abs(curPts.count() - pFrame->m_pts.count());
		if (d <= dpts)
		{
			dpts = d;
			nearestIndex = i;
			pVDNearest = pFrame;
		}
	}

	if (!pVDNearest)
		return false;

	// D. 按匹配下标 trim：匹配帧尽量靠近队首，保留 0/1 缓冲
	while (nearestIndex >= 2)
	{
		STextureFrame* pData = m_queVideoData.front();
		m_queVideoData.pop_front();
		ReleaseVideoData(pData);
		--nearestIndex;
	}

	videoData.m_bFrameChange = (videoData.m_pts != (*pVDNearest).m_pts);
	if (videoData.m_bFrameChange)
	{
		// 拷贝数据
		for (int i = 0; i < (int)m_vecTexs.size(); i++)
		{
			if (pVDNearest->m_texs[i].texNode0.pTex)
				m_pD3dDeviceContext->CopyResource(m_vecTexs[i].texNode0.pTex.Get(), pVDNearest->m_texs[i].texNode0.pTex.Get());
			if (pVDNearest->m_texs[i].texNode1.pTex)
				m_pD3dDeviceContext->CopyResource(m_vecTexs[i].texNode1.pTex.Get(), pVDNearest->m_texs[i].texNode1.pTex.Get());
			if (pVDNearest->m_texs[i].texNode2.pTex)
				m_pD3dDeviceContext->CopyResource(m_vecTexs[i].texNode2.pTex.Get(), pVDNearest->m_texs[i].texNode2.pTex.Get());
		}
		videoData = *pVDNearest;
		videoData.m_texs = m_vecTexs;

		m_pD3dDeviceContext->Flush();

		auto ts = QueryHighTickCountMS();
		while (!m_bEnd.load())
		{
			auto hr = m_pD3dDeviceContext->GetData(m_pFlushQuery.Get(), NULL, 0U, 0U);
			if (hr != S_FALSE)
			{
				if (hr == DXGI_ERROR_DEVICE_REMOVED)
				{
					printf("The GPU device instance has been suspended. Use GetDeviceRemovedReason to determine the appropriate action.\n");
					hr = m_pD3dDevice->GetDeviceRemovedReason();
				}
				break;
			}

			if ((QueryHighTickCountMS() - ts >= 34))	// 等两帧时间
			{
				hr = E_FAIL;
				printf("Copy Resource Fail to break.\n");
				break;
			}
		}
	}

	if (m_bDebugLog)
		AppendVideoFetchLog(curPts, *pVDNearest);

	return true;
}

void CVAPlayer::AppendVideoFetchLog(const microseconds& curPts, const STextureFrame& matched)
{
	std::stringstream ssFrameSequence;
	std::stringstream ssFramePts;
	for (auto& e : m_queVideoData)
	{
		ssFrameSequence << ", " << e->nFrameIndex;
		ssFramePts << ", " << e->m_pts.count();
	}

	std::stringstream ssLog;
	ssLog << "tid: " << GetCurrentThreadId()
		<< ", expertPts: " << curPts.count()
		<< ", retPts: " << matched.m_pts.count()
		<< ", FrameIdx: " << matched.nFrameIndex
		<< ", Seq: " << ssFrameSequence.str()
		<< ", AllPts: " << ssFramePts.str()
		<< ", Deque: " << (int)m_queVideoData.size()
		<< "\n";

	m_vecLogStr.push_back(ssLog.str());
}

bool CVAPlayer::GetAudioData(microseconds& curPts, BYTE* pByte, int nLen)
{
	if (!pByte || !m_pVAPlayer)
		return false;

	microseconds begin = milliseconds(GetTickCount64());

	__int64 nExpectIndex = curPts.count() / 1000000 * m_videoFps + Round64(curPts.count() % 1000000 * 1.0f / m_videoFrameTime.count());

	while (nLen)
	{
		QWriteLocker lock(&m_audioMutex);
		if (m_queAudioData.empty())
		{
			if (m_bAudioDecComplete)
			{
				return false;
			}

			microseconds now = milliseconds(GetTickCount64());
			if (now - begin < microseconds(m_pVAPlayer->GetAudioWaitTime()))
				continue;

			//if (now - begin >= microseconds(m_pVAPlayer->GetAudioWaitTime()))
			{
				int size = std::min(512, nLen);	//获取不到音频数据时静音
				ZeroMemory(pByte, size);

				pByte += size;
				nLen -= size;
				//std::cout<<"CVAPlayer: audio decoding too slow\n";
			}

			continue;
		}

		AUDIO_DATA* pAudioData = m_queAudioData.front();
		// 保证音画同步，当音频队列首个对象是首帧时，应当读取对象也是首帧
		// 如果当前是最后阶段，但是解码队列是初始阶段，直接退出
		if (m_bSyncAudio == false)
		{
			if (nExpectIndex <= 2)
				m_bSyncAudio = true;
			if (pAudioData->pts.count() - m_minAudioPts <= m_minAudioPts)
			{
				return false;
			}
		}

		//__int64 nCurIndex = pAudioData->pts.count() / 1000000 * m_videoFps + Round64(pAudioData->pts.count() % 1000000 * 1.0f / m_videoFrameTime.count());
		//if (nCurIndex <= m_minIndex + 2 && nExpectIndex > nCurIndex + 2)
		//	break;

		if (pAudioData->newpts < curPts && pAudioData->pBuf == pAudioData->pSeekTo) // zhai, 播放速度，重新计算后的pts 
		{
			//std::cout << "delete pts = " << pAudioData->pts.count() / 1000ll <<", curpts = "<<curPts.count()/1000ll <<std::endl;
			delete pAudioData;
			m_queAudioData.pop_front();
			continue;
		}

		int size = std::min(nLen, pAudioData->nDataLen);

		//std::cout << "audio pts = " << pAudioData->pts.count() / 1000ll <<", curpts = " << curPts.count() / 1000ll << std::endl;

		if (pAudioData->pSeekTo)
			memcpy(pByte, pAudioData->pSeekTo, size);

		pByte += size;
		nLen -= size;

		pAudioData->nDataLen -= size;
		if (pAudioData->nDataLen)
		{
			pAudioData->pSeekTo += size;
		}
		else
		{
			delete pAudioData;
			m_queAudioData.pop_front();	
		}
	}

	return true;
}

bool CVAPlayer::GetAudioPty(int gpuIndex, SAudioPlayerProp& pty)
{
	if (m_pVAPlayer)
		return m_pVAPlayer->GetAudioPty(gpuIndex, pty);

	return true;
}

bool CVAPlayer::GetAudioPty(SAudioPlayerProp& pty)
{
	return GetAudioPty((int)m_decoderKey, pty);
}

void CVAPlayer::ReleaseVideoData(STextureFrame* data, bool bReSave)
{
	if (!data)
		return;

	if(bReSave)
	{
		data->nFrameIndex = -1;
		{
			std::lock_guard<std::mutex> lock(m_resaveMutex);
			m_queResaveVideoData.push(data);
		}
		data = nullptr;
	}
	
	SafeDelete(data);
}
