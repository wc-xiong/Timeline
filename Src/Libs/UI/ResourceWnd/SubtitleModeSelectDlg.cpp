#include "SubtitleModeSelectDlg.h"

using namespace UICommon;

#include "UICommon/UITranslate.h"
#include "UICommon/Messagebox.h"
#include "UICommon/LineEditEx.h"
#include "UICommon/ModeSelectButton.h"
#include "Model/GlobalModel/GlobalModel.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/LogModel.h"
#include "Model/GlobalModel/SoftwareSettingData.h"
#include "Common/UICommon/LogTranslate.h"
#include "Common/CommonFunctionFileOp.h"
#include "../../Model/ResourceModel/ResourceModel.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMovie>
#include <QSpinBox>
#include <QFileDialog>
#include "ResWndUISizeDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CSubtitleModeSelectDlg::CSubtitleModeSelectDlg(QWidget *parent, const QString& strPath, bool IsDrop)
	: CFramelessDialog(parent), strFullPath(strPath), m_IsDrop(IsDrop)
{
	InitWidget();

	CreateConnect();
}

CSubtitleModeSelectDlg::~CSubtitleModeSelectDlg()
{
}


void CSubtitleModeSelectDlg::InitWidget()
{
	setFixedSize(TableModeSelectDlg_Width * 0.5, SysDpiZoom(390));
	//setMinimumSize(TableModeSelectDlg_Width, TableModeSelectDlg_Height);
	setDialogResizable(false);
	setAttribute(Qt::WA_ShowModal, true);
	setCancelButtonVisible(false);
	setWindowTitle(CResourceTranslate::SubtitleTextSelectTitle());

	m_pBtnSubtitle = new CModeSelectBtn(":/dark/images/ResourceWnd/SubtitleMode/Subtitle.png", nSizeDefine_2, this);
	m_pBtnSubtitle->setContentsMargins(0, 0, 0, 0);
	m_pBtnSubtitle->setProperty("transparentBkBtn", true);
	m_pBtnSubtitle->setCheckable(true);
	m_pBtnSubtitle->setFixedSize(TableModeSelectBtn_Width, TableModeSelectBtn_Height);
	m_pBtnSubtitle->setAutoExclusive(true);
	m_pBtnSubtitle->setChecked(true);

	/*m_pBtnTeleprompter = new CModeSelectBtn(":/dark/images/ResourceWnd/SubtitleMode/Teleprompter.svg", nSizeDefine_2, this);
	m_pBtnTeleprompter->setContentsMargins(0, 0, 0, 0);
	m_pBtnTeleprompter->setProperty("transparentBkBtn", true);
	m_pBtnTeleprompter->setCheckable(true);
	m_pBtnTeleprompter->setFixedSize(TableModeSelectBtn_Width, TableModeSelectBtn_Height);
	m_pBtnTeleprompter->setAutoExclusive(true);*/

	QLabel* pSubTitleModeLabel = new QLabel(this);
	//QLabel* pTeleproModeLabel = new QLabel(this);

	//pTeleproModeLabel->setMargin(0);
	pSubTitleModeLabel->setMargin(0);
	pSubTitleModeLabel->setProperty("PPtModeSelectTitle", true);
	//pTeleproModeLabel->setProperty("PPtModeSelectTitle", true);
	pSubTitleModeLabel->setText(CCommonTranslate::Subtitle());
	//pTeleproModeLabel->setText(CCommonTranslate::Teleprompter());

	QLabel* pSubTitleDesc = new QLabel(this);
	//QLabel* pTeleproDesc = new QLabel(this);

	pSubTitleDesc->setProperty("ModeSelectDesc", true);
	//pTeleproDesc->setProperty("ModeSelectDesc", true);
	pSubTitleDesc->setWordWrap(true);
	//pTeleproDesc->setWordWrap(true);
	pSubTitleDesc->setAlignment(Qt::AlignTop);
	//pTeleproDesc->setAlignment(Qt::AlignTop);
	pSubTitleDesc->setText(CResourceTranslate::SubtitleSelectText());
	//pTeleproDesc->setText(CResourceTranslate::TeleprompterSelectText());

	QVBoxLayout* pSubTitleLayout = new QVBoxLayout;
	pSubTitleLayout->setSpacing(0);
	pSubTitleLayout->setContentsMargins(0, 0, 0, 0);
	pSubTitleLayout->addWidget(m_pBtnSubtitle);
	pSubTitleLayout->addSpacing(nSizeDefine_12);
	pSubTitleLayout->addWidget(pSubTitleModeLabel);
	pSubTitleLayout->addSpacing(nSizeDefine_8);
	pSubTitleLayout->addWidget(pSubTitleDesc);
	pSubTitleLayout->addStretch();

	/*QVBoxLayout* pTeleprompterLayout = new QVBoxLayout;
	pTeleprompterLayout->setSpacing(0);
	pTeleprompterLayout->setMargin(0);
	pTeleprompterLayout->addWidget(m_pBtnTeleprompter);
	pTeleprompterLayout->addSpacing(nSizeDefine_12);
	pTeleprompterLayout->addWidget(pTeleproModeLabel);
	pTeleprompterLayout->addSpacing(nSizeDefine_8);
	pTeleprompterLayout->addWidget(pTeleproDesc);
	pTeleprompterLayout->addStretch();*/

	QHBoxLayout* pLayout = new QHBoxLayout;
	pLayout->setSpacing(nSizeDefine_24);
	pLayout->setMargin(0);
	pLayout->addLayout(pSubTitleLayout);
	//pLayout->addLayout(pTeleprompterLayout);

	QVBoxLayout* pMainLayout = new QVBoxLayout;
	pMainLayout->setContentsMargins(nSizeDefine_39, nSizeDefine_24, nSizeDefine_24, nSizeDefine_16);
	if (!strFullPath.isEmpty())
	{
		setFixedSize(TableModeSelectDlg_Width, TableModeSelectDlg_Height);
		// 路径
		QHBoxLayout* pPathLayout = new QHBoxLayout;
		pPathLayout->setSpacing(0);
		pPathLayout->setMargin(0);

		QLabel* pLabelPathText = new QLabel(this);
		pLabelPathText->setFixedSize(nSizeDefine_96, nSizeDefine_16);
		pLabelPathText->setText(CResourceTranslate::PPTFilePath());

		QLabel* pLabelPath = new QLabel(this);
		pLabelPathText->setFixedHeight(nSizeDefine_16);
		pLabelPath->setText(strFullPath);

		pPathLayout->addWidget(pLabelPathText);
		pPathLayout->addWidget(pLabelPath);
		pMainLayout->addLayout(pPathLayout);
		pMainLayout->addSpacing(nSizeDefine_16);
	}

	pMainLayout->addLayout(pLayout);

	setLayout(pMainLayout);
}

void CSubtitleModeSelectDlg::CreateConnect()
{
	connect(m_pOKButton, &QPushButton::clicked, this, &CSubtitleModeSelectDlg::slotBtnClicked);
	connect(m_pBtnSubtitle, &QPushButton::clicked, this, &CSubtitleModeSelectDlg::slotBtnClicked);
	//connect(m_pBtnTeleprompter, &QPushButton::clicked, this, &CSubtitleModeSelectDlg::slotBtnClicked);
	//connect(m_pBtnTeleprompter, &CModeSelectBtn::signalDoubleClicked, this, &CSubtitleModeSelectDlg::slotBtnDoubleClicked);
	connect(m_pBtnSubtitle, &CModeSelectBtn::signalDoubleClicked, this, &CSubtitleModeSelectDlg::slotBtnDoubleClicked);
}

void CSubtitleModeSelectDlg::OpenResFileSys()
{
	if (m_IsDrop)
		return;

	if (m_bSubtitle)
	{
		QString strFileFilter = QString("Subtitle Files(*lrc *srt *txt)");
		QFileDialog fileDlg(parentWidget());
		fileDlg.setWindowTitle(u8"GrandMapping");
		fileDlg.setNameFilter(strFileFilter);
		fileDlg.setFileMode(QFileDialog::ExistingFiles);
		fileDlg.setDirectory(g_pSoftwareSetting->GetRecentOpenPath());

		if (fileDlg.exec() == QDialog::Accepted)
		{
			QStringList listFileName = fileDlg.selectedFiles();
			if (listFileName.isEmpty())
				return;

			QFileInfo fInfo(listFileName.front());
			g_pSoftwareSetting->SetRecentOpenPath(fInfo.dir().absolutePath());

			m_strSubtitleFilePath = listFileName;

			LOG_OPT_PC(CLogRecordTranslate::LogProperty(), CPropertyTranslate::ResuorcePropertyWnd(),
				CCommonTranslate::Subtitle(), CCommonTranslate::Subtitle(), CSubtitleTranslate::ImprotFile(), CLogRecordTranslate::Save());
		}
		else
		{
			LOG_OPT_PC(CLogRecordTranslate::LogProperty(), CPropertyTranslate::ResuorcePropertyWnd(),
				CCommonTranslate::Subtitle(), CCommonTranslate::Subtitle(), CSubtitleTranslate::ImprotFile(), CLogRecordTranslate::Close());
		}
	}
}

void CSubtitleModeSelectDlg::slotBtnClicked()
{
	QObject* pSender = sender();
	if (pSender == m_pOKButton)
	{
		OpenResFileSys(); //字幕打开文件系统
		QDialog::accept();
		return;
	}
		
	m_bSubtitle = (pSender == m_pBtnSubtitle);

	m_pBtnSubtitle->setChecked(m_bSubtitle);
	//m_pBtnTeleprompter->setChecked(!m_bSubtitle);
}

void CSubtitleModeSelectDlg::slotBtnDoubleClicked()
{
	QObject* pSender = sender();
	m_bSubtitle = (pSender == m_pBtnSubtitle);

	OpenResFileSys(); //字幕打开文件系统
	QDialog::accept();
}
