#pragma once
#include "UICommonGlobal.h"
#include <QCompleter>

class UICOMMON_EXPORT CCompleterEx : public QCompleter
{
	Q_OBJECT

public:
	CCompleterEx(QWidget* pParent = 0);

	CCompleterEx(const QStringList &list, QObject *parent = nullptr);

	CCompleterEx(QAbstractItemModel *model, QObject *parent = nullptr);

	~CCompleterEx();


};
