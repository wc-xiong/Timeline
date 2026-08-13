#pragma once
#include "UICommonGlobal.h"
#include <QScrollBar>

//悬浮滑动条

class UICOMMON_EXPORT CSuspendedScrollBar : public QScrollBar
{
	Q_OBJECT
public:
	CSuspendedScrollBar(Qt::Orientation t, QWidget * parent = 0);
	~CSuspendedScrollBar();

	void SetScrollMode(int mode);
public slots:
	void slotValueChange(int nValue);					//当使用该组件的控件的滚动条发出valueChanged信号时，该信号被触发
	void slotRangeChanged(int nMin, int nMax);	//当使用该组件的控件的滚动条发出rangeChanged信号时，该信号被触发

private:
	int m_nScrollMode = 0;
};

 
