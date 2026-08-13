#include "TextResource.h"
#include "UICommon/UITranslate.h"
#include <iostream>
#include <QCoreApplication>
#include <QTextDocument>
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Model/GlobalModel/GlobalModel.h"

#include "UICommon/UICommonSizeDefine.h"
#include "Common/ResourceDefine.h"
#include "Common/CommonDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CTextResource::CTextResource()
{
	m_nDuration = g_pProjectSetting->LoadDefaultToolDuration();

	m_nResType = ERT_Text;
	m_nShowType = ETRT_TegularText;
	m_nColorTextType = EDCPT_Forbidden;

	m_strContent = CTextPropertyFrameTranslate::DefaultText();
	m_szResolution = QSize(1920, 1080);
	m_bBcakGround = false;
	m_BGColor = QColor(0, 0, 0);

	m_textFont.setFamily("Microsoft YaHei");

	m_textColor = Qt::white;
	m_nHAlignMent = Qt::AlignLeft;
}

CTextResource::~CTextResource()
{
}

CTextResource::CTextResource(const CTextResource& other)
{
	*this = other;
}

CTextResource& CTextResource::operator=(const CTextResource& other)
{
	*((CResourceBase*)(this)) = other;

	m_nShowType = other.m_nShowType;

	m_strContent = other.m_strContent;
	
	m_bBcakGround = other.m_bBcakGround;
	m_BGColor = other.m_BGColor;
	m_textFont = other.m_textFont;
	m_textColor = other.m_textColor;

	m_nHAlignMent = other.m_nHAlignMent;

	m_nColorTextType = other.m_nColorTextType;

	m_pColorTextFillParam = other.m_pColorTextFillParam;
	m_pColorTextOutlineParam = other.m_pColorTextOutlineParam;
	m_pColorTextShadeParam = other.m_pColorTextShadeParam;
	m_pColorTextBackgroundParam = other.m_pColorTextBackgroundParam;


	return *this;
}


void CTextResource::SetHasBackGround(bool b) 
{ 
	m_bBcakGround = b; 
}

void CTextResource::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	CResourceBase::SaveDataToRapidJson(root, allocator);
	root.AddMember("nTextType", m_nShowType, allocator);
	root.AddMember("strContent", rapidjson::Value(m_strContent.toUtf8().constData(), allocator), allocator);
	root.AddMember("bBackGround", m_bBcakGround, allocator);
	root.AddMember("BGColorR", m_BGColor.red(), allocator);
	root.AddMember("BGColorG", m_BGColor.green(), allocator);
	root.AddMember("BGColorB", m_BGColor.blue(), allocator);
	root.AddMember("BGColorA", m_BGColor.alpha(), allocator);
	root.AddMember("textColorR", m_textColor.red(), allocator);
	root.AddMember("textColorG", m_textColor.green(), allocator);
	root.AddMember("textColorB", m_textColor.blue(), allocator);
	root.AddMember("textColorA", m_textColor.alpha(), allocator);
	root.AddMember("textFamily", rapidjson::Value(m_textFont.family().toUtf8().constData(), allocator), allocator);
	root.AddMember("textBold", m_textFont.bold(), allocator);
	root.AddMember("textUnderline", m_textFont.underline(), allocator);
	root.AddMember("textItalic", m_textFont.italic(), allocator);
	root.AddMember("HAlignMent", m_nHAlignMent, allocator);
	root.AddMember("ColorTextType", m_nColorTextType, allocator);

	rapidjson::Value fillObj(rapidjson::kObjectType);
	fillObj.AddMember("bFillEnable", m_pColorTextFillParam.bFillEnable, allocator);
	fillObj.AddMember("nFillType", m_pColorTextFillParam.nFillType, allocator);
	fillObj.AddMember("dOpacity", m_pColorTextFillParam.dOpacity, allocator);
	fillObj.AddMember("nGradientColorNum", m_pColorTextFillParam.nGradientColorNum, allocator);
	fillObj.AddMember("strTexturePath", rapidjson::Value(m_pColorTextFillParam.strTexturePath.toUtf8().constData(), allocator), allocator);
	fillObj.AddMember("nSelectedColorIndex", m_pColorTextFillParam.nSelectedColorIndex, allocator);
	fillObj.AddMember("FillColorR", m_pColorTextFillParam.FillColor.red(), allocator);
	fillObj.AddMember("FillColorG", m_pColorTextFillParam.FillColor.green(), allocator);
	fillObj.AddMember("FillColorB", m_pColorTextFillParam.FillColor.blue(), allocator);
	fillObj.AddMember("FillColorA", m_pColorTextFillParam.FillColor.alpha(), allocator);
	for (int i = 0; i < m_pColorTextFillParam.nGradientColorNum; ++i)
	{
		if (i < m_pColorTextFillParam.m_vecColorPosition.size())
			fillObj.AddMember(rapidjson::Value(QString("m_vecColorPosition_%1").arg(i).toUtf8().constData(), allocator), m_pColorTextFillParam.m_vecColorPosition[i], allocator);
		if (i < m_pColorTextFillParam.m_vecFillColor.size())
		{
			fillObj.AddMember(rapidjson::Value(QString("m_vecFillColorR_%1").arg(i).toUtf8().constData(), allocator), m_pColorTextFillParam.m_vecFillColor[i].red(), allocator);
			fillObj.AddMember(rapidjson::Value(QString("m_vecFillColorG_%1").arg(i).toUtf8().constData(), allocator), m_pColorTextFillParam.m_vecFillColor[i].green(), allocator);
			fillObj.AddMember(rapidjson::Value(QString("m_vecFillColorB_%1").arg(i).toUtf8().constData(), allocator), m_pColorTextFillParam.m_vecFillColor[i].blue(), allocator);
			fillObj.AddMember(rapidjson::Value(QString("m_vecFillColorA_%1").arg(i).toUtf8().constData(), allocator), m_pColorTextFillParam.m_vecFillColor[i].alpha(), allocator);
		}
	}
	fillObj.AddMember("nImportTexturePathNum", m_pColorTextFillParam.nImportTexturePathNum, allocator);
	for (int i = 0; i < m_pColorTextFillParam.nImportTexturePathNum; ++i)
	{
		if (i < m_pColorTextFillParam.m_vecImportTexturePath.size())
			fillObj.AddMember(rapidjson::Value(QString("vecImportTexturePath_%1").arg(i).toUtf8().constData(), allocator), rapidjson::Value(m_pColorTextFillParam.m_vecImportTexturePath[i].toUtf8().constData(), allocator), allocator);
	}
	fillObj.AddMember("nGradientDirection", m_pColorTextFillParam.nGradientDirection, allocator);
	fillObj.AddMember("nTextureFillType", m_pColorTextFillParam.nTextureFillType, allocator);
	fillObj.AddMember("nGradientScrollType", m_pColorTextFillParam.nGradientScrollType, allocator);
	fillObj.AddMember("nTextureScrollType", m_pColorTextFillParam.nTextureScrollType, allocator);
	fillObj.AddMember("nScrollSpeed", m_pColorTextFillParam.nScrollSpeed, allocator);
	root.AddMember("SColorTextFillParam", fillObj, allocator);

	rapidjson::Value outlineObj(rapidjson::kObjectType);
	outlineObj.AddMember("bOutlineEnable", m_pColorTextOutlineParam.bOutlineEnable, allocator);
	outlineObj.AddMember("dOpacity", m_pColorTextOutlineParam.dOpacity, allocator);
	outlineObj.AddMember("nOutlineThickness", m_pColorTextOutlineParam.nOutlineThickness, allocator);
	outlineObj.AddMember("OutlineColorR", m_pColorTextOutlineParam.OutlineColor.red(), allocator);
	outlineObj.AddMember("OutlineColorG", m_pColorTextOutlineParam.OutlineColor.green(), allocator);
	outlineObj.AddMember("OutlineColorB", m_pColorTextOutlineParam.OutlineColor.blue(), allocator);
	outlineObj.AddMember("OutlineColorA", m_pColorTextOutlineParam.OutlineColor.alpha(), allocator);
	root.AddMember("SColorTextOutlineParam", outlineObj, allocator);

	rapidjson::Value shadeObj(rapidjson::kObjectType);
	shadeObj.AddMember("bShadeEnable", m_pColorTextShadeParam.bShadeEnable, allocator);
	shadeObj.AddMember("dOpacity", m_pColorTextShadeParam.dAmbiguity, allocator);
	shadeObj.AddMember("nShadeAngle", m_pColorTextShadeParam.nShadeAngle, allocator);
	shadeObj.AddMember("nShadeDistance", m_pColorTextShadeParam.nShadeDistance, allocator);
	shadeObj.AddMember("ShadeColorR", m_pColorTextShadeParam.ShadeColor.red(), allocator);
	shadeObj.AddMember("ShadeColorG", m_pColorTextShadeParam.ShadeColor.green(), allocator);
	shadeObj.AddMember("ShadeColorB", m_pColorTextShadeParam.ShadeColor.blue(), allocator);
	shadeObj.AddMember("ShadeColorA", m_pColorTextShadeParam.ShadeColor.alpha(), allocator);
	root.AddMember("SColorTextShadeParam", shadeObj, allocator);

	rapidjson::Value bgObj(rapidjson::kObjectType);
	bgObj.AddMember("bBackgroundEnable", m_pColorTextBackgroundParam.bBackgroundEnable, allocator);
	bgObj.AddMember("nBackgroundType", m_pColorTextBackgroundParam.nBackgroundType, allocator);
	bgObj.AddMember("dOpacity", m_pColorTextBackgroundParam.dOpacity, allocator);
	bgObj.AddMember("dFillet", m_pColorTextBackgroundParam.dFillet, allocator);
	bgObj.AddMember("dWidth", m_pColorTextBackgroundParam.dWidth, allocator);
	bgObj.AddMember("dHeight", m_pColorTextBackgroundParam.dHeight, allocator);
	bgObj.AddMember("BackgroundColorR", m_pColorTextBackgroundParam.BackgroundColor.red(), allocator);
	bgObj.AddMember("BackgroundColorG", m_pColorTextBackgroundParam.BackgroundColor.green(), allocator);
	bgObj.AddMember("BackgroundColorB", m_pColorTextBackgroundParam.BackgroundColor.blue(), allocator);
	bgObj.AddMember("BackgroundColorA", m_pColorTextBackgroundParam.BackgroundColor.alpha(), allocator);
	root.AddMember("SColorTextBackgroundParam", bgObj, allocator);
}

bool CTextResource::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	CResourceBase::LoadDataFromRapidJson(root);
	if (!root.IsObject())
		return false;
	m_nShowType = root.HasMember("nTextType") ? root["nTextType"].GetInt() : 0;

	m_strContent = (root.HasMember("strContent") && root["strContent"].IsString()) ? QString::fromUtf8(root["strContent"].GetString()) : QString();


	{
		m_BGColor = QColor(
			root.HasMember("BGColorR") ? root["BGColorR"].GetInt() : 0,
			root.HasMember("BGColorG") ? root["BGColorG"].GetInt() : 0,
			root.HasMember("BGColorB") ? root["BGColorB"].GetInt() : 0,
			root.HasMember("BGColorA") ? root["BGColorA"].GetInt() : 255);
		m_textColor = QColor(
			root.HasMember("textColorR") ? root["textColorR"].GetInt() : 255,
			root.HasMember("textColorG") ? root["textColorG"].GetInt() : 255,
			root.HasMember("textColorB") ? root["textColorB"].GetInt() : 255,
			root.HasMember("textColorA") ? root["textColorA"].GetInt() : 255);
		SetHasBackGround(root.HasMember("bBackGround") ? root["bBackGround"].GetBool() : false);//颜色的读取要在这之前

		// 字体设置
		m_textFont.setFamily((root.HasMember("textFamily") && root["textFamily"].IsString()) ? QString::fromUtf8(root["textFamily"].GetString()) : QString());
		m_textFont.setBold(root.HasMember("textBold") ? root["textBold"].GetBool() : false);
		m_textFont.setUnderline(root.HasMember("textUnderline") ? root["textUnderline"].GetBool() : false);
		m_textFont.setItalic(root.HasMember("textItalic") ? root["textItalic"].GetBool() : false);

		m_nHAlignMent = root.HasMember("HAlignMent") ? root["HAlignMent"].GetInt() : Qt::AlignLeft;

		m_nColorTextType = root.HasMember("ColorTextType") ? root["ColorTextType"].GetInt() : EDCPT_Forbidden;

		if (root.HasMember("SColorTextFillParam") && root["SColorTextFillParam"].IsObject())
		{
			const rapidjson::Value& param = root["SColorTextFillParam"];
			m_pColorTextFillParam.bFillEnable = param.HasMember("bFillEnable") ? param["bFillEnable"].GetBool() : false;
			m_pColorTextFillParam.nFillType = param.HasMember("nFillType") ? param["nFillType"].GetInt() : 0;
			m_pColorTextFillParam.dOpacity = param.HasMember("dOpacity") ? param["dOpacity"].GetDouble() : 1.0;
			m_pColorTextFillParam.nGradientColorNum = param.HasMember("nGradientColorNum") ? param["nGradientColorNum"].GetInt() : 0;
			m_pColorTextFillParam.strTexturePath = (param.HasMember("strTexturePath") && param["strTexturePath"].IsString()) ? QString::fromUtf8(param["strTexturePath"].GetString()) : QString();
			m_pColorTextFillParam.nSelectedColorIndex = param.HasMember("nSelectedColorIndex") ? param["nSelectedColorIndex"].GetInt() : -1;
			m_pColorTextFillParam.FillColor = QColor(
				param.HasMember("FillColorR") ? param["FillColorR"].GetInt() : 0,
				param.HasMember("FillColorG") ? param["FillColorG"].GetInt() : 0,
				param.HasMember("FillColorB") ? param["FillColorB"].GetInt() : 0,
				param.HasMember("FillColorA") ? param["FillColorA"].GetInt() : 255);

			m_pColorTextFillParam.m_vecColorPosition.resize(m_pColorTextFillParam.nGradientColorNum);
			m_pColorTextFillParam.m_vecFillColor.resize(m_pColorTextFillParam.nGradientColorNum);
			for (int i = 0; i < m_pColorTextFillParam.nGradientColorNum; ++i)
			{
				const QByteArray posKey = QString("m_vecColorPosition_%1").arg(i).toUtf8();
				const QByteArray colorR = QString("m_vecFillColorR_%1").arg(i).toUtf8();
				const QByteArray colorG = QString("m_vecFillColorG_%1").arg(i).toUtf8();
				const QByteArray colorB = QString("m_vecFillColorB_%1").arg(i).toUtf8();
				const QByteArray colorA = QString("m_vecFillColorA_%1").arg(i).toUtf8();
				m_pColorTextFillParam.m_vecColorPosition[i] = param.HasMember(posKey.constData()) ? param[posKey.constData()].GetDouble() : 0.0;
				m_pColorTextFillParam.m_vecFillColor[i] = QColor(
					param.HasMember(colorR.constData()) ? param[colorR.constData()].GetInt() : 0,
					param.HasMember(colorG.constData()) ? param[colorG.constData()].GetInt() : 0,
					param.HasMember(colorB.constData()) ? param[colorB.constData()].GetInt() : 0,
					param.HasMember(colorA.constData()) ? param[colorA.constData()].GetInt() : 255);
			}

			m_pColorTextFillParam.nImportTexturePathNum = param.HasMember("nImportTexturePathNum") ? param["nImportTexturePathNum"].GetInt() : 0;
			m_pColorTextFillParam.m_vecImportTexturePath.resize(m_pColorTextFillParam.nImportTexturePathNum);
			for (int i = 0; i < m_pColorTextFillParam.nImportTexturePathNum; ++i)
			{
				const QByteArray pathKey = QString("vecImportTexturePath_%1").arg(i).toUtf8();
				m_pColorTextFillParam.m_vecImportTexturePath[i] = (param.HasMember(pathKey.constData()) && param[pathKey.constData()].IsString()) ? QString::fromUtf8(param[pathKey.constData()].GetString()) : QString();
			}
			m_pColorTextFillParam.nGradientDirection = param.HasMember("nGradientDirection") ? param["nGradientDirection"].GetInt() : 0;
			m_pColorTextFillParam.nTextureFillType = param.HasMember("nTextureFillType") ? param["nTextureFillType"].GetInt() : 0;
			m_pColorTextFillParam.nGradientScrollType = param.HasMember("nGradientScrollType") ? param["nGradientScrollType"].GetInt() : 0;
			m_pColorTextFillParam.nTextureScrollType = param.HasMember("nTextureScrollType") ? param["nTextureScrollType"].GetInt() : 0;
			m_pColorTextFillParam.nScrollSpeed = param.HasMember("nScrollSpeed") ? param["nScrollSpeed"].GetInt() : 1;
		}

		if (root.HasMember("SColorTextOutlineParam") && root["SColorTextOutlineParam"].IsObject())
		{
			const rapidjson::Value& param = root["SColorTextOutlineParam"];
			m_pColorTextOutlineParam.bOutlineEnable = param.HasMember("bOutlineEnable") ? param["bOutlineEnable"].GetBool() : false;
			m_pColorTextOutlineParam.dOpacity = param.HasMember("dOpacity") ? param["dOpacity"].GetDouble() : 1.0;
			m_pColorTextOutlineParam.nOutlineThickness = param.HasMember("nOutlineThickness") ? param["nOutlineThickness"].GetInt() : 5;
			m_pColorTextOutlineParam.OutlineColor = QColor(
				param.HasMember("OutlineColorR") ? param["OutlineColorR"].GetInt() : 0,
				param.HasMember("OutlineColorG") ? param["OutlineColorG"].GetInt() : 0,
				param.HasMember("OutlineColorB") ? param["OutlineColorB"].GetInt() : 0,
				param.HasMember("OutlineColorA") ? param["OutlineColorA"].GetInt() : 255);
		}

		if (root.HasMember("SColorTextShadeParam") && root["SColorTextShadeParam"].IsObject())
		{
			const rapidjson::Value& param = root["SColorTextShadeParam"];
			m_pColorTextShadeParam.bShadeEnable = param.HasMember("bShadeEnable") ? param["bShadeEnable"].GetBool() : false;
			m_pColorTextShadeParam.dAmbiguity = param.HasMember("dOpacity") ? param["dOpacity"].GetDouble() : 0.74;
			m_pColorTextShadeParam.nShadeAngle = param.HasMember("nShadeAngle") ? param["nShadeAngle"].GetInt() : 45;
			m_pColorTextShadeParam.nShadeDistance = param.HasMember("nShadeDistance") ? param["nShadeDistance"].GetInt() : 35;
			m_pColorTextShadeParam.ShadeColor = QColor(
				param.HasMember("ShadeColorR") ? param["ShadeColorR"].GetInt() : 0,
				param.HasMember("ShadeColorG") ? param["ShadeColorG"].GetInt() : 0,
				param.HasMember("ShadeColorB") ? param["ShadeColorB"].GetInt() : 0,
				param.HasMember("ShadeColorA") ? param["ShadeColorA"].GetInt() : 255);
		}

		if (root.HasMember("SColorTextBackgroundParam") && root["SColorTextBackgroundParam"].IsObject())
		{
			const rapidjson::Value& param = root["SColorTextBackgroundParam"];
			m_pColorTextBackgroundParam.bBackgroundEnable = param.HasMember("bBackgroundEnable") ? param["bBackgroundEnable"].GetBool() : false;
			m_pColorTextBackgroundParam.nBackgroundType = param.HasMember("nBackgroundType") ? param["nBackgroundType"].GetInt() : 0;
			m_pColorTextBackgroundParam.dOpacity = param.HasMember("dOpacity") ? param["dOpacity"].GetDouble() : 1.0;
			m_pColorTextBackgroundParam.dFillet = param.HasMember("dFillet") ? param["dFillet"].GetDouble() : 0.0;
			m_pColorTextBackgroundParam.dWidth = param.HasMember("dWidth") ? param["dWidth"].GetDouble() : 0.0;
			m_pColorTextBackgroundParam.dHeight = param.HasMember("dHeight") ? param["dHeight"].GetDouble() : 0.0;
			m_pColorTextBackgroundParam.BackgroundColor = QColor(
				param.HasMember("BackgroundColorR") ? param["BackgroundColorR"].GetInt() : 255,
				param.HasMember("BackgroundColorG") ? param["BackgroundColorG"].GetInt() : 190,
				param.HasMember("BackgroundColorB") ? param["BackgroundColorB"].GetInt() : 47,
				param.HasMember("BackgroundColorA") ? param["BackgroundColorA"].GetInt() : 255);
		}
	}

	return true;
}

void CTextResource::SetShowType(int nType)
{
	m_nShowType = nType;
}

int CTextResource::GetBackgroundType()
{
	if (m_nShowType == ETRT_TegularText)
		return 0;
	else
		return m_pColorTextBackgroundParam.nBackgroundType;
}

bool CTextResource::IsAnimationText()
{ 
	if (m_nShowType == ETRT_TegularText)
		return false;

	if (!m_pColorTextFillParam.bFillEnable)
		return false;

	if (m_pColorTextFillParam.nFillType == en_Fill_Gradient)
	{
		switch (m_pColorTextFillParam.nGradientScrollType)
		{
		case en_Fill_Static:
		{
			return false;
		}
		case en_Fill_MoveLeft:
		case en_Fill_MoveRight:
		case en_Fill_OpenLeft2Right:
		case en_Fill_CloseLeft2Right:
		{
			return m_pColorTextFillParam.nGradientDirection == EGD_Horizontal; //填充方向水平
		}
		case en_Fill_MoveUp:
		case en_Fill_MoveDown:
		case en_Fill_OpenUp2Down:
		case en_Fill_CloseUp2Down:
		{
			return m_pColorTextFillParam.nGradientDirection == EGD_Vertical; //填充方向垂直
		}
		case en_Fill_Clockwise:
		case en_Fill_Counterclockwise:
		{
			return true;
		}
		default:
			break;
		}
		return false;
	}
	else if (m_pColorTextFillParam.nFillType == en_Fill_SolidColor)
		return false;
	else
		return m_pColorTextFillParam.nTextureScrollType != en_Fill_Static; //填充的滚动方式不是静止
}

//幻彩字-填充
inline void		CTextResource::SetCTFParamEnable(bool bEnable) { m_pColorTextFillParam.bFillEnable = bEnable; }
inline void		CTextResource::SetCTFParamFillType(int nType) { m_pColorTextFillParam.nFillType = nType; }
inline void		CTextResource::SetCTFParamFillColor(QColor Color) { m_pColorTextFillParam.FillColor = Color; }

void		CTextResource::SetCTFParamOpacity(double dOpacity) 
{
	m_pColorTextFillParam.dOpacity = dOpacity;
	if (m_pColorTextFillParam.nFillType == en_Fill_SolidColor)
		m_pColorTextFillParam.FillColor.setAlpha(m_pColorTextFillParam.dOpacity * 255);
	else if (m_pColorTextFillParam.nFillType == en_Fill_Gradient)
	{
		for (int i = 0; i < m_pColorTextFillParam.nGradientColorNum; i++)
			m_pColorTextFillParam.m_vecFillColor[i].setAlpha(m_pColorTextFillParam.dOpacity * 255);
	}
}

inline void		CTextResource::SetCTFParamGradientColorNum(int nValue) { m_pColorTextFillParam.nGradientColorNum += nValue; }

inline void		CTextResource::SetCTFParamGradientColor(QColor color, int nValue) 
{ 
	m_pColorTextFillParam.nSelectedColorIndex = nValue;
	if (nValue == -1)
		return;
	m_pColorTextFillParam.m_vecFillColor[nValue] = color;
}
inline void		CTextResource::SetCTFParamGradientPosition(int nValue, double dPosition) { m_pColorTextFillParam.m_vecColorPosition[nValue] = dPosition; }

inline void		CTextResource::SetCTFParamFillTexturechange(QString strPath) { m_pColorTextFillParam.strTexturePath = strPath; }
inline void		CTextResource::SetCTFParamImportTexturechange(QVector<QString> &vecStr) 
{
	m_pColorTextFillParam.m_vecImportTexturePath = vecStr;
	m_pColorTextFillParam.nImportTexturePathNum = vecStr.size();
}

inline void		CTextResource::SetCTFParamGradientDirection(int nType) { m_pColorTextFillParam.nGradientDirection = nType; }
inline void 	CTextResource::SetCTFParamTextureFillType(int nType) { m_pColorTextFillParam.nTextureFillType = nType; }
inline void 	CTextResource::SetCTFParamGradientScrollType(int nValue) { m_pColorTextFillParam.nGradientScrollType = nValue; }
inline void 	CTextResource::SetCTFParamTextureScrollType(int nValue) { m_pColorTextFillParam.nTextureScrollType = nValue; }
inline void		CTextResource::SetCTFParamScrollSpeed(int nValue) { m_pColorTextFillParam.nScrollSpeed = nValue; }

//幻彩字-描边
inline void		CTextResource::SetCTOParamEnable(bool bEnable) { m_pColorTextOutlineParam.bOutlineEnable = bEnable; }
inline void		CTextResource::SetCTOParamOutlineColor(QColor Color) { m_pColorTextOutlineParam.OutlineColor = Color; }
inline void		CTextResource::SetCTOParamThickness(int nValue) { m_pColorTextOutlineParam.nOutlineThickness = nValue; }
inline void		CTextResource::SetCTOParamOpacity(double dValue) {
	m_pColorTextOutlineParam.dOpacity = dValue;
	m_pColorTextOutlineParam.OutlineColor.setAlpha(m_pColorTextOutlineParam.dOpacity * 255);
}

//幻彩字-阴影
inline void	CTextResource::SetCTSParamEnable(bool bEnable){ m_pColorTextShadeParam.bShadeEnable = bEnable; }
inline void	CTextResource::SetCTSParamShadeColor(QColor Color){ m_pColorTextShadeParam.ShadeColor = Color; }
inline void	CTextResource::SetCTSParamAngle(int nValue) { m_pColorTextShadeParam.nShadeAngle = nValue; }
inline void	CTextResource::SetCTSParamDistance(int nValue) { m_pColorTextShadeParam.nShadeDistance = nValue; }
inline void	CTextResource::SetCTSParamAmbiguity(double dValue) { m_pColorTextShadeParam.dAmbiguity = dValue; }

//幻彩字-背景
inline void		CTextResource::SetCTBParamEnable(bool bEnable) { m_pColorTextBackgroundParam.bBackgroundEnable = bEnable; }
inline void		CTextResource::SetCTSParamBackgroundType(int bType) { m_pColorTextBackgroundParam.nBackgroundType = bType; }
inline void		CTextResource::SetCTBParamBackgroundColor(QColor Color) { m_pColorTextBackgroundParam.BackgroundColor = Color; }

inline void		CTextResource::SetCTBParamOpacity(double dValue) 
{
	m_pColorTextBackgroundParam.dOpacity = dValue;
	m_pColorTextBackgroundParam.BackgroundColor.setAlpha(m_pColorTextBackgroundParam.dOpacity * 255);
}
inline void		CTextResource::SetCTBParamFillet(double dValue) {m_pColorTextBackgroundParam.dFillet = dValue;}
inline void		CTextResource::SetCTBParamWidth(double dValue) {m_pColorTextBackgroundParam.dWidth = dValue;}
inline void		CTextResource::SetCTBParamHeight(double dValue) {m_pColorTextBackgroundParam.dHeight = dValue;}