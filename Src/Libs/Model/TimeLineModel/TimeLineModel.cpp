#include "TimeLineModel.h"

#include "TimeLineLayer.h"
#include "TimeLineSection.h"
#include "TimeLineTag.h"
#include "TimeLineNote.h"
#include "TimeLineTransFx.h"
#include "TimeLineObject.h"
#include "Common/Common/TransationTypeDefine.h"
#include "Common/CommonRapidJson.h"
#include "Model/GlobalModel/GlobalModel.h"

#include <QDebug>
#include <algorithm>
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Common/SignalCenter/GSNetSignalCenter.h"
#include "UI/TimeLineWnd/TimeLineGlobalDefine.h"
#include "ConvertHelper.h"
//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CTimeLineModel::CTimeLineModel()
	: m_id(QUuid::createUuid())
{}

CTimeLineModel::~CTimeLineModel()
{
	for (auto& iter : m_hashLayers)
	{
		iter.second.reset();
	}
}

CTimeLineModel::CTimeLineModel(const CTimeLineModel & other)
{
	m_id = other.m_id;
	m_duration = other.m_duration;
	m_playEndActionType = other.m_playEndActionType;
	m_sectionPlayMode = other.m_sectionPlayMode;
	m_fTimeLineFps = other.m_fTimeLineFps;
	m_bTimeLineDropFrame = other.m_bTimeLineDropFrame;
	m_vecLayers = other.m_vecLayers;
	m_nLayerIndex = other.m_nLayerIndex;

	for (const auto& iter : other.m_hashLayers)
	{
		auto pNode = std::shared_ptr<ITimeLineLayer>(dynamic_cast<ITimeLineLayer*>(iter.second->Copy()));
		m_hashLayers[pNode->GetId()] = pNode;
	}

	for (const auto& pSection : other.m_listSections)
	{
		auto pCopy = std::shared_ptr<ITimeLineSection>(dynamic_cast<ITimeLineSection*>(pSection->Copy()));
		if (pCopy)
		{
			m_listSections.push_back(pCopy);
		}
	}

	for (const auto& pTag : other.m_vecTags)
	{
		auto pCopy = std::shared_ptr<ITimeLineTag>(dynamic_cast<ITimeLineTag*>(pTag->Copy()));
		if (pCopy)
		{
			m_vecTags.push_back(pCopy);
		}
	}

	for (const auto& pNote : other.m_vecNotes)
	{
		auto pCopy = std::shared_ptr<ITimeLineNote>(dynamic_cast<ITimeLineNote*>(pNote->Copy()));
		if (pCopy)
		{
			m_vecNotes.push_back(pCopy);
		}
	}

	for (const auto& pTransFx : other.m_vecTransFxs)
	{
		auto pCopy = std::shared_ptr<ITimeLineTransFx>(dynamic_cast<ITimeLineTransFx*>(pTransFx->Copy()));
		if (pCopy)
		{
			m_vecTransFxs.push_back(pCopy);
		}
	}
}

CTimeLineModel & CTimeLineModel::operator=(const CTimeLineModel & other)
{
	if (this == &other)
		return *this;

	std::unique_lock<std::shared_mutex> lock(m_mtx);

	m_id = other.m_id;
	m_duration = other.m_duration;
	m_playEndActionType = other.m_playEndActionType;
	m_sectionPlayMode = other.m_sectionPlayMode;
	m_fTimeLineFps = other.m_fTimeLineFps;
	m_bTimeLineDropFrame = other.m_bTimeLineDropFrame;
	m_vecLayers = other.m_vecLayers;

	m_hashLayers.clear();
	for (const auto& iter : other.m_hashLayers)
	{
		auto pNode = std::shared_ptr<ITimeLineLayer>(dynamic_cast<ITimeLineLayer*>(iter.second->Copy()));
		m_hashLayers[pNode->GetId()] = pNode;
	}
	m_nLayerIndex = other.m_nLayerIndex;

	m_listSections.clear();
	for (const auto& pSection : other.m_listSections)
	{
		auto pCopy = std::shared_ptr<ITimeLineSection>(dynamic_cast<ITimeLineSection*>(pSection->Copy()));
		if (pCopy)
		{
			m_listSections.push_back(pCopy);
		}
	}

	m_vecTags.clear();
	for (const auto& pTag : other.m_vecTags)
	{
		auto pCopy = std::shared_ptr<ITimeLineTag>(dynamic_cast<ITimeLineTag*>(pTag->Copy()));
		if (pCopy)
		{
			m_vecTags.push_back(pCopy);
		}
	}

	m_vecNotes.clear();
	for (const auto& pNote : other.m_vecNotes)
	{
		auto pCopy = std::shared_ptr<ITimeLineNote>(dynamic_cast<ITimeLineNote*>(pNote->Copy()));
		if (pCopy)
		{
			m_vecNotes.push_back(pCopy);
		}
	}

	m_vecTransFxs.clear();
	for (const auto& pTransFx : other.m_vecTransFxs)
	{
		auto pCopy = std::shared_ptr<ITimeLineTransFx>(dynamic_cast<ITimeLineTransFx*>(pTransFx->Copy()));
		if (pCopy)
		{
			m_vecTransFxs.push_back(pCopy);
		}
	}
	return *this;
}

QUuid CTimeLineModel::GetId()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_id;
}

void CTimeLineModel::SetId(const QUuid& id)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_id = id;
}

void CTimeLineModel::ResetModel()
{
	m_duration = normalizeDurationMs(std::chrono::milliseconds(7200000));
	m_playEndActionType = 0;
	m_sectionPlayMode = ITimeLineModel::SectionPlayMode_Play;
	m_nLayerIndex = 0;

	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_hashLayers.clear();
	m_vecLayers.clear();
	m_listSections.clear();
	m_vecTags.clear();
	m_vecNotes.clear();
	m_vecTransFxs.clear();
	m_nSectionCount = 1;
	m_nTagCount = 1;
	m_nNoteCount = 1;
}

void CTimeLineModel::InitSection()
{
	m_listSections.clear();
	constexpr int SectionCount = 5;
	const auto sectionLen = m_duration / SectionCount;
	for (int i = 0; i < SectionCount; ++i)
	{
		auto pSection = std::shared_ptr<ITimeLineSection>(new CTimeLineSection());
		pSection->SetSectionName(GetFirstNoRepeatSectionName());
		pSection->SetSectionStartTime(normalizeTimeMs(sectionLen * i));
		pSection->SetSectionEndTime(normalizeTimeMs(sectionLen * (i + 1)));
		m_listSections.push_back(pSection);
	}
}

std::pair<QUuid, QUuid> CTimeLineModel::SplitSection(const std::chrono::milliseconds& splitTime)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);

		// 分割点对齐到帧边界（Floor）
	const auto alignedSplit = normalizeTimeMs(splitTime);

	auto it = m_listSections.begin();
	for (; it != m_listSections.end(); ++it)
	{
		auto pSection = *it;
		if (pSection->GetSectionStartTime() < alignedSplit && alignedSplit < pSection->GetSectionEndTime())
			break;
	}

	if (it == m_listSections.end())
		return { QUuid(), QUuid() };

	auto pOldSection = *it;
	std::chrono::milliseconds oldEndTime = pOldSection->GetSectionEndTime();
	QString oldName = pOldSection->GetSectionName();

	pOldSection->SetSectionEndTime(alignedSplit);
	auto pNewSection = std::shared_ptr<ITimeLineSection>(new CTimeLineSection());
	pNewSection->SetSectionName(GetFirstNoRepeatSectionName());
	pNewSection->SetSectionStartTime(alignedSplit);
	pNewSection->SetSectionEndTime(oldEndTime);

	// 将新 Section 插入到原 Section 之后
	auto insertPos = it;
	++insertPos;
	m_listSections.insert(insertPos, pNewSection);

	return { pOldSection->GetId(), pNewSection->GetId() };
}

std::pair<QUuid, QUuid> CTimeLineModel::MergeSection(const std::chrono::milliseconds& mergeTime)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);

	auto it = m_listSections.begin();
	for (; it != m_listSections.end(); ++it)
	{
		auto pSection = *it;
		if (pSection->GetSectionStartTime() <= mergeTime && mergeTime < pSection->GetSectionEndTime())
			break;
	}

	if (it == m_listSections.end() || it == m_listSections.begin())
		return { QUuid(), QUuid() };

	auto pCurSection = *it;
	auto prevIt = std::prev(it);
	auto pPrevSection = *prevIt;

	pPrevSection->SetSectionEndTime(pCurSection->GetSectionEndTime());
	QUuid removedId = pCurSection->GetId();

	m_listSections.erase(it);
	m_nSectionCount--;
	return { pPrevSection->GetId(), removedId };
}

void CTimeLineModel::DelSection(std::shared_ptr<ITimeLineSection> ppItem)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_listSections.remove(ppItem);
	return;
}

void CTimeLineModel::DelSection(QUuid id)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	auto section = GetSection(id).lock();
	DelSection(section);
	return;
}

void CTimeLineModel::DoModifySection(std::chrono::milliseconds & offset, std::shared_ptr<ITimeLineSection> operateSection, int type)
{
	if (!operateSection) return;

	std::chrono::milliseconds newPos;
	if (EDragSide::EDragLeft == type)
	{
		newPos = operateSection->GetSectionStartTime() + offset;
		newPos = max(0ms, newPos);
		newPos = normalizeTimeMs(newPos); // 帧对齐 Floor
		auto sections = GetAllSections();
		// 从右到左合并被覆盖的Section
		// 1. 合并素材 2. 合并Section
		for (auto it = sections.rbegin(); it != sections.rend(); ++it)
		{
			auto section = *it;
			if (!section) continue;
			if (section->GetSectionEndTime() <= operateSection->GetSectionStartTime()
				&& section->GetSectionStartTime() >= newPos
				&& section->GetSectionEndTime() > newPos
				)
			{
				QUuid leftSectionId = GetPreSectionId(section->GetId());
				std::shared_ptr<ITimeLineSection>toMergeSection, mergedSection;
				// 被覆盖的是头部Section时调整素材合并逻辑
				if (leftSectionId.isNull())
				{
					leftSectionId = operateSection->GetId();
					toMergeSection = GetSection(leftSectionId).lock();
					mergedSection = GetSection(section->GetId()).lock();
					if (!toMergeSection || !mergedSection) return;
					toMergeSection->SetSectionStartTime(mergedSection->GetSectionStartTime());
				}
				else
				{
					toMergeSection = GetSection(leftSectionId).lock();
					mergedSection = GetSection(section->GetId()).lock();
					if (!toMergeSection || !mergedSection) return;
					toMergeSection->SetSectionEndTime(mergedSection->GetSectionEndTime());
				}
				DelSection(section);
			}
		}

		QUuid preSectionId = GetPreSectionId(operateSection->GetId());
		auto preSection = GetSection(preSectionId).lock();
		if (preSection)
		{
			preSection->SetSectionEndTime(newPos);
		}
		operateSection->SetSectionStartTime(newPos);
	}
	else if (EDragSide::EDragRight == type)
	{
		newPos = operateSection->GetSectionEndTime() + offset;
		newPos = normalizeTimeMs(newPos); // 帧对齐 Floor
		newPos = min(GetDuration(), newPos);
		auto sections = GetAllSections();
		//从左到右合并被覆盖的Section
		for (auto section : sections)
		{
			if (section->GetSectionStartTime() >= operateSection->GetSectionEndTime()
				&& section->GetSectionStartTime() < newPos
				&& section->GetSectionEndTime() <= newPos
				)
			{
				if (!section) continue;
				QUuid rightSectionId = GetNextSectionId(section->GetId());
				std::shared_ptr<ITimeLineSection>toMergeSection, mergedSection;
				// 被覆盖的是尾部Section时调整素材合并逻辑
				if (rightSectionId.isNull())
				{
					rightSectionId = operateSection->GetId();
					toMergeSection = GetSection(rightSectionId).lock();
					mergedSection = GetSection(section->GetId()).lock();
					if (!toMergeSection || !mergedSection) return;
					toMergeSection->SetSectionEndTime(mergedSection->GetSectionEndTime());
				}
				else
				{
					toMergeSection = GetSection(rightSectionId).lock();
					mergedSection = GetSection(section->GetId()).lock();
					if (!toMergeSection || !mergedSection) return;
					toMergeSection->SetSectionStartTime(mergedSection->GetSectionStartTime());
				}
				DelSection(section);
			}
		}
		QUuid nextSectionId = GetNextSectionId(operateSection->GetId());
		auto nextSection = GetSection(nextSectionId).lock();
		if (nextSection)
		{
			nextSection->SetSectionStartTime(newPos);
		}
		operateSection->SetSectionEndTime(newPos);
	}
}

bool CTimeLineModel::ReplaceAllSectionsFromRapidJson(const rapidjson::Value& sectionsArray)
{
	if (!sectionsArray.IsArray())
		return false;

	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_listSections.clear();

	for (const auto& jsonSection : sectionsArray.GetArray())
	{
		auto pSection = std::shared_ptr<ITimeLineSection>(new CTimeLineSection());
		if (!pSection->LoadDataFromRapidJson(jsonSection))
			return false;
		m_listSections.push_back(pSection);
	}

	m_nSectionCount = static_cast<int>(m_listSections.size()) + 1;
	return true;
}

std::shared_ptr<ITimeLineObject> CTimeLineModel::CreateObject(const QUuid& id, int type)
{
	return std::shared_ptr<ITimeLineObject>(new CTimeLineObject(id, type));
}

std::vector<std::weak_ptr<ITimeLineObject>> CTimeLineModel::GetObjects(const std::chrono::milliseconds& ms, bool bReverse)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	std::vector<std::weak_ptr<ITimeLineObject>> vecObjects;

	if (bReverse)
	{
		for (auto iter = m_vecLayers.rbegin(); iter != m_vecLayers.rend(); iter++)
		{
			auto it = m_hashLayers.find(*iter);
			if (it != m_hashLayers.end())
			{
				auto p = it->second->GetObject(ms).lock();
				if (p)
				{
					vecObjects.push_back(p);
				}
			}
		}
	}
	else
	{
		for (const auto& id : m_vecLayers)
		{
			auto it = m_hashLayers.find(id);
			if (it != m_hashLayers.end())
			{
				auto p = it->second->GetObject(ms).lock();
				if (p)
				{
					vecObjects.push_back(p);
				}
			}
		}
	}

	return vecObjects;
}

std::vector<std::weak_ptr<ITimeLineObject>> CTimeLineModel::GetPreObjects(const std::chrono::milliseconds& ms)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	std::vector<std::weak_ptr<ITimeLineObject>> vecObjects;

	for (const auto& id : m_vecLayers)
	{
		auto it = m_hashLayers.find(id);
		if (it != m_hashLayers.end())
		{
			auto p = it->second->GetObject(ms, false).lock();
			if (p)
			{
				vecObjects.push_back(p);
			}
		}
	}

	return vecObjects;
}

std::weak_ptr<ITimeLineObject> CTimeLineModel::GetTimeLineObject(const QUuid& id)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);

	for (auto& it : m_hashLayers)
	{
		auto p = it.second->GetObject(id).lock();
		if (p)
			return p;
	}

	return std::weak_ptr<ITimeLineObject>();
}

std::vector<QUuid> CTimeLineModel::GetObjectIds(const std::chrono::milliseconds& ms, int type, int Order)
{
	std::vector<QUuid> res;

	std::shared_lock<std::shared_mutex> lock(m_mtx);

	if (OrderType_Descending==Order)
	{
		for (auto iter = m_vecLayers.rbegin(); iter != m_vecLayers.rend(); iter++)
		{
			auto it = m_hashLayers.find(*iter);
			if (it != m_hashLayers.end())
			{
				auto p = it->second->GetObject(ms).lock();
				if (p)
				{
					if (ITimeLineObject::ObjectType_None == type)
						res.push_back(p->GetId());
					else if (p->GetType() == type)
						res.push_back(p->GetId());
				}
			}
		}
	}
	else if(OrderType_Ascending == Order)
	{
		for (const auto& id : m_vecLayers)
		{
			auto it = m_hashLayers.find(id);
			if (it != m_hashLayers.end())
			{
				auto p = it->second->GetObject(ms).lock();
				if (p)
				{
					if (ITimeLineObject::ObjectType_None == type)
						res.push_back(p->GetId());
					else if (p->GetType() == type)
						res.push_back(p->GetId());
				}
			}
		}
	}
	else
	{
		for (const auto& it : m_hashLayers)
		{
			auto p = it.second->GetObject(ms).lock();
			if (p)
			{
				if (ITimeLineObject::ObjectType_None == type)
					res.push_back(p->GetId());
				else if (p->GetType() == type)
					res.push_back(p->GetId());
			}
		}
	}

	return res;
}

std::vector<QUuid> CTimeLineModel::GetAllObjectIds(int type)
{
	std::vector<QUuid> res;

	std::shared_lock<std::shared_mutex> lock(m_mtx);
	for (const auto& it : m_hashLayers)
	{
		auto ids = it.second->GetAllObjectIDs();
		for (const auto& id : ids)
		{
			auto p = it.second->GetObject(id).lock();
			if (p)
			{
				if (ITimeLineObject::ObjectType_None == type)
					res.push_back(p->GetId());
				else if (p->GetType() == type)
					res.push_back(p->GetId());
			}
		}
	}

	return res;
}

bool CTimeLineModel::HasTimelineObjects()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	for (const auto& it : m_hashLayers)
	{
		if (it.second && it.second->HasTimelineObjects())
			return true;
	}
	return false;
}

std::shared_ptr<ITimeLineLayer> CTimeLineModel::CreateLayer()
{
	auto pLayer = std::shared_ptr<ITimeLineLayer>(new CTimeLineLayer());
	pLayer->SetAudioMachine(g_pGlobalModel->GetMachineId(), g_pGlobalModel->GetMachineName());
	pLayer->SetAudioDevice(QStringLiteral("@default"), QStringLiteral("@default"));
	return pLayer;
}

int CTimeLineModel::GetNewLayerIndex()
{
	return ++m_nLayerIndex;
}

void CTimeLineModel::AddLayer(std::shared_ptr<ITimeLineLayer> p,int index)
{
	{
		std::unique_lock<std::shared_mutex> lock(m_mtx);
		auto it = m_hashLayers.find(p->GetId());
		if (it == m_hashLayers.end())
		{
			m_hashLayers.insert({ p->GetId(), p });
			if (index >= 0 && index < m_vecLayers.size())
				m_vecLayers.insert(m_vecLayers.begin() + index, p->GetId());
			else
				m_vecLayers.push_back(p->GetId());
		}
	}
	emit g_pGSNetSignal->signalAddLayer(p->GetId(), index);
}

void CTimeLineModel::MoveLayer(const QUuid& id, int index)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	if (index < 0 || index >= m_vecLayers.size())
		return;

	auto it = std::find(m_vecLayers.begin(), m_vecLayers.end(),id);
	m_vecLayers.erase(it);

	if (index >= m_vecLayers.size()) 
		m_vecLayers.push_back(id);
	else 
		m_vecLayers.insert(m_vecLayers.begin() + index, id);
}

void CTimeLineModel::DeleteLayer(const QUuid& id)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_hashLayers.erase(id);
	m_vecLayers.erase(std::remove(m_vecLayers.begin(), m_vecLayers.end(), id), m_vecLayers.end());
}

int CTimeLineModel::GetLayerCount()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_vecLayers.size();
}

std::vector<QUuid> CTimeLineModel::GetAllLayerIDs()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_vecLayers;
}

std::weak_ptr<ITimeLineLayer> CTimeLineModel::GetLayer(const QUuid& id)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto it = m_hashLayers.find(id);
	if (it != m_hashLayers.end())
	{
		return it->second;
	}
	else
	{
		auto iter = std::find_if(m_hashLayers.begin(), m_hashLayers.end(), [&](const auto& it){
			return it.second->IsObjectExist(id);
		});
		if (iter != m_hashLayers.end())
		{
			return iter->second;
		}
	}
	return std::weak_ptr<ITimeLineLayer>();
}

int CTimeLineModel::GetLayerIndex(const QUuid& id)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	int res = -1;
	int index = 0;
	for (const auto& it : m_vecLayers)
	{
		if (id == it)
		{
			res = index;
			break;
		}

		index++;
	}

	return res;
}

std::chrono::milliseconds CTimeLineModel::GetDuration()
{
	return m_duration;
}

void CTimeLineModel::SetDuration(const std::chrono::milliseconds& msPlayTime)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	// 时间线总时长对齐到帧边界（Floor）
	const auto alignedDuration = normalizeDurationMs(msPlayTime);
	const auto oldDuration = m_duration;
	if (oldDuration == alignedDuration)
		return;

	m_duration = alignedDuration;

	if (alignedDuration > oldDuration)
	{
		syncLastSectionEndToDurationUnlocked();
	}
	else if (alignedDuration < oldDuration)
	{
		while (m_listSections.size() > 1)
		{
			auto pLast = m_listSections.back();
			if (!pLast || pLast->GetSectionStartTime() < alignedDuration)
				break;

			m_listSections.pop_back();
			--m_nSectionCount;
		}
		syncLastSectionEndToDurationUnlocked();
	}

	lock.unlock();
	if (!m_listSections.empty())
	{
		g_pGSNetSignal->notifySectionPositionChanged();
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_TimeLineWnd,
			WndType::WndType_TimeLineScale
			| WndType_LayersView | WndType_HorScroll);
	}
}

int CTimeLineModel::GetPlayEndActionType()
{
	return m_playEndActionType;
}

void CTimeLineModel::SetPlayEndActionType(int type)
{
	m_playEndActionType = type;
}

float CTimeLineModel::GetTimeLineFps()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_fTimeLineFps;
}

void CTimeLineModel::SetTimeLineFps(float fps)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_fTimeLineFps = fps;
}

bool CTimeLineModel::GetTimeLineDropFrame()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_bTimeLineDropFrame;
}

void CTimeLineModel::SetTimeLineDropFrame(bool dropFrame)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_bTimeLineDropFrame = dropFrame;
}

void CTimeLineModel::SetNoniusPos(const std::chrono::milliseconds & ms)
{
	m_noniusPos = ms;
}

std::chrono::milliseconds CTimeLineModel::GetNoniusPos()
{
	return m_noniusPos;
}

int CTimeLineModel::GetSectionPlayMode()
{
	return m_sectionPlayMode;
}

void CTimeLineModel::SetSectionPlayMode(int mode)
{
	m_sectionPlayMode = mode;
}

std::chrono::milliseconds CTimeLineModel::GetMaxUsefulDuration()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	std::chrono::milliseconds ms{0};
	for (auto it : m_hashLayers)
	{
		if (ms < it.second->GetMaxTime())
			ms = it.second->GetMaxTime();
	}

	return ms;
}

void CTimeLineModel::SetMaxUsefulDuration()
{
	// 必须走 SetDuration：同步段落边界，并刷新时间线刻度 / 图层 / 横滚
	const auto useful = GetMaxUsefulDuration();
	SetDuration(std::max(useful, std::chrono::milliseconds(10000)));
}

const std::vector<std::shared_ptr<ITimeLineSection>> CTimeLineModel::GetAllSections()
{
	std::vector<std::shared_ptr<ITimeLineSection>> sections;
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	for (auto section : m_listSections)
	{
		sections.push_back(section);
	}
	return sections;
}

std::weak_ptr<ITimeLineSection> CTimeLineModel::GetSection(QUuid & id)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	for (auto section : m_listSections)
	{
		if (section && section->GetId() == id)
			return section;
	}
	return std::weak_ptr<ITimeLineSection>();
}

std::weak_ptr<ITimeLineSection> CTimeLineModel::GetSection(const std::chrono::milliseconds & ms)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);

	for (auto section : m_listSections)
	{
		if (!section) continue;
		if (section->GetSectionStartTime() <= ms && ms < section->GetSectionEndTime())
		{
			return section;
		}
	}
	return std::weak_ptr<ITimeLineSection>();
}

QUuid CTimeLineModel::GetPreSectionId(QUuid id)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto preSection = m_listSections.begin();
	auto curSection = m_listSections.begin();
	for (; curSection != m_listSections.end(); curSection++)
	{
		if (!(*curSection)) continue;
		if ((*curSection)->GetId() == id)
		{
			std::shared_ptr<ITimeLineSection> preNode = (*preSection);
			if (preNode && preNode->GetId() != id)
				return preNode->GetId();
		}
		if (curSection != m_listSections.begin())
			preSection++;
	}
	return QUuid();
}

QUuid CTimeLineModel::GetNextSectionId(QUuid id)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto curSection = m_listSections.begin();
	auto nextSection = std::next(curSection);
	for (; curSection != m_listSections.end() && nextSection != m_listSections.end(); curSection++)
	{
		if (!(*curSection)) continue;
		if ((*curSection)->GetId() == id && (*nextSection))
		{
			return (*nextSection)->GetId();
		}
		nextSection++;
	}
	return QUuid();
}

QUuid CTimeLineModel::GetFirstSectionId()
{
	if(m_listSections.empty()) return QUuid();
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	auto curSection = m_listSections.begin();
	std::shared_ptr<ITimeLineSection> curNode = (*curSection);
	if(curNode) return curNode->GetId();
	return QUuid();
}

#define TimelineModel_Id_Json "TimelineModel_Id"
#define TimelineModel_Duration_Json "TimelineModel_Duration"
#define TimelineModel_PlayEndActionType_Json "TimelineModel_PlayEndActionType"
#define TimelineModel_SectionPlayMode_Json "TimelineModel_SectionPlayMode"
#define TimelineModel_Layers_Json "TimelineModel_Layers"
#define TimelineModel_Layers_Index "TimelineModel_Index"
#define TimelineModel_Sections_Json "TimelineModel_Section"
#define TimelineModel_Tags_Json "TimelineModel_Tags"
#define TimelineModel_Notes_Json "TimelineModel_Notes"
#define TimelineModel_TransFxs_Json "TimelineModel_TransFxs"
#define TimelineModel_TimeLineFps_Json "TimelineModel_TimeLineFps"
#define TimelineModel_TimeLineDropFrame_Json "TimelineModel_TimeLineDropFrame"

bool CTimeLineModel::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	if (!root.IsObject())
	{
		return false;
	}

	if (root.HasMember(TimelineModel_Id_Json) && root[TimelineModel_Id_Json].IsString())
	{
		m_id = QUuid::fromString(QString::fromUtf8(root[TimelineModel_Id_Json].GetString()));
	}
	else
	{
		m_id = QUuid::createUuid();
	}

	if (!root.HasMember(TimelineModel_Duration_Json) || !root[TimelineModel_Duration_Json].IsInt())
	{
		return false;
	}
	m_duration = normalizeDurationMs(std::chrono::milliseconds(root[TimelineModel_Duration_Json].GetInt()));

	if (!root.HasMember(TimelineModel_PlayEndActionType_Json) || !root[TimelineModel_PlayEndActionType_Json].IsInt())
	{
		return false;
	}
	m_playEndActionType = root[TimelineModel_PlayEndActionType_Json].GetInt();

	if (root.HasMember(TimelineModel_SectionPlayMode_Json) && root[TimelineModel_SectionPlayMode_Json].IsInt())
		m_sectionPlayMode = root[TimelineModel_SectionPlayMode_Json].GetInt();
	else
		m_sectionPlayMode = ITimeLineModel::SectionPlayMode_Play;

	if (root.HasMember(TimelineModel_TimeLineFps_Json) && root[TimelineModel_TimeLineFps_Json].IsNumber())
		m_fTimeLineFps = static_cast<float>(root[TimelineModel_TimeLineFps_Json].GetDouble());
	else
		m_fTimeLineFps = 30.0f;

	if (root.HasMember(TimelineModel_TimeLineDropFrame_Json) && root[TimelineModel_TimeLineDropFrame_Json].IsBool())
		m_bTimeLineDropFrame = root[TimelineModel_TimeLineDropFrame_Json].GetBool();
	else
		m_bTimeLineDropFrame = false;

	if (!root.HasMember(TimelineModel_Layers_Index) || !root[TimelineModel_Layers_Index].IsInt())
	{
		return false;
	}
	m_nLayerIndex = root[TimelineModel_Layers_Index].GetInt();

	if (!root.HasMember(TimelineModel_Layers_Json) || !root[TimelineModel_Layers_Json].IsArray())
	{
		return false;
	}
	const auto& jsonLayers = root[TimelineModel_Layers_Json];

	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_vecLayers.clear();
	m_hashLayers.clear();
	for (const auto& jsonLayer : jsonLayers.GetArray())
	{
		auto pLayer = std::shared_ptr<ITimeLineLayer>(new CTimeLineLayer());
		if (pLayer->LoadDataFromRapidJson(jsonLayer))
		{
			m_hashLayers.insert({ pLayer->GetId(),pLayer });
			m_vecLayers.push_back(pLayer->GetId());
		}
		else
		{
			return false;
		}
	}

	if (root.HasMember(TimelineModel_Sections_Json) && root[TimelineModel_Sections_Json].IsArray())
	{
		const auto& jsonSections = root[TimelineModel_Sections_Json];
		for (const auto& jsonSection : jsonSections.GetArray())
		{
			auto pSection = std::shared_ptr<ITimeLineSection>(new CTimeLineSection());
			if (pSection->LoadDataFromRapidJson(jsonSection))
			{
				m_listSections.push_back(pSection);
				m_nSectionCount++;
			}
		}
	}

	if (root.HasMember(TimelineModel_Tags_Json) && root[TimelineModel_Tags_Json].IsArray())
	{
		const auto& jsonTags = root[TimelineModel_Tags_Json];
		for (const auto& jsonTag : jsonTags.GetArray())
		{
			auto pTag = std::shared_ptr<ITimeLineTag>(new CTimeLineTag());
			if (pTag->LoadDataFromRapidJson(jsonTag))
			{
				m_vecTags.push_back(pTag);
				m_nTagCount++;
			}
		}
	}

	if (root.HasMember(TimelineModel_Notes_Json) && root[TimelineModel_Notes_Json].IsArray())
	{
		const auto& jsonNotes = root[TimelineModel_Notes_Json];
		for (const auto& jsonNote : jsonNotes.GetArray())
		{
			auto pNote = std::shared_ptr<ITimeLineNote>(new CTimeLineNote());
			if (pNote->LoadDataFromRapidJson(jsonNote))
			{
				m_vecNotes.push_back(pNote);
				m_nNoteCount++;
			}
		}
	}

	if (root.HasMember(TimelineModel_TransFxs_Json) && root[TimelineModel_TransFxs_Json].IsArray())
	{
		const auto& jsonTransFxs = root[TimelineModel_TransFxs_Json];
		for (const auto& jsonTransFx : jsonTransFxs.GetArray())
		{
			auto pTransFx = std::shared_ptr<ITimeLineTransFx>(new CTimeLineTransFx());
			if (pTransFx->LoadDataFromRapidJson(jsonTransFx))
			{
				m_vecTransFxs.push_back(pTransFx);
			}
		}
	}

	return true;
}

void CTimeLineModel::SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator)
{
	root.SetObject();
	root.AddMember(rapidjson::StringRef(TimelineModel_Id_Json), rapidjson::Value(m_id.toString().toUtf8().constData(), allocator), allocator);
	root.AddMember(rapidjson::StringRef(TimelineModel_Duration_Json), static_cast<int>(m_duration.count()), allocator);
	root.AddMember(rapidjson::StringRef(TimelineModel_PlayEndActionType_Json), m_playEndActionType, allocator);
	root.AddMember(rapidjson::StringRef(TimelineModel_SectionPlayMode_Json), m_sectionPlayMode, allocator);
	root.AddMember(rapidjson::StringRef(TimelineModel_TimeLineFps_Json), m_fTimeLineFps, allocator);
	root.AddMember(rapidjson::StringRef(TimelineModel_TimeLineDropFrame_Json), m_bTimeLineDropFrame, allocator);
	root.AddMember(rapidjson::StringRef(TimelineModel_Layers_Index), m_nLayerIndex, allocator);

	std::shared_lock<std::shared_mutex> lock(m_mtx);
	rapidjson::Value jsonLayers(rapidjson::kArrayType);
	for (const auto& id : m_vecLayers)
	{
		auto it = m_hashLayers.find(id);
		if (it != m_hashLayers.end())
		{
			rapidjson::Value layer(rapidjson::kObjectType);
			it->second->SaveDataToRapidJson(layer, allocator);
			jsonLayers.PushBack(layer, allocator);
		}
	}
	root.AddMember(rapidjson::StringRef(TimelineModel_Layers_Json), jsonLayers, allocator);

	rapidjson::Value jsonSections(rapidjson::kArrayType);
	for (const auto& pSection : m_listSections)
	{
		rapidjson::Value section(rapidjson::kObjectType);
		pSection->SaveDataToRapidJson(section, allocator);
		jsonSections.PushBack(section, allocator);
	}
	root.AddMember(rapidjson::StringRef(TimelineModel_Sections_Json), jsonSections, allocator);

	rapidjson::Value jsonTags(rapidjson::kArrayType);
	for (const auto& pTag : m_vecTags)
	{
		rapidjson::Value tag(rapidjson::kObjectType);
		pTag->SaveDataToRapidJson(tag, allocator);
		jsonTags.PushBack(tag, allocator);
	}
	root.AddMember(rapidjson::StringRef(TimelineModel_Tags_Json), jsonTags, allocator);

	rapidjson::Value jsonNotes(rapidjson::kArrayType);
	for (const auto& pNote : m_vecNotes)
	{
		rapidjson::Value note(rapidjson::kObjectType);
		pNote->SaveDataToRapidJson(note, allocator);
		jsonNotes.PushBack(note, allocator);
	}
	root.AddMember(rapidjson::StringRef(TimelineModel_Notes_Json), jsonNotes, allocator);

	rapidjson::Value jsonTransFxs(rapidjson::kArrayType);
	for (const auto& pTransFx : m_vecTransFxs)
	{
		rapidjson::Value transFx(rapidjson::kObjectType);
		pTransFx->SaveDataToRapidJson(transFx, allocator);
		jsonTransFxs.PushBack(transFx, allocator);
	}
	root.AddMember(rapidjson::StringRef(TimelineModel_TransFxs_Json), jsonTransFxs, allocator);
}

std::shared_ptr<ITimeLineTag> CTimeLineModel::CreateTag()
{
	std::shared_ptr<ITimeLineTag> tag(new CTimeLineTag());
	tag->SetTagName(GetFirstNoRepeatTagName());
	return tag;
}

void CTimeLineModel::AddTag(std::shared_ptr<ITimeLineTag> pTag)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_vecTags.push_back(pTag);
}

void CTimeLineModel::DeleteTag(const QUuid& id)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);

	for (auto it = m_vecTags.begin(); it != m_vecTags.end(); ++it)
	{
		if ((*it)->GetId() == id)
		{
			m_vecTags.erase(it);
			m_nTagCount--;
			return;
		}
	}
}

std::weak_ptr<ITimeLineTag> CTimeLineModel::GetTag(const QUuid& id)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);

	for (auto pTag : m_vecTags)
	{
		if (pTag && pTag->GetId() == id)
			return pTag;
	}
	return std::weak_ptr<ITimeLineTag>();
}

const std::vector<std::shared_ptr<ITimeLineTag>>& CTimeLineModel::GetAllTags()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_vecTags;
}

std::shared_ptr<ITimeLineNote> CTimeLineModel::CreateNote()
{
	std::shared_ptr<ITimeLineNote> note(new CTimeLineNote());
	note->SetNoteName(GetFirstNoRepeatNoteName());
	return note;
}

void CTimeLineModel::AddNote(std::shared_ptr<ITimeLineNote> pNote)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_vecNotes.push_back(pNote);
}

void CTimeLineModel::DeleteNote(const QUuid& id)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);

	for (auto it = m_vecNotes.begin(); it != m_vecNotes.end(); ++it)
	{
		if ((*it)->GetId() == id)
		{
			m_vecNotes.erase(it);
			m_nNoteCount--;
			return;
		}
	}
}

std::weak_ptr<ITimeLineNote> CTimeLineModel::GetNote(const QUuid& id)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);

	for (auto pNote : m_vecNotes)
	{
		if (pNote && pNote->GetId() == id)
			return pNote;
	}
	return std::weak_ptr<ITimeLineNote>();
}

const std::vector<std::shared_ptr<ITimeLineNote>>& CTimeLineModel::GetAllNotes()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_vecNotes;
}

std::shared_ptr<ITimeLineTransFx> CTimeLineModel::CreateTransFx()
{
	return std::shared_ptr<ITimeLineTransFx>(new CTimeLineTransFx());
}

void CTimeLineModel::AddTransFx(std::shared_ptr<ITimeLineTransFx> pTransFx)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_vecTransFxs.push_back(pTransFx);
}

void CTimeLineModel::DeleteTransFx(const QUuid& id)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);

	for (auto it = m_vecTransFxs.begin(); it != m_vecTransFxs.end(); ++it)
	{
		if ((*it)->GetId() == id)
		{
			m_vecTransFxs.erase(it);
			return;
		}
	}
}

void CTimeLineModel::DeleteTransFxByObject(const QUuid& objectId)
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);

	m_vecTransFxs.erase(
		std::remove_if(m_vecTransFxs.begin(), m_vecTransFxs.end(),
			[&objectId](const std::shared_ptr<ITimeLineTransFx>& pTransFx)
			{
				if (!pTransFx)
					return false;
				return pTransFx->GetLeftObjectId() == objectId || pTransFx->GetRightObjectId() == objectId;
			}),
		m_vecTransFxs.end());
}

std::weak_ptr<ITimeLineTransFx> CTimeLineModel::GetTransFx(const QUuid& id)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);

	for (auto pTransFx : m_vecTransFxs)
	{
		if (pTransFx && pTransFx->GetId() == id)
			return pTransFx;
	}
	return std::weak_ptr<ITimeLineTransFx>();
}

std::weak_ptr<ITimeLineTransFx> CTimeLineModel::FindTransFxByObjects(const QUuid& leftId, const QUuid& rightId)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);

	for (auto pTransFx : m_vecTransFxs)
	{
		if (pTransFx && pTransFx->GetLeftObjectId() == leftId && pTransFx->GetRightObjectId() == rightId)
			return pTransFx;
	}
	return std::weak_ptr<ITimeLineTransFx>();
}

const std::vector<std::shared_ptr<ITimeLineTransFx>>& CTimeLineModel::GetAllTransFxs()
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	return m_vecTransFxs;
}

std::vector<std::shared_ptr<ITimeLineTransFx>> CTimeLineModel::GetTransFxsByObject(const QUuid& objectId)
{
	std::shared_lock<std::shared_mutex> lock(m_mtx);
	std::vector<std::shared_ptr<ITimeLineTransFx>> result;

	for (auto pTransFx : m_vecTransFxs)
	{
		if (!pTransFx)
			continue;
		if (pTransFx->GetLeftObjectId() == objectId || pTransFx->GetRightObjectId() == objectId)
			result.push_back(pTransFx);
	}
	return result;
}

void CTimeLineModel::RebuildTransFxFromObjects()
{
	std::unique_lock<std::shared_mutex> lock(m_mtx);
	m_vecTransFxs.clear();

	auto calcOverlapDuration = [](std::chrono::milliseconds startA, std::chrono::milliseconds durA,
		std::chrono::milliseconds startB, std::chrono::milliseconds durB) -> std::chrono::milliseconds
	{
		const auto endA = startA + durA;
		const auto endB = startB + durB;
		const auto overlapStart = std::max(startA, startB);
		const auto overlapEnd = std::min(endA, endB);
		if (overlapEnd <= overlapStart)
			return std::chrono::milliseconds(0);
		return overlapEnd - overlapStart;
	};

	for (const auto& layerId : m_vecLayers)
	{
		const auto layerIt = m_hashLayers.find(layerId);
		if (layerIt == m_hashLayers.end())
			continue;

		auto pLayer = layerIt->second;
		if (!pLayer)
			continue;

		struct SObjectTime
		{
			QUuid id;
			std::chrono::milliseconds start;
			std::chrono::milliseconds duration;
		};

		std::vector<SObjectTime> objects;
		for (const auto& objId : pLayer->GetAllObjectIDs())
		{
			auto pObject = pLayer->GetObject(objId).lock();
			if (!pObject)
				continue;
			objects.push_back({ objId, pObject->GetStartTime(), pObject->GetDuration() });
		}

		for (size_t i = 0; i < objects.size(); ++i)
		{
			for (size_t j = i + 1; j < objects.size(); ++j)
			{
				const auto& a = objects[i];
				const auto& b = objects[j];
				const auto overlapDur = calcOverlapDuration(a.start, a.duration, b.start, b.duration);
				if (overlapDur.count() <= 0)
					continue;

				const bool aIsLeft = a.start <= b.start;
				const auto& left = aIsLeft ? a : b;
				const auto& right = aIsLeft ? b : a;

				auto pTransFx = std::shared_ptr<ITimeLineTransFx>(new CTimeLineTransFx());
				pTransFx->SetLayerId(layerId);
				pTransFx->SetLeftObjectId(left.id);
				pTransFx->SetRightObjectId(right.id);
				pTransFx->SetDuration(overlapDur);
				pTransFx->SetTransType(ETFT_Fade);
				m_vecTransFxs.push_back(pTransFx);
			}
		}
	}
}

QString CTimeLineModel::GetFirstNoRepeatSectionName()
{
	QVector<QString> vecNames;
	for (auto section : m_listSections)
	{
		vecNames.append(section->GetSectionName()); 
	}
	QString name = "Section";
	QString sectionName = name + QString::number(m_nSectionCount);
	while (vecNames.contains(sectionName))
	{
		m_nSectionCount++;
		sectionName = name + QString::number(m_nSectionCount);
	}
	m_nSectionCount++;
	return sectionName;
}

QString CTimeLineModel::GetFirstNoRepeatTagName()
{
	QVector<QString> vecNames;
	for (auto tag : m_vecTags)
	{
		vecNames.append(tag->GetTagName());
	}
	QString name = "Tag";
	QString tagName = name + QString::number(m_nTagCount);
	while (vecNames.contains(tagName))
	{
		m_nTagCount++;
		tagName = name + QString::number(m_nTagCount);
	}
	m_nTagCount++;
	return tagName;
}

QString CTimeLineModel::GetFirstNoRepeatNoteName()
{
	QVector<QString> vecNames;
	for (auto note : m_vecNotes)
	{
		vecNames.append(note->GetNoteName());
	}
	QString name = "Note";
	QString noteName = name + QString::number(m_nNoteCount);
	while (vecNames.contains(noteName))
	{
		m_nNoteCount++;
		noteName = name + QString::number(m_nNoteCount);
	}
	m_nNoteCount++;
	return noteName;
}

std::chrono::milliseconds CTimeLineModel::normalizeTimeMs(std::chrono::milliseconds ms) const
{
	// 时间点帧对齐：使用 Floor
	return g_pConvertHelper->formatTimeWithFrame(ms, RoundingMode::Floor);
}

std::chrono::milliseconds CTimeLineModel::normalizeDurationMs(std::chrono::milliseconds ms) const
{
	// 时长帧对齐：使用 Floor；正时长至少保留1帧
	auto aligned = g_pConvertHelper->formatTimeWithFrame(ms, RoundingMode::Floor);
	if (ms.count() > 0 && aligned.count() == 0)
		aligned = g_pConvertHelper->frameToTimes(1);
	return aligned;
}

bool CTimeLineModel::hasClipAtPosition(const std::chrono::milliseconds& position) const
{
	for (const auto& layerId : m_vecLayers)
	{
		const auto layerIt = m_hashLayers.find(layerId);
		if (layerIt == m_hashLayers.end())
			continue;

		if (layerIt->second->GetObject(position, true).lock())
			return true;
	}
	return false;
}

std::chrono::milliseconds CTimeLineModel::calcMaxUsefulDurationUnlocked() const
{
	std::chrono::milliseconds ms{ 0 };
	for (const auto& layerPair : m_hashLayers)
	{
		if (ms < layerPair.second->GetMaxTime())
			ms = layerPair.second->GetMaxTime();
	}
	return ms;
}

void CTimeLineModel::rippleInsertOnLayers(const std::chrono::milliseconds& position,
	const std::chrono::milliseconds& length)
{
	for (const auto& layerId : m_vecLayers)
	{
		const auto layerIt = m_hashLayers.find(layerId);
		if (layerIt == m_hashLayers.end())
			continue;

		auto pLayer = layerIt->second;
		if (pLayer->IsLock())
			continue;

		const auto objIds = pLayer->GetAllObjectIDs();
		for (const auto& objId : objIds)
		{
			auto pObj = pLayer->GetObject(objId).lock();
			if (!pObj)
				continue;

			const auto start = pObj->GetStartTime();
			const auto end = start + pObj->GetDuration();
			if (start > position)
			{
				pObj->SetStartTime(start + length);
				pLayer->MoveObject(GetId(), objId);
			}
			else if (start <= position && position < end)
			{
				pObj->SetDuration(pObj->GetDuration() + length);
			}
		}
	}
}

void CTimeLineModel::rippleRemoveOnLayers(const std::chrono::milliseconds& position,
	const std::chrono::milliseconds& length)
{
	const auto removeEnd = position + length;
	for (const auto& layerId : m_vecLayers)
	{
		const auto layerIt = m_hashLayers.find(layerId);
		if (layerIt == m_hashLayers.end())
			continue;

		auto pLayer = layerIt->second;
		if (pLayer->IsLock())
			continue;

		std::vector<QUuid> toDelete;
		const auto objIds = pLayer->GetAllObjectIDs();
		for (const auto& objId : objIds)
		{
			auto pObj = pLayer->GetObject(objId).lock();
			if (!pObj)
				continue;

			const auto start = pObj->GetStartTime();
			const auto duration = pObj->GetDuration();
			const auto end = start + duration;

			if (end <= position)
			{
				continue;
			}
			if (start >= removeEnd)
			{
				pObj->SetStartTime(start - length);
				pLayer->MoveObject(GetId(), objId);
			}
			else if (start >= position && end <= removeEnd)
			{
				toDelete.push_back(objId);
			}
			else if (start < position && end > removeEnd)
			{
				pObj->SetDuration(duration - length);
			}
			else if (start < position && end > position && end <= removeEnd)
			{
				pObj->SetDuration(position - start);
			}
			else if (start >= position && start < removeEnd && end > removeEnd)
			{
				pObj->SetStartTime(position);
				pObj->SetDuration(end - removeEnd);
				pLayer->MoveObject(GetId(), objId);
			}
		}

		for (const auto& delId : toDelete)
			pLayer->DeleteObject(GetId(), delId);
	}
}

void CTimeLineModel::adjustSectionsForInsert(const std::chrono::milliseconds& position,
	const std::chrono::milliseconds& length)
{
	for (auto& pSection : m_listSections)
	{
		if (!pSection)
			continue;

		const auto secStart = pSection->GetSectionStartTime();
		const auto secEnd = pSection->GetSectionEndTime();
		if (secStart <= position && position < secEnd)
		{
			pSection->SetSectionEndTime(secEnd + length);
		}
		else if (secStart > position)
		{
			pSection->SetSectionStartTime(secStart + length);
			pSection->SetSectionEndTime(secEnd + length);
		}
	}
}

void CTimeLineModel::adjustSectionsForRemove(const std::chrono::milliseconds& position,
	const std::chrono::milliseconds& length)
{
	const auto removeEnd = position + length;
	std::list<std::shared_ptr<ITimeLineSection>> result;

	for (auto& pSection : m_listSections)
	{
		if (!pSection)
			continue;

		const auto secStart = pSection->GetSectionStartTime();
		const auto secEnd = pSection->GetSectionEndTime();

		if (secEnd <= position)
		{
			result.push_back(pSection);
		}
		else if (removeEnd <= secStart)
		{
			pSection->SetSectionStartTime(secStart - length);
			pSection->SetSectionEndTime(secEnd - length);
			result.push_back(pSection);
		}
		else if (secStart < position && removeEnd < secEnd)
		{
			pSection->SetSectionEndTime(secEnd - length);
			result.push_back(pSection);
		}
		else if (secStart < position && position < secEnd && secEnd <= removeEnd)
		{
			pSection->SetSectionEndTime(position);
			result.push_back(pSection);
		}
		else if (position <= secStart && secStart < removeEnd && removeEnd < secEnd)
		{
			pSection->SetSectionStartTime(position);
			pSection->SetSectionEndTime(secEnd - length);
			result.push_back(pSection);
		}
		else
		{

		}
	}

	result.sort([](const std::shared_ptr<ITimeLineSection>& sectionA,
		const std::shared_ptr<ITimeLineSection>& sectionB) {
		if (!sectionA || !sectionB)
			return false;
		return sectionA->GetSectionStartTime() < sectionB->GetSectionStartTime();
	});

	const auto removedCount = static_cast<int>(m_listSections.size() - result.size());
	if (removedCount > 0)
		m_nSectionCount -= removedCount;

	m_listSections.swap(result);
}

void CTimeLineModel::syncLastSectionEndToDurationUnlocked()
{
	if (m_listSections.empty())
		return;

	auto pLast = m_listSections.back();
	if (pLast)
		pLast->SetSectionEndTime(m_duration);
}

bool CTimeLineModel::InsertTime(const std::chrono::milliseconds& positionIn,
	const std::chrono::milliseconds& lengthIn, ETimeEditScope scope)
{
	const auto position = normalizeTimeMs(positionIn);
	const auto length = normalizeDurationMs(lengthIn);
	if (position.count() < 0 || length.count() <= 0)
		return false;

	std::unique_lock<std::shared_mutex> lock(m_mtx);

	if (scope == ETimeEditScope::LayersOnly && !hasClipAtPosition(position))
		return false;

	rippleInsertOnLayers(position, length);

	if (scope == ETimeEditScope::LayersAndSection)
	{
		adjustSectionsForInsert(position, length);
		m_duration = normalizeDurationMs(m_duration + length);
		m_duration = normalizeDurationMs(std::max(m_duration, calcMaxUsefulDurationUnlocked()));
		m_duration = normalizeDurationMs(std::max(m_duration, std::chrono::milliseconds(10000)));
		syncLastSectionEndToDurationUnlocked();
	}
	else
	{
		const auto maxUseful = calcMaxUsefulDurationUnlocked();
		if (maxUseful > m_duration)
			m_duration = normalizeDurationMs(maxUseful);
	}

	return true;
}

bool CTimeLineModel::RemoveTime(const std::chrono::milliseconds& positionIn,
	const std::chrono::milliseconds& lengthIn, ETimeEditScope scope)
{
	const auto position = normalizeTimeMs(positionIn);
	const auto length = normalizeDurationMs(lengthIn);
	if (position.count() < 0 || length.count() <= 0)
		return false;

	if (position + length > m_duration)
		return false;

	std::unique_lock<std::shared_mutex> lock(m_mtx);

	rippleRemoveOnLayers(position, length);

	if (scope == ETimeEditScope::LayersAndSection)
	{
		adjustSectionsForRemove(position, length);
		m_duration = normalizeDurationMs(m_duration - length);
		m_duration = normalizeDurationMs(std::max(m_duration, calcMaxUsefulDurationUnlocked()));
		m_duration = normalizeDurationMs(std::max(m_duration, std::chrono::milliseconds(10000)));
		syncLastSectionEndToDurationUnlocked();
	}
	else
	{
		const auto maxUseful = calcMaxUsefulDurationUnlocked();
		if (maxUseful > m_duration)
			m_duration = normalizeDurationMs(maxUseful);
	}

	return true;
}