#pragma once
#include "UICommonGlobal.h"
#include <QScrollBar>

class UICOMMON_EXPORT CSrollBarEx : public QScrollBar
{
	Q_OBJECT
public:
	explicit CSrollBarEx(Qt::Orientation direct, QWidget* parent = 0);
	~CSrollBarEx();

public slots:
	void slotValueChange(int val);
	void slotRangeChange(int min, int max);


protected:
	virtual void mouseMoveEvent(QMouseEvent *event) override;
};
