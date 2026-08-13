#pragma once

#include <QColor>
#include <rapidjson/document.h>

#include "CommonFunctionApi.h"
#include "CommonAboutCustomization.h"

//创建测试图数据数据结构
//基本信息
struct SBaseInfo
{
	//int   m_iX = 200;			//X坐标
	//int   m_iY = 200;			//Y坐标
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
		m_pureColor(QColor(236, 164, 32)),
		m_gradientColor1(QColor(0, 242, 96)),
		m_gradientColor2(QColor(0, 125, 241)),
		m_chessColor1(QColor(0, 208, 255)),
		m_chessColor2(QColor(76, 81, 196))
	{

	}
};

//序号
struct SOrder
{
	bool   m_bSelected = false;                      //是否开启序号模式
	QColor m_color = QColor(255, 255, 255);			//序号模式颜色
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
	bool   m_bSelected = true;                                //是否开启边框模式
	QColor m_color = QColor(240, 240, 240);			    //边框模式颜色
	int    m_iWidth = 8;					            //边框宽度
};

//圆
struct SCircle
{
	bool   m_bSelected = true;                               //是否开启圆模式
	QColor m_color = QColor(240, 240, 240);		             //圆模式颜色
	int    m_iWidth = 10;					                 //圆宽度
	int    m_iRadius = 450;					                 //圆半径（平面网格等）；天空盒首次默认见 kSkyboxCircleRadiusDefault
};

// 天空盒面中心圆默认半径（勿改 SCircle::m_iRadius，避免影响平面网格）
static const int kSkyboxCircleRadiusDefault = 100;

//对角线
struct SDiagonal
{
	bool   m_bSelected = true;                             //是否开启对角线模式
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
	QString m_strGrandShowPath = ":/dark/images/ResourceWnd/TestChart/logo.png";	//默认 logo 路径
	QString m_strLogPath = "";                         //logo路径
	QString m_strIdentifier = "A";                     //编号
	QColor  m_color = QColor(0, 0, 0);		           //箱体个数模式颜色
	int     m_iFontSize = 100;                         //字号
};

//文字信息
struct STextInfo
{
	bool   m_bSelected = true;                            //是否设置文字
	QString m_strText = COMPANY_WEBSITE;		  //文字信息
	int     m_iFontSize = 60;                             //字号
	QColor  m_color = QColor(0, 0, 0);		             //箱体个数模式颜色
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

//测试图类型
enum ETestChartType
{
	TestChartType_PlaneGrid = 0,	// 平面网格
	TestChartType_SingleFisheye,	// 单鱼眼
	TestChartType_DualFisheye,		// 双鱼眼
	TestChartType_Skybox,			// 天空盒
};

//双鱼眼排列方式
enum EDualFisheyeLayout
{
	DualFisheyeLayout_LR = 0,	// 左右排列 → 2N×N（默认 4096×2048）
	DualFisheyeLayout_TB,		// 上下排列 → N×2N（默认 2048×4096）
};

// 双鱼眼默认单眼边长 N（左右=2N×N，上下=N×2N）
constexpr int kDualFisheyeDefaultEyeN = 2048;

//单鱼眼测试图参数
struct SSingleFisheyeParam
{
	bool bShowBoundary = true;			// 鱼眼边界
	bool bShowAngleCircle = true;		// 角度刻度圆
	bool bShowGridSubdiv = true;		// 网格细分圆（依赖角度刻度圆）
	bool bShowScaleRadial = true;		// 刻度放射线
	bool bShowSubdivRadial = true;		// 细分放射线（依赖刻度放射线）
	bool bShowCenterRef = true;			// 中心参考圆
	int  fullFovDeg = 180;				// 完整视场角 2~180
	int  tickIntervalDeg = 10;			// 刻度间隔 1~90 且 <= F/2
	int  fontSizePx = 52;				// 角度数字字号 8~128
	int  gridSubdivCount = 2;			// 相邻刻度圆之间细分数量 0~10
	int  scaleRadialCount = 12;			// 刻度放射线数量（4 的倍数）
	int  subdivRadialCount = 2;			// 相邻刻度线之间细分放射线数量（仅绘制用，无界面项，固定默认值）
	QColor bgColor = QColor(236, 164, 32);	// 背景色（与现有纯色橙一致）
	QColor gridColor = QColor(255, 255, 255);	// 网格/放射线颜色
	int  lineWidth = 3;					// 线宽
};

//双鱼眼测试图参数（单侧视场角固定 90°；与单鱼眼相同含义的字段默认值保持一致）
struct SDualFisheyeParam
{
	EDualFisheyeLayout layout = DualFisheyeLayout_LR;
	bool bShowBoundary = true;
	bool bShowAngleCircle = true;
	bool bShowGridSubdiv = true;
	bool bShowScaleRadial = true;
	bool bShowSubdivRadial = true;
	bool bShowCenterRef = true;
	int  tickIntervalDeg = 10;			// 1~90
	int  fontSizePx = 30;				// 8~128（与单鱼眼一致）
	int  gridSubdivCount = 1;			// 0~10
	int  scaleRadialCount = 12;			// 4 的倍数
	int  subdivRadialCount = 1;			// 仅绘制用，与单鱼眼默认一致
	QColor bgColor = QColor(236, 164, 32);
	QColor gridColor = QColor(255, 255, 255);
	int  lineWidth = 3;					// 与单鱼眼一致
};

//天空盒测试图参数
struct SSkyboxParam
{
	bool bShowFaceName = true;
	QColor colorUp = QColor(50, 120, 220);		// 上-蓝
	QColor colorDown = QColor(0, 200, 220);		// 下-青
	QColor colorLeft = QColor(40, 180, 80);		// 左-绿
	QColor colorRight = QColor(140, 80, 200);	// 右-紫
	QColor colorFront = QColor(236, 164, 32);	// 前-橙
	QColor colorBack = QColor(220, 60, 120);		// 后-粉红
};

//各测试图类型共用属性快照（切类型时互不影响）
struct STestChartTypeSnapshot
{
	bool bValid = false;
	int resW = 0;		// 资源分辨率宽
	int resH = 0;		// 资源分辨率高
	SBaseInfo  baseInfo;
	SColorModeSelect  colorModeSelect;
	SOrder  order;
	SSplineLine   spline;
	SCheek  cheek;
	SCircle circle;
	SDiagonal diagonal;
	SResolution  resolution;
	SBoxNum    boxNum;
	SLogo      LogoInfo;
	STextInfo   textInfo;

	void SaveToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) const
	{
		root.SetObject();
		root.AddMember("bValid", bValid, allocator);
		root.AddMember("resW", resW, allocator);
		root.AddMember("resH", resH, allocator);

		rapidjson::Value baseInfoObj(rapidjson::kObjectType);
		baseInfoObj.AddMember("ContentW", baseInfo.m_iWidth, allocator);
		baseInfoObj.AddMember("ContentH", baseInfo.m_iHeight, allocator);
		baseInfoObj.AddMember("ContentRow", baseInfo.m_iRow, allocator);
		baseInfoObj.AddMember("ContentCol", baseInfo.m_iCol, allocator);
		root.AddMember("BaseInfo", baseInfoObj, allocator);

		rapidjson::Value colorModeSelectObj(rapidjson::kObjectType);
		colorModeSelectObj.AddMember("ColorMode", colorModeSelect.eColorMode, allocator);
		colorModeSelectObj.AddMember("PureColor", rapidjson::Value(ColorToString(colorModeSelect.m_pureColor).toUtf8().constData(), allocator), allocator);
		colorModeSelectObj.AddMember("GradientColor1", rapidjson::Value(ColorToString(colorModeSelect.m_gradientColor1).toUtf8().constData(), allocator), allocator);
		colorModeSelectObj.AddMember("GradientColor2", rapidjson::Value(ColorToString(colorModeSelect.m_gradientColor2).toUtf8().constData(), allocator), allocator);
		colorModeSelectObj.AddMember("ChessColor1", rapidjson::Value(ColorToString(colorModeSelect.m_chessColor1).toUtf8().constData(), allocator), allocator);
		colorModeSelectObj.AddMember("ChessColor2", rapidjson::Value(ColorToString(colorModeSelect.m_chessColor2).toUtf8().constData(), allocator), allocator);
		root.AddMember("ColorModeSelect", colorModeSelectObj, allocator);

		rapidjson::Value orderObj(rapidjson::kObjectType);
		orderObj.AddMember("bSelected", order.m_bSelected, allocator);
		orderObj.AddMember("Color", rapidjson::Value(ColorToString(order.m_color).toUtf8().constData(), allocator), allocator);
		orderObj.AddMember("iMode", order.m_iMode, allocator);
		root.AddMember("Order", orderObj, allocator);

		rapidjson::Value sPlineLineObj(rapidjson::kObjectType);
		sPlineLineObj.AddMember("bSelected", spline.m_bSelected, allocator);
		sPlineLineObj.AddMember("Color", rapidjson::Value(ColorToString(spline.m_color).toUtf8().constData(), allocator), allocator);
		sPlineLineObj.AddMember("iWidth", spline.m_iWidth, allocator);
		root.AddMember("SplineLine", sPlineLineObj, allocator);

		rapidjson::Value cheekObj(rapidjson::kObjectType);
		cheekObj.AddMember("bSelected", cheek.m_bSelected, allocator);
		cheekObj.AddMember("Color", rapidjson::Value(ColorToString(cheek.m_color).toUtf8().constData(), allocator), allocator);
		cheekObj.AddMember("iWidth", cheek.m_iWidth, allocator);
		root.AddMember("Cheek", cheekObj, allocator);

		rapidjson::Value circleObj(rapidjson::kObjectType);
		circleObj.AddMember("bSelected", circle.m_bSelected, allocator);
		circleObj.AddMember("Color", rapidjson::Value(ColorToString(circle.m_color).toUtf8().constData(), allocator), allocator);
		circleObj.AddMember("iWidth", circle.m_iWidth, allocator);
		circleObj.AddMember("iRadius", circle.m_iRadius, allocator);
		root.AddMember("Circle", circleObj, allocator);

		rapidjson::Value diagonalObj(rapidjson::kObjectType);
		diagonalObj.AddMember("bSelected", diagonal.m_bSelected, allocator);
		diagonalObj.AddMember("Color", rapidjson::Value(ColorToString(diagonal.m_color).toUtf8().constData(), allocator), allocator);
		diagonalObj.AddMember("iWidth", diagonal.m_iWidth, allocator);
		root.AddMember("Diagonal", diagonalObj, allocator);

		rapidjson::Value resolutionObj(rapidjson::kObjectType);
		resolutionObj.AddMember("bSelected", resolution.m_bSelected, allocator);
		resolutionObj.AddMember("Color", rapidjson::Value(ColorToString(resolution.m_color).toUtf8().constData(), allocator), allocator);
		resolutionObj.AddMember("iFontSize", resolution.m_iFontSize, allocator);
		root.AddMember("Resolution", resolutionObj, allocator);

		rapidjson::Value boxNumObj(rapidjson::kObjectType);
		boxNumObj.AddMember("bSelected", boxNum.m_bSelected, allocator);
		boxNumObj.AddMember("Color", rapidjson::Value(ColorToString(boxNum.m_color).toUtf8().constData(), allocator), allocator);
		boxNumObj.AddMember("iFontSize", boxNum.m_iFontSize, allocator);
		root.AddMember("BoxNum", boxNumObj, allocator);

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
		root.AddMember("Logo", logoObj, allocator);

		rapidjson::Value textInfoObj(rapidjson::kObjectType);
		textInfoObj.AddMember("bSelected", textInfo.m_bSelected, allocator);
		textInfoObj.AddMember("Color", rapidjson::Value(ColorToString(textInfo.m_color).toUtf8().constData(), allocator), allocator);
		textInfoObj.AddMember("strText", rapidjson::Value(textInfo.m_strText.toUtf8().constData(), allocator), allocator);
		textInfoObj.AddMember("iFontSize", textInfo.m_iFontSize, allocator);
		root.AddMember("TextInfo", textInfoObj, allocator);
	}

	void LoadFromRapidJson(const rapidjson::Value& obj)
	{
		if (!obj.IsObject())
			return;

		bValid = obj.HasMember("bValid") ? obj["bValid"].GetBool() : false;
		resW = obj.HasMember("resW") ? obj["resW"].GetInt() : 0;
		resH = obj.HasMember("resH") ? obj["resH"].GetInt() : 0;

		if (obj.HasMember("BaseInfo") && obj["BaseInfo"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["BaseInfo"];
			baseInfo.m_iWidth = tmpobj.HasMember("ContentW") ? tmpobj["ContentW"].GetInt() : 0;
			baseInfo.m_iHeight = tmpobj.HasMember("ContentH") ? tmpobj["ContentH"].GetInt() : 0;
			baseInfo.m_iRow = tmpobj.HasMember("ContentRow") ? tmpobj["ContentRow"].GetInt() : 0;
			baseInfo.m_iCol = tmpobj.HasMember("ContentCol") ? tmpobj["ContentCol"].GetInt() : 0;
		}

		if (obj.HasMember("ColorModeSelect") && obj["ColorModeSelect"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["ColorModeSelect"];
			colorModeSelect.eColorMode = (EColorMode)(tmpobj.HasMember("ColorMode") ? tmpobj["ColorMode"].GetInt() : 0);
			colorModeSelect.m_pureColor = StringToColor(tmpobj.HasMember("PureColor") && tmpobj["PureColor"].IsString() ? QString::fromUtf8(tmpobj["PureColor"].GetString()) : QString());
			colorModeSelect.m_gradientColor1 = StringToColor(tmpobj.HasMember("GradientColor1") && tmpobj["GradientColor1"].IsString() ? QString::fromUtf8(tmpobj["GradientColor1"].GetString()) : QString());
			colorModeSelect.m_gradientColor2 = StringToColor(tmpobj.HasMember("GradientColor2") && tmpobj["GradientColor2"].IsString() ? QString::fromUtf8(tmpobj["GradientColor2"].GetString()) : QString());
			colorModeSelect.m_chessColor1 = StringToColor(tmpobj.HasMember("ChessColor1") && tmpobj["ChessColor1"].IsString() ? QString::fromUtf8(tmpobj["ChessColor1"].GetString()) : QString());
			colorModeSelect.m_chessColor2 = StringToColor(tmpobj.HasMember("ChessColor2") && tmpobj["ChessColor2"].IsString() ? QString::fromUtf8(tmpobj["ChessColor2"].GetString()) : QString());
		}

		if (obj.HasMember("Order") && obj["Order"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["Order"];
			order.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			order.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			order.m_iMode = tmpobj.HasMember("iMode") ? tmpobj["iMode"].GetInt() : 0;
		}

		if (obj.HasMember("SplineLine") && obj["SplineLine"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["SplineLine"];
			spline.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			spline.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			spline.m_iWidth = tmpobj.HasMember("iWidth") ? tmpobj["iWidth"].GetInt() : 1;
		}

		if (obj.HasMember("Cheek") && obj["Cheek"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["Cheek"];
			cheek.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			cheek.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			cheek.m_iWidth = tmpobj.HasMember("iWidth") ? tmpobj["iWidth"].GetInt() : 1;
		}

		if (obj.HasMember("Circle") && obj["Circle"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["Circle"];
			circle.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			circle.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			circle.m_iWidth = tmpobj.HasMember("iWidth") ? tmpobj["iWidth"].GetInt() : 1;
			circle.m_iRadius = tmpobj.HasMember("iRadius") ? tmpobj["iRadius"].GetInt() : 10;
		}

		if (obj.HasMember("Diagonal") && obj["Diagonal"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["Diagonal"];
			diagonal.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			diagonal.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			diagonal.m_iWidth = tmpobj.HasMember("iWidth") ? tmpobj["iWidth"].GetInt() : 1;
		}

		if (obj.HasMember("Resolution") && obj["Resolution"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["Resolution"];
			resolution.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			resolution.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			resolution.m_iFontSize = tmpobj.HasMember("iFontSize") ? tmpobj["iFontSize"].GetInt() : 12;
		}

		if (obj.HasMember("BoxNum") && obj["BoxNum"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["BoxNum"];
			boxNum.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			boxNum.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			boxNum.m_iFontSize = tmpobj.HasMember("iFontSize") ? tmpobj["iFontSize"].GetInt() : 12;
		}

		if (obj.HasMember("Logo") && obj["Logo"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["Logo"];
			LogoInfo.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			LogoInfo.m_iMode = tmpobj.HasMember("iMode") ? tmpobj["iMode"].GetInt() : 0;
			LogoInfo.m_iZoom = tmpobj.HasMember("iZoom") ? tmpobj["iZoom"].GetInt() : 100;
			LogoInfo.m_iTransparency = tmpobj.HasMember("iTransparency") ? tmpobj["iTransparency"].GetInt() : 100;
			LogoInfo.m_iX = tmpobj.HasMember("iX") ? tmpobj["iX"].GetInt() : 0;
			LogoInfo.m_iY = tmpobj.HasMember("iY") ? tmpobj["iY"].GetInt() : 0;
			LogoInfo.m_strGrandShowPath = tmpobj.HasMember("strGrandShowPath") && tmpobj["strGrandShowPath"].IsString() ? QString::fromUtf8(tmpobj["strGrandShowPath"].GetString()) : QString();
			LogoInfo.m_strLogPath = tmpobj.HasMember("strLogPath") && tmpobj["strLogPath"].IsString() ? QString::fromUtf8(tmpobj["strLogPath"].GetString()) : QString();
			LogoInfo.m_strIdentifier = tmpobj.HasMember("strIdentifier") && tmpobj["strIdentifier"].IsString() ? QString::fromUtf8(tmpobj["strIdentifier"].GetString()) : QString();
			LogoInfo.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			LogoInfo.m_iFontSize = tmpobj.HasMember("iFontSize") ? tmpobj["iFontSize"].GetInt() : 12;
		}

		if (obj.HasMember("TextInfo") && obj["TextInfo"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["TextInfo"];
			textInfo.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			textInfo.m_strText = tmpobj.HasMember("strText") && tmpobj["strText"].IsString() ? QString::fromUtf8(tmpobj["strText"].GetString()) : QString();
			textInfo.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			textInfo.m_iFontSize = tmpobj.HasMember("iFontSize") ? tmpobj["iFontSize"].GetInt() : 12;
		}
	}
};

typedef struct STestChartData
{
	ETestChartType chartType = TestChartType_PlaneGrid;	// 测试图类型
	SSingleFisheyeParam singleFisheye;	// 单鱼眼参数（切类型后保留）
	SDualFisheyeParam dualFisheye;		// 双鱼眼参数（切类型后保留）
	SSkyboxParam skybox;				// 天空盒参数（切类型后保留）
	SBaseInfo  baseInfo;                //基本信息 
	SColorModeSelect  colorModeSelect;  //颜色模式 
	SOrder  order;                      //序号 
	SSplineLine   spline;           //分隔线
	SCheek  cheek;                      //边框 
	SCircle circle;                     //圆 
	SDiagonal diagonal;                 //对角线
	SResolution  resolution;			//分辨率
	SBoxNum    boxNum;                  //箱体数
	SLogo      LogoInfo;                    //logo信息
	STextInfo   textInfo;               //文字信息
	// 各类型共用属性快照（切换类型时保存/恢复，互不影响）
	STestChartTypeSnapshot snapPlane;
	STestChartTypeSnapshot snapSingle;
	STestChartTypeSnapshot snapDual;
	STestChartTypeSnapshot snapSkybox;

	STestChartData()
	{

	}

	STestChartTypeSnapshot& SnapshotOf(ETestChartType type)
	{
		switch (type)
		{
		case TestChartType_SingleFisheye: return snapSingle;
		case TestChartType_DualFisheye: return snapDual;
		case TestChartType_Skybox: return snapSkybox;
		default: return snapPlane;
		}
	}

	const STestChartTypeSnapshot& SnapshotOf(ETestChartType type) const
	{
		switch (type)
		{
		case TestChartType_SingleFisheye: return snapSingle;
		case TestChartType_DualFisheye: return snapDual;
		case TestChartType_Skybox: return snapSkybox;
		default: return snapPlane;
		}
	}

	// 将当前 active 共用属性保存到指定类型快照
	void CaptureActiveToSnapshot(ETestChartType type, int nResW, int nResH)
	{
		STestChartTypeSnapshot& s = SnapshotOf(type);
		s.bValid = true;
		s.resW = nResW;
		s.resH = nResH;
		s.baseInfo = baseInfo;
		s.colorModeSelect = colorModeSelect;
		s.order = order;
		s.spline = spline;
		s.cheek = cheek;
		s.circle = circle;
		s.diagonal = diagonal;
		s.resolution = resolution;
		s.boxNum = boxNum;
		s.LogoInfo = LogoInfo;
		s.textInfo = textInfo;
	}

	// 将指定类型快照写回 active；无效时重置为默认共用属性，返回是否有效
	bool ApplySnapshotToActive(ETestChartType type)
	{
		const STestChartTypeSnapshot& s = SnapshotOf(type);
		if (!s.bValid)
		{
			STestChartData def;
			baseInfo = def.baseInfo;
			colorModeSelect = def.colorModeSelect;
			order = def.order;
			spline = def.spline;
			cheek = def.cheek;
			circle = def.circle;
			diagonal = def.diagonal;
			resolution = def.resolution;
			boxNum = def.boxNum;
			LogoInfo = def.LogoInfo;
			textInfo = def.textInfo;
			// 天空盒首次无快照：圆半径 100；其它类型保持 SCircle 默认 450
			if (type == TestChartType_Skybox)
				circle.m_iRadius = kSkyboxCircleRadiusDefault;
			return false;
		}
		baseInfo = s.baseInfo;
		colorModeSelect = s.colorModeSelect;
		order = s.order;
		spline = s.spline;
		cheek = s.cheek;
		circle = s.circle;
		diagonal = s.diagonal;
		resolution = s.resolution;
		boxNum = s.boxNum;
		LogoInfo = s.LogoInfo;
		textInfo = s.textInfo;
		return true;
	}

	void SaveDataToRapidJson(rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator) const
	{
		root.SetObject();
		root.AddMember("ChartType", static_cast<int>(chartType), allocator);

		rapidjson::Value fisheyeObj(rapidjson::kObjectType);
		fisheyeObj.AddMember("bShowBoundary", singleFisheye.bShowBoundary, allocator);
		fisheyeObj.AddMember("bShowAngleCircle", singleFisheye.bShowAngleCircle, allocator);
		fisheyeObj.AddMember("bShowGridSubdiv", singleFisheye.bShowGridSubdiv, allocator);
		fisheyeObj.AddMember("bShowScaleRadial", singleFisheye.bShowScaleRadial, allocator);
		fisheyeObj.AddMember("bShowSubdivRadial", singleFisheye.bShowSubdivRadial, allocator);
		fisheyeObj.AddMember("bShowCenterRef", singleFisheye.bShowCenterRef, allocator);
		fisheyeObj.AddMember("fullFovDeg", singleFisheye.fullFovDeg, allocator);
		fisheyeObj.AddMember("tickIntervalDeg", singleFisheye.tickIntervalDeg, allocator);
		fisheyeObj.AddMember("fontSizePx", singleFisheye.fontSizePx, allocator);
		fisheyeObj.AddMember("gridSubdivCount", singleFisheye.gridSubdivCount, allocator);
		fisheyeObj.AddMember("scaleRadialCount", singleFisheye.scaleRadialCount, allocator);
		fisheyeObj.AddMember("subdivRadialCount", singleFisheye.subdivRadialCount, allocator);
		fisheyeObj.AddMember("bgColor", rapidjson::Value(ColorToString(singleFisheye.bgColor).toUtf8().constData(), allocator), allocator);
		fisheyeObj.AddMember("gridColor", rapidjson::Value(ColorToString(singleFisheye.gridColor).toUtf8().constData(), allocator), allocator);
		fisheyeObj.AddMember("lineWidth", singleFisheye.lineWidth, allocator);
		root.AddMember("SingleFisheye", fisheyeObj, allocator);

		rapidjson::Value dualObj(rapidjson::kObjectType);
		dualObj.AddMember("layout", static_cast<int>(dualFisheye.layout), allocator);
		dualObj.AddMember("bShowBoundary", dualFisheye.bShowBoundary, allocator);
		dualObj.AddMember("bShowAngleCircle", dualFisheye.bShowAngleCircle, allocator);
		dualObj.AddMember("bShowGridSubdiv", dualFisheye.bShowGridSubdiv, allocator);
		dualObj.AddMember("bShowScaleRadial", dualFisheye.bShowScaleRadial, allocator);
		dualObj.AddMember("bShowSubdivRadial", dualFisheye.bShowSubdivRadial, allocator);
		dualObj.AddMember("bShowCenterRef", dualFisheye.bShowCenterRef, allocator);
		dualObj.AddMember("tickIntervalDeg", dualFisheye.tickIntervalDeg, allocator);
		dualObj.AddMember("fontSizePx", dualFisheye.fontSizePx, allocator);
		dualObj.AddMember("gridSubdivCount", dualFisheye.gridSubdivCount, allocator);
		dualObj.AddMember("scaleRadialCount", dualFisheye.scaleRadialCount, allocator);
		dualObj.AddMember("subdivRadialCount", dualFisheye.subdivRadialCount, allocator);
		dualObj.AddMember("bgColor", rapidjson::Value(ColorToString(dualFisheye.bgColor).toUtf8().constData(), allocator), allocator);
		dualObj.AddMember("gridColor", rapidjson::Value(ColorToString(dualFisheye.gridColor).toUtf8().constData(), allocator), allocator);
		dualObj.AddMember("lineWidth", dualFisheye.lineWidth, allocator);
		root.AddMember("DualFisheye", dualObj, allocator);

		rapidjson::Value skyObj(rapidjson::kObjectType);
		skyObj.AddMember("bShowFaceName", skybox.bShowFaceName, allocator);
		skyObj.AddMember("colorUp", rapidjson::Value(ColorToString(skybox.colorUp).toUtf8().constData(), allocator), allocator);
		skyObj.AddMember("colorDown", rapidjson::Value(ColorToString(skybox.colorDown).toUtf8().constData(), allocator), allocator);
		skyObj.AddMember("colorLeft", rapidjson::Value(ColorToString(skybox.colorLeft).toUtf8().constData(), allocator), allocator);
		skyObj.AddMember("colorRight", rapidjson::Value(ColorToString(skybox.colorRight).toUtf8().constData(), allocator), allocator);
		skyObj.AddMember("colorFront", rapidjson::Value(ColorToString(skybox.colorFront).toUtf8().constData(), allocator), allocator);
		skyObj.AddMember("colorBack", rapidjson::Value(ColorToString(skybox.colorBack).toUtf8().constData(), allocator), allocator);
		root.AddMember("Skybox", skyObj, allocator);

		rapidjson::Value baseInfoObj(rapidjson::kObjectType);
		baseInfoObj.AddMember("ContentW", baseInfo.m_iWidth, allocator);
		baseInfoObj.AddMember("ContentH", baseInfo.m_iHeight, allocator);
		baseInfoObj.AddMember("ContentRow", baseInfo.m_iRow, allocator);
		baseInfoObj.AddMember("ContentCol", baseInfo.m_iCol, allocator);
		root.AddMember("BaseInfo", baseInfoObj, allocator);

		rapidjson::Value colorModeSelectObj(rapidjson::kObjectType);
		colorModeSelectObj.AddMember("ColorMode", colorModeSelect.eColorMode, allocator);
		colorModeSelectObj.AddMember("PureColor", rapidjson::Value(ColorToString(colorModeSelect.m_pureColor).toUtf8().constData(), allocator), allocator);
		colorModeSelectObj.AddMember("GradientColor1", rapidjson::Value(ColorToString(colorModeSelect.m_gradientColor1).toUtf8().constData(), allocator), allocator);
		colorModeSelectObj.AddMember("GradientColor2", rapidjson::Value(ColorToString(colorModeSelect.m_gradientColor2).toUtf8().constData(), allocator), allocator);
		colorModeSelectObj.AddMember("ChessColor1", rapidjson::Value(ColorToString(colorModeSelect.m_chessColor1).toUtf8().constData(), allocator), allocator);
		colorModeSelectObj.AddMember("ChessColor2", rapidjson::Value(ColorToString(colorModeSelect.m_chessColor2).toUtf8().constData(), allocator), allocator);
		root.AddMember("ColorModeSelect", colorModeSelectObj, allocator);

		rapidjson::Value orderObj(rapidjson::kObjectType);
		orderObj.AddMember("bSelected", order.m_bSelected, allocator);
		orderObj.AddMember("Color", rapidjson::Value(ColorToString(order.m_color).toUtf8().constData(), allocator), allocator);
		orderObj.AddMember("iMode", order.m_iMode, allocator);
		root.AddMember("Order", orderObj, allocator);

		rapidjson::Value sPlineLineObj(rapidjson::kObjectType);
		sPlineLineObj.AddMember("bSelected", spline.m_bSelected, allocator);
		sPlineLineObj.AddMember("Color", rapidjson::Value(ColorToString(spline.m_color).toUtf8().constData(), allocator), allocator);
		sPlineLineObj.AddMember("iWidth", spline.m_iWidth, allocator);
		root.AddMember("SplineLine", sPlineLineObj, allocator);

		rapidjson::Value cheekObj(rapidjson::kObjectType);
		cheekObj.AddMember("bSelected", cheek.m_bSelected, allocator);
		cheekObj.AddMember("Color", rapidjson::Value(ColorToString(cheek.m_color).toUtf8().constData(), allocator), allocator);
		cheekObj.AddMember("iWidth", cheek.m_iWidth, allocator);
		root.AddMember("Cheek", cheekObj, allocator);

		rapidjson::Value circleObj(rapidjson::kObjectType);
		circleObj.AddMember("bSelected", circle.m_bSelected, allocator);
		circleObj.AddMember("Color", rapidjson::Value(ColorToString(circle.m_color).toUtf8().constData(), allocator), allocator);
		circleObj.AddMember("iWidth", circle.m_iWidth, allocator);
		circleObj.AddMember("iRadius", circle.m_iRadius, allocator);
		root.AddMember("Circle", circleObj, allocator);

		rapidjson::Value diagonalObj(rapidjson::kObjectType);
		diagonalObj.AddMember("bSelected", diagonal.m_bSelected, allocator);
		diagonalObj.AddMember("Color", rapidjson::Value(ColorToString(diagonal.m_color).toUtf8().constData(), allocator), allocator);
		diagonalObj.AddMember("iWidth", diagonal.m_iWidth, allocator);
		root.AddMember("Diagonal", diagonalObj, allocator);

		rapidjson::Value resolutionObj(rapidjson::kObjectType);
		resolutionObj.AddMember("bSelected", resolution.m_bSelected, allocator);
		resolutionObj.AddMember("Color", rapidjson::Value(ColorToString(resolution.m_color).toUtf8().constData(), allocator), allocator);
		resolutionObj.AddMember("iFontSize", resolution.m_iFontSize, allocator);
		root.AddMember("Resolution", resolutionObj, allocator);

		rapidjson::Value boxNumObj(rapidjson::kObjectType);
		boxNumObj.AddMember("bSelected", boxNum.m_bSelected, allocator);
		boxNumObj.AddMember("Color", rapidjson::Value(ColorToString(boxNum.m_color).toUtf8().constData(), allocator), allocator);
		boxNumObj.AddMember("iFontSize", boxNum.m_iFontSize, allocator);
		root.AddMember("BoxNum", boxNumObj, allocator);

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
		root.AddMember("Logo", logoObj, allocator);

		rapidjson::Value textInfoObj(rapidjson::kObjectType);
		textInfoObj.AddMember("bSelected", textInfo.m_bSelected, allocator);
		textInfoObj.AddMember("Color", rapidjson::Value(ColorToString(textInfo.m_color).toUtf8().constData(), allocator), allocator);
		textInfoObj.AddMember("strText", rapidjson::Value(textInfo.m_strText.toUtf8().constData(), allocator), allocator);
		textInfoObj.AddMember("iFontSize", textInfo.m_iFontSize, allocator);
		root.AddMember("TextInfo", textInfoObj, allocator);

		rapidjson::Value snapPlaneObj(rapidjson::kObjectType);
		snapPlane.SaveToRapidJson(snapPlaneObj, allocator);
		root.AddMember("SnapPlane", snapPlaneObj, allocator);
		rapidjson::Value snapSingleObj(rapidjson::kObjectType);
		snapSingle.SaveToRapidJson(snapSingleObj, allocator);
		root.AddMember("SnapSingle", snapSingleObj, allocator);
		rapidjson::Value snapDualObj(rapidjson::kObjectType);
		snapDual.SaveToRapidJson(snapDualObj, allocator);
		root.AddMember("SnapDual", snapDualObj, allocator);
		rapidjson::Value snapSkyboxObj(rapidjson::kObjectType);
		snapSkybox.SaveToRapidJson(snapSkyboxObj, allocator);
		root.AddMember("SnapSkybox", snapSkyboxObj, allocator);
	}

	void LoadDataFromRapidJson(const rapidjson::Value& obj)
	{
		if (!obj.IsObject())
		{
			return;
		}

		chartType = static_cast<ETestChartType>(obj.HasMember("ChartType") ? obj["ChartType"].GetInt() : TestChartType_PlaneGrid);

		if (obj.HasMember("SingleFisheye") && obj["SingleFisheye"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["SingleFisheye"];
			singleFisheye.bShowBoundary = tmpobj.HasMember("bShowBoundary") ? tmpobj["bShowBoundary"].GetBool() : true;
			singleFisheye.bShowAngleCircle = tmpobj.HasMember("bShowAngleCircle") ? tmpobj["bShowAngleCircle"].GetBool() : true;
			singleFisheye.bShowGridSubdiv = tmpobj.HasMember("bShowGridSubdiv") ? tmpobj["bShowGridSubdiv"].GetBool() : true;
			singleFisheye.bShowScaleRadial = tmpobj.HasMember("bShowScaleRadial") ? tmpobj["bShowScaleRadial"].GetBool() : true;
			singleFisheye.bShowSubdivRadial = tmpobj.HasMember("bShowSubdivRadial") ? tmpobj["bShowSubdivRadial"].GetBool() : true;
			singleFisheye.bShowCenterRef = tmpobj.HasMember("bShowCenterRef") ? tmpobj["bShowCenterRef"].GetBool() : true;
			singleFisheye.fullFovDeg = tmpobj.HasMember("fullFovDeg") ? tmpobj["fullFovDeg"].GetInt() : 180;
			singleFisheye.tickIntervalDeg = tmpobj.HasMember("tickIntervalDeg") ? tmpobj["tickIntervalDeg"].GetInt() : 10;
			singleFisheye.fontSizePx = tmpobj.HasMember("fontSizePx") ? tmpobj["fontSizePx"].GetInt() : 52;
			singleFisheye.gridSubdivCount = tmpobj.HasMember("gridSubdivCount") ? tmpobj["gridSubdivCount"].GetInt() : 2;
			singleFisheye.scaleRadialCount = tmpobj.HasMember("scaleRadialCount") ? tmpobj["scaleRadialCount"].GetInt() : 4;
			singleFisheye.subdivRadialCount = tmpobj.HasMember("subdivRadialCount") ? tmpobj["subdivRadialCount"].GetInt() : 2;
			singleFisheye.bgColor = StringToColor(tmpobj.HasMember("bgColor") && tmpobj["bgColor"].IsString()
				? QString::fromUtf8(tmpobj["bgColor"].GetString()) : QString());
			if (!singleFisheye.bgColor.isValid())
				singleFisheye.bgColor = QColor(236, 164, 32);
			singleFisheye.gridColor = StringToColor(tmpobj.HasMember("gridColor") && tmpobj["gridColor"].IsString()
				? QString::fromUtf8(tmpobj["gridColor"].GetString()) : QString());
			if (!singleFisheye.gridColor.isValid())
				singleFisheye.gridColor = QColor(255, 255, 255);
			singleFisheye.lineWidth = tmpobj.HasMember("lineWidth") ? tmpobj["lineWidth"].GetInt() : 3;
		}

		if (obj.HasMember("DualFisheye") && obj["DualFisheye"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["DualFisheye"];
			dualFisheye.layout = static_cast<EDualFisheyeLayout>(tmpobj.HasMember("layout") ? tmpobj["layout"].GetInt() : DualFisheyeLayout_LR);
			dualFisheye.bShowBoundary = tmpobj.HasMember("bShowBoundary") ? tmpobj["bShowBoundary"].GetBool() : true;
			dualFisheye.bShowAngleCircle = tmpobj.HasMember("bShowAngleCircle") ? tmpobj["bShowAngleCircle"].GetBool() : true;
			dualFisheye.bShowGridSubdiv = tmpobj.HasMember("bShowGridSubdiv") ? tmpobj["bShowGridSubdiv"].GetBool() : true;
			dualFisheye.bShowScaleRadial = tmpobj.HasMember("bShowScaleRadial") ? tmpobj["bShowScaleRadial"].GetBool() : true;
			dualFisheye.bShowSubdivRadial = tmpobj.HasMember("bShowSubdivRadial") ? tmpobj["bShowSubdivRadial"].GetBool() : true;
			dualFisheye.bShowCenterRef = tmpobj.HasMember("bShowCenterRef") ? tmpobj["bShowCenterRef"].GetBool() : true;
			dualFisheye.tickIntervalDeg = tmpobj.HasMember("tickIntervalDeg") ? tmpobj["tickIntervalDeg"].GetInt() : 10;
			dualFisheye.fontSizePx = tmpobj.HasMember("fontSizePx") ? tmpobj["fontSizePx"].GetInt() : 52;
			dualFisheye.gridSubdivCount = tmpobj.HasMember("gridSubdivCount") ? tmpobj["gridSubdivCount"].GetInt() : 2;
			dualFisheye.scaleRadialCount = tmpobj.HasMember("scaleRadialCount") ? tmpobj["scaleRadialCount"].GetInt() : 4;
			dualFisheye.subdivRadialCount = tmpobj.HasMember("subdivRadialCount") ? tmpobj["subdivRadialCount"].GetInt() : 2;
			dualFisheye.bgColor = StringToColor(tmpobj.HasMember("bgColor") && tmpobj["bgColor"].IsString()
				? QString::fromUtf8(tmpobj["bgColor"].GetString()) : QString());
			if (!dualFisheye.bgColor.isValid())
				dualFisheye.bgColor = QColor(236, 164, 32);
			dualFisheye.gridColor = StringToColor(tmpobj.HasMember("gridColor") && tmpobj["gridColor"].IsString()
				? QString::fromUtf8(tmpobj["gridColor"].GetString()) : QString());
			if (!dualFisheye.gridColor.isValid())
				dualFisheye.gridColor = QColor(255, 255, 255);
			dualFisheye.lineWidth = tmpobj.HasMember("lineWidth") ? tmpobj["lineWidth"].GetInt() : 3;
		}

		if (obj.HasMember("Skybox") && obj["Skybox"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["Skybox"];
			skybox.bShowFaceName = tmpobj.HasMember("bShowFaceName") ? tmpobj["bShowFaceName"].GetBool() : true;
			auto loadClr = [&](const char* key, QColor& out, const QColor& def) {
				out = StringToColor(tmpobj.HasMember(key) && tmpobj[key].IsString()
					? QString::fromUtf8(tmpobj[key].GetString()) : QString());
				if (!out.isValid())
					out = def;
			};
			loadClr("colorUp", skybox.colorUp, QColor(50, 120, 220));
			loadClr("colorDown", skybox.colorDown, QColor(0, 200, 220));
			loadClr("colorLeft", skybox.colorLeft, QColor(40, 180, 80));
			loadClr("colorRight", skybox.colorRight, QColor(140, 80, 200));
			loadClr("colorFront", skybox.colorFront, QColor(236, 164, 32));
			loadClr("colorBack", skybox.colorBack, QColor(220, 60, 120));
		}

		if (obj.HasMember("BaseInfo") && obj["BaseInfo"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["BaseInfo"];
			baseInfo.m_iWidth = tmpobj.HasMember("ContentW") ? tmpobj["ContentW"].GetInt() : 0;
			baseInfo.m_iHeight = tmpobj.HasMember("ContentH") ? tmpobj["ContentH"].GetInt() : 0;
			baseInfo.m_iRow = tmpobj.HasMember("ContentRow") ? tmpobj["ContentRow"].GetInt() : 0;
			baseInfo.m_iCol = tmpobj.HasMember("ContentCol") ? tmpobj["ContentCol"].GetInt() : 0;
		}

		if (obj.HasMember("ColorModeSelect") && obj["ColorModeSelect"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["ColorModeSelect"];
			colorModeSelect.eColorMode = (EColorMode)(tmpobj.HasMember("ColorMode") ? tmpobj["ColorMode"].GetInt() : 0);
			colorModeSelect.m_pureColor = StringToColor(tmpobj.HasMember("PureColor") && tmpobj["PureColor"].IsString() ? QString::fromUtf8(tmpobj["PureColor"].GetString()) : QString());
			colorModeSelect.m_gradientColor1 = StringToColor(tmpobj.HasMember("GradientColor1") && tmpobj["GradientColor1"].IsString() ? QString::fromUtf8(tmpobj["GradientColor1"].GetString()) : QString());
			colorModeSelect.m_gradientColor2 = StringToColor(tmpobj.HasMember("GradientColor2") && tmpobj["GradientColor2"].IsString() ? QString::fromUtf8(tmpobj["GradientColor2"].GetString()) : QString());
			colorModeSelect.m_chessColor1 = StringToColor(tmpobj.HasMember("ChessColor1") && tmpobj["ChessColor1"].IsString() ? QString::fromUtf8(tmpobj["ChessColor1"].GetString()) : QString());
			colorModeSelect.m_chessColor2 = StringToColor(tmpobj.HasMember("ChessColor2") && tmpobj["ChessColor2"].IsString() ? QString::fromUtf8(tmpobj["ChessColor2"].GetString()) : QString());
		}

		if (obj.HasMember("Order") && obj["Order"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["Order"];
			order.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			order.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			order.m_iMode = tmpobj.HasMember("iMode") ? tmpobj["iMode"].GetInt() : 0;
		}

		if (obj.HasMember("SplineLine") && obj["SplineLine"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["SplineLine"];
			spline.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			spline.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			spline.m_iWidth = tmpobj.HasMember("iWidth") ? tmpobj["iWidth"].GetInt() : 1;
		}

		if (obj.HasMember("Cheek") && obj["Cheek"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["Cheek"];
			cheek.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			cheek.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			cheek.m_iWidth = tmpobj.HasMember("iWidth") ? tmpobj["iWidth"].GetInt() : 1;
		}

		if (obj.HasMember("Circle") && obj["Circle"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["Circle"];
			circle.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			circle.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			circle.m_iWidth = tmpobj.HasMember("iWidth") ? tmpobj["iWidth"].GetInt() : 1;
			circle.m_iRadius = tmpobj.HasMember("iRadius") ? tmpobj["iRadius"].GetInt() : 10;
		}

		if (obj.HasMember("Diagonal") && obj["Diagonal"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["Diagonal"];
			diagonal.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			diagonal.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			diagonal.m_iWidth = tmpobj.HasMember("iWidth") ? tmpobj["iWidth"].GetInt() : 1;
		}

		if (obj.HasMember("Resolution") && obj["Resolution"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["Resolution"];
			resolution.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			resolution.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			resolution.m_iFontSize = tmpobj.HasMember("iFontSize") ? tmpobj["iFontSize"].GetInt() : 12;
		}

		if (obj.HasMember("BoxNum") && obj["BoxNum"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["BoxNum"];
			boxNum.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			boxNum.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			boxNum.m_iFontSize = tmpobj.HasMember("iFontSize") ? tmpobj["iFontSize"].GetInt() : 12;
		}

		if (obj.HasMember("Logo") && obj["Logo"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["Logo"];
			LogoInfo.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			LogoInfo.m_iMode = tmpobj.HasMember("iMode") ? tmpobj["iMode"].GetInt() : 0;
			LogoInfo.m_iZoom = tmpobj.HasMember("iZoom") ? tmpobj["iZoom"].GetInt() : 100;
			LogoInfo.m_iTransparency = tmpobj.HasMember("iTransparency") ? tmpobj["iTransparency"].GetInt() : 100;
			LogoInfo.m_iX = tmpobj.HasMember("iX") ? tmpobj["iX"].GetInt() : 0;
			LogoInfo.m_iY = tmpobj.HasMember("iY") ? tmpobj["iY"].GetInt() : 0;
			LogoInfo.m_strGrandShowPath = tmpobj.HasMember("strGrandShowPath") && tmpobj["strGrandShowPath"].IsString() ? QString::fromUtf8(tmpobj["strGrandShowPath"].GetString()) : QString();
			LogoInfo.m_strLogPath = tmpobj.HasMember("strLogPath") && tmpobj["strLogPath"].IsString() ? QString::fromUtf8(tmpobj["strLogPath"].GetString()) : QString();
			LogoInfo.m_strIdentifier = tmpobj.HasMember("strIdentifier") && tmpobj["strIdentifier"].IsString() ? QString::fromUtf8(tmpobj["strIdentifier"].GetString()) : QString();
			LogoInfo.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			LogoInfo.m_iFontSize = tmpobj.HasMember("iFontSize") ? tmpobj["iFontSize"].GetInt() : 12;
		}

		if (obj.HasMember("TextInfo") && obj["TextInfo"].IsObject())
		{
			const rapidjson::Value& tmpobj = obj["TextInfo"];
			textInfo.m_bSelected = tmpobj.HasMember("bSelected") ? tmpobj["bSelected"].GetBool() : false;
			textInfo.m_strText = tmpobj.HasMember("strText") && tmpobj["strText"].IsString() ? QString::fromUtf8(tmpobj["strText"].GetString()) : QString();
			textInfo.m_color = StringToColor(tmpobj.HasMember("Color") && tmpobj["Color"].IsString() ? QString::fromUtf8(tmpobj["Color"].GetString()) : QString());
			textInfo.m_iFontSize = tmpobj.HasMember("iFontSize") ? tmpobj["iFontSize"].GetInt() : 12;
		}

		if (obj.HasMember("SnapPlane") && obj["SnapPlane"].IsObject())
			snapPlane.LoadFromRapidJson(obj["SnapPlane"]);
		if (obj.HasMember("SnapSingle") && obj["SnapSingle"].IsObject())
			snapSingle.LoadFromRapidJson(obj["SnapSingle"]);
		if (obj.HasMember("SnapDual") && obj["SnapDual"].IsObject())
			snapDual.LoadFromRapidJson(obj["SnapDual"]);
		if (obj.HasMember("SnapSkybox") && obj["SnapSkybox"].IsObject())
			snapSkybox.LoadFromRapidJson(obj["SnapSkybox"]);

	}

}TestChartData_t;
