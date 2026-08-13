#include "RemoveTimeDlg.h"

#include "Common/UICommon/TimeEditWidget.h"
#include "Common/UICommon/UICommonSizeDefine.h"
#include "Common/UICommon/UITranslate.h"
#include "Common/UICommon/Messagebox.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include <QVBoxLayout>
#include <QPushButton>
#include "TimelineWinMacros.h"
//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

using namespace std::chrono_literals;

namespace
{
	constexpr int kLabelWidth = 104;
	constexpr int kDefaultRemoveLengthMs = 1000;

	void ConfigureTimeEdit(CTimeEditWidget* pEdit, int minMs, int maxMs)
	{
		pEdit->SetShowMs(true);
		pEdit->SetMinMaxTime(minMs, maxMs);
	}
}

CRemoveTimeDlg::CRemoveTimeDlg(QWidget* parent, std::chrono::milliseconds defaultPosition)
	: CFramelessDialog(parent)
{
	InitWidgets();
	CreateConnections();
	UpdateTranslateText();

	m_pPositionEdit->SetTime(static_cast<int>(defaultPosition.count()));
	m_pLengthEdit->SetTime(kDefaultRemoveLengthMs);
}

CRemoveTimeDlg::~CRemoveTimeDlg()
{
}

std::chrono::milliseconds CRemoveTimeDlg::GetPosition() const
{
	return std::chrono::milliseconds(m_pPositionEdit->GetMillisecond());
}

std::chrono::milliseconds CRemoveTimeDlg::GetLength() const
{
	return std::chrono::milliseconds(m_pLengthEdit->GetMillisecond());
}

ETimeEditScope CRemoveTimeDlg::GetScope() const
{
	return ETimeEditScope::LayersOnly;
}

void CRemoveTimeDlg::InitWidgets()
{
	setFixedSize(SysDpiZoom(496), SysDpiZoom(180));

	m_pPositionEdit = new CTimeEditWidget(this);
	m_pPositionEdit->SetLabelConfig(SysDpiZoom(kLabelWidth), Qt::AlignRight | Qt::AlignVCenter, nSizeDefine_16);
	m_pPositionEdit->SetFps(g_pProjectSetting->LoadTimeLineFps());
	if (g_pTimeLineModel)
	{
		const int maxMs = static_cast<int>(g_pTimeLineModel->GetDuration().count());
		ConfigureTimeEdit(m_pPositionEdit, 0, maxMs);
	}
	else
	{
		ConfigureTimeEdit(m_pPositionEdit, 0, 24 * 3600 * 1000);
	}

	m_pLengthEdit = new CTimeEditWidget(this);
	m_pLengthEdit->SetLabelConfig(SysDpiZoom(kLabelWidth), Qt::AlignRight | Qt::AlignVCenter, nSizeDefine_16);
	m_pLengthEdit->SetFps(g_pProjectSetting->LoadTimeLineFps());
	if (g_pTimeLineModel)
	{
		const int maxMs = g_pTimeLineModel
			? static_cast<int>(g_pTimeLineModel->GetDuration().count())
			: 24 * 3600 * 1000;
		ConfigureTimeEdit(m_pLengthEdit, 1, maxMs);
	}

	auto* pMainLayout = new QVBoxLayout;
	pMainLayout->setMargin(0);
	pMainLayout->setSpacing(nSizeDefine_20);
	pMainLayout->setContentsMargins(nSizeDefine_14, nSizeDefine_32, nSizeDefine_72, 0);
	pMainLayout->addWidget(m_pPositionEdit);
	pMainLayout->addWidget(m_pLengthEdit);
	pMainLayout->addStretch();
	setLayout(pMainLayout);
}

void CRemoveTimeDlg::CreateConnections()
{
	m_pOKButton->disconnect();
	connect(m_pOKButton, &QAbstractButton::clicked, this, [this]() {
		if (!ValidateInput())
		{
			UICommon::CMessageBox::warning(this, "GrandMapping",
				CTimeLineWndTranslate::TimeEditInvalidParam(), QMessageBox::Ok);
			return;
		}
		QDialog::accept();
	});
}

void CRemoveTimeDlg::UpdateTranslateText()
{
	SetTitleName(CTimeLineWndTranslate::RemoveTimeDlgTitle());
	m_pPositionEdit->SetName(CTimeLineWndTranslate::TimelinePosition());
	m_pLengthEdit->SetName(CTimeLineWndTranslate::RemoveLength());

	m_pOKButton->setText(CCommonTranslate::OK());
	m_pCancelButton->setText(CCommonTranslate::Cancel());
}

bool CRemoveTimeDlg::ValidateInput() const
{
	const auto position = GetPosition();
	const auto length = GetLength();
	if (position.count() < 0 || length.count() <= 0)
		return false;

	if (position + length > g_pTimeLineModel->GetDuration())
		return false;
	return true;
}
