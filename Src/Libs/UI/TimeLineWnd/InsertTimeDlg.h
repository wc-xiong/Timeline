#pragma once

#include "Common/UICommon/Framelessdialog.h"
#include "Common/Common/ITimelineInterface.h"

#include <chrono>

class CTimeEditWidget;

class CInsertTimeDlg : public UICommon::CFramelessDialog
{
	Q_OBJECT
public:
	CInsertTimeDlg(QWidget* parent, std::chrono::milliseconds defaultPosition);
	~CInsertTimeDlg();

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
