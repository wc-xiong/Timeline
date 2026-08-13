#include "RenameDialog.h"

using namespace UICommon;

#include "UITranslate.h"
#include "Messagebox.h"

#include <QLineEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRegExp>
#include <QRegExpValidator>
#include "UICommonSizeDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CRenameDialog::CRenameDialog(QWidget * parent, const QString& name)
	: CFramelessDialog(parent), m_pLineEidt(nullptr)
{
	InitWidget();

	m_pLineEidt->setText(name);
	m_pLineEidt->setFocus();
	m_pLineEidt->selectAll();

	CreateConnect();
}

CRenameDialog::~CRenameDialog()
{
}

void CRenameDialog::InitWidget()
{
	setWindowTitle(CCommonTranslate::Rename());
	setFixedSize(SysDpiZoom(408), SysDpiZoom(144));
	setDialogResizable(false);
	setAttribute(Qt::WA_ShowModal, true);
	
	m_pLineEidt = new QLineEdit(this);
	m_pLineEidt->setFixedSize(SysDpiZoom(360), UICommon_CtrlHeight);

	// 设置输入过滤
	QRegExp reg("^[^\\\\/:*?\"<>|]{1,256}$");
	m_pLineEidt->setValidator(new QRegExpValidator(reg, this));

	QVBoxLayout* layout = new QVBoxLayout;
	layout->setSpacing(0);
	layout->setContentsMargins(nSizeDefine_24, nSizeDefine_24, nSizeDefine_24, nSizeDefine_24);
	layout->addWidget(m_pLineEidt);
	layout->addStretch();

	setLayout(layout);

	bottomLayout()->addSpacing(nSizeDefine_6);
}

void CRenameDialog::CreateConnect()
{
	m_pOKButton->disconnect();
	connect(m_pOKButton, &QPushButton::clicked, this, &CRenameDialog::slotOkClicked);
}

void CRenameDialog::slotOkClicked()
{
	QString text = m_pLineEidt->text();
	bool bEmptyName = true;
	for (auto it : text)
	{
		if (it != ' ')
		{
			bEmptyName = false;
			break;
		}
	}
	if (bEmptyName)
	{
		CMessageBox::information(this, "GrandMapping", CCommonTranslate::RenameEmpty());
		return;
	}

	emit signalRenameChange(text);
	QDialog::accept();
}

QString UICommon::CRenameDialog::GetText() const
{
	return m_pLineEidt->text();
}

void UICommon::CRenameDialog::SetNameMaxLenght(int nLenght)
{
	m_pLineEidt->setMaxLength(nLenght);
}
