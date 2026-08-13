#pragma once

#include "UICommon/Framelessdialog.h"
#include "Common/ResourceDefine.h"
#include <QPushButton>

using namespace UICommon;
class CModeSelectBtn;

class CSubtitleModeSelectDlg : public CFramelessDialog
{
	Q_OBJECT
public:
	explicit CSubtitleModeSelectDlg(QWidget *parent, const QString& strPath = QString(), bool IsDrop = false);
	virtual ~CSubtitleModeSelectDlg();

	bool GetSelectMode() const
	{
		return m_bSubtitle;
	}
	QStringList GetSubtitleFilePath() const
	{
		return m_strSubtitleFilePath;
	}

protected:
	void InitWidget();
	void CreateConnect();
	void OpenResFileSys();

protected slots:
	void slotBtnClicked();

	void slotBtnDoubleClicked();

protected:
	CModeSelectBtn* m_pBtnSubtitle = nullptr;
	//CModeSelectBtn* m_pBtnTeleprompter = nullptr;

	bool m_bSubtitle = true;
	bool m_IsDrop = false;
	const QString strFullPath;
	QStringList m_strSubtitleFilePath = QStringList();
	QString m_strTeleprompterFilePath = QString();
};