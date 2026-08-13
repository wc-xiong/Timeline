#pragma once
#include "ContentViewData.h"

#include <QWidget>
#include <QPen>
#include <QPixmap>

class CNoniusViewWidget :public QWidget
{
	Q_OBJECT
public:
	CNoniusViewWidget(QWidget* pParent = 0);
	~CNoniusViewWidget();

	void	UpdateView();

signals:
	void	signalPlayWheel();
	void	signalPlayWheelCycle();

protected:
	void    paintEvent(QPaintEvent*) override;
	enum {
		Pen_PlayheadThick,	// 游标实线 TIMELINE_CLR_Thick
		Pen_PlayheadThin,	// 游标虚线 TIMELINE_CLR_Thin
		Pen_PlayheadRecord,	// 录制中游标实线（红色）
		Pen_PlayheadRecordThin, // 录制中游标虚线（红色）
		Pen_Snap,			// 绘制附加线
		Pen_DragLine,		// 绘制图层拖拽线
		Pen_Num
	};
	QPen m_pens[Pen_Num];

	QPixmap m_Pixmaxp;			// 游标标头（固定实色）
	QPixmap m_PixmaxpRecord;	// 录制中游标标头（红色）
	bool m_bPlayheadThick = true;	// 播放中游标实/虚线切换
};