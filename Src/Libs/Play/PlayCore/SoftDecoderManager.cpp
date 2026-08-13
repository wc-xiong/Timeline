#include "SoftDecoderManager.h"
#include "VADecoder.h"

#include <Windows.h>
#include "Common/Common/CommonNonPlatform.h"

#include <vector>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

namespace PlayCore
{

static std::shared_ptr<CSoftDecoderManager> g_pSoftDecoderManager = nullptr;
static std::mutex g_mtxInstance;

std::shared_ptr<CSoftDecoderManager> CSoftDecoderManager::Instance()
{
	std::lock_guard<std::mutex> lock(g_mtxInstance);
	if (!g_pSoftDecoderManager)
		g_pSoftDecoderManager = std::shared_ptr<CSoftDecoderManager>(new CSoftDecoderManager());
	return g_pSoftDecoderManager;
}

void CSoftDecoderManager::Shutdown()
{
	std::shared_ptr<CSoftDecoderManager> mgr;
	{
		std::lock_guard<std::mutex> lock(g_mtxInstance);
		mgr = std::move(g_pSoftDecoderManager);
	}

	if (mgr)
		mgr->ClearAll();
}

std::shared_ptr<CVABaseDecoder> CSoftDecoderManager::CreateOrBind(
	const QString& key,
	SPlayerResource pr,
	const std::string& filePath,
	EDecoderType decType,
	bool bNeedVideo,
	bool bLocate,
	std::chrono::milliseconds position,
	bool& bCreated)
{
	std::lock_guard<std::recursive_mutex> locker(m_mtx);

	// 先来后到：为本卡 PR 分配到达序号，出声归属按最小到达序号
	pr.arrivalSeq = m_nextArrivalSeq++;

	auto it = m_softDecoders.find(key);
	if (it != m_softDecoders.end())
	{
		bCreated = false;
		LOGA() << "SoftShare: act=Attach key= " << key.toStdString()
			<< " gpu= " << pr.gpuIndex
			<< " arrivalSeq= " << pr.arrivalSeq
			<< " prCount= " << (it->second->AddPlayerResource(pr) + 0)
			<< " tid= " << GetCurrentThreadId();
		return it->second;
	}

	auto pDecoder = std::make_shared<CVADecoder>();
	// 音频流恒开启：出声归属按先来后到（最小到达序号），创建者天然首到
	if (!pDecoder->Init(filePath.c_str(), decType, pr, bNeedVideo, true, bLocate, position))
	{
		LOGA() << "SoftShare: act=CreateFail key= " << key.toStdString()
			<< " gpu= " << pr.gpuIndex
			<< " no-fallback=1"
			<< " tid= " << GetCurrentThreadId();
		return nullptr;
	}

	bCreated = true;
	m_softDecoders[key] = pDecoder;
	LOGA() << "SoftShare: act=Create key= " << key.toStdString()
		<< " gpu= " << pr.gpuIndex
		<< " arrivalSeq= " << pr.arrivalSeq
		<< " tid= " << GetCurrentThreadId();

	return pDecoder;
}

void CSoftDecoderManager::Finalize(const QString& key, const std::shared_ptr<CVABaseDecoder>& decoder, UINT gpuIndex)
{
	if (!decoder)
		return;

	std::shared_ptr<CVABaseDecoder> toUnInit;
	{
		std::lock_guard<std::recursive_mutex> locker(m_mtx);

		auto it = m_softDecoders.find(key);
		if (it != m_softDecoders.end() && it->second.get() == decoder.get())
		{
			// 摘本卡 PR（持 PR 写锁，返回即断联）；计数归零则摘表
			if (decoder->RemovePlayerResources((int)gpuIndex) == 0)
			{
				m_softDecoders.erase(it);
				toUnInit = decoder;
			}
		}
	}

	LOGA() << "SoftShare: act=Remove key= " << key.toStdString()
		<< " gpu= " << gpuIndex
		<< " erase= " << (toUnInit ? 1 : 0)
		<< " tid= " << GetCurrentThreadId();

	// 锁外 UnInit（join 解码线程、关流）
	if (toUnInit)
		toUnInit->UnInit();
}

void CSoftDecoderManager::ClearAll()
{
	std::vector<std::shared_ptr<CVABaseDecoder>> leftovers;
	{
		std::lock_guard<std::recursive_mutex> locker(m_mtx);
		leftovers.reserve(m_softDecoders.size());
		for (auto& kv : m_softDecoders)
			leftovers.push_back(kv.second);
		m_softDecoders.clear();
	}

	LOGA() << "SoftShare: act=ClearAll count= " << (int)leftovers.size();

	for (auto& decoder : leftovers)
	{
		if (decoder)
			decoder->UnInit();
	}
}

size_t CSoftDecoderManager::Size() const
{
	std::lock_guard<std::recursive_mutex> locker(m_mtx);
	return m_softDecoders.size();
}

} // namespace PlayCore
