#include "ResDemuxer.h"

#include "MediaInfoDemuxer.h"
#include "LoadSubtitleOrTeleprompterFile.h"
#include "Common/CommonFunctionApi.h"
#include "Common/ResourceDefine.h"
#include "Common/ThreadPool.h"
#include "boost/thread/condition_variable.hpp"
#include "boost/thread/mutex.hpp"
#include <thread>

#pragma comment(lib, "libboost_thread-vc141-mt-x64-1_69.lib")
#pragma comment(lib, "libboost_chrono-vc141-mt-x64-1_69.lib")

//#pragma execution_character_set("utf-8")

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

class CResDemuxerImpl
{
public:
	CResDemuxerImpl();
	~CResDemuxerImpl();

	// 解析素材（素材ID, 素材路径， 素材类型, 素材文件名无格式后缀）
	void	Demuxer(const QUuid& uuid, const QString& strPath, int type = -1, const QString& strName = "");
	void	Demuxer(std::shared_ptr<SResFileDemuxInfo> pResInfo);
	void	MediaDemux(std::shared_ptr<SResFileDemuxInfo> media);

	void	Start();
	void	Stop();
	void	Process();

	//删除未解析素材
	void	DelAllResDemuxer();

	std::queue<std::shared_ptr<SResFileDemuxInfo> >	m_queResource;	// 待解析的素材队列

public:
	boost::mutex m_mutex;
	boost::condition_variable	m_condition;
	bool m_bIsStop;
	//任务分配线程
	std::thread* m_pThread;

	//媒体解复用线程池
	CThreadPool* m_pThreadPool = nullptr;

};

CResDemuxerImpl::CResDemuxerImpl() :m_pThread(nullptr), m_bIsStop(true)
{
	Start();
}

CResDemuxerImpl::~CResDemuxerImpl()
{
	Stop();
}

void CResDemuxerImpl::Start()
{
	m_bIsStop = false;

	if (!m_pThread)
		m_pThread = new std::thread(&CResDemuxerImpl::Process, this);

	if (!m_pThreadPool)
		m_pThreadPool = new CThreadPool();
	m_pThreadPool->InitThreadPool(12);
}

void CResDemuxerImpl::Stop()
{
	if (!m_pThread)
		return;

	{
		boost::lock_guard<boost::mutex> lock(m_mutex);
		m_queResource.swap(std::queue<std::shared_ptr<SResFileDemuxInfo>>());
	}

	m_bIsStop = true;
	m_condition.notify_all();

	if (m_pThread->joinable())
		m_pThread->join();

	if (m_pThread)
	{
		delete m_pThread;
		m_pThread = nullptr;
	}

	if (m_pThreadPool)
	{
		m_pThreadPool->StopThreadPool();
		delete m_pThreadPool;
		m_pThreadPool = nullptr;
	}
}

void CResDemuxerImpl::MediaDemux(std::shared_ptr<SResFileDemuxInfo> media)
{
	// 根据类型找对应的解复用器
	IBaseDemuxer* demuxer = nullptr;
	switch (media->type)
	{
	case EResourceType::ERT_Video:
	{
		demuxer = new CVideoDemuxer();
	}
	break;
	case EResourceType::ERT_Audio:
	{
		demuxer = new CAudioDemuxer();
	}
	break;
	case EResourceType::ERT_Picture:
	{
		demuxer = new CPictureDemuxer();
	}
	break;
	case EResourceType::ERT_Subtitle:
	case EResourceType::ERT_Teleprompter:
	{
		demuxer = new CSubtitleDemuxer();
	}
	break;
	default:
		assert(0);
		break;
	}

	if (demuxer)
	{
		bool bSuccess = demuxer->Demuxer(media);
		media->bDemuxerSuccess = bSuccess;
	}
	else
	{
		media->type = EResourceType::ERT_Invalid;
		media->bDemuxerSuccess = false;
	}

	// 解析素材
	if (media->type != EResourceType::ERT_Subtitle && media->type != EResourceType::ERT_Teleprompter)
		emit g_pResDemuxer->signalDemuxerFinish(media);

	if (demuxer)
	{
		delete demuxer;
		demuxer = nullptr;
	}
}

void CResDemuxerImpl::Process()
{
	std::queue<std::shared_ptr<SResFileDemuxInfo>> queueMedia;

	while (!m_bIsStop)
	{
		{
			boost::unique_lock<boost::mutex> lock(m_mutex);
			while (m_queResource.empty())
			{
				if (m_bIsStop)
					break;

				m_condition.wait(lock);
			}

			if (m_bIsStop)
				break;

			if (m_queResource.empty())
				continue;

			m_queResource.swap(queueMedia);

			lock.unlock();
		}

		while (!queueMedia.empty())
		{
			if (m_bIsStop)
				break;

			auto media = queueMedia.front();
			queueMedia.pop();

			switch ((EResourceType)media->type)
			{
			case EResourceType::ERT_Video:
			case EResourceType::ERT_Audio:
			case EResourceType::ERT_Picture:
			case EResourceType::ERT_Subtitle:
			case EResourceType::ERT_Teleprompter:
				m_pThreadPool->EnqueueTask([=]() {
					MediaDemux(media);
				});
				break;
			default:
				break;
			}

		}
	}
}

void CResDemuxerImpl::DelAllResDemuxer()
{
	// 先删除队列所有还未解复用的素材
	{
		boost::lock_guard<boost::mutex> lock(m_mutex);
		m_queResource.swap(std::queue<std::shared_ptr<SResFileDemuxInfo>>());
	}

	if (m_pThreadPool)
		m_pThreadPool->CleanTaskQueue();
}

void CResDemuxerImpl::Demuxer(const QUuid& uuid, const QString& strPath, int type, const QString& strName)
{
	if (m_bIsStop)
		Start();

	{
		std::shared_ptr<SResFileDemuxInfo> pResInfo = nullptr;
		switch (type)
		{
		case EResourceType::ERT_Video:
		{
			pResInfo = std::make_shared<SMediaInfo>();
		}
		break;
		case EResourceType::ERT_Audio:
		{
			pResInfo = std::make_shared<SMediaInfo>();
		}
		break;
		case EResourceType::ERT_Picture:
		{
			pResInfo = std::make_shared<SMediaInfo>();
		}
		break;
		default:
			pResInfo = std::make_shared<SResFileDemuxInfo>();
			//assert(0);
			break;
		}

		pResInfo->id = uuid;
		pResInfo->path = strPath.toStdString();
		pResInfo->type = type;
		pResInfo->name = strName.toStdString();

		{
			boost::lock_guard<boost::mutex> lock(m_mutex);
			m_queResource.push(pResInfo);
		}

	}

	m_condition.notify_all();
}

void CResDemuxerImpl::Demuxer(std::shared_ptr<SResFileDemuxInfo> pResInfo)
{
	{
		boost::lock_guard<boost::mutex> lock(m_mutex);
		m_queResource.push(pResInfo);
	}

	m_condition.notify_all();
}

CResDemuxer* CResDemuxer::GetInstance()
{
	static CResDemuxer instance;
	return &instance;
}

CResDemuxer::CResDemuxer()
{
	d_ptr = new CResDemuxerImpl;
}

CResDemuxer::~CResDemuxer()
{
	if (d_ptr)
	{
		delete d_ptr;
		d_ptr = nullptr;
	}
}

void CResDemuxer::DelAllResDemuxer()
{
	d_ptr->DelAllResDemuxer();
}

void CResDemuxer::Stop()
{
	d_ptr->Stop();
}

void CResDemuxer::slotDemuxer(const QUuid& uuid, const QString& strPath, int type, const QString& strName)
{
	d_ptr->Demuxer(uuid, strPath, type, strName);
}
