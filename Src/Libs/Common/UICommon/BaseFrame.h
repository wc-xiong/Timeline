#pragma once
#include "UICommonGlobal.h"
#include <QFrame>

// 继承QFrame的界面都继承此基类

class UICOMMON_EXPORT CBaseFrame : public QFrame
{
	Q_OBJECT

public:
	CBaseFrame(QWidget* pParent = 0);
	~CBaseFrame();

	virtual void hide();
	virtual void show();

protected:
	void changeEvent(QEvent *event)	override;

	// 翻译文本
	virtual void UpdateTranslateText(){}
};
