#pragma once
#include "PropertyFrame.h"
#include "Common/Maths/Matrix3x2.h"
#include <QUuid>

class CComboBoxWidget;
class CTimeEditWidget;
class CPushButton;

class CTimeLinePropertyFrame : public CPropertyFrame
{
	Q_OBJECT
public:
	CTimeLinePropertyFrame(QWidget* pParent = nullptr);   // 标准构造函数
	virtual ~CTimeLinePropertyFrame();

	// 选中媒体后，更新媒体对应信息
	void UpdateProperty() override;
	void UpdateTranslateText();

protected:
	void InitWidget();
	void CreateConnects();

private:
	CTimeEditWidget* m_pPlayDuration = nullptr;
	CComboBoxWidget* m_pEndOperateType = nullptr;
	CPushButton* m_pDeleteTail = nullptr;
};
