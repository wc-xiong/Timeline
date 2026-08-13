#pragma once
#include "UICommonGlobal.h"
#include "LineEditEx.h"
#include <QLineEdit>
#include <QStyleOptionFrame>

class UICOMMON_EXPORT CSearchLineEdit : public CLineEditEx
{
	Q_OBJECT

public:
	CSearchLineEdit(QWidget* pParent = 0);
	~CSearchLineEdit();

public:

protected:
	void paintEvent(QPaintEvent *event) override;

	virtual void focusOutEvent(QFocusEvent *event) override;
	virtual void leaveEvent(QEvent* event) override;
	virtual void mousePressEvent(QMouseEvent *) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *) override;

protected slots:


protected:
	int m_iconSize;
	QStyleOptionFrame m_opt;

	bool m_bMouseOver;
	bool m_bDeletePressed = false;

	QString m_strSearchIconPath;
	QString m_strDeleteIconPath;
};

