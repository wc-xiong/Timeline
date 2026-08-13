#include "FxProperty.h"
#include "common/IResourceInterface.h"
#include "common/XEffectCommon.h"
#include "FxFactor.h"
#include <memory.h>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end


CFxProperty::CFxProperty()
{
}

CFxProperty::~CFxProperty()
{
	//std::cout << "CMediaBaseProperty::~CMediaBaseProperty()" << std::endl;
	for (auto& fx : m_vecFactor)
	{
		fx.reset();
	}

	m_vecFactor.clear();
}

CFxProperty::CFxProperty(const CFxProperty & data)
{
	m_vecFactor = data.m_vecFactor;
}

CFxProperty & CFxProperty::operator=(const CFxProperty & data)
{
	if (this == &data)
		return *this;

	m_vecFactor = data.m_vecFactor;
	
	return *this;
}

CFxProperty::CFxProperty(std::shared_ptr<IFxProperty> pdata)
{
	CopyProperty(pdata);
}

void CFxProperty::ClearAllFxFacyor()
{
	m_vecFactor.clear();
}

void CFxProperty::DeleteFxFactor(int type)
{
	for (auto iter = m_vecFactor.begin(); iter < m_vecFactor.end();)
	{
		auto item = *iter;
		if (-1 == type)//删除所有特效
		{
			item.reset();
			iter = m_vecFactor.erase(iter);
			continue;
		}
		if (item->GetType() == type)//单个删除
		{
			item.reset();
			m_vecFactor.erase(iter);
			break;
		}
		iter++;
	}
}

void CFxProperty::AddFxFaxtor(std::shared_ptr<IFxFactor> pFxFactor, int nIndex)
{
	if (nIndex >= 0 && nIndex < m_vecFactor.size())
		m_vecFactor.insert(m_vecFactor.begin() + nIndex, pFxFactor);
	else
		m_vecFactor.push_back(pFxFactor);
}

std::shared_ptr<IFxFactor> CFxProperty::GetFxFactor(int type)
{
	for (auto iter : m_vecFactor)
	{
		if (iter->GetType() == type)
			return iter;
	}
	return nullptr;
}

std::shared_ptr<IFxFactor> CFxProperty::GetFxFactorWithIndex(int type, int& nIndex)
{
	nIndex = 0;
	for (auto iter : m_vecFactor)
	{
		if (iter->GetType() == type)
			return iter;

		nIndex++;
	}

	nIndex = -1;
	return nullptr;
}

std::vector<std::shared_ptr<IFxFactor>> CFxProperty::GetAllFxFactor()
{
	return m_vecFactor;
}

bool CFxProperty::hasFactor()
{
	return !m_vecFactor.empty();
}

void CFxProperty::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	root.SetObject();
	int num = (int)m_vecFactor.size();
	root.AddMember("num", num, allocator);
	for (int i = 0; i < num; i++)
	{
		rapidjson::Value ObjItem(rapidjson::kObjectType);
		ObjItem.AddMember("type", m_vecFactor[i]->GetType(), allocator);
		rapidjson::Value factorRapid(rapidjson::kObjectType);
		m_vecFactor[i]->SaveDataToRapidJson(factorRapid, allocator);
		ObjItem.AddMember("data", factorRapid, allocator);
		root.AddMember(rapidjson::Value(QString::number(i).toUtf8().constData(), allocator), ObjItem, allocator);
	}
}

void CFxProperty::LoadDataFromRapidJson(const rapidjson::Value &root)
{
	if (!root.IsObject() || !root.HasMember("num"))
		return;
	int num = root["num"].GetInt();

	for (int i = 0; i < num; i++)
	{
		const QByteArray idxKey = QString::number(i).toUtf8();
		if (!root.HasMember(idxKey.constData()) || !root[idxKey.constData()].IsObject())
			continue;
		const rapidjson::Value& ObjItem = root[idxKey.constData()];
		if (!ObjItem.HasMember("type") || !ObjItem["type"].IsInt())
			continue;
		int type = ObjItem["type"].GetInt();

		auto p = std::shared_ptr<IFxFactor>(new CFxFactor(type));
		if (ObjItem.HasMember("data") && ObjItem["data"].IsObject())
			p->LoadDataFromRapidJson(ObjItem["data"]);

		m_vecFactor.push_back(p);
	}
}

void CFxProperty::CopyProperty(std::shared_ptr<IFxProperty> pdata)
{
	m_vecFactor.clear();

	std::vector<std::shared_ptr<IFxFactor>> vecFxs = pdata->GetAllFxFactor();
	for (auto it : vecFxs)
		m_vecFactor.push_back(std::shared_ptr<IFxFactor>(new CFxFactor(it)));
}
