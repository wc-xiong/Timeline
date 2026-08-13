#include "SubtitleResource.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Model/GlobalModel/GlobalModel.h"

#include "UICommon/UICommonSizeDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

#define DEFAULT_WIDTH 1920
#define DEFAULT_HEIGHT 1080

CSubtitleResource::CSubtitleResource()
{
	m_nResType = ERT_Subtitle;
	m_Thumbnail.load(":/dark/images/ResourceWnd/Thumbmail/Normal/Subtitle.svg");
	m_Thumbnail = m_Thumbnail.scaled(Res_ThumbnailWidth, Res_ThumbnailHeight);

	m_nDuration = g_pProjectSetting->LoadDefaultToolDuration();
	m_szResolution = QSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

	m_clrFont = Qt::white;

	m_textFont.setFamily("Microsoft YaHei");
	m_textFont.setPixelSize(48);
}

CSubtitleResource::CSubtitleResource(const CSubtitleResource & other)
	: CResourceBase(other)
{
	m_vecSubtitleDatas = other.m_vecSubtitleDatas;
	m_bShowHorDir = other.m_bShowHorDir;
	m_textFont = other.m_textFont;
	m_clrFont = other.m_clrFont;
	m_bBkClrEnable = other.m_bBkClrEnable;
	m_clrBk = other.m_clrBk;
}

CSubtitleResource& CSubtitleResource::operator=(const CSubtitleResource& other)
{
	if (this != &other)
	{
		*((CResourceBase*)this) = other;

		m_vecSubtitleDatas = other.m_vecSubtitleDatas;
		m_bShowHorDir = other.m_bShowHorDir;
		m_textFont = other.m_textFont;
		m_clrFont = other.m_clrFont;
		m_bBkClrEnable = other.m_bBkClrEnable;
		m_clrBk = other.m_clrBk;
	}
	return *this;
}

CSubtitleResource::~CSubtitleResource()
{

}

void CSubtitleResource::SetSubtitleDatas(std::vector<SSubtitleDataNode> vecDatas)
{
	std::unique_lock<std::shared_mutex> Loker(m_mtxVecSubtitle);
	m_vecSubtitleDatas = vecDatas;
}

std::vector<SSubtitleDataNode> CSubtitleResource::GetSubtitleDatas()
{
	std::shared_lock<std::shared_mutex> Loker(m_mtxVecSubtitle);
	return m_vecSubtitleDatas;
}

bool CSubtitleResource::GetSubTitleEmpty()
{
	std::shared_lock<std::shared_mutex> Loker(m_mtxVecSubtitle);

	return m_vecSubtitleDatas.empty();
}

SSubtitleDataNode CSubtitleResource::GetIndexSubtitleNode(int nIndex)
{
	std::shared_lock<std::shared_mutex> Loker(m_mtxVecSubtitle);

	if(nIndex <0 || nIndex>= m_vecSubtitleDatas.size())
		return SSubtitleDataNode();
	return m_vecSubtitleDatas[nIndex];
}

int CSubtitleResource::GetPlayTextIndexByTime(__int64 nMs)
{
	std::shared_lock<std::shared_mutex> Loker(m_mtxVecSubtitle);

	if (m_vecSubtitleDatas.empty())
		return -1;
	for (int i = 0; i < m_vecSubtitleDatas.size(); i++)
	{
		if (m_vecSubtitleDatas[i].nBeginTime <= nMs && m_vecSubtitleDatas[i].nEndTime > nMs)
			return i;
	}
	return -1;
}

void CSubtitleResource::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	CResourceBase::SaveDataToRapidJson(root, allocator);
	root.AddMember("bShowHorDir", m_bShowHorDir, allocator);
	root.AddMember("textColorR", m_clrFont.red(), allocator);
	root.AddMember("textColorG", m_clrFont.green(), allocator);
	root.AddMember("textColorB", m_clrFont.blue(), allocator);
	root.AddMember("textColorA", m_clrFont.alpha(), allocator);
	root.AddMember("textFamily", rapidjson::Value(m_textFont.family().toUtf8().constData(), allocator), allocator);
	root.AddMember("textBold", m_textFont.bold(), allocator);
	root.AddMember("textUnderline", m_textFont.underline(), allocator);
	root.AddMember("textItalic", m_textFont.italic(), allocator);
	root.AddMember("textPixelSize", m_textFont.pixelSize(), allocator);
	root.AddMember("bBackGround", m_bBkClrEnable, allocator);
	root.AddMember("BGColorR", m_clrBk.red(), allocator);
	root.AddMember("BGColorG", m_clrBk.green(), allocator);
	root.AddMember("BGColorB", m_clrBk.blue(), allocator);
	root.AddMember("BGColorA", m_clrBk.alpha(), allocator);

	rapidjson::Value arSubtitleDataObj(rapidjson::kArrayType);
	for (auto it : m_vecSubtitleDatas)
	{
		rapidjson::Value objNode(rapidjson::kObjectType);
		objNode.AddMember("BeginTime", static_cast<int64_t>(it.nBeginTime), allocator);
		objNode.AddMember("EndTime", static_cast<int64_t>(it.nEndTime), allocator);
		objNode.AddMember("text", rapidjson::Value(it.strText.toUtf8().constData(), allocator), allocator);
		arSubtitleDataObj.PushBack(objNode, allocator);
	}
	root.AddMember("SubtitleDatas", arSubtitleDataObj, allocator);
}

bool CSubtitleResource::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	CResourceBase::LoadDataFromRapidJson(root);
	if (!root.IsObject())
		return false;

	m_bShowHorDir = root.HasMember("bShowHorDir") ? root["bShowHorDir"].GetBool() : m_bShowHorDir;

	m_clrFont = QColor(
		root.HasMember("textColorR") ? root["textColorR"].GetInt() : 255,
		root.HasMember("textColorG") ? root["textColorG"].GetInt() : 255,
		root.HasMember("textColorB") ? root["textColorB"].GetInt() : 255,
		root.HasMember("textColorA") ? root["textColorA"].GetInt() : 255);
	m_textFont.setFamily((root.HasMember("textFamily") && root["textFamily"].IsString()) ? QString::fromUtf8(root["textFamily"].GetString()) : QString());
	m_textFont.setBold(root.HasMember("textBold") ? root["textBold"].GetBool() : false);
	m_textFont.setUnderline(root.HasMember("textUnderline") ? root["textUnderline"].GetBool() : false);
	m_textFont.setItalic(root.HasMember("textItalic") ? root["textItalic"].GetBool() : false);
	m_textFont.setPixelSize(root.HasMember("textPixelSize") ? root["textPixelSize"].GetInt() : 24);

	m_bBkClrEnable = root.HasMember("bBackGround") ? root["bBackGround"].GetBool() : false;
	m_clrBk = QColor(
		root.HasMember("BGColorR") ? root["BGColorR"].GetInt() : 0,
		root.HasMember("BGColorG") ? root["BGColorG"].GetInt() : 0,
		root.HasMember("BGColorB") ? root["BGColorB"].GetInt() : 0,
		root.HasMember("BGColorA") ? root["BGColorA"].GetInt() : 255);

	m_vecSubtitleDatas.clear();
	if (root.HasMember("SubtitleDatas") && root["SubtitleDatas"].IsArray())
	{
		const rapidjson::Value& arSubtitleDataObj = root["SubtitleDatas"];
		for (rapidjson::SizeType i = 0; i < arSubtitleDataObj.Size(); ++i)
		{
			const rapidjson::Value& objNode = arSubtitleDataObj[i];
			if (!objNode.IsObject())
				continue;
			SSubtitleDataNode node;
			node.nBeginTime = objNode.HasMember("BeginTime") ? objNode["BeginTime"].GetInt64() : 0;
			node.nEndTime = objNode.HasMember("EndTime") ? objNode["EndTime"].GetInt64() : 0;
			node.strText = (objNode.HasMember("text") && objNode["text"].IsString()) ? QString::fromUtf8(objNode["text"].GetString()) : QString();
			m_vecSubtitleDatas.push_back(node);
		}
	}

	return true;
}
