#include "TeleprompterResource.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/GlobalModel.h"

#include "UICommon/UICommonSizeDefine.h"
#include "UICommon/UITranslate.h"

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

CTeleprompterResource::CTeleprompterResource()
{
	m_nResType = ERT_Teleprompter;
	m_Thumbnail.load(":/dark/images/ResourceWnd/Thumbmail/Normal/Teleprompter.svg");
	m_Thumbnail = m_Thumbnail.scaled(Res_ThumbnailWidth, Res_ThumbnailHeight);

	m_nDuration = 0;
	m_szResolution = QSize(1920, 1080);

	m_clrFont = Qt::white;
	m_textFont.setFamily("Microsoft YaHei");
	m_textFont.setPixelSize(48);
}

CTeleprompterResource::CTeleprompterResource(const CTeleprompterResource & other)
	: CResourceBase(other), CTurnPageResource(other)
{
	m_vecTeleprompterDatas = other.m_vecTeleprompterDatas;
	m_bShowHorDir = other.m_bShowHorDir;
	m_textFont = other.m_textFont;
	m_clrFont = other.m_clrFont;
	m_bBkClrEnable = other.m_bBkClrEnable;
	m_clrBk = other.m_clrBk;
}

CTeleprompterResource::~CTeleprompterResource()
{
}

void CTeleprompterResource::AddDefaultTeleprompterItem(int index)
{
	if (index > m_vecTeleprompterDatas.size())
		return;

	STeleprompterItem node;
	if (m_vecTeleprompterDatas.empty() || index == m_vecTeleprompterDatas.size() - 1)
		m_vecTeleprompterDatas.push_back(node);
	else
		m_vecTeleprompterDatas.emplace(m_vecTeleprompterDatas.begin() + index + 1, node);

	m_nDuration = m_vecTeleprompterDatas.size();
}


bool CTeleprompterResource::DeleteTeleprompterItem(int index)
{
	m_vecTeleprompterDatas.erase(m_vecTeleprompterDatas.begin() + index);
	m_nDuration = m_vecTeleprompterDatas.size();
	return true;
}

void CTeleprompterResource::SetTeleprompterItemDur(int index, long long duration)
{
	if (index < 0 || index >= m_vecTeleprompterDatas.size())
		return ;

	m_vecTeleprompterDatas[index].nDuration = duration;
}

void CTeleprompterResource::SetTeleprompterItemText(int index, const QString &text)
{
	if (index < 0 || index >= m_vecTeleprompterDatas.size())
		return;

	m_vecTeleprompterDatas[index].strText = text;
}

void CTeleprompterResource::SetTeleprompterData(std::vector<STeleprompterItem> itemList)
{
	m_vecTeleprompterDatas = itemList;
	m_nDuration = m_vecTeleprompterDatas.size();
}

std::vector<STeleprompterItem> CTeleprompterResource::GetTeleprompterData()
{
	return m_vecTeleprompterDatas;
}

long long CTeleprompterResource::GetTeleprompterItemDur(int index)
{
	if (index < 0 || index >= m_vecTeleprompterDatas.size())
		return -1;

	return m_vecTeleprompterDatas[index].nDuration;
}

QString CTeleprompterResource::GetTeleprompterItemText(int index)
{
	if (index < 0 || index >= m_vecTeleprompterDatas.size())
		return QString();

	return m_vecTeleprompterDatas[index].strText;
}

void CTeleprompterResource::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	CResourceBase::SaveDataToRapidJson(root, allocator);
	root.AddMember("textColorR", m_clrFont.red(), allocator);
	root.AddMember("textColorG", m_clrFont.green(), allocator);
	root.AddMember("textColorB", m_clrFont.blue(), allocator);
	root.AddMember("textColorA", m_clrFont.alpha(), allocator);
	root.AddMember("textFamily", rapidjson::Value(m_textFont.family().toUtf8().constData(), allocator), allocator);
	root.AddMember("textBold", m_textFont.bold(), allocator);
	root.AddMember("textUnderline", m_textFont.underline(), allocator);
	root.AddMember("textItalic", m_textFont.italic(), allocator);
	root.AddMember("textFontSize", m_textFont.pixelSize(), allocator);
	root.AddMember("HAlignMent", m_nAlignment, allocator);
	root.AddMember("bBackGround", m_bBkClrEnable, allocator);
	root.AddMember("BGColorR", m_clrBk.red(), allocator);
	root.AddMember("BGColorG", m_clrBk.green(), allocator);
	root.AddMember("BGColorB", m_clrBk.blue(), allocator);
	root.AddMember("BGColorA", m_clrBk.alpha(), allocator);
	root.AddMember("horShowDir", m_bShowHorDir, allocator);

	rapidjson::Value arSubtitleDataObj(rapidjson::kArrayType);
	for (auto it : m_vecTeleprompterDatas)
	{
		rapidjson::Value objNode(rapidjson::kObjectType);
		objNode.AddMember("duration", static_cast<int64_t>(it.nDuration), allocator);
		objNode.AddMember("text", rapidjson::Value(it.strText.toUtf8().constData(), allocator), allocator);
		arSubtitleDataObj.PushBack(objNode, allocator);
	}
	root.AddMember("teleprompterItemData", arSubtitleDataObj, allocator);
}

bool CTeleprompterResource::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	CResourceBase::LoadDataFromRapidJson(root);
	if (!root.IsObject())
		return false;

	m_clrFont = QColor(
		root.HasMember("textColorR") ? root["textColorR"].GetInt() : 255,
		root.HasMember("textColorG") ? root["textColorG"].GetInt() : 255,
		root.HasMember("textColorB") ? root["textColorB"].GetInt() : 255,
		root.HasMember("textColorA") ? root["textColorA"].GetInt() : 255);
	m_textFont.setFamily((root.HasMember("textFamily") && root["textFamily"].IsString()) ? QString::fromUtf8(root["textFamily"].GetString()) : QString());
	m_textFont.setBold(root.HasMember("textBold") ? root["textBold"].GetBool() : false);
	m_textFont.setUnderline(root.HasMember("textUnderline") ? root["textUnderline"].GetBool() : false);
	m_textFont.setItalic(root.HasMember("textItalic") ? root["textItalic"].GetBool() : false);
	m_nAlignment = root.HasMember("HAlignMent") ? root["HAlignMent"].GetInt() : 0;
	m_textFont.setPixelSize(root.HasMember("textFontSize") ? root["textFontSize"].GetInt() : 24);
	m_bShowHorDir = root.HasMember("horShowDir") ? root["horShowDir"].GetBool() : m_bShowHorDir;


	m_bBkClrEnable = root.HasMember("bBackGround") ? root["bBackGround"].GetBool() : false;
	m_clrBk = QColor(
		root.HasMember("BGColorR") ? root["BGColorR"].GetInt() : 0,
		root.HasMember("BGColorG") ? root["BGColorG"].GetInt() : 0,
		root.HasMember("BGColorB") ? root["BGColorB"].GetInt() : 0,
		root.HasMember("BGColorA") ? root["BGColorA"].GetInt() : 255);

	{
		m_vecTeleprompterDatas.clear();
		if (root.HasMember("teleprompterItemData") && root["teleprompterItemData"].IsArray())
		{
			const rapidjson::Value& arSubtitleDataObj = root["teleprompterItemData"];
			for (rapidjson::SizeType i = 0; i < arSubtitleDataObj.Size(); ++i)
			{
				const rapidjson::Value& objNode = arSubtitleDataObj[i];
				if (!objNode.IsObject())
					continue;
				STeleprompterItem node;
				node.nDuration = objNode.HasMember("duration") ? objNode["duration"].GetInt64() : 0;
				node.strText = (objNode.HasMember("text") && objNode["text"].IsString()) ? QString::fromUtf8(objNode["text"].GetString()) : QString();
				m_vecTeleprompterDatas.push_back(node);
			}
		}
		m_nDuration = m_vecTeleprompterDatas.size();
	}

	//m_bIsKeepOpen = obj.value("KeepOpen").toBool();
	return true;
}
