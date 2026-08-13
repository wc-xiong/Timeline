#pragma once
#include "UICommonGlobal.h"
#include <QPushButton>
#include <QPixmap>

class UICOMMON_EXPORT CPushButton : public QPushButton
{
	Q_OBJECT

public:
	CPushButton(QWidget* pParent = 0);
	~CPushButton();

	void ShowLightLine();

	void HideLightLine();

	void SetText(const QString& str);
protected:

	void mousePressEvent(QMouseEvent *e) override;

	void mouseReleaseEvent(QMouseEvent *e) override;
	void showEvent(QShowEvent *event) override;
	void paintEvent(QPaintEvent* event) override;
protected:
	QString m_strBtnTxt;
	bool m_bElideText = false;
protected:
	bool m_bIsShowTopLightLine = true;
};

class UICOMMON_EXPORT CSwitchButton :public CPushButton
{
	Q_OBJECT

public:
	CSwitchButton(QWidget* pParent = 0);
	~CSwitchButton();

	void SetName(QString &strLeftName, QString &strRightName);
	void SetState(bool b) { m_bState = b; }
	bool GetState() { return m_bState; }

	void SetStyle(bool b) { m_bStyle = b; }

signals:
	void signalStateChange(bool);

public slots:
	void slotButtonClicked();

protected:
	void paintEvent(QPaintEvent *) override;

private:
	bool  m_bStyle = true;
	bool  m_bState = false;
	QString m_strLeftName;
	QString m_strRightName;
};
