#pragma once
#include "UICommon/BaseFrame.h"

class CPropertyFrame: public CBaseFrame
{
	Q_OBJECT
public:
	CPropertyFrame(QWidget* pParent = nullptr) :CBaseFrame(pParent) {};   // 标准构造函数
	virtual ~CPropertyFrame() = default;

	// 选中媒体后，更新媒体对应信息
	virtual void UpdateProperty() = 0;
};
