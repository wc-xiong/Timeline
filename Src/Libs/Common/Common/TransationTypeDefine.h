#pragma once
#include "LanguageDefine.h"
#include <QUuid>
#include <QByteArray>
#include <QString>

// 转场特效类型
enum ETransFxType
{
	ETFT_None = 0,
	ETFT_Random,				//随机

	//改变像素类特效
	ETFT_Fade,					//淡入淡出

	//改变顶点类特效
	ETFT_LeftMove,				//左移
	ETFT_RightMove,				//右移
	ETFT_UpMove,					//上移
	ETFT_DownMove,				//下移

	ETFT_LeftTopMove,			//左上移
	ETFT_LeftBottomMove,			//左下移
	ETFT_RightTopMove,			//右上移
	ETFT_RightBottomMove,		//右下移

	ETFT_LeftCover,				//左覆盖
	ETFT_TopCover,				//上覆盖
	ETFT_RightCover,			//右覆盖
	ETFT_BottomCover,			//下覆盖

	ETFT_LeftTopCover,			//左上直线覆盖
	ETFT_RightTopCover,			//右上直线覆盖
	ETFT_LeftBottomCover,		//左下直线覆盖
	ETFT_RightBottomCover,		//右下直线覆盖

	ETFT_LeftTopObliqueCover,	//左上斜线覆盖
	ETFT_RightTopObliqueCover,	//右上斜线覆盖
	ETFT_LeftBottomObliqueCover,	//左下斜线覆盖
	ETFT_RightBottomObliqueCover,//右下斜线覆盖

	ETFT_GrillesHorizontal,		//水平百叶
	ETFT_GrillesVertical,		//垂直百叶

	ETFT_LeftRightOpen,			//左右对开
	ETFT_TopBottomOpen,			//上下对开
	ETFT_LeftRightClose,			//左右闭合
	ETFT_TopBottomClose,			//上下闭合

	ETFT_MosaicLarge,					//马赛克（大）
	ETFT_MosaicMiddle,					//马赛克（中）
	ETFT_MosaicSmall,					//马赛克（小）

	ETFT_RightRotFull,			//右旋360°
	ETFT_RightRotHalf,			//右旋180°
	ETFT_RightRotQuarter,		//右旋90°
	ETFT_LeftRotFull,			//左旋360°
	ETFT_LeftRotHalf,			//左旋180°
	ETFT_LeftRotQuarter,			//左旋90°

	ETFT_SmallToLargeCenter,		//由小变大（中间）
	ETFT_SmallToLargeLeftTop,	//由小变大（左上）
	ETFT_SmallToLargeLeftBottom,	//由小变大（左下）
	ETFT_SmallToLargeRightTop,	//由小变大（右上）
	ETFT_SmallToLargeRightBottom,//由小变大（右下）

	ETFT_CenterToBorderRect,		//中间向四周（矩形）
	ETFT_BorderToCenterRect,		//四周向中间（矩形）
	ETFT_CenterToBorderLozenge,	//中间向四周（菱形）
	ETFT_BorderToCenterLozenge,	//四周向中间（菱形）
	ETFT_CenterToBorderCross,	//中间向四周（十字）
	ETFT_BorderToCenterCross,	//四周向中间（十字）

	ETFT_PerspectiveRotationLeft,	//透视旋转（左）
	ETFT_PerspectiveRotationRight,	//透视旋转（右）
	ETFT_PerspectiveRotationTop,		//透视旋转（上）
	ETFT_PerspectiveRotationBottom,	//透视旋转（下）

	ETFT_Count,
};