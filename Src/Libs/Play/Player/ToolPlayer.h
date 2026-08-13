#pragma once

#include "Player.h"
#include "PlayerGlobal.h"
#include <QImage>
#include <QFont>
#include "boost/thread/condition_variable.hpp"
#include "boost/thread/mutex.hpp"

#include "CommonDrawApi.h"

class IResourceBase;
class PLAYER_EXPORT CToolPlayer : public CBasePlayer,public IToolPlayer
{
public:
	CToolPlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx);
	virtual ~CToolPlayer();

	/** 停止并 join 重绘线程。派生类析构中若需先释放绘制资源，必须先调用本函数。 */
	void StopUpdateThread();

public:
	virtual bool Prepare();
	virtual bool GetTextView(const microseconds & curTime, SDrawTextureData& videoData);

protected:
	virtual bool DrawImage();
	virtual void SetThreadStart();
    virtual void  UpdateImageInfo();

	/*
		brief: 根据传入原绘制区域与原画面分辨率的比例，获取自适应后绘制区域
		fLeftRatio: 原绘制区域左边距比例
		fTopRatio: 原绘制区域上边距比例
		fDrawWidthRatio: 原绘制区域宽度比例
		fDrawHeightRatio: 原绘制区域高度比例
		rcDraw: 自适应大小后的区域
		szCur: 当前媒体绘制大小，默认值为当前媒体大小
	*/
	void MapToDrawRect(float fLeftRatio, float fTopRatio, float fDrawWidthRatio, float fDrawHeightRatio, QRect &rcDraw, QSize szCur = QSize(-1, -1));

	/*
		brief: 根据传入原绘制区域大小与原画面分辨率的比例，获取自适应后绘制分辨率
		fDrawWidthRatio: 原绘制区域宽度比例
		fDrawHeightRatio: 原绘制区域高度比例
		szDraw: 自适应大小后的分辨率
		szCur: 当前媒体绘制大小，默认值为当前媒体大小
	*/
	void MapToDrawSize(float fDrawWidthRatio, float fDrawHeightRatio, QSize &szDraw, QSize szCur = QSize(-1, -1));

	/*
		brief: 根据传入原绘制区域位置坐标，获取自适应后绘制位置坐标
		ptDraw: 绘制区域坐标（传入为原绘制区域坐标，函数更新为自适应大小后绘制区域坐标）
		szResOrigin: 原画面区域分辨率
		szCur: 当前媒体绘制大小，默认值为当前媒体大小
	*/
	void MapToDrawPoint(QPoint &ptDraw, const QSize &szResOrigin, QSize szCur = QSize(-1, -1));
	
	/*
		brief: 根据传入原绘制区域位置坐标与画面分辨率的比例，获取自适应后绘制位置坐标
		fLeftRatio: 原绘制区域左边距比例
		fTopRatio: 原绘制区域上边距比例
		ptDraw: 自适应大小后的位置坐标
		szCur: 当前媒体绘制大小，默认值为当前媒体大小
	*/
	void MapToDrawPoint(float fLeftRatio, float fTopRatio, QPoint &ptDraw, QSize szCur = QSize(-1, -1));

	/*
		brief: 根据传入原绘制区域长度、原画面宽度或高度、输入长度类型（宽度或者高度），获取自适应后绘制宽度
		nLenght: 绘制区域长度（传入为原绘制区域长度，函数更新为自适应大小后绘制区域长度）
		nResOriginLenght: 原画面宽度或高度
		nCurLenght:当前绘制长度，默认根据"nRelyType"选择为当前媒体宽高
		nRelyType：输入长度类型（宽度或者高度）
	*/
	void MapToDrawLenght(int &nLenght, int nResOriginLenght, int nCurLenght = -1, int nRelyType = EZRT_Width);
	
	/*
		brief: 根据传入原绘制区域长度与原画面宽度或高度比例，获取自适应后绘制宽度
		nLenght: 函数更新为自适应大小后绘制区域长度
		fDrawWidthRatio：原绘制区域宽度与原画面宽度比例
		szCur: 当前媒体绘制大小，默认值为当前媒体大小
		nRelyType：输入长度类型（宽度或者高度）
	*/
	void MapToDrawLenght(int &nLenght, float fDrawWidthRatio, QSize szCur = QSize(-1, -1), int nRelyType = EZRT_Width);

protected:
	QImage m_image;	// 当前媒体绘制图片

	QSize m_szMedia;	// 当前媒体绘制大小

	std::thread * m_pThreadUpdateImage = nullptr;
	std::atomic_bool m_bExit;

	int m_nInterval = 1000 / 30;//重绘检测间隔

	boost::mutex m_mutexCond;
	boost::condition_variable m_condUpdate;
};
