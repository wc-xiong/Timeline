#pragma once

#include "UICommonGlobal.h"
#include "Framelessdialog.h"
#include <vector>

class QVBoxLayout;

namespace UICommon {
	class UICOMMON_EXPORT CShowTextDialog : public CFramelessDialog
	{
		Q_OBJECT
	public:
		explicit CShowTextDialog(QWidget *parent);
		explicit CShowTextDialog(QWidget *parent, const QString &strTitle, QStringList strListTexts);
		~CShowTextDialog();

		void SetText(QStringList strListTexts);
		void SetTitle(const QString &strText);

	protected:
		void InitWidget();

		void ClearAllLayoutItem();

	protected:
		QVBoxLayout* m_pVEditLayout = nullptr;
	};
}

