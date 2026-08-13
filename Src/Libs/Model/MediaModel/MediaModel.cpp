#include "MediaModel.h"
#include "MediaNode.h"
#include "Common/IMediaInterface.h"

#include "EffectProperty.h"

#include <QUrl>
#include <QRect>
#include <QDebug>

#include "Common/SignalCenter/GSNetSignalCenter.h"
#include "Net/NetCommon/TransDataDefine.h"
#include "Model/ResourceModel/ResourceModelExport.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CMediaModel::CMediaModel():m_pGlobalProperty(new CEffectProperty) {}

CMediaModel::~CMediaModel()
{
	for (auto iter : m_mapMediaNodes)
	{
		iter.second.reset();
	}
}

CMediaModel::CMediaModel(const CMediaModel & other)
{
	m_mapMediaNodes.clear();
	for (auto iter : other.m_mapMediaNodes)
	{
		auto pNode = std::shared_ptr<IMediaNode>(dynamic_cast<IMediaNode*>(iter.second->Copy()));
		m_mapMediaNodes[pNode->GetMediaId()] = pNode;
	}
	m_vecMeidaNodes = other.m_vecMeidaNodes;

	m_pGlobalProperty = std::make_shared<CEffectProperty>(other.m_pGlobalProperty);
}

CMediaModel & CMediaModel::operator=(const CMediaModel & other)
{
	if (this == &other)
		return *this;

	std::unique_lock<std::shared_mutex> lock(m_mtxMedia);
	m_mapMediaNodes.clear();
	for (auto iter : other.m_mapMediaNodes)
	{
		auto pNode = std::shared_ptr<IMediaNode>(dynamic_cast<IMediaNode*>(iter.second->Copy()));
		m_mapMediaNodes[pNode->GetMediaId()] = pNode;
	}
	m_vecMeidaNodes = other.m_vecMeidaNodes;
	m_pGlobalProperty->CopyProperty(other.m_pGlobalProperty);

	return *this;
}

IMediaNode* CMediaModel::CreateReplaceMedia(const QUuid & mediaId)
{
	QUuid id;
	id = QUuid::createUuid();
	auto MediaNode = new CMediaNode(id);
	auto beforeMediaNode = GetMediaNode(mediaId).lock();
	if (beforeMediaNode)
	{
		MediaNode->SetDrawRect(beforeMediaNode->GetDrawRect());
		MediaNode->SetMediaRotateAngle(beforeMediaNode->GetMediaRotateAngle());
		return MediaNode;
	}
	else
		return nullptr;
}

void CMediaModel::DeleteMediaNode(const QUuid &id)
{
	QUuid resId;
	{
		std::unique_lock<std::shared_mutex> lock(m_mtxMedia);
		auto iter = m_mapMediaNodes.find(id);
		if (iter != m_mapMediaNodes.end())
		{
			if (iter->second)
				resId = iter->second->GetResourceId();
			iter->second.reset();
			m_mapMediaNodes.erase(id);
		}
		m_vecMeidaNodes.erase(std::remove_if(m_vecMeidaNodes.begin(), m_vecMeidaNodes.end(),
			[id](QUuid x) { return x == id; }),
			m_vecMeidaNodes.end());
	}

	// 时间线删除片段时回退素材引用计数，否则素材库删除会一直被“使用中”拦住
	if (!resId.isNull())
	{
		if (auto pRes = g_pResourceModel->GetResourceNode(resId))
		{
			const int n = pRes->GetResourceUseCount();
			pRes->SetResourceUseCount(n > 0 ? n - 1 : 0);
		}
	}

	emit g_pGSNetSignal->signalMediaNodeOp(ETDT_DeleteMediaNode, id);
}

void CMediaModel::ClearMediaNodes()
{
	{
		std::unique_lock<std::shared_mutex> lock(m_mtxMedia);
		for (auto iter : m_mapMediaNodes)
		{
			iter.second.reset();
		}
		m_mapMediaNodes.clear();
		m_vecMeidaNodes.clear();
	}


	emit g_pGSNetSignal->signalNoDataOp(ETDT_ClearAllMedia);
}

void CMediaModel::AddMedia(std::shared_ptr<IMediaNode> ptr)
{
	{
		std::unique_lock<std::shared_mutex> lock(m_mtxMedia);

		m_mapMediaNodes[ptr->GetMediaId()] = ptr;
		m_vecMeidaNodes.push_back(ptr->GetMediaId());
	}


	emit g_pGSNetSignal->signalMediaNodeOp(ETDT_AddMediaNode, ptr->GetMediaId());
}

std::shared_ptr<IMediaNode> CMediaModel::CreateMediaNode()
{
	return std::shared_ptr<IMediaNode>(new CMediaNode(QUuid::createUuid()));;
}


std::vector<QUuid> CMediaModel::GetAllMediaId()
{
	std::shared_lock<std::shared_mutex> lock(m_mtxMedia);
	return m_vecMeidaNodes;
}

std::weak_ptr<IMediaNode> CMediaModel::GetMediaNode(const QUuid &id)
{
	std::shared_lock<std::shared_mutex> lock(m_mtxMedia);
	auto iter = m_mapMediaNodes.find(id);
	if (iter != m_mapMediaNodes.end())
		return iter->second;

	return std::weak_ptr<IMediaNode>();
}

std::weak_ptr<IEffectProperty> CMediaModel::GetGlobalProperty()
{
	return m_pGlobalProperty;
}

void CMediaModel::ResetGlobalProperty()
{
	//m_pGlobalProperty->
}

std::vector<QUuid> CMediaModel::GetResMeadiaIds(const QUuid& resId)
{
	std::vector<QUuid> vecIds;
	std::shared_lock<std::shared_mutex> lock(m_mtxMedia);
	for (const auto& it : m_mapMediaNodes)
	{
		if (it.second->GetResourceId() != resId)
			continue;
		vecIds.push_back(it.second->GetMediaId());
	}

	return vecIds;
}

D2D1_RECT_D CMediaModel::GetBoundingRectPts(const std::vector<QUuid>& vecId)
{
	D2D1_RECT_D rc;
	rc.left = DBL_MAX;
	rc.right = DBL_MAX * -1.0f;
	rc.top = DBL_MAX;
	rc.bottom = DBL_MAX * -1.0f;

	std::shared_lock<std::shared_mutex> lock(m_mtxMedia);
	for (const auto& mediaId : vecId)
	{
		auto iter = m_mapMediaNodes.find(mediaId);
		if (iter == m_mapMediaNodes.end())
			continue;

		std::vector<CPointD> vecPt = iter->second->GetSizePoint(true);
		//std::vector<CPointD> vecPt = iter->second->GetDrawSizeRectWithoutRotate();
		for (const auto& pt : vecPt)
		{
			rc.left = pt.x > rc.left ? rc.left : pt.x;
			rc.right = pt.x < rc.right ? rc.right : pt.x;
			rc.top = pt.y > rc.top ? rc.top : pt.y;
			rc.bottom = pt.y < rc.bottom ? rc.bottom : pt.y;
		}
		
	}

	if (rc.Width() <= 0)
	{
		rc.left = rc.left - 0.5;
		rc.right = rc.left + 1.0;
	}
	if (rc.Height() <= 0)
	{
		rc.top = rc.top - 0.5;
		rc.bottom = rc.top + 1.0;
	}

	return rc;
}

bool CMediaModel::IsMediaExist(const QUuid &uuid)
{
	std::shared_lock<std::shared_mutex> lock(m_mtxMedia);
	auto iter = m_mapMediaNodes.find(uuid);
	if (iter != m_mapMediaNodes.end())
		return true;

	return false;
}

bool CMediaModel::IsMediaExistByPlayerId(const QUuid & uuid)
{
	std::shared_lock<std::shared_mutex> lock(m_mtxMedia);
	for (auto it : m_mapMediaNodes)
	{
		if (it.second->GetResourceId() == uuid)
			return true;
	}

	return false;
}

void CMediaModel::SaveMediaModelToRapidJson(rapidjson::Value &rapidRoot, rapidjson::Document::AllocatorType &allocator)
{
	rapidRoot.SetObject();
	rapidjson::Value mediaArray(rapidjson::kArrayType);
	
	// 保证meida信息按添加顺序存入，读取时也能有序读出
	for (const QUuid& id : m_vecMeidaNodes)
	{
		auto iter = m_mapMediaNodes.find(id);
		if (iter != m_mapMediaNodes.end() && iter->second)
		{
			rapidjson::Value mediaRapidObj(rapidjson::kObjectType);
			iter->second->SaveDataToRapidJson(mediaRapidObj, allocator);
			mediaArray.PushBack(mediaRapidObj, allocator);
		}
	}

	rapidRoot.AddMember(rapidjson::Value(Xml_Project_ModelData_Media.toUtf8().constData(), allocator), mediaArray, allocator);
	rapidjson::Value globalPropertyRapidObj(rapidjson::kObjectType);
	m_pGlobalProperty->SaveDataToRapidJson(globalPropertyRapidObj, allocator);
	rapidRoot.AddMember(rapidjson::Value("GlobalEffectProperty", allocator), globalPropertyRapidObj, allocator);
}
void CMediaModel::LoadMediaModelFromRapidJson(const rapidjson::Value & root)
{
	m_vecMeidaNodes.clear();
	m_mapMediaNodes.clear();

	const QByteArray mediaKey = Xml_Project_ModelData_Media.toUtf8();
	if (!root.IsObject() || !root.HasMember(mediaKey.constData()) || !root[mediaKey.constData()].IsArray() || !root.HasMember("GlobalEffectProperty"))
		return;

	const rapidjson::Value& mediaArray = root[mediaKey.constData()];

	for (rapidjson::SizeType i = 0; i < mediaArray.Size(); ++i)
	{
		const rapidjson::Value& mediaObj = mediaArray[i];
		if (!mediaObj.IsObject() || !mediaObj.HasMember("MediaId") || !mediaObj["MediaId"].IsString())
			continue;

		QString idStr = QString::fromUtf8(mediaObj["MediaId"].GetString());
		QUuid mediaId(idStr);
		if (mediaId.isNull())
			continue;

		// 创建节点
		// 创建节点
		auto pMediaNode = std::shared_ptr<IMediaNode>(new CMediaNode(mediaId));
		pMediaNode->LoadDataFromRapidJson(mediaObj);

		m_vecMeidaNodes.push_back(mediaId);
		m_mapMediaNodes[mediaId] = pMediaNode;
	}
	if (root["GlobalEffectProperty"].IsObject())
		m_pGlobalProperty->LoadDataFromRapidJson(root["GlobalEffectProperty"]);
}
