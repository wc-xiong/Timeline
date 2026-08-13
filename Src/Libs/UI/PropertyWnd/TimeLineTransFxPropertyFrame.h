#pragma once
#include "PropertyFrame.h"
#include <QUuid>

class CComboBoxWidget;
class CTimeEditWidget;
class QFrame;

/** 转场属性：类型 + 时长 */
class CTimeLineTransFxPropertyFrame : public CPropertyFrame
{
	Q_OBJECT
public:
	CTimeLineTransFxPropertyFrame(QWidget* pParent = nullptr);
	virtual ~CTimeLineTransFxPropertyFrame();

	void UpdateProperty() override;
	void SetTransFxID(const QUuid& id) { m_transFxID = id; }
	void UpdateTranslateText();

protected:
	void InitWidget();
	void CreateConnects();
	void ClearEdit();
	void EnableEdit();
	void RefreshTransTypeCombo();
	int TransTypeToComboIndex(int transType) const;
	int ComboIndexToTransType(int index) const;

protected slots:
	void slotTransTypeChange(int index);
	void slotDurationFinish();

private:
	QUuid m_transFxID;
	QFrame* m_pTransFxFrame = nullptr;
	CComboBoxWidget* m_pComboTransType = nullptr;
	CTimeEditWidget* m_pDurationTimeEdit = nullptr;
};
