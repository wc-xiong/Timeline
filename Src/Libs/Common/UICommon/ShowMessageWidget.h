#pragma once
#include "UICommonGlobal.h"
#include <QWidget>
#include <QLabel>
#include <QPushButton>

// 继承QFrame的界面都继承此基类

enum EShowMessageType
{
	ESMT_Succ = 0,
	ESMT_Fail = 1,
	ESMT_Warning = 2,
	ESMT_Normal = 3,
};

class UICOMMON_EXPORT CShowMessageWidget : public QWidget
{
	Q_OBJECT

public:
	~CShowMessageWidget();

	static void ShowMessage(int nShowTime, const QString& strText, QWidget* pParent = 0, EShowMessageType type = ESMT_Succ, const QString& strBtnText = QString());
	// 在指定窗口内显示 tip（不回退主窗口；pHost 为空则不显示）
	static void ShowMessageInWidget(int nShowTime, const QString& strText, QWidget* pHost, EShowMessageType type = ESMT_Succ, const QString& strBtnText = QString());

public:
	void SetShowTime(int nShowTime);
	void SetText(const QString& strText);
	void SetBtnText(const QString& strText);
	void SetTypeProperty(EShowMessageType type);

protected:
	void InitWidget();
	void InitLayout();
	virtual void show();

protected:
	QLabel* m_pMessageTxtLabel = nullptr;			//提示文本子标签
	QLabel* m_pMessageIconLabel = nullptr;			//提示图标子标签

	QPushButton* m_pToolBtn = nullptr;
	int m_nShowTime = 1000;

private:
	CShowMessageWidget(QWidget* pParent = 0);
	void UpdateWidgetSizeByContent();
	static void ShowMessageOnHost(int nShowTime, const QString& strText, QWidget* pHost, EShowMessageType type, const QString& strBtnText);
};
