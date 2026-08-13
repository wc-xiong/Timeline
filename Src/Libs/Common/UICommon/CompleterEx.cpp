#include "CompleterEx.h"
#include "UICommonSizeDefine.h"
#include <QWidget>
#include <QStyledItemDelegate>
#include <QAbstractItemView>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CCompleterEx::CCompleterEx(QWidget* pParent):
	QCompleter(pParent)
{
	popup()->setItemDelegate(new QStyledItemDelegate(this));//NOTE 必须 否则部分style不生效
	popup()->setProperty("CompleterView", true);
}

CCompleterEx::CCompleterEx(const QStringList &list, QObject *parent /*= nullptr*/)
	: QCompleter(list, parent)
{
	popup()->setItemDelegate(new QStyledItemDelegate(this));//NOTE 必须 否则部分style不生效
	popup()->setProperty("CompleterView", true);
}

CCompleterEx::CCompleterEx(QAbstractItemModel *model, QObject *parent /*= nullptr*/)
	: QCompleter(model, parent)
{
	popup()->setItemDelegate(new QStyledItemDelegate(this));//NOTE 必须 否则部分style不生效
	popup()->setProperty("CompleterView", true);
}

CCompleterEx::~CCompleterEx()
{
}
