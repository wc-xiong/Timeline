#pragma once
#include "UICommonGlobal.h"
#include <QCheckBox>

class UICOMMON_EXPORT CCheckBoxEx : public QCheckBox
{
	Q_OBJECT

public:
	CCheckBoxEx(QWidget* pParent = 0);
	~CCheckBoxEx();

public:
	void SetChecked(bool bChecked);

	void SetCheckState(Qt::CheckState state);


	void setText(const QString &text);

	QString GetRealText() {
		return m_strText;
	}

	void SetBoxAndSpaceWidth(int nWidth) { m_nBoxAndSpaceWidth = nWidth; }

protected:

	virtual void resizeEvent(QResizeEvent *event) override;

	virtual void mousePressEvent(QMouseEvent *event);

private:
	QString m_strText;
	int m_nBoxAndSpaceWidth;
};
