#pragma once

#include "UICommonGlobal.h"
#include "Framelessdialog.h"

class QLineEdit;

namespace UICommon {
	class UICOMMON_EXPORT CRenameDialog : public CFramelessDialog
	{
		Q_OBJECT
	public:
		explicit CRenameDialog(QWidget *parent, const QString& strText);
		~CRenameDialog();

		QString GetText() const;

		void SetNameMaxLenght(int nLenght);

	signals:
		void signalRenameChange(const QString&);

	protected:
		void InitWidget();
		void CreateConnect();

	protected slots:
		void slotOkClicked();

	protected:
		QLineEdit* m_pLineEidt;
	};
}

