#pragma once
#include <QUuid>
#include <QJsonObject>
#include <QString>
#include <QObject>
#include <QColor>
#include <QFont>

#include "CommonFunctionApi.h"
#include "CommonAboutCustomization.h"

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
#define Xml_ResourceModel_Duration "Duration"                          // 时长
#define Xml_ResourceModel_Width "Width"                                // 视频宽度
#define Xml_ResourceModel_Height "Height"                              // 视频高度
#define Xml_ResourceModel_SupportHWDEC "SupportHWDEC"                  // 是否支持硬件解码
#define Xml_ResourceModel_HWDEC "HWDEC"                                // 是否启用硬件解码（运行结果）
#define Xml_ResourceModel_DecodingMode "DecodingMode"                  // 解码偏好（EDecoding）
#define Xml_ResourceModel_VideoFramerate "videoFramerate"              // 视频帧率
#define Xml_ResourceModel_VidioFormat "vidioFormat"                    // 视频格式（保留原拼写错误）
#define Xml_ResourceModel_VideoBitrate "videoBitrate"                  // 视频码率
#define Xml_ResourceModel_AudioChannels "AudioChannels"                // 音频声道数
#define Xml_ResourceModel_AudioFormat "audioFormat"                    // 音频格式
#define Xml_ResourceModel_AudioSamplerate "audioSamplerate"            // 音频采样率
#define Xml_ResourceModel_AlwaysOpen "AlwaysOpen"                      // 是否保持开启

#define Xml_ResourceModel_Pic_NOpt "nOpt"                              // 操作类型值
#define Xml_ResourceModel_Pic_OptFilePaths "OptFilePaths"              // 操作文件路径集合
#define Xml_ResourceModel_Pic_Num "num"                                // 路径数量
#define Xml_ResourceModel_Pic_OptType "OptType"                        // 操作类型
#define Xml_ResourceModel_Pic_FilePath "FilePath"                      // 文件路径
#define Xml_ResourceModel_Pic_ExifOri "ExifOri"                        // EXIF方向信息


#define Xml_ResourceModel_TestChartData "TestChartData"

#define Max_CustomizedResource_Num 2000

//素材类型
enum EResourceType
{
	ERT_Invalid = -1,
	ERT_Video,
	ERT_Picture,
	ERT_TestChart,
	ERT_Audio,
	ERT_Folder,
	ERT_VideoCapture,
	ERT_AudioCapture,
	ERT_WebMedia,
	ERT_Text,
	ERT_Ndi,
	ERT_Subtitle,
	ERT_Teleprompter,
	ERT_TimeLine,
	//新素材类型 请添加到 这行注释 的 上一行, 后面几个素材类型相关公共接口也需要检查一下
	ERT_Count,
};

static const QString strVideoFormat = "*.mp4 *.3gp *.m2t *.m2ts *.bhd *.asx *.mpe *.vcd *.dvd *.dat *.m4v *.webm *.rmvb *.rm *.wmv *.asf *.mpeg *.mpg *.ts *.tp *.mov *.flv *.avi *.mkv *.vob *.m2v";
static const QString strAudioFormat = "*.m4a *.mp3 *.wav *.flac *.aac";
static const QString strPictureFormat = "*.bmp *.png *.jpg *.tiff *.tif *.tga *.webp *.jpeg *.gif *.dds *.exr *.dpx";
static const QString strOnlyPictureFormat = "*.bmp *.png *.jpg *.tiff *.tif *.tga *.webp *.jpeg *.gif *.dds *.exr *.dpx";
static const QString strSubtitle = "*.lrc *.srt *.txt";
static const QString strSubtitleOrTeleprompte = "*.txt";

#define RES_OtherType -2

//通过素材路径获取素材类型
static EResourceType GetFileResType(const QString &strFilePath)
{
	int nIndex = strFilePath.lastIndexOf('.');
	QString szSuffix = strFilePath.right(strFilePath.length() - nIndex - 1);
	szSuffix = szSuffix.toLower();

	if (strVideoFormat.contains(szSuffix, Qt::CaseSensitive))
		return ERT_Video;

	if (strAudioFormat.contains(szSuffix, Qt::CaseSensitive))
		return ERT_Audio;

	if (strPictureFormat.contains(szSuffix, Qt::CaseSensitive))
		return ERT_Picture;

	if (strSubtitle.contains(szSuffix, Qt::CaseSensitive))
		return ERT_Subtitle;

	if (strSubtitleOrTeleprompte.contains(szSuffix, Qt::CaseSensitive))
		return ERT_Teleprompter;

	return ERT_Invalid;
}

// 判断是否是文件素材
static bool IsFileResource(int nType)
{
	switch (nType)
	{
	case ERT_Video:
	case ERT_Audio:
	case ERT_Picture:
		return true;
	default:
		break;
	}
	return false;
}

//判断是否支持切片
static bool IsSupportSliceResource(int nType)
{
	switch (nType)
	{
	case ERT_AudioCapture:
	case ERT_Audio:
		return false;
	default:
		break;
	}
	return true;
}

// 判断是翻页素材
static bool IsSupportPageTurn(int nType)
{
	switch (nType)
	{
	case ERT_Teleprompter:
		return true;
	default:
		break;
	}

	return false;
}

static bool IsSupportFillType(int nType)
{
	switch (nType)
	{
	case ERT_Video:
	case ERT_Audio:
	case ERT_Picture:
	case ERT_AudioCapture:
	case ERT_VideoCapture:
	case ERT_WebMedia:
	case ERT_Ndi:
		return true;
	default:
		break;
	}

	return false;
}

// 是否为流媒体素材
static bool IsStreamResource(int nType)
{
	switch (nType)
	{
	case ERT_AudioCapture:
	case ERT_VideoCapture:
	case ERT_WebMedia:
	case ERT_Ndi:
		return true;
	default:
		break;
	}

	return false;
}

// 素材是否支持声音
static bool IsSupportVolum(int nType)
{
	switch (nType)
	{
	case ERT_Video:
	case ERT_Audio:
	case ERT_AudioCapture:
	case ERT_WebMedia:
	case ERT_Ndi:
		return true;
	default:
		break;
	}

	return false;
}

// 是否有实际缩略图
static bool IsHasRealThumb(int nType)
{
	switch (nType)
	{
	case ERT_Audio:
	case ERT_AudioCapture:
		return false;
	default:
		break;
	}
	return true;
}

// 是否支持填充模式
static bool IsSupportFillMode(int nType)
{
	switch (nType)
	{
	case ERT_TestChart:
	case ERT_Text:
	case ERT_Subtitle:
	case ERT_Teleprompter:
		return false;
	default:
		break;
	}
	return true;
}


static bool IsSupportFlip(int nType)
{
	switch (nType)
	{
	case ERT_Audio:
	case ERT_AudioCapture:
		return false;
	default:
		break;
	}

	return true;
}

// 素材是否需要推流
static bool IsNeedStreamPush(int nType)
{
	switch (nType)
	{
	case ERT_Audio:
	case ERT_AudioCapture:
	case ERT_Folder:
		return false;
	default:
		break;
	}
	return true;
}

// 推流是否使用带alpha通道的图片
static bool IsStreamWithAlpha(int nType)
{
	switch (nType)
	{
	case ERT_Picture:
	case ERT_Text:
	case ERT_TestChart:
	case ERT_Subtitle:
	case ERT_Teleprompter:
		return true;
	default:
		break;
	}
	return false;
}

// 是否有原始时长
static bool IsAudioVideo(int nType)
{
	switch (nType)
	{
	case ERT_Video:
	case ERT_Audio:
		return true;
	default:
		return false;
	}
	return false;
}

// 是否是小工具
static bool IsToolResource(int nType)
{
	switch (nType)
	{
	case ERT_Text:
	case ERT_TestChart:
	case ERT_Subtitle:
	case ERT_Teleprompter:
		return true;
	default:
		break;
	}

	return false;
}

// 是否支持特效(创建的渲染节点为FxRenderItem的子类)
static bool IsSupportFx(int nType)
{
	switch (nType)
	{
	case ERT_Video:
	case ERT_Picture:
	case ERT_VideoCapture:
	case ERT_WebMedia:
	case ERT_Ndi:
		return true;
	default:
		return false;
	}
	return false;
}

// 是否支持滚动(创建的渲染节点为MediaScrollRenderItem的子类)
static bool IsSupportScroll(int nType)
{
	switch (nType)
	{
	case ERT_Text:
		return true;
	default:
		return false;
	}
	return false;
}

// 是否持续打开
static bool IsAlwaysOpenOption(int nType)
{
	switch (nType)
	{
	case ERT_VideoCapture:
	case ERT_AudioCapture:
	case ERT_WebMedia:
	case ERT_Ndi:
	case ERT_Teleprompter:
		return true;
	default:
		return false;
	}
	return false;
}

static bool IsSupportModifyScrollDirection(int nType)
{
	switch (nType)
	{
	case ERT_Text:
		return true;
	default:
		return false;
	}
	return false;
}

//素材项是否支持时长显示
static bool IsSupportShowDuration(int nType)
{
	switch (nType)
	{
	case ERT_Audio:
	case ERT_Video:
		return true;
	default:
		return false;
	}
	return false;
}


static bool IsSupportStitchGroupRes(int nType)
{
	switch (nType)
	{
	case ERT_Picture:
	case ERT_Text:
		return true;
	default:
		return false;
	}
	return false;
}

static bool IsNeedDrawDeafultPic(int nType)
{
	switch (nType)
	{
	case ERT_Subtitle:
	case ERT_Teleprompter:
		return true;
	default:
		return false;
	}
	return false;
}

// 素材库是否显示分辨率
static bool IsResWndShowReslution(int nType)
{
	switch (nType)
	{
	case ERT_Folder:
	case ERT_Audio:
	case ERT_AudioCapture:
		return false;
	default:
		break;
	}
	return true;
}

// 小工具是否支持幻彩字效果
static bool IsSupportColorTextRes(int nType)
{
	switch (nType)
	{
	case ERT_Text:
		return true;
	default:
		return false;
	}
	return false;
}

// 支持同步Seek进度条
static bool IsMediaProgressRes(int nType)
{
	switch (nType)
	{
	case ERT_Video:
	case ERT_Audio:
	case ERT_Picture:
	case ERT_VideoCapture:
	case ERT_AudioCapture:
	case ERT_WebMedia:
	case ERT_Text:
	case ERT_TestChart:
	case ERT_Ndi:
	case ERT_Subtitle:
		return true;
	default:
		return false;
	}
	return false;
}

static bool IsHasRealThumbnail(int nType)
{
	switch (nType)
	{
	case ERT_Video:
	case ERT_Picture:
		return true;
	default:
		return false;
	}
	return false;
}

// 预览区Render播放需要等待拿到纹理类型
static bool IsNeedWaitTexRenderType(int nType)
{
	switch (nType)
	{
	case ERT_Video:
	case ERT_Audio:
	case ERT_Picture:
		return true;
	default:
		break;
	}
	return false;
}

static bool IsNeedShowAudioChannelMap(int nType)
{
	switch (nType)
	{
	case ERT_Video:
	case ERT_Audio:
		return true;
	default:
		break;
	}
	return false;
}

// 素材播放支持Seek操作
static bool IsResSupportSeek(int nType)
{
	switch (nType)
	{
	case ERT_Video:
	case ERT_Audio:
	case ERT_Teleprompter:
	case ERT_Subtitle:
		return true;
	default:
		break;
	}
	return false;
}

// 素材库筛选-是否为其他素材
static bool IsOtherRes(int nType)
{
	switch (nType)
	{
	case ERT_Video:
	case ERT_Audio:
	case ERT_Picture:
		return false;
	default:
		break;
	}
	return true;
}

// 是否支持媒体绑定
static bool IsSupportMediaBind(int nType)
{
	switch (nType)
	{
	case ERT_Subtitle:
	case ERT_Teleprompter:
		return true;
	default:
		break;
	}

	return false;
}

//是否支持拖拽生成媒体
static bool IsSupportCreateMedia(int nType)
{
	switch (nType)
	{
	case ERT_Video:
	case ERT_Picture:
	case ERT_TestChart:
	case ERT_Audio:
	case ERT_AudioCapture:
	case ERT_VideoCapture:
	case ERT_WebMedia:
	case ERT_Text:
	case ERT_Ndi:
	case ERT_Subtitle:
	case ERT_Teleprompter:
		return true;
	default:
		break;
	}

	return false;
}
// 素材是否需要多机传输同步
static bool IsNeedsFileTransfer(int resType)
{
	switch (resType)
	{
	case ERT_Video:
	case ERT_Picture:
	case ERT_Audio:
		return true;
	default:
		return false;
	}
}

enum EPicOptimizeType		// 图片优化
{
	EPOT_1_1 = 0,
	EPOT_1_2,
	EPOT_1_4,
	EPOT_1_8,
	EPOT_1_16,
	EPOT_Auto,
};

#define MIN_PIC_OPT_WIDTH		(1920)
#define MIN_PIC_OPT_HEIGHT		(1080)

#define AUTO_PIC_OPT_WIDTH		(3840)
#define AUTO_PIC_OPT_HEIGHT		(2160)

// 行间距类型
enum ELineSpacingType
{
	ELST_SingleSpacing = 0,	// 单倍行距
	ELST_OnePointFivexSpacing,	// 1.5倍行距
	ELST_DoubleSpacing,  // 双倍行距
	ELST_Minimum,  // 最小值
	ELST_Fixed,	// 固定值
	ELST_MultipleSpacing,	// 多倍行距
};


//////////////////////////////文本素材类型///////////////////////////////////
enum ETextResourceType
{
	ETRT_TegularText = 0, // 常规文本
	ETRT_ColorText, // 幻彩字
};

////////////////////////////图片exif信息 旋转角度定义////////////////////////////////////
enum EExifOrientation
{
	EEO_1 = 1,			// 旋转0
	EEO_2,				// 水平翻转
	EEO_3,				// 顺时针旋转180
	EEO_4,				// 垂直翻转
	EEO_5,				// 顺时针旋转90 + 水平翻转
	EEO_6,				// 顺时针旋转90
	EEO_7,				// 顺时针旋转90 + 垂直翻转
	EEO_8				// 逆时针旋转90
};


// 字幕数据节点定义
struct SSubtitleDataNode
{
	__int64 nBeginTime;
	__int64 nEndTime;
	QString strText;

	SSubtitleDataNode() {
		nBeginTime = -1;
		nEndTime = -1;
	}

	SSubtitleDataNode(__int64 nBegin, __int64 nEnd, const QString &str)
		: nBeginTime(nBegin)
		, nEndTime(nEnd)
		, strText(str)
	{
	}
};

struct STeleprompterItem
{
	int nDuration;
	QString strText;

	STeleprompterItem() {
		nDuration = 2000;
		strText = "";
	}
	STeleprompterItem(const QString &str)
		: strText(str)
	{
		nDuration = 2000;
	}

	STeleprompterItem(const STeleprompterItem& other)
	{
		nDuration = other.nDuration;
		strText = other.strText;
	}

	STeleprompterItem& operator=(const STeleprompterItem& other)
	{
		nDuration = other.nDuration;
		strText = other.strText;

		return *this;
	}
};

// 同步或者异步Seek
enum EProgressSeekType
{
	EPST_Sync = 0,			//同步
	EPST_ASync,			//独立
	EPST_Count
};

//创建测试图数据数据结构
//基本信息
//struct SBaseInfo
//{
//	int   m_iWidth = 1920;		//高度
//	int   m_iHeight = 1080;	//高度
//	int   m_iRow = 9;		//行
//	int   m_iCol = 16;		//列
//};
//
////颜色模式
//enum EColorMode
//{
//	PURECOLOR = 0,     //纯色模式
//	GRADIENT,          //渐变模式
//	CHESSBOARD,        //棋盘模式
//	TESTCARD,          //测试卡模式
//};
//
////ORDER模式
//enum EOrderMode
//{
//	LETTER = 0,      //字母模式
//	NUMBER           //数字
//};
//
////LOGO模式
//enum ELogoMode
//{
//	GRADSHOW = 0,     //gradshow模式
//	IDENTIFIER,       //编号模式
//	LOGO,             //logo模式
//};
//
////模式选择
//struct SColorModeSelect
//{
//	EColorMode  eColorMode; //当前颜色模式
//	//---------------------颜色模式数据---------------------------------
//	QColor m_pureColor;//纯色模式
//	QColor m_gradientColor1;	//渐变模式
//	QColor m_gradientColor2;
//	QColor m_chessColor1;      //棋盘模式
//	QColor m_chessColor2;
//	SColorModeSelect() :
//		eColorMode(PURECOLOR),
//		m_pureColor(QColor(218, 158, 48)),
//		m_gradientColor1(QColor(0, 242, 96)),
//		m_gradientColor2(QColor(0, 125, 241)),
//		m_chessColor1(QColor(76, 255, 229)),
//		m_chessColor2(QColor(86, 108, 234))
//	{
//
//	}
//};
//
////序号
//struct SOrder
//{
//	bool   m_bSelected = false;                     //是否开启序号模式
//	QColor m_color = QColor(0, 0, 0);			    //序号模式颜色
//	int    m_iMode = EOrderMode::LETTER;            //默认字母模式
//	int    m_iFontSize = 22;                        //序号字体大小
//};
//
////分割线
//struct SSplineLine
//{
//	bool   m_bSelected = false;								//是否开启分割线模式
//	QColor m_color = QColor(240, 240, 240);		            //分割线模式颜色
//	int    m_iWidth = 1;					                //分割线宽度
//};
//
////边框
//struct SCheek
//{
//	bool   m_bSelected = true;                          //是否开启边框模式
//	QColor m_color = QColor(240, 240, 240);			    //边框模式颜色
//	int    m_iWidth = 8;					            //边框宽度
//};
//
////圆
//struct SCircle
//{
//	bool   m_bSelected = true;                               //是否开启圆模式
//	QColor m_color = QColor(240, 240, 240);		             //圆模式颜色
//	int    m_iWidth = 10;					                 //圆宽度
//	int    m_iRadius = 450;					                 //圆半径
//};
//
////对角线
//struct SDiagonal
//{
//	bool   m_bSelected = true;                      //是否开启对角线模式
//	QColor m_color = QColor(240, 240, 240);		    //对角线模式颜色
//	int    m_iWidth = 10;					        //对角线宽度
//};
//
////分辨率
//struct SResolution
//{
//	bool   m_bSelected = true;                           //是否开启分辨率模式
//	QColor m_color = QColor(0, 0, 0);					 //分辨率模式颜色
//	int    m_iFontSize = 43;					         //分辨率字体大小
//};
//
////箱体个数
//struct SBoxNum
//{
//	bool   m_bSelected = false;                           //是否开启箱体个数
//	QColor m_color = QColor(0, 0, 0);					  //箱体个数模式颜色
//	int    m_iFontSize = 43;							  //箱体个数字体大小
//};
//
//
////logo设置信息
//struct SLogo
//{
//	bool    m_bSelected = true;                          //是否设置logo
//	int     m_iMode = ELogoMode::GRADSHOW;				//logo模式
//	int     m_iZoom = 100;							    //缩放
//	int     m_iTransparency = 100;                      //透明度
//	int     m_iX = 960;                                //X坐标
//	int     m_iY = 421;                                //Y坐标
//	QString m_strGrandShowPath = ":/dark/images/ResourceWnd/TestChart/logo.png";                   //默认 logo 路径
//	QString m_strLogPath = "";                         //logo路径
//	QString m_strIdentifier = "A";                     //编号
//	QColor  m_color = QColor(0, 0, 0);		           //箱体个数模式颜色
//	int     m_iFontSize = 100;                         //字号
//};
//
////文字信息
//struct STextInfo
//{
//	bool   m_bSelected = true;                    //是否设置文字
//	QString m_strText = COMPANY_WEBSITE;		  //文字信息
//	int     m_iFontSize = 60;                     //字号
//	QColor  m_color = QColor(0, 0, 0);		      //箱体个数模式颜色
//};
//
////其余颜色模式
//enum ERestMode
//{
//	ORDER = 0,  //序号
//	SPLITLINE,  //分割线
//	CHEEK,      //边框
//	CIRCLE,     //圆
//	DIAGONAL,   //对角线
//	RESOLUTIONINFO, //分辨率
//	BOXNUM,     //箱体个数
//	LOGOINFO,   //LOGO信息
//	TEXTINFO,   //文字信息
//};
//
//typedef struct STestChartData
//{
//	SBaseInfo  baseInfo;                //基本信息 
//	SColorModeSelect  colorModeSelect;  //颜色模式 
//	SOrder  order;                      //序号 
//	SSplineLine   spline;				//分隔线
//	SCheek  cheek;                      //边框 
//	SCircle circle;                     //圆 
//	SDiagonal diagonal;                 //对角线
//	SResolution  resolution;			//分辨率
//	SBoxNum    boxNum;                  //箱体数
//	SLogo      LogoInfo;                //logo信息
//	STextInfo   textInfo;               //文字信息
//	STestChartData()
//	{
//
//	}
//
//	QJsonObject STestChartData::SaveDataToJson() const
//	{
//		QJsonObject root;
//
//		// 基本信息
//		QJsonObject baseInfoObj;
//		baseInfoObj.insert("ContentW", baseInfo.m_iWidth);
//		baseInfoObj.insert("ContentH", baseInfo.m_iHeight);
//		baseInfoObj.insert("ContentRow", baseInfo.m_iRow);
//		baseInfoObj.insert("ContentCol", baseInfo.m_iCol);
//		root.insert("BaseInfo", baseInfoObj);
//
//		// 颜色模式选择
//		QJsonObject colorModeObj;
//		colorModeObj.insert("ColorMode", static_cast<int>(colorModeSelect.eColorMode));
//		colorModeObj.insert("PureColor", ColorToString(colorModeSelect.m_pureColor));
//		colorModeObj.insert("GradientColor1", ColorToString(colorModeSelect.m_gradientColor1));
//		colorModeObj.insert("GradientColor2", ColorToString(colorModeSelect.m_gradientColor2));
//		colorModeObj.insert("ChessColor1", ColorToString(colorModeSelect.m_chessColor1));
//		colorModeObj.insert("ChessColor2", ColorToString(colorModeSelect.m_chessColor2));
//		root.insert("ColorModeSelect", colorModeObj);
//
//		// 序号信息
//		QJsonObject orderObj;
//		orderObj.insert("bSelected", order.m_bSelected);
//		orderObj.insert("Color", ColorToString(order.m_color));
//		orderObj.insert("iMode", order.m_iMode);
//		orderObj.insert("iFontSize", order.m_iFontSize);
//		root.insert("Order", orderObj);
//
//		// 分割线
//		QJsonObject splineObj;
//		splineObj.insert("bSelected", spline.m_bSelected);
//		splineObj.insert("Color", ColorToString(spline.m_color));
//		splineObj.insert("iWidth", spline.m_iWidth);
//		root.insert("SplineLine", splineObj);
//
//		// 边框
//		QJsonObject cheekObj;
//		cheekObj.insert("bSelected", cheek.m_bSelected);
//		cheekObj.insert("Color", ColorToString(cheek.m_color));
//		cheekObj.insert("iWidth", cheek.m_iWidth);
//		root.insert("Cheek", cheekObj);
//
//		// 圆
//		QJsonObject circleObj;
//		circleObj.insert("bSelected", circle.m_bSelected);
//		circleObj.insert("Color", ColorToString(circle.m_color));
//		circleObj.insert("iWidth", circle.m_iWidth);
//		circleObj.insert("iRadius", circle.m_iRadius);
//		root.insert("Circle", circleObj);
//
//		// 对角线
//		QJsonObject diagonalObj;
//		diagonalObj.insert("bSelected", diagonal.m_bSelected);
//		diagonalObj.insert("Color", ColorToString(diagonal.m_color));
//		diagonalObj.insert("iWidth", diagonal.m_iWidth);
//		root.insert("Diagonal", diagonalObj);
//
//		// 分辨率
//		QJsonObject resolutionObj;
//		resolutionObj.insert("bSelected", resolution.m_bSelected);
//		resolutionObj.insert("Color", ColorToString(resolution.m_color));
//		resolutionObj.insert("iFontSize", resolution.m_iFontSize);
//		root.insert("Resolution", resolutionObj);
//
//		// 箱体数
//		QJsonObject boxNumObj;
//		boxNumObj.insert("bSelected", boxNum.m_bSelected);
//		boxNumObj.insert("Color", ColorToString(boxNum.m_color));
//		boxNumObj.insert("iFontSize", boxNum.m_iFontSize);
//		root.insert("BoxNum", boxNumObj);
//
//		// Logo
//		QJsonObject logoObj;
//		logoObj.insert("bSelected", LogoInfo.m_bSelected);
//		logoObj.insert("iMode", LogoInfo.m_iMode);
//		logoObj.insert("iZoom", LogoInfo.m_iZoom);
//		logoObj.insert("iTransparency", LogoInfo.m_iTransparency);
//		logoObj.insert("iX", LogoInfo.m_iX);
//		logoObj.insert("iY", LogoInfo.m_iY);
//		logoObj.insert("strGrandShowPath", LogoInfo.m_strGrandShowPath);
//		logoObj.insert("strLogPath", LogoInfo.m_strLogPath);
//		logoObj.insert("strIdentifier", LogoInfo.m_strIdentifier);
//		logoObj.insert("Color", ColorToString(LogoInfo.m_color));
//		logoObj.insert("iFontSize", LogoInfo.m_iFontSize);
//		root.insert("Logo", logoObj);
//
//		// 文字
//		QJsonObject textObj;
//		textObj.insert("bSelected", textInfo.m_bSelected);
//		textObj.insert("strText", textInfo.m_strText);
//		textObj.insert("Color", ColorToString(textInfo.m_color));
//		textObj.insert("iFontSize", textInfo.m_iFontSize);
//		root.insert("TextInfo", textObj);
//
//		return root;
//	}
//	
//	void STestChartData::LoadDataFromJson(const QJsonObject& root)
//	{
//		// 解析基本信息
//		if (root.contains("BaseInfo") && root.value("BaseInfo").isObject()) {
//			QJsonObject baseInfoObj = root.value("BaseInfo").toObject();
//			baseInfo.m_iWidth = baseInfoObj.value("ContentW").toInt(0);
//			baseInfo.m_iHeight = baseInfoObj.value("ContentH").toInt(0);
//			baseInfo.m_iRow = baseInfoObj.value("ContentRow").toInt(0);
//			baseInfo.m_iCol = baseInfoObj.value("ContentCol").toInt(0);
//		}
//
//		// 解析颜色模式
//		if (root.contains("ColorModeSelect") && root.value("ColorModeSelect").isObject()) {
//			QJsonObject cmObj = root.value("ColorModeSelect").toObject();
//			colorModeSelect.eColorMode = static_cast<EColorMode>(cmObj.value("ColorMode").toInt(0));
//			colorModeSelect.m_pureColor = StringToColor(cmObj.value("PureColor").toString());
//			colorModeSelect.m_gradientColor1 = StringToColor(cmObj.value("GradientColor1").toString());
//			colorModeSelect.m_gradientColor2 = StringToColor(cmObj.value("GradientColor2").toString());
//			colorModeSelect.m_chessColor1 = StringToColor(cmObj.value("ChessColor1").toString());
//			colorModeSelect.m_chessColor2 = StringToColor(cmObj.value("ChessColor2").toString());
//		}
//
//		// 解析序号信息
//		if (root.contains("Order") && root.value("Order").isObject()) {
//			QJsonObject obj = root.value("Order").toObject();
//			order.m_bSelected = obj.value("bSelected").toBool(false);
//			order.m_color = StringToColor(obj.value("Color").toString());
//			order.m_iMode = obj.value("iMode").toInt(0);
//			order.m_iFontSize = obj.value("iFontSize").toInt(12);
//		}
//
//		// 解析分割线信息
//		if (root.contains("SplineLine") && root.value("SplineLine").isObject()) {
//			QJsonObject obj = root.value("SplineLine").toObject();
//			spline.m_bSelected = obj.value("bSelected").toBool(false);
//			spline.m_color = StringToColor(obj.value("Color").toString());
//			spline.m_iWidth = obj.value("iWidth").toInt(1);
//		}
//
//		// 解析边框信息
//		if (root.contains("Cheek") && root.value("Cheek").isObject()) {
//			QJsonObject obj = root.value("Cheek").toObject();
//			cheek.m_bSelected = obj.value("bSelected").toBool(false);
//			cheek.m_color = StringToColor(obj.value("Color").toString());
//			cheek.m_iWidth = obj.value("iWidth").toInt(1);
//		}
//
//		// 解析圆信息
//		if (root.contains("Circle") && root.value("Circle").isObject()) {
//			QJsonObject obj = root.value("Circle").toObject();
//			circle.m_bSelected = obj.value("bSelected").toBool(false);
//			circle.m_color = StringToColor(obj.value("Color").toString());
//			circle.m_iWidth = obj.value("iWidth").toInt(1);
//			circle.m_iRadius = obj.value("iRadius").toInt(10);
//		}
//
//		// 解析对角线信息
//		if (root.contains("Diagonal") && root.value("Diagonal").isObject()) {
//			QJsonObject obj = root.value("Diagonal").toObject();
//			diagonal.m_bSelected = obj.value("bSelected").toBool(false);
//			diagonal.m_color = StringToColor(obj.value("Color").toString());
//			diagonal.m_iWidth = obj.value("iWidth").toInt(1);
//		}
//
//		// 解析分辨率信息
//		if (root.contains("Resolution") && root.value("Resolution").isObject()) {
//			QJsonObject obj = root.value("Resolution").toObject();
//			resolution.m_bSelected = obj.value("bSelected").toBool(false);
//			resolution.m_color = StringToColor(obj.value("Color").toString());
//			resolution.m_iFontSize = obj.value("iFontSize").toInt(12);
//		}
//
//		// 解析箱体数信息
//		if (root.contains("BoxNum") && root.value("BoxNum").isObject()) {
//			QJsonObject obj = root.value("BoxNum").toObject();
//			boxNum.m_bSelected = obj.value("bSelected").toBool(false);
//			boxNum.m_color = StringToColor(obj.value("Color").toString());
//			boxNum.m_iFontSize = obj.value("iFontSize").toInt(12);
//		}
//
//		// 解析 Logo 信息
//		if (root.contains("Logo") && root.value("Logo").isObject()) {
//			QJsonObject obj = root.value("Logo").toObject();
//			LogoInfo.m_bSelected = obj.value("bSelected").toBool(false);
//			LogoInfo.m_iMode = obj.value("iMode").toInt(0);
//			LogoInfo.m_iZoom = obj.value("iZoom").toInt(100);
//			LogoInfo.m_iTransparency = obj.value("iTransparency").toInt(100);
//			LogoInfo.m_iX = obj.value("iX").toInt(0);
//			LogoInfo.m_iY = obj.value("iY").toInt(0);
//			LogoInfo.m_strGrandShowPath = obj.value("strGrandShowPath").toString();
//			LogoInfo.m_strLogPath = obj.value("strLogPath").toString();
//			LogoInfo.m_strIdentifier = obj.value("strIdentifier").toString();
//			LogoInfo.m_color = StringToColor(obj.value("Color").toString());
//			LogoInfo.m_iFontSize = obj.value("iFontSize").toInt(12);
//		}
//
//		// 解析文字信息
//		if (root.contains("TextInfo") && root.value("TextInfo").isObject()) {
//			QJsonObject obj = root.value("TextInfo").toObject();
//			textInfo.m_bSelected = obj.value("bSelected").toBool(false);
//			textInfo.m_strText = obj.value("strText").toString("Text");
//			textInfo.m_color = StringToColor(obj.value("Color").toString());
//			textInfo.m_iFontSize = obj.value("iFontSize").toInt(12);
//		}
//	}
//
//	void LoadDataFromXml(const QDomNode &root)
//	{
//		QDomNode Item;
//		QDomNode Node;
//
//		//解析基本信息
//		Item = root.namedItem("BaseInfo");
//		baseInfo.m_iWidth = Item.namedItem("ContentW").firstChild().nodeValue().toInt();
//		baseInfo.m_iHeight = Item.namedItem("ContentH").firstChild().nodeValue().toInt();
//		baseInfo.m_iRow = Item.namedItem("ContentRow").firstChild().nodeValue().toInt();
//		baseInfo.m_iCol = Item.namedItem("ContentCol").firstChild().nodeValue().toInt();
//
//		//解析颜色模式
//		Item = root.namedItem("ColorModeSelect");
//		colorModeSelect.eColorMode = (EColorMode)Item.namedItem("ColorMode").firstChild().nodeValue().toInt();
//		colorModeSelect.m_pureColor = StringToColor(Item.namedItem("PureColor").firstChild().nodeValue());
//		colorModeSelect.m_gradientColor1 = StringToColor(Item.namedItem("GradientColor1").firstChild().nodeValue());
//		colorModeSelect.m_gradientColor2 = StringToColor(Item.namedItem("GradientColor2").firstChild().nodeValue());
//		colorModeSelect.m_chessColor1 = StringToColor(Item.namedItem("ChessColor1").firstChild().nodeValue());
//		colorModeSelect.m_chessColor2 = StringToColor(Item.namedItem("ChessColor2").firstChild().nodeValue());
//
//		//解析序号信息
//		Item = root.namedItem("Order");
//		order.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
//		order.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
//		order.m_iMode = Item.namedItem("iMode").firstChild().nodeValue().toInt();
//		order.m_iFontSize = Item.namedItem("iFontSize").firstChild().nodeValue().toInt();
//
//		//解析分割线信息
//		Item = root.namedItem("SplineLine");
//		spline.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
//		spline.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
//		spline.m_iWidth = Item.namedItem("iWidth").firstChild().nodeValue().toInt();
//
//		//解析边框信息
//		Item = root.namedItem("Cheek");
//		cheek.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
//		cheek.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
//		cheek.m_iWidth = Item.namedItem("iWidth").firstChild().nodeValue().toInt();
//
//		//解析圆信息
//		Item = root.namedItem("Circle");
//		circle.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
//		circle.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
//		circle.m_iWidth = Item.namedItem("iWidth").firstChild().nodeValue().toInt();
//		circle.m_iRadius = Item.namedItem("iRadius").firstChild().nodeValue().toInt();
//
//		//解析对角线信息
//		Item = root.namedItem("Diagonal");
//		diagonal.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
//		diagonal.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
//		diagonal.m_iWidth = Item.namedItem("iWidth").firstChild().nodeValue().toInt();
//
//		//解析分辨率信息
//		Item = root.namedItem("Resolution");
//		resolution.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
//		resolution.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
//		resolution.m_iFontSize = Item.namedItem("iFontSize").firstChild().nodeValue().toInt();
//
//		//解析箱体数信息
//		Item = root.namedItem("BoxNum");
//		boxNum.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
//		boxNum.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
//		boxNum.m_iFontSize = Item.namedItem("iFontSize").firstChild().nodeValue().toInt();
//
//		//解析logo信息
//		Item = root.namedItem("Logo");
//		LogoInfo.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
//		LogoInfo.m_iMode = Item.namedItem("iMode").firstChild().nodeValue().toInt();
//		LogoInfo.m_iZoom = Item.namedItem("iZoom").firstChild().nodeValue().toInt();
//		LogoInfo.m_iTransparency = Item.namedItem("iTransparency").firstChild().nodeValue().toInt();
//		LogoInfo.m_iX = Item.namedItem("iX").firstChild().nodeValue().toInt();
//		LogoInfo.m_iY = Item.namedItem("iY").firstChild().nodeValue().toInt();
//		LogoInfo.m_strGrandShowPath = Item.namedItem("strGrandShowPath").firstChild().nodeValue();
//		LogoInfo.m_strLogPath = Item.namedItem("strLogPath").firstChild().nodeValue();
//		LogoInfo.m_strIdentifier = Item.namedItem("strIdentifier").firstChild().nodeValue();
//		LogoInfo.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
//		LogoInfo.m_iFontSize = Item.namedItem("iFontSize").firstChild().nodeValue().toInt();
//
//		//解析文字信息
//		Item = root.namedItem("TextInfo");
//		textInfo.m_bSelected = Item.namedItem("bSelected").firstChild().nodeValue().toInt();
//		textInfo.m_strText = Item.namedItem("strText").firstChild().nodeValue();
//		textInfo.m_color = StringToColor(Item.namedItem("Color").firstChild().nodeValue());
//		textInfo.m_iFontSize = Item.namedItem("iFontSize").firstChild().nodeValue().toInt();
//
//	}
//
//}TestChartData_t;
