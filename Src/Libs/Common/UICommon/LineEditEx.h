#pragma once
#include "UICommonGlobal.h"
#include <QLineEdit>

class UICOMMON_EXPORT CLineEditEx : public QLineEdit
{
	Q_OBJECT

public:
	CLineEditEx(QWidget* pParent = 0);
	~CLineEditEx();

public:
	// 由于出省略号时已经改变了原字符串，获取当前字符串需要使用此函数
	QString GetRealText() const;
	void setText(const QString& text);

	void SetSendMsgToParent(bool bValue) {
		m_bSendMsgToParent = bValue;
	}

	void setEditWidthTolerance(int nValue) { m_nEditWidthTolerance = nValue; }
protected:
	//void paintEvent(QPaintEvent *event) override;

	virtual void resizeEvent(QResizeEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *) override;
	virtual void mouseReleaseEvent(QMouseEvent *) override;

	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void focusInEvent(QFocusEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event)override;

	virtual void UpdateTextElide();


protected slots:
	void slotTextEdited(const QString &text);
	void slotEditFinished();

protected:
	QString m_strText;
	int m_nEditWidthTolerance = 5;

	bool m_bEditing = false;

	bool m_bSendMsgToParent = true;
};

inline QString CLineEditEx::GetRealText() const { return m_strText; }
