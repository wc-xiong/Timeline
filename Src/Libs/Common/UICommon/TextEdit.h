#pragma once

#include "UICommonGlobal.h"
#include <QTextEdit>


class QColor;
class CSuspendedScrollBar;

class UICOMMON_EXPORT CTextEdit : public QTextEdit
{
	Q_OBJECT

public:
	CTextEdit(QWidget *parent = nullptr);
	~CTextEdit();

	void SeTextFrameFormatMargin(int nMargin);

	void UseFloatVerticalScrollBar();

	void UseFloatHerticalScrollBar();

public slots:
	void slotVerticalValueChange(int nValue);
	void slotHorizontalValueChange(int nValue);

protected:
	virtual void mousePressEvent(QMouseEvent *) override;
	//void keyPressEvent(QKeyEvent *e);
	virtual void focusInEvent(QFocusEvent *event) override;
	void inputMethodEvent(QInputMethodEvent *e)override;
	void insertFromMimeData(const QMimeData * source)override;

	void keyPressEvent(QKeyEvent *e) override;

	void paintEvent(QPaintEvent *event) override;

	void resizeEvent(QResizeEvent* event) override;
	void enterEvent(QEvent* event) override;
	void leaveEvent(QEvent* event) override;

	bool m_bIsUseFloatVerticalScrollBar = false;
	CSuspendedScrollBar* m_pVerticalScrollBar = nullptr;

	bool m_bIsUseFloatHorizontalScrollBar = false;
	CSuspendedScrollBar* m_pHorizontalScrollBar = nullptr;
};