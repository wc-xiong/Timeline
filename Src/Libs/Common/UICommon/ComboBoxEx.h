#pragma once
#include "UICommonGlobal.h"
#include <QComboBox>

class UICOMMON_EXPORT CComboBoxEx : public QComboBox
{
	Q_OBJECT

public:
	CComboBoxEx(QWidget* pParent = 0, bool bTranslucent = true, bool bSendToParent = true);
	~CComboBoxEx();

	void SetCurrentIndex(int nIndex);
	void SetEditable(bool bEditedable);
	void SetPlaceholder(bool b);
	void SetTextAlignment(Qt::Alignment nAlignment);
protected:
	//ComboBox未展开时不吞滚动条
	virtual void wheelEvent(QWheelEvent *event) override;

	virtual void showPopup() override;
	virtual void hidePopup() override;

	bool eventFilter(QObject *watched, QEvent *event) override;

protected:
	virtual void mousePressEvent(QMouseEvent *event);

private:
	bool m_bSendToParent = true;
	bool m_bPlaceholder = false;
	bool m_bPlaceholderPressed = false;
};
