#pragma once
#include "UICommonGlobal.h"

#include <QRadioButton>

class UICOMMON_EXPORT CRadioButton : public QRadioButton
{
	Q_OBJECT
public:
	CRadioButton(QWidget *parent = nullptr);
	virtual ~CRadioButton();

	void	SetText(const QString& text, int nIndicatorSize);
	QString	GetText();

private:
	QString m_strText;
};
