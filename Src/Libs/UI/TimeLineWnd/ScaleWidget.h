#pragma once
#include "ContentViewData.h"

class QWheelEvent;
#include "UICommon/BaseFrame.h"
#include <QPoint>

class CScaleWidget :public QWidget
{
	Q_OBJECT
public:
	CScaleWidget(QWidget* pParent = 0);
	~CScaleWidget();

	void	UpdateView();
	void	Clear();

protected:
	void	mousePressEvent(QMouseEvent*)override;
	void	mouseMoveEvent(QMouseEvent*)override;
	void	mouseReleaseEvent(QMouseEvent*)override;
	void	wheelEvent(QWheelEvent* e) override;
	void    paintEvent(QPaintEvent*) override;

signals:
	void	signalUpdateView(int);

protected:
	QPoint	m_MousePressPoint;
};
