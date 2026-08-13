#pragma once
#include "UICommonGlobal.h"
#include <QLineEdit>
#include <QTimer>
#include <QProxyStyle>

class CCustomLineEditProxyStyle : public QProxyStyle
{
public:
	int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;
};

class UICOMMON_EXPORT CColorLineedit : public QLineEdit
{
	Q_OBJECT
public:
	CColorLineedit(QWidget* parent = nullptr);
	virtual ~CColorLineedit();
	virtual void paintEvent(QPaintEvent *event) override;

	void StopTimer();

private slots:
	void slotTextEdit(const QString &);
	void slotUpdateCursorRect();

protected:
	bool m_bShowCursor = false;
	QTimer *m_pUpdateTimer = nullptr;
	CCustomLineEditProxyStyle *m_pProxyStyle = nullptr;
};
