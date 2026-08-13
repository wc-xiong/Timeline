#pragma once

#include "Common/UICommon/Framelessdialog.h"
#include "Common/Common/ITimelineInterface.h"

#include <chrono>

class CTimeEditWidget;

class CRemoveTimeDlg : public UICommon::CFramelessDialog
{
	Q_OBJECT
public:
	CRemoveTimeDlg(QWidget* parent, std::chrono::milliseconds defaultPosition);
	~CRemoveTimeDlg();

	std::chrono::milliseconds GetPosition() const;
	std::chrono::milliseconds GetLength() const;
	ETimeEditScope GetScope() const;

private:
	void InitWidgets();
	void CreateConnections();
	void UpdateTranslateText();
	bool ValidateInput() const;

private:
	CTimeEditWidget* m_pPositionEdit = nullptr;
	CTimeEditWidget* m_pLengthEdit = nullptr;
};
