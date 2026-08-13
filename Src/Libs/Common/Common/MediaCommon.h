#pragma once

#include <unordered_map>
#include <QFont>
#include <QColor>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QFontDatabase>
#include "CommonFunctionApi.h"
#include <chrono>
using namespace std::chrono;

//媒体位置
#define MAX_MEDIA_POS_X 999999
#define MAX_MEDIA_POS_Y 999999
#define MIN_MEDIA_POS_X -999999
#define MIN_MEDIA_POS_Y -999999


//媒体分辨率最大值
#define MAX_MEDIA_SIZE_W 999999
#define MAX_MEDIA_SIZE_H 999999
#define MIN_MEDIA_SIZE_W 1
#define MIN_MEDIA_SIZE_H 1

//文本最大分辨率
#define MAX_RESOURCE_WIDTH	8192
#define MAX_RESOURCE_HEIGHT	8192

enum EnTransFxStage
{
	ETFS_In,
	ETFS_Normal,
	ETFS_Out,
};

struct STransInfo
{
	int nTransType;			// 转场特效类型
	EnTransFxStage stage;		//转场阶段
	float fProgress;			//转场进度
	bool bTransition;		//是否需要转场

	STransInfo()
	{
		Reset();
	}

	void Reset()
	{
		stage = ETFS_Normal;
		fProgress = 1.0f;
		bTransition = true;
		nTransType = -1;
	}
};

// 媒体填充类型
enum EMediaFillType
{
	EMFT_FullWindow = 0,	// 全屏
	EMFT_KeepRatio,			// 保持比例
	EMFT_NoScale,			// 无放缩
};

// 媒体混合类型
enum EMediaBlendType
{
	EMBT_Cover = 0,	// 覆盖
	EMBT_Overlay, // 叠加
};

enum EMediaClipFlipMode : unsigned int // 翻转模式
{
	ECFM_NoFlip = 0x0000,
	ECFM_HorFlip = 0x0001, // 左右翻转
	ECFM_VerFlip = 0x0010, // 上下翻转
};

//滚动方式
enum EScrollType
{
	EST_Stop = 0,		//静止
	EST_Left2Right,	//从左到右
	EST_Top2Bottom,	//从上到下
	EST_Right2Left,	//从右到左
	EST_Bottom2Top,	//从下到上
};

//幻彩字属性界面-填充-填充方式
enum
{
	en_Fill_SolidColor = 0,		//纯色填充
	en_Fill_Gradient,			//渐变填充
	en_Fill_Texture,			//纹理填充
};

//填充方向
enum EGradientDirection
{
	EGD_Horizontal = 0,
	EGD_Vertical,
};

//幻彩字属性界面-填充-填充滚动方式
enum ColotTextFillScroll
{
	en_Fill_Static = 0,
	en_Fill_MoveLeft,
	en_Fill_MoveRight,
	en_Fill_MoveUp,
	en_Fill_MoveDown,
	en_Fill_OpenLeft2Right,
	en_Fill_CloseLeft2Right,
	en_Fill_OpenUp2Down,
	en_Fill_CloseUp2Down,
	en_Fill_Clockwise,
	en_Fill_Counterclockwise,
};

enum EMediaPageTurnType
{
	EMPTT_Manul = 0,	// 手动
	EMPTT_Auto,		// 自动
	EMPTT_Scroll,	// 滚动
};

// 媒体播放中数据，进度，时长，状态
struct SMediaPlayData
{
	__int64 nProgress;	// 进度
	__int64 nDuration;	// 时长
	int nPlayState;	// 播放状态（控制状态）

	SMediaPlayData() {
		nProgress = 0;
		nDuration = 0;
		nPlayState = -1;
	}

	SMediaPlayData(__int64 nPro, int nState, __int64 nDura) :
		nProgress(nPro), nPlayState(nState), nDuration(nDura) {

	}
};


// 媒体翻页特效信息
struct SMediaPageTurnFxInfo
{
	int nType;				// 翻页特效index
	int nDuration;	// 翻页特效时长ms

	// 入场特效类型
	// 入场时间
	// 出场类型
	// 出场时间

	SMediaPageTurnFxInfo()
	{
		nType = 0;
		nDuration = 1000;
	}

	SMediaPageTurnFxInfo(const SMediaPageTurnFxInfo& other)
	{
		*this = other;
	}

	void Reset()
	{
		nType = 0;
		nDuration = 1000;
	}

	SMediaPageTurnFxInfo(std::shared_ptr<SMediaPageTurnFxInfo> pOther)
	{
		CpoyProperty(pOther);
	}

	void CpoyProperty(std::shared_ptr<SMediaPageTurnFxInfo> pOther)
	{
		nType = pOther->nType;
		nDuration = pOther->nDuration;
	}

	SMediaPageTurnFxInfo& operator=(const SMediaPageTurnFxInfo& other)
	{
		nType = other.nType;
		nDuration = other.nDuration;

		return *this;
	}

	QJsonObject SaveDataToJson()
	{
		QJsonObject obj;

		obj.insert("nPageTurnFxType", nType);
		obj.insert("nPageTurnFxDuration", nDuration );
	
		return obj;
	}
	void LoadDataFromJson(const QJsonObject& obj)
	{
		nType = obj.value("nPageTurnFxType").toInt();
		nDuration = obj.value("nPageTurnFxDuration").toInt();
	}
};

enum EExternControllType
{
	EECT_Normal = 0,	// 按照小工具自己滚动方式滚动
	EECT_KeepCurrent,	// 保持当前状态
	EECT_Stop,	// 停止
};

// 媒体滚动参数
struct SMediaScrollParam
{
	bool bIsDisableScrollType = false;	// 是否禁用滚动类型（仅文本需要）
	int	 nScrollType = 0;			// 滚动类型
	int  nScrollTypeCahce = 0;		// 滚动缓存
	float fScrollSpeed = 1.0f;		// 滚动速度
	bool bEndToEnd = false;			// 首尾相接
	int	 nEndSpace = 0;				// 首尾间隔

	float fNowX = 0;					//当前X
	float fNowY = 0;					//当前Y

	int	 nScrollWndWidth = 0;		// 滚动窗口宽度
	int	 nScrollWndHeight = 0;		// 滚动窗口高度
	int	 nTotalWidth = 0;			// 文本总长度 
	int	 nTotalHeight = 0;			// 文本总高度

	QColor bgColor = {0,0,0,0};
	bool bBkClrEnable = false;

	int nExternControllType = EECT_Normal;

	bool bParamUpdate = false;		//滚动类型是否更新过,用于渲染的时候的线程同步

	SMediaScrollParam()
	{
		bIsDisableScrollType = false;
		nScrollType = 0;
		fScrollSpeed = 1.0f;
		bEndToEnd = false;
		nEndSpace = 5;
		nScrollWndHeight = 0;
		nScrollWndWidth = 0;
		nTotalWidth = 0;
		nTotalHeight = 0;

		fNowX = 0;
		fNowY = 0;
		nScrollTypeCahce = 0;
	}

	SMediaScrollParam(const SMediaScrollParam& other)
	{
		*this = other;
	}

	void Reset()
	{
		bIsDisableScrollType = false;
		nScrollType = 0;
		fScrollSpeed = 1.0f;
		bEndToEnd = false;
		nEndSpace = 5;
		nScrollWndHeight = 0;
		nScrollWndWidth = 0;
		nTotalWidth = 0;
		nTotalHeight = 0;

		fNowX = 0;
		fNowY = 0;
		bParamUpdate = false;
		nScrollTypeCahce = 0;

		bBkClrEnable = false;
	}

	SMediaScrollParam(std::shared_ptr<SMediaScrollParam> pOther)
	{
		CpoyProperty(pOther);
	}

	void CpoyProperty(std::shared_ptr<SMediaScrollParam> pOther)
	{
		bIsDisableScrollType = pOther->bIsDisableScrollType;
		nScrollType = pOther->nScrollType;
		fScrollSpeed = pOther->fScrollSpeed;
		bEndToEnd = pOther->bEndToEnd;
		nEndSpace = pOther->nEndSpace;
		nScrollWndWidth = pOther->nScrollWndWidth;
		nScrollWndHeight = pOther->nScrollWndHeight;
		nTotalWidth = pOther->nTotalWidth;
		nTotalHeight = pOther->nTotalHeight;
		fNowX = pOther->fNowX;
		fNowY = pOther->fNowY;

		bParamUpdate = pOther->bParamUpdate;
		nExternControllType = pOther->nExternControllType;

		bBkClrEnable = pOther->bBkClrEnable;
		bgColor = pOther->bgColor;
	}

	SMediaScrollParam& operator=(const SMediaScrollParam& other)
	{
		bIsDisableScrollType = other.bIsDisableScrollType;
		nScrollType = other.nScrollType;
		fScrollSpeed = other.fScrollSpeed;
		bEndToEnd = other.bEndToEnd;
		nEndSpace = other.nEndSpace;
		nScrollWndWidth = other.nScrollWndWidth;
		nScrollWndHeight = other.nScrollWndHeight;
		nTotalWidth = other.nTotalWidth;
		nTotalHeight = other.nTotalHeight;
		fNowX = other.fNowX;
		fNowY = other.fNowY;

		bParamUpdate = other.bParamUpdate;
		nExternControllType = other.nExternControllType;

		bBkClrEnable = other.bBkClrEnable;
		bgColor = other.bgColor;

		return *this;
	}

	QJsonObject SaveDataToJson()
	{
		QJsonObject obj;

		obj.insert("bIsDisableScrollType", bIsDisableScrollType);
		obj.insert("bEndToEnd", bEndToEnd);
		obj.insert("fScrollSpeed", fScrollSpeed + FLT_EPSILON);
		obj.insert("nEndSpace", nEndSpace);
		obj.insert("nScrollType", nScrollType);
		obj.insert("ScrollWndWidth", nScrollWndWidth);
		obj.insert("ScrollWndHeight", nScrollWndHeight);
		obj.insert("TotalTextWidth", nTotalWidth);
		obj.insert("TotalTextHeight", nTotalHeight);

		return obj;
	}
	void LoadDataFromJson(const QJsonObject& obj)
	{
		bIsDisableScrollType = obj.value("bIsDisableScrollType").toBool();
		bEndToEnd = obj.value("bEndToEnd").toBool();
		fScrollSpeed = obj.value("fScrollSpeed").toDouble();
		nEndSpace = obj.value("nEndSpace").toInt();
		nScrollType = obj.value("nScrollType").toInt();
		nScrollWndWidth = obj.value("ScrollWndWidth").toInt();
		nScrollWndHeight = obj.value("ScrollWndHeight").toInt();
		nTotalWidth = obj.value("TotalTextWidth").toInt();
		nTotalHeight = obj.value("TotalTextHeight").toInt();
	}
};

//幻彩字-填充参数
struct SColorTextFillParam
{
	bool bFillEnable = false;						// 小工具是否启用了填充
	int	 nFillType = 0;								// 填充类型

	QColor FillColor;								// 纯色填充颜色/渐变填充颜色选择按钮
	QVector<QColor>	 m_vecFillColor;				// 渐变填充颜色组
	int nSelectedColorIndex;						// 选择的渐变颜色索引
	QVector<double> m_vecColorPosition;				// 渐变填充颜色位置
	double	 dOpacity = 0;							// 透明度
	int	 nGradientColorNum = 0;						// 渐变填充颜色数量
	QString strTexturePath = nullptr;				// 纹理填充-纹理路径
	int nImportTexturePathNum = 0;					// 纹理填充-导入纹理路径数量
	QVector<QString> m_vecImportTexturePath;		// 纹理填充-导入纹理路径
	int nGradientDirection = 0;						// 渐变填充方向
	int nTextureFillType = 0;						// 纹理填充方式
	int nGradientScrollType = 0;							// 填充滚动方式
	int nTextureScrollType = 0;							// 填充滚动方式
	int nScrollSpeed = 1;						// 填充滚动速度

	SColorTextFillParam()
	{
		bFillEnable = false;
		nFillType = 0;
		dOpacity = 1;
		FillColor = QColor(0, 0, 0, dOpacity*255);

		nSelectedColorIndex = -1;
		nGradientColorNum = 2;
		m_vecFillColor.push_back(QColor(0x3297F3));
		m_vecFillColor.push_back(QColor(0xFCB701));

		m_vecColorPosition.push_back(0);
		m_vecColorPosition.push_back(1);
		
		strTexturePath = QString(":/dark/images/PropertyWnd/FillProperty/FillProperty_0/Checked/Normal_rect.png");
		
		nImportTexturePathNum = 0;

		nGradientDirection = 0;
		nTextureFillType = 0;
		nGradientScrollType = 0;
		nTextureScrollType = 0;
		nScrollSpeed = 1;
	}

	SColorTextFillParam(const SColorTextFillParam& other)
	{
		*this = other;
	}

	SColorTextFillParam& operator=(const SColorTextFillParam& other)
	{
		bFillEnable = other.bFillEnable;
		nFillType = other.nFillType;
		FillColor = other.FillColor;
		dOpacity = other.dOpacity;
		nGradientColorNum = other.nGradientColorNum;
		nSelectedColorIndex = other.nSelectedColorIndex;

		m_vecFillColor = other.m_vecFillColor;
		m_vecColorPosition = other.m_vecColorPosition;

		strTexturePath = other.strTexturePath;

		nImportTexturePathNum = other.nImportTexturePathNum;
		m_vecImportTexturePath = other.m_vecImportTexturePath;

		nGradientDirection = other.nGradientDirection;
		nTextureFillType = other.nTextureFillType;
		nGradientScrollType = other.nGradientScrollType;
		nTextureScrollType = other.nTextureScrollType;
		nScrollSpeed = other.nScrollSpeed;

		return *this;
	}

	QJsonObject SaveDataToJson()
	{
		QJsonObject obj;

		obj.insert("bFillEnable", bFillEnable);
		obj.insert("nFillType", nFillType);
		obj.insert("dOpacity", dOpacity);
		obj.insert("nGradientColorNum", nGradientColorNum);
		obj.insert("strTexturePath", strTexturePath);
		obj.insert("nSelectedColorIndex", nSelectedColorIndex);

		obj.insert("FillColorR", FillColor.red());
		obj.insert("FillColorG", FillColor.green());
		obj.insert("FillColorB", FillColor.blue());
		obj.insert("FillColorA", FillColor.alpha());

		for (int i = 0; i < nGradientColorNum; i++)
		{
			obj.insert("m_vecColorPosition_" + QString::number(i), m_vecColorPosition[i]);
		}


		for (int i = 0; i < nGradientColorNum; i++)
		{
			obj.insert("m_vecFillColorR_" + QString::number(i), m_vecFillColor[i].red());
			obj.insert("m_vecFillColorG_" + QString::number(i), m_vecFillColor[i].green());
			obj.insert("m_vecFillColorB_" + QString::number(i), m_vecFillColor[i].blue());
			obj.insert("m_vecFillColorA_" + QString::number(i), m_vecFillColor[i].alpha());
		}

		obj.insert("nImportTexturePathNum", nImportTexturePathNum);

		for (int i = 0; i < nImportTexturePathNum; i++)
		{
			obj.insert("vecImportTexturePath_" + QString::number(i), m_vecImportTexturePath[i]);
		}

		obj.insert("nGradientDirection", nGradientDirection);
		obj.insert("nTextureFillType", nTextureFillType);
		obj.insert("nGradientScrollType", nGradientScrollType);
		obj.insert("nTextureScrollType", nTextureScrollType);
		obj.insert("nScrollSpeed", nScrollSpeed);

		return obj;
	}
	
	void LoadDataFromJson(const QJsonObject& obj)
	{
		bFillEnable = obj.value("bFillEnable").toBool();
		nFillType = obj.value("nFillType").toInt();
		dOpacity = obj.value("dOpacity").toDouble();
		nGradientColorNum = obj.value("nGradientColorNum").toInt();
		strTexturePath = obj.value("strTexturePath").toString();
		nSelectedColorIndex = obj.value("nSelectedColorIndex").toInt();

		FillColor=QColor(
						obj.value("FillColorR").toInt(),
						obj.value("FillColorG").toInt(),
						obj.value("FillColorB").toInt(),
						obj.value("FillColorA").toInt()
						);

		m_vecColorPosition.resize(nGradientColorNum);
		m_vecFillColor.resize(nGradientColorNum);


		for (int i = 0; i < nGradientColorNum; i++)
		{
			m_vecColorPosition[i] = obj.value("m_vecColorPosition_" + QString::number(i)).toDouble();
		}

		for (int i = 0; i < nGradientColorNum; i++)
		{
			m_vecFillColor[i] = 
				QColor(
						obj.value("m_vecFillColorR_" + QString::number(i)).toInt(),
						obj.value("m_vecFillColorG_" + QString::number(i)).toInt(),
						obj.value("m_vecFillColorB_" + QString::number(i)).toInt(),
						obj.value("m_vecFillColorA_" + QString::number(i)).toInt()
					  );
		}
		nImportTexturePathNum = obj.value("nImportTexturePathNum").toInt();
		m_vecImportTexturePath.resize(nImportTexturePathNum);
		for (int i = 0; i < nImportTexturePathNum; i++)
		{
			m_vecImportTexturePath[i] = obj.value("vecImportTexturePath_" + QString::number(i)).toString();
		}

		nGradientDirection = obj.value("nGradientDirection").toInt();
		nTextureFillType = obj.value("nTextureFillType").toInt();
		nGradientScrollType = obj.value("nGradientScrollType").toInt();
		nTextureScrollType = obj.value("nTextureScrollType").toInt();
		nScrollSpeed = obj.value("nScrollSpeed").toInt();
	}
};

//幻彩字-描边参数
struct SColorTextOutlineParam
{
	bool	bOutlineEnable = false;				// 小工具是否启用了描边

	QColor	 OutlineColor;						// 颜色
	int		 nOutlineThickness = 0;					// 描边粗细
	double	 dOpacity = 0;							// 透明度
	
	SColorTextOutlineParam()
	{
		bOutlineEnable = false;
		nOutlineThickness = 5;
		dOpacity = 1;
		OutlineColor = QColor(0, 0, 0, dOpacity * 255);
	}

	SColorTextOutlineParam(const SColorTextOutlineParam& other)
	{
		*this = other;
	}

	SColorTextOutlineParam& operator=(const SColorTextOutlineParam& other)
	{
		bOutlineEnable = other.bOutlineEnable;
		OutlineColor = other.OutlineColor;
		nOutlineThickness = other.nOutlineThickness;
		dOpacity = other.dOpacity;

		return *this;
	}

	void CopyProperty(std::shared_ptr<SColorTextOutlineParam> ptr)
	{
		bOutlineEnable = ptr->bOutlineEnable;
		OutlineColor = ptr->OutlineColor;
		nOutlineThickness = ptr->nOutlineThickness;
		dOpacity = ptr->dOpacity;
	}

	QJsonObject SaveDataToJson()
	{
		QJsonObject obj;

		obj.insert("bOutlineEnable", bOutlineEnable);
		obj.insert("dOpacity", dOpacity);
		obj.insert("nOutlineThickness", nOutlineThickness);

		obj.insert("OutlineColorR", OutlineColor.red());
		obj.insert("OutlineColorG", OutlineColor.green());
		obj.insert("OutlineColorB", OutlineColor.blue());
		obj.insert("OutlineColorA", OutlineColor.alpha());

		return obj;
	}

	void LoadDataFromJson(const QJsonObject& obj)
	{
		bOutlineEnable = obj.value("bOutlineEnable").toBool();
		dOpacity = obj.value("dOpacity").toDouble();
		nOutlineThickness = obj.value("nOutlineThickness").toInt();

		OutlineColor = QColor(
			obj.value("OutlineColorR").toInt(),
			obj.value("OutlineColorG").toInt(),
			obj.value("OutlineColorB").toInt(),
			obj.value("OutlineColorA").toInt()
		);
	}
};

//幻彩字-阴影参数
struct SColorTextShadeParam
{
	bool	bShadeEnable = false;				// 小工具是否启用

	QColor	 ShadeColor;						// 颜色
	int		 nShadeAngle = 0;						// 阴影角度
	int		 nShadeDistance = 0;					// 阴影距离
	double	 dAmbiguity = 0;						// 模糊度

	SColorTextShadeParam()
	{
		bShadeEnable = false;
		ShadeColor = QColor(0, 0, 0,255);
		nShadeAngle = 45;
		nShadeDistance = 35;
		dAmbiguity = 0.74;
	}

	SColorTextShadeParam(const SColorTextShadeParam& other)
	{
		*this = other;
	}

	SColorTextShadeParam& operator=(const SColorTextShadeParam& other)
	{
		bShadeEnable = other.bShadeEnable;
		ShadeColor = other.ShadeColor;
		nShadeAngle = other.nShadeAngle;
		nShadeDistance = other.nShadeDistance;
		dAmbiguity = other.dAmbiguity;

		return *this;
	}

	void CopyProperty(std::shared_ptr<SColorTextShadeParam> ptr)
	{
		bShadeEnable = ptr->bShadeEnable;
		ShadeColor = ptr->ShadeColor;
		nShadeAngle = ptr->nShadeAngle;
		nShadeDistance = ptr->nShadeDistance;
		dAmbiguity = ptr->dAmbiguity;
	}

	QJsonObject SaveDataToJson()
	{
		QJsonObject obj;

		obj.insert("bShadeEnable", bShadeEnable);
		obj.insert("dOpacity", dAmbiguity);
		obj.insert("nShadeAngle", nShadeAngle);
		obj.insert("nShadeDistance", nShadeDistance);

		obj.insert("ShadeColorR", ShadeColor.red());
		obj.insert("ShadeColorG", ShadeColor.green());
		obj.insert("ShadeColorB", ShadeColor.blue());
		obj.insert("ShadeColorA", ShadeColor.alpha());

		return obj;
	}

	void LoadDataFromJson(const QJsonObject& obj)
	{
		bShadeEnable = obj.value("bShadeEnable").toBool();
		dAmbiguity = obj.value("dOpacity").toDouble();
		nShadeAngle = obj.value("nShadeAngle").toInt();
		nShadeDistance = obj.value("nShadeDistance").toInt();

		ShadeColor = QColor(
			obj.value("ShadeColorR").toInt(),
			obj.value("ShadeColorG").toInt(),
			obj.value("ShadeColorB").toInt(),
			obj.value("ShadeColorA").toInt()
		);
	}
};

//幻彩字-背景参数
struct SColorTextBackgroundParam
{
	bool	bBackgroundEnable = false;				// 小工具是否启用了背景
	int		nBackgroundType = 0;					// 背景类型
	QColor	 BackgroundColor;						// 颜色
	double	 dOpacity = 0;							// 透明度
	double	 dFillet = 0;							// 圆角
	double	 dWidth = 0;							// 宽度
	double	 dHeight = 0;							// 高度


	SColorTextBackgroundParam()
	{
		bBackgroundEnable = false;
		nBackgroundType = 0;
		dOpacity = 1;
		BackgroundColor = QColor(255, 190, 47, dOpacity * 255);
		dFillet = 0;
		dWidth = 0;
		dHeight = 0;
	}

	SColorTextBackgroundParam(const SColorTextBackgroundParam& other)
	{
		*this = other;
	}

	SColorTextBackgroundParam& operator=(const SColorTextBackgroundParam& other)
	{
		bBackgroundEnable = other.bBackgroundEnable;
		nBackgroundType = other.nBackgroundType;
		BackgroundColor = other.BackgroundColor;
		dOpacity = other.dOpacity;
		dFillet = other.dFillet;
		dWidth = other.dWidth;
		dHeight = other.dHeight;

		return *this;
	}

	void CopyProperty(std::shared_ptr<SColorTextBackgroundParam> ptr)
	{
		bBackgroundEnable = ptr->bBackgroundEnable;
		nBackgroundType = ptr->nBackgroundType;
		BackgroundColor = ptr->BackgroundColor;
		dOpacity = ptr->dOpacity;
		dFillet = ptr->dFillet;
		dWidth = ptr->dWidth;
		dHeight = ptr->dHeight;
	}

	QJsonObject SaveDataToJson()
	{
		QJsonObject obj;

		obj.insert("bBackgroundEnable", bBackgroundEnable);
		obj.insert("nBackgroundType", nBackgroundType);
		obj.insert("dOpacity", dOpacity);
		obj.insert("dFillet", dFillet);
		obj.insert("dWidth", dWidth);
		obj.insert("dHeight", dHeight);

		obj.insert("BackgroundColorR", BackgroundColor.red());
		obj.insert("BackgroundColorG", BackgroundColor.green());
		obj.insert("BackgroundColorB", BackgroundColor.blue());
		obj.insert("BackgroundColorA", BackgroundColor.alpha());

		return obj;
	}

	void LoadDataFromJson(const QJsonObject& obj)
	{
		bBackgroundEnable = obj.value("bBackgroundEnable").toBool();
		nBackgroundType = obj.value("nBackgroundType").toInt();
		dOpacity = obj.value("dOpacity").toDouble();
		dFillet = obj.value("dFillet").toDouble();
		dWidth = obj.value("dWidth").toDouble();
		dHeight = obj.value("dHeight").toDouble();

		BackgroundColor = QColor(
			obj.value("BackgroundColorR").toInt(),
			obj.value("BackgroundColorG").toInt(),
			obj.value("BackgroundColorB").toInt(),
			obj.value("BackgroundColorA").toInt()
		);
	}
};


struct SColorPty
{
	SColorPty() {};
	SColorPty(const SColorPty& other)
	{
		luminance = other.luminance;
		contrast = other.contrast;
		hue = other.hue;
		saturability = other.saturability;

		red = other.red;
		green = other.green;
		blue = other.blue;
		alpha = other.alpha;
	}

	SColorPty& operator=(const SColorPty& other)
	{
		luminance = other.luminance;
		contrast = other.contrast;
		hue = other.hue;
		saturability = other.saturability;

		red = other.red;
		green = other.green;
		blue = other.blue;
		alpha = other.alpha;

		return *this;
	}

	bool operator==(const SColorPty& other)
	{
		return ((luminance == other.luminance)
			&& (contrast == other.contrast)
			&& (hue == other.hue)
			&& (saturability == other.saturability)
			&& (red == other.red)
			&& (green == other.green)
			&& (blue == other.blue)
			&& (alpha == other.alpha));
	}

	void SetDefault()
	{
		luminance = 0.0f;
		contrast = 0.0f;
		hue = 0.0f;
		saturability = 0.0f;
		red = 1.0f;
		green = 1.0f;
		blue = 1.0f;
		alpha = 1.0f;
	}

	float luminance = 0.0f;
	float contrast = 0.0f;
	float hue = 0.0f;
	float saturability = 0.0f;

	float red = 1.0f;
	float green = 1.0f;
	float blue = 1.0f;
	float alpha = 1.0f;
};


// 返回FontFamily，跳过不支持字体（属性窗使用）
inline bool GetFontNotSupport(QString strFont)
{
	if (strFont == "System" || strFont == "Terminal" || strFont == "Small Fonts" || strFont == "MT Extra" || strFont == "MS Serif"
		|| strFont == "MS Sans Serif" || strFont == "Fixedsys" || strFont == "Courier")
		return true;
	else
		return false;
}

inline std::vector<QString> GetFontFamilyList()
{
	std::vector<QString> vecFontFamily;
	QFontDatabase	m_fontData;
	for (int i = 0; i < m_fontData.families().size(); i++)
	{
		QString strFont = m_fontData.families()[i];
		if (!GetFontNotSupport(strFont))
			vecFontFamily.push_back(strFont);
	}
	return vecFontFamily;
}