#pragma once
#include "Common/CommonDefine.h"
#include "Common/CommonFunctionApi.h"
#include "Common/CommonAboutCustomization.h"

#include <QUuid>
#include <QFont>
#include <QColor>
#include <QString>
#include <QObject>
#include <QDomNode>
#include <rapidjson/document.h>
static const QString strVideoFormat = "*.mp4 *.m4v *.mpeg *.mov *.flv *.avi *.mkv";
//static const QString strVideoFormat = "*.mp4 *.3gp *.m2t *.m2ts *.bhd *.asx *.mpe *.vcd *.dvd *.dat *.m4v *.webm *.rmvb *.rm *.wmv *.asf *.mpeg *.mpg *.ts *.tp *.mov *.flv *.avi *.mkv *.vob";
//static const QString strPictureFormat = "*.bmp *.jpg *.tiff *.tga *.webp *.jpeg *.gif";
//static const QString strVideoFormat = "*.mp4";
static const QString strPictureFormat = "*.jpg *.png *.webp";

#define Xml_ResourceModel_ResourcesCounter "ResourcesCounter"
#define Xml_ResourceModel_Number "Number"
#define Xml_ResourceModel_ResourcesSequence "ResourcesSequence"
#define Xml_ResourceModel_ResourcesData "ResourcesData"
#define Xml_ResourceModel_ResourceType "ResourceType"
#define Xml_ResourceModel_ResourceID "ResourceID"
#define Xml_ResourceModel_ResourceName "ResourceName"
#define Xml_ResourceModel_ResourceFullName "ResourceFullName"
#define Xml_ResourceModel_ResourceUsedCount "ResourceUsedCount"
#define Xml_ResourceModel_ResourcePath "ResourcePath"
#define Xml_ResourceModel_ResourceFormat "ResourceFormat"
#define Xml_ResourceModel_TestChartData "TestChartData"

//通过素材路径获取素材类型
static EResourceType GetFileResType(const QString &strFilePath)
{
	int nIndex = strFilePath.lastIndexOf('.');
	QString szSuffix = strFilePath.right(strFilePath.length() - nIndex - 1);
	szSuffix = szSuffix.toLower();

	if (strVideoFormat.contains(szSuffix, Qt::CaseSensitive))
		return ERT_Video;

	if (strPictureFormat.contains(szSuffix, Qt::CaseSensitive))
		return ERT_Picture;

	return ERT_Invalid;
}

// 判断是否是文件素材
static bool IsFileResource(int nType)
{
	switch (nType)
	{
	case ERT_Video:
	case ERT_Picture:
		return true;
	default:
		break;
	}
	return false;
}

// 素材是否支持声音
static bool IsSupportVolum(int nType, bool bPicGroup)
{
	switch (nType)
	{
	case ERT_Video:
		return true;
	default:
		break;
	}
	return false;
}

// 是否支持填充模式
static bool IsSupportFillMode(int nType)
{
	switch (nType)
	{
	case ERT_TestChart:
		return false;
	default:
		break;
	}
	return true;
}

// 是否有原始时长
static bool IsAudioVideo(int nType)
{
	switch (nType)
	{
	case ERT_Video:
		return true;
	default:
		return false;
	}
	return false;
}


//创建测试图数据数据结构
//基本信息
struct SBaseInfo
{
	int   m_iWidth = 1920;		//高度
	int   m_iHeight = 1080;	//高度
	int   m_iRow = 9;		//行
	int   m_iCol = 16;		//列
};

//颜色模式
enum EColorMode
{
	PURECOLOR = 0,     //纯色模式
	GRADIENT,          //渐变模式
	CHESSBOARD,        //棋盘模式
	TESTCARD,          //测试卡模式
};

//ORDER模式
enum EOrderMode
{
	LETTER = 0,      //字母模式
	NUMBER           //数字
};

//LOGO模式
enum ELogoMode
{
	GRADSHOW = 0,     //gradshow模式
	IDENTIFIER,       //编号模式
	LOGO,             //logo模式
};

//模式选择
struct SColorModeSelect
{
	EColorMode  eColorMode; //当前颜色模式
	//---------------------颜色模式数据---------------------------------
	QColor m_pureColor;//纯色模式
	QColor m_gradientColor1;	//渐变模式
	QColor m_gradientColor2;
	QColor m_chessColor1;      //棋盘模式
	QColor m_chessColor2;
	SColorModeSelect() :
		eColorMode(PURECOLOR),
		m_pureColor(QColor(218, 158, 48)),
		m_gradientColor1(QColor(0, 242, 96)),
		m_gradientColor2(QColor(0, 125, 241)),
		m_chessColor1(QColor(76, 255, 229)),
		m_chessColor2(QColor(86, 108, 234))
	{

	}
};

//序号
struct SOrder
{
	bool   m_bSelected = false;                     //是否开启序号模式
	QColor m_color = QColor(0, 0, 0);			    //序号模式颜色
	int    m_iMode = EOrderMode::LETTER;            //默认字母模式
	int    m_iFontSize = 22;                        //序号字体大小
};

//分割线
struct SSplineLine
{
	bool   m_bSelected = false;								//是否开启分割线模式
	QColor m_color = QColor(240, 240, 240);		            //分割线模式颜色
	int    m_iWidth = 1;					                //分割线宽度
};

//边框
struct SCheek
{
	bool   m_bSelected = true;                          //是否开启边框模式
	QColor m_color = QColor(240, 240, 240);			    //边框模式颜色
	int    m_iWidth = 8;					            //边框宽度
};

//圆
struct SCircle
{
	bool   m_bSelected = true;                               //是否开启圆模式
	QColor m_color = QColor(240, 240, 240);		             //圆模式颜色
	int    m_iWidth = 10;					                 //圆宽度
	int    m_iRadius = 450;					                 //圆半径
};

//对角线
struct SDiagonal
{
	bool   m_bSelected = true;                      //是否开启对角线模式
	QColor m_color = QColor(240, 240, 240);		    //对角线模式颜色
	int    m_iWidth = 10;					        //对角线宽度
};

//分辨率
struct SResolution
{
	bool   m_bSelected = true;                           //是否开启分辨率模式
	QColor m_color = QColor(0, 0, 0);					 //分辨率模式颜色
	int    m_iFontSize = 43;					         //分辨率字体大小
};

//箱体个数
struct SBoxNum
{
	bool   m_bSelected = false;                           //是否开启箱体个数
	QColor m_color = QColor(0, 0, 0);					  //箱体个数模式颜色
	int    m_iFontSize = 43;							  //箱体个数字体大小
};


//logo设置信息
struct SLogo
{
	bool    m_bSelected = true;                          //是否设置logo
	int     m_iMode = ELogoMode::GRADSHOW;				//logo模式
	int     m_iZoom = 100;							    //缩放
	int     m_iTransparency = 100;                      //透明度
	int     m_iX = 960;                                //X坐标
	int     m_iY = 421;                                //Y坐标
	QString m_strGrandShowPath = ":/dark/images/ResourceWnd/TestChart/logo.png";                   //默认 logo 路径
	QString m_strLogPath = "";                         //logo路径
	QString m_strIdentifier = "A";                     //编号
	QColor  m_color = QColor(0, 0, 0);		           //箱体个数模式颜色
	int     m_iFontSize = 100;                         //字号
};

//文字信息
struct STextInfo
{
	bool   m_bSelected = true;                    //是否设置文字
	QString m_strText = COMPANY_WEBSITE;		  //文字信息
	int     m_iFontSize = 60;                     //字号
	QColor  m_color = QColor(0, 0, 0);		      //箱体个数模式颜色
};

//其余颜色模式
enum ERestMode
{
	ORDER = 0,  //序号
	SPLITLINE,  //分割线
	CHEEK,      //边框
	CIRCLE,     //圆
	DIAGONAL,   //对角线
	RESOLUTIONINFO, //分辨率
	BOXNUM,     //箱体个数
	LOGOINFO,   //LOGO信息
	TEXTINFO,   //文字信息
};

typedef struct STestChartData
{
	SBaseInfo  baseInfo;                //基本信息 
	SColorModeSelect  colorModeSelect;  //颜色模式 
	SOrder  order;                      //序号 
	SSplineLine   spline;				//分隔线
	SCheek  cheek;                      //边框 
	SCircle circle;                     //圆 
	SDiagonal diagonal;                 //对角线
	SResolution  resolution;			//分辨率
	SBoxNum    boxNum;                  //箱体数
	SLogo      LogoInfo;                //logo信息
	STextInfo   textInfo;               //文字信息
	STestChartData()
	{

	}

	void STestChartData::SaveDataToRapidJson(rapidjson::Value& rapidRoot, rapidjson::Document::AllocatorType& allocator) const
	{
		rapidRoot.SetObject();
		rapidjson::Value baseInfoObj(rapidjson::kObjectType);
		baseInfoObj.AddMember("ContentW", baseInfo.m_iWidth, allocator);
		baseInfoObj.AddMember("ContentH", baseInfo.m_iHeight, allocator);
		baseInfoObj.AddMember("ContentRow", baseInfo.m_iRow, allocator);
		baseInfoObj.AddMember("ContentCol", baseInfo.m_iCol, allocator);
		rapidRoot.AddMember("BaseInfo", baseInfoObj, allocator);

		rapidjson::Value colorModeObj(rapidjson::kObjectType);
		colorModeObj.AddMember("ColorMode", static_cast<int>(colorModeSelect.eColorMode), allocator);
		colorModeObj.AddMember("PureColor", rapidjson::Value(ColorToString(colorModeSelect.m_pureColor).toUtf8().constData(), allocator), allocator);
		colorModeObj.AddMember("GradientColor1", rapidjson::Value(ColorToString(colorModeSelect.m_gradientColor1).toUtf8().constData(), allocator), allocator);
		colorModeObj.AddMember("GradientColor2", rapidjson::Value(ColorToString(colorModeSelect.m_gradientColor2).toUtf8().constData(), allocator), allocator);
		colorModeObj.AddMember("ChessColor1", rapidjson::Value(ColorToString(colorModeSelect.m_chessColor1).toUtf8().constData(), allocator), allocator);
		colorModeObj.AddMember("ChessColor2", rapidjson::Value(ColorToString(colorModeSelect.m_chessColor2).toUtf8().constData(), allocator), allocator);
		rapidRoot.AddMember("ColorModeSelect", colorModeObj, allocator);

		rapidjson::Value orderObj(rapidjson::kObjectType);
		orderObj.AddMember("bSelected", order.m_bSelected, allocator);
		orderObj.AddMember("Color", rapidjson::Value(ColorToString(order.m_color).toUtf8().constData(), allocator), allocator);
		orderObj.AddMember("iMode", order.m_iMode, allocator);
		orderObj.AddMember("iFontSize", order.m_iFontSize, allocator);
		rapidRoot.AddMember("Order", orderObj, allocator);

		rapidjson::Value splineObj(rapidjson::kObjectType);
		splineObj.AddMember("bSelected", spline.m_bSelected, allocator);
		splineObj.AddMember("Color", rapidjson::Value(ColorToString(spline.m_color).toUtf8().constData(), allocator), allocator);
		splineObj.AddMember("iWidth", spline.m_iWidth, allocator);
		rapidRoot.AddMember("SplineLine", splineObj, allocator);

		rapidjson::Value cheekObj(rapidjson::kObjectType);
		cheekObj.AddMember("bSelected", cheek.m_bSelected, allocator);
		cheekObj.AddMember("Color", rapidjson::Value(ColorToString(cheek.m_color).toUtf8().constData(), allocator), allocator);
		cheekObj.AddMember("iWidth", cheek.m_iWidth, allocator);
		rapidRoot.AddMember("Cheek", cheekObj, allocator);

		rapidjson::Value circleObj(rapidjson::kObjectType);
		circleObj.AddMember("bSelected", circle.m_bSelected, allocator);
		circleObj.AddMember("Color", rapidjson::Value(ColorToString(circle.m_color).toUtf8().constData(), allocator), allocator);
		circleObj.AddMember("iWidth", circle.m_iWidth, allocator);
		circleObj.AddMember("iRadius", circle.m_iRadius, allocator);
		rapidRoot.AddMember("Circle", circleObj, allocator);

		rapidjson::Value diagonalObj(rapidjson::kObjectType);
		diagonalObj.AddMember("bSelected", diagonal.m_bSelected, allocator);
		diagonalObj.AddMember("Color", rapidjson::Value(ColorToString(diagonal.m_color).toUtf8().constData(), allocator), allocator);
		diagonalObj.AddMember("iWidth", diagonal.m_iWidth, allocator);
		rapidRoot.AddMember("Diagonal", diagonalObj, allocator);

		rapidjson::Value resolutionObj(rapidjson::kObjectType);
		resolutionObj.AddMember("bSelected", resolution.m_bSelected, allocator);
		resolutionObj.AddMember("Color", rapidjson::Value(ColorToString(resolution.m_color).toUtf8().constData(), allocator), allocator);
		resolutionObj.AddMember("iFontSize", resolution.m_iFontSize, allocator);
		rapidRoot.AddMember("Resolution", resolutionObj, allocator);

		rapidjson::Value boxNumObj(rapidjson::kObjectType);
		boxNumObj.AddMember("bSelected", boxNum.m_bSelected, allocator);
		boxNumObj.AddMember("Color", rapidjson::Value(ColorToString(boxNum.m_color).toUtf8().constData(), allocator), allocator);
		boxNumObj.AddMember("iFontSize", boxNum.m_iFontSize, allocator);
		rapidRoot.AddMember("BoxNum", boxNumObj, allocator);

		rapidjson::Value logoObj(rapidjson::kObjectType);
		logoObj.AddMember("bSelected", LogoInfo.m_bSelected, allocator);
		logoObj.AddMember("iMode", LogoInfo.m_iMode, allocator);
		logoObj.AddMember("iZoom", LogoInfo.m_iZoom, allocator);
		logoObj.AddMember("iTransparency", LogoInfo.m_iTransparency, allocator);
		logoObj.AddMember("iX", LogoInfo.m_iX, allocator);
		logoObj.AddMember("iY", LogoInfo.m_iY, allocator);
		logoObj.AddMember("strGrandShowPath", rapidjson::Value(LogoInfo.m_strGrandShowPath.toUtf8().constData(), allocator), allocator);
		logoObj.AddMember("strLogPath", rapidjson::Value(LogoInfo.m_strLogPath.toUtf8().constData(), allocator), allocator);
		logoObj.AddMember("strIdentifier", rapidjson::Value(LogoInfo.m_strIdentifier.toUtf8().constData(), allocator), allocator);
		logoObj.AddMember("Color", rapidjson::Value(ColorToString(LogoInfo.m_color).toUtf8().constData(), allocator), allocator);
		logoObj.AddMember("iFontSize", LogoInfo.m_iFontSize, allocator);
		rapidRoot.AddMember("Logo", logoObj, allocator);

		rapidjson::Value textObj(rapidjson::kObjectType);
		textObj.AddMember("bSelected", textInfo.m_bSelected, allocator);
		textObj.AddMember("strText", rapidjson::Value(textInfo.m_strText.toUtf8().constData(), allocator), allocator);
		textObj.AddMember("Color", rapidjson::Value(ColorToString(textInfo.m_color).toUtf8().constData(), allocator), allocator);
		textObj.AddMember("iFontSize", textInfo.m_iFontSize, allocator);
		rapidRoot.AddMember("TextInfo", textObj, allocator);
	}
	
	void STestChartData::LoadDataFromRapidJson(const rapidjson::Value& rapidRoot)
	{
		if (!rapidRoot.IsObject())
			return;

		if (rapidRoot.HasMember("BaseInfo") && rapidRoot["BaseInfo"].IsObject()) {
			const rapidjson::Value& baseInfoObj = rapidRoot["BaseInfo"];
			baseInfo.m_iWidth = baseInfoObj.HasMember("ContentW") ? baseInfoObj["ContentW"].GetInt() : 0;
			baseInfo.m_iHeight = baseInfoObj.HasMember("ContentH") ? baseInfoObj["ContentH"].GetInt() : 0;
			baseInfo.m_iRow = baseInfoObj.HasMember("ContentRow") ? baseInfoObj["ContentRow"].GetInt() : 0;
			baseInfo.m_iCol = baseInfoObj.HasMember("ContentCol") ? baseInfoObj["ContentCol"].GetInt() : 0;
		}

		if (rapidRoot.HasMember("ColorModeSelect") && rapidRoot["ColorModeSelect"].IsObject()) {
			const rapidjson::Value& cmObj = rapidRoot["ColorModeSelect"];
			colorModeSelect.eColorMode = static_cast<EColorMode>(cmObj.HasMember("ColorMode") ? cmObj["ColorMode"].GetInt() : 0);
			colorModeSelect.m_pureColor = StringToColor(cmObj.HasMember("PureColor") && cmObj["PureColor"].IsString() ? QString::fromUtf8(cmObj["PureColor"].GetString()) : QString());
			colorModeSelect.m_gradientColor1 = StringToColor(cmObj.HasMember("GradientColor1") && cmObj["GradientColor1"].IsString() ? QString::fromUtf8(cmObj["GradientColor1"].GetString()) : QString());
			colorModeSelect.m_gradientColor2 = StringToColor(cmObj.HasMember("GradientColor2") && cmObj["GradientColor2"].IsString() ? QString::fromUtf8(cmObj["GradientColor2"].GetString()) : QString());
			colorModeSelect.m_chessColor1 = StringToColor(cmObj.HasMember("ChessColor1") && cmObj["ChessColor1"].IsString() ? QString::fromUtf8(cmObj["ChessColor1"].GetString()) : QString());
			colorModeSelect.m_chessColor2 = StringToColor(cmObj.HasMember("ChessColor2") && cmObj["ChessColor2"].IsString() ? QString::fromUtf8(cmObj["ChessColor2"].GetString()) : QString());
		}

		if (rapidRoot.HasMember("Order") && rapidRoot["Order"].IsObject()) {
			const rapidjson::Value& obj = rapidRoot["Order"];
			order.m_bSelected = obj.HasMember("bSelected") ? obj["bSelected"].GetBool() : false;
			order.m_color = StringToColor(obj.HasMember("Color") && obj["Color"].IsString() ? QString::fromUtf8(obj["Color"].GetString()) : QString());
			order.m_iMode = obj.HasMember("iMode") ? obj["iMode"].GetInt() : 0;
			order.m_iFontSize = obj.HasMember("iFontSize") ? obj["iFontSize"].GetInt() : 12;
		}

		if (rapidRoot.HasMember("SplineLine") && rapidRoot["SplineLine"].IsObject()) {
			const rapidjson::Value& obj = rapidRoot["SplineLine"];
			spline.m_bSelected = obj.HasMember("bSelected") ? obj["bSelected"].GetBool() : false;
			spline.m_color = StringToColor(obj.HasMember("Color") && obj["Color"].IsString() ? QString::fromUtf8(obj["Color"].GetString()) : QString());
			spline.m_iWidth = obj.HasMember("iWidth") ? obj["iWidth"].GetInt() : 1;
		}

		if (rapidRoot.HasMember("Cheek") && rapidRoot["Cheek"].IsObject()) {
			const rapidjson::Value& obj = rapidRoot["Cheek"];
			cheek.m_bSelected = obj.HasMember("bSelected") ? obj["bSelected"].GetBool() : false;
			cheek.m_color = StringToColor(obj.HasMember("Color") && obj["Color"].IsString() ? QString::fromUtf8(obj["Color"].GetString()) : QString());
			cheek.m_iWidth = obj.HasMember("iWidth") ? obj["iWidth"].GetInt() : 1;
		}

		if (rapidRoot.HasMember("Circle") && rapidRoot["Circle"].IsObject()) {
			const rapidjson::Value& obj = rapidRoot["Circle"];
			circle.m_bSelected = obj.HasMember("bSelected") ? obj["bSelected"].GetBool() : false;
			circle.m_color = StringToColor(obj.HasMember("Color") && obj["Color"].IsString() ? QString::fromUtf8(obj["Color"].GetString()) : QString());
			circle.m_iWidth = obj.HasMember("iWidth") ? obj["iWidth"].GetInt() : 1;
			circle.m_iRadius = obj.HasMember("iRadius") ? obj["iRadius"].GetInt() : 10;
		}

		if (rapidRoot.HasMember("Diagonal") && rapidRoot["Diagonal"].IsObject()) {
			const rapidjson::Value& obj = rapidRoot["Diagonal"];
			diagonal.m_bSelected = obj.HasMember("bSelected") ? obj["bSelected"].GetBool() : false;
			diagonal.m_color = StringToColor(obj.HasMember("Color") && obj["Color"].IsString() ? QString::fromUtf8(obj["Color"].GetString()) : QString());
			diagonal.m_iWidth = obj.HasMember("iWidth") ? obj["iWidth"].GetInt() : 1;
		}

		if (rapidRoot.HasMember("Resolution") && rapidRoot["Resolution"].IsObject()) {
			const rapidjson::Value& obj = rapidRoot["Resolution"];
			resolution.m_bSelected = obj.HasMember("bSelected") ? obj["bSelected"].GetBool() : false;
			resolution.m_color = StringToColor(obj.HasMember("Color") && obj["Color"].IsString() ? QString::fromUtf8(obj["Color"].GetString()) : QString());
			resolution.m_iFontSize = obj.HasMember("iFontSize") ? obj["iFontSize"].GetInt() : 12;
		}

		if (rapidRoot.HasMember("BoxNum") && rapidRoot["BoxNum"].IsObject()) {
			const rapidjson::Value& obj = rapidRoot["BoxNum"];
			boxNum.m_bSelected = obj.HasMember("bSelected") ? obj["bSelected"].GetBool() : false;
			boxNum.m_color = StringToColor(obj.HasMember("Color") && obj["Color"].IsString() ? QString::fromUtf8(obj["Color"].GetString()) : QString());
			boxNum.m_iFontSize = obj.HasMember("iFontSize") ? obj["iFontSize"].GetInt() : 12;
		}

		if (rapidRoot.HasMember("Logo") && rapidRoot["Logo"].IsObject()) {
			const rapidjson::Value& obj = rapidRoot["Logo"];
			LogoInfo.m_bSelected = obj.HasMember("bSelected") ? obj["bSelected"].GetBool() : false;
			LogoInfo.m_iMode = obj.HasMember("iMode") ? obj["iMode"].GetInt() : 0;
			LogoInfo.m_iZoom = obj.HasMember("iZoom") ? obj["iZoom"].GetInt() : 100;
			LogoInfo.m_iTransparency = obj.HasMember("iTransparency") ? obj["iTransparency"].GetInt() : 100;
			LogoInfo.m_iX = obj.HasMember("iX") ? obj["iX"].GetInt() : 0;
			LogoInfo.m_iY = obj.HasMember("iY") ? obj["iY"].GetInt() : 0;
			LogoInfo.m_strGrandShowPath = (obj.HasMember("strGrandShowPath") && obj["strGrandShowPath"].IsString()) ? QString::fromUtf8(obj["strGrandShowPath"].GetString()) : QString();
			LogoInfo.m_strLogPath = (obj.HasMember("strLogPath") && obj["strLogPath"].IsString()) ? QString::fromUtf8(obj["strLogPath"].GetString()) : QString();
			LogoInfo.m_strIdentifier = (obj.HasMember("strIdentifier") && obj["strIdentifier"].IsString()) ? QString::fromUtf8(obj["strIdentifier"].GetString()) : QString();
			LogoInfo.m_color = StringToColor(obj.HasMember("Color") && obj["Color"].IsString() ? QString::fromUtf8(obj["Color"].GetString()) : QString());
			LogoInfo.m_iFontSize = obj.HasMember("iFontSize") ? obj["iFontSize"].GetInt() : 12;
		}

		if (rapidRoot.HasMember("TextInfo") && rapidRoot["TextInfo"].IsObject()) {
			const rapidjson::Value& obj = rapidRoot["TextInfo"];
			textInfo.m_bSelected = obj.HasMember("bSelected") ? obj["bSelected"].GetBool() : false;
			textInfo.m_strText = (obj.HasMember("strText") && obj["strText"].IsString()) ? QString::fromUtf8(obj["strText"].GetString()) : QString("Text");
			textInfo.m_color = StringToColor(obj.HasMember("Color") && obj["Color"].IsString() ? QString::fromUtf8(obj["Color"].GetString()) : QString());
			textInfo.m_iFontSize = obj.HasMember("iFontSize") ? obj["iFontSize"].GetInt() : 12;
		}
	}

	void LoadDataFromXml(const QDomNode &root)
	{
		QDomNode Item;
		QDomNode Node;

		//解析基本信息
		Item = root.namedItem("BaseInfo");
		baseInfo.m_iWidth = Item.namedItem("ContentW").firstChild().nodeValue().toInt();
		baseInfo.m_iHeight = Item.namedItem("ContentH").firstChild().nodeValue().toInt();
		baseInfo.m_iRow = Item.namedItem("ContentRow").firstChild().nodeValue().toInt();
		baseInfo.m_iCol = Item.namedItem("ContentCol").firstChild().nodeValue().toInt();

		//解析颜色模式
		Item = root.namedItem("ColorModeSelect");
		colorModeSelect.eColorMode = (EColorMode)Item.namedItem("ColorMode").firstChild().nodeValue().toInt();
		colorModeSelect.m_pureColor = StringToColor(Item.namedItem("PureColor").firstChild().nodeValue());
		colorModeSelect.m_gradientColor1 = StringToColor(Item.namedItem("GradientColor1").firstChild().nodeValue());
		colorModeSelect.m_gradientColor2 = StringToColor(Item.namedItem("GradientColor2").firstChild().nodeValue());
		colorModeSelect.m_chessColor1 = StringToColor(Item.namedItem("ChessColor1").firstChild().nodeValue());
		colorModeSelect.m_chessColor2 = StringToColor(Item.namedItem("ChessColor2").firstChild().nodeValue());

		//解析序号信息
		Item = root.namedItem("Order");
		order.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
		order.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
		order.m_iMode = Item.namedItem("iMode").firstChild().nodeValue().toInt();
		order.m_iFontSize = Item.namedItem("iFontSize").firstChild().nodeValue().toInt();

		//解析分割线信息
		Item = root.namedItem("SplineLine");
		spline.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
		spline.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
		spline.m_iWidth = Item.namedItem("iWidth").firstChild().nodeValue().toInt();

		//解析边框信息
		Item = root.namedItem("Cheek");
		cheek.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
		cheek.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
		cheek.m_iWidth = Item.namedItem("iWidth").firstChild().nodeValue().toInt();

		//解析圆信息
		Item = root.namedItem("Circle");
		circle.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
		circle.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
		circle.m_iWidth = Item.namedItem("iWidth").firstChild().nodeValue().toInt();
		circle.m_iRadius = Item.namedItem("iRadius").firstChild().nodeValue().toInt();

		//解析对角线信息
		Item = root.namedItem("Diagonal");
		diagonal.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
		diagonal.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
		diagonal.m_iWidth = Item.namedItem("iWidth").firstChild().nodeValue().toInt();

		//解析分辨率信息
		Item = root.namedItem("Resolution");
		resolution.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
		resolution.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
		resolution.m_iFontSize = Item.namedItem("iFontSize").firstChild().nodeValue().toInt();

		//解析箱体数信息
		Item = root.namedItem("BoxNum");
		boxNum.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
		boxNum.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
		boxNum.m_iFontSize = Item.namedItem("iFontSize").firstChild().nodeValue().toInt();

		//解析logo信息
		Item = root.namedItem("Logo");
		LogoInfo.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
		LogoInfo.m_iMode = Item.namedItem("iMode").firstChild().nodeValue().toInt();
		LogoInfo.m_iZoom = Item.namedItem("iZoom").firstChild().nodeValue().toInt();
		LogoInfo.m_iTransparency = Item.namedItem("iTransparency").firstChild().nodeValue().toInt();
		LogoInfo.m_iX = Item.namedItem("iX").firstChild().nodeValue().toInt();
		LogoInfo.m_iY = Item.namedItem("iY").firstChild().nodeValue().toInt();
		LogoInfo.m_strGrandShowPath = Item.namedItem("strGrandShowPath").firstChild().nodeValue();
		LogoInfo.m_strLogPath = Item.namedItem("strLogPath").firstChild().nodeValue();
		LogoInfo.m_strIdentifier = Item.namedItem("strIdentifier").firstChild().nodeValue();
		LogoInfo.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
		LogoInfo.m_iFontSize = Item.namedItem("iFontSize").firstChild().nodeValue().toInt();

		//解析文字信息
		Item = root.namedItem("TextInfo");
		textInfo.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
		textInfo.m_strText = Item.namedItem("strText").firstChild().nodeValue();
		textInfo.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
		textInfo.m_iFontSize = Item.namedItem("iFontSize").firstChild().nodeValue().toInt();

	}

}TestChartData_t;
