#include "ProgressDialogEx.h"

#include "UICommonSizeDefine.h"
#include "Common/UICommon/UITranslate.h"

#include <QLabel>
#include <QDebug>
#include <QBitmap>
#include <QVariant>
#include <QPainter>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QProgressBar>

using namespace UICommon;

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CProgressDialogEx::CProgressDialogEx(int nType, int nValue1, int nValue2, QWidget *parent /* = nullptr */)
	:CFramelessDialog(parent)
{
	m_nProgressType = nType;
	if (m_nProgressType == EPT_Timing)
	{
		m_nMinValue = 0;
		m_nMaxValue = nValue1;
		m_nTimeInterval = nValue2;
	}
	else
	{
		m_nMinValue = nValue1;
		m_nMaxValue = nValue2;
	}

	InitWidget();

	connect(this, &CProgressDialogEx::signalUpdateProgress, this, &CProgressDialogEx::slotUpdateProgress);
}

CProgressDialogEx::CProgressDialogEx(QWidget *parent /*= nullptr*/)
	:CFramelessDialog(parent)
{
	InitWidget();

	connect(this, &CProgressDialogEx::signalUpdateProgress, this, &CProgressDialogEx::slotUpdateProgress);
}

CProgressDialogEx::~CProgressDialogEx()
{
	qDebug() << "~CProgressDialogEx";
}

void CProgressDialogEx::SetTimeConfig(int nTimeLenght, int nTimeInterval)
{
	m_nCurValue = 0;
	m_nProgressType = EPT_Timing;
	m_nMinValue = 0;
	m_nMaxValue = nTimeLenght;
	m_nTimeInterval = nTimeInterval;
	m_pProgress->setRange(m_nMinValue, m_nMaxValue);
}

void CProgressDialogEx::SetMinMaxRange(int nMinValue, int nMaxValue, int nProgressType)
{
	m_nCurValue = 0;
	m_nProgressType = nProgressType;
	if(m_nProgressType == EPT_RangeInt)
		m_pProgress->setFixedSize(SysDpiZoom(352), SysDpiZoom(6));
	else
		m_pProgress->setFixedSize(SysDpiZoom(404), SysDpiZoom(6));

	m_nMinValue = nMinValue;
	m_nMaxValue = nMaxValue;
	m_pLabelValue->setVisible((m_nMaxValue - m_nMinValue > 0));
	m_pLabelContain->setFixedWidth(m_nMaxValue - m_nMinValue <= 0 ? nSizeDefine_20 : nSizeDefine_72);
	m_pProgress->setRange(m_nMinValue, m_nMaxValue);
}

void CProgressDialogEx::SetProgressValue(int nValue)
{
	emit signalUpdateProgress(nValue);
}

void CProgressDialogEx::SetTitleName(const QString &strText)
{
	m_pTitleBar->SetTitleName(strText);
}

void CProgressDialogEx::slotAddValue(int nValue)
{
	m_nCurValue += nValue;
	SetProgressValue(m_nCurValue);
}

void CProgressDialogEx::slotSetValue(int nValue)
{
	SetProgressValue(nValue);
}

void CProgressDialogEx::slotAddValueAndSetTitle(int nValue, const QString &title)
{
	slotAddValue(nValue);

	//m_pTitleBar->SetTitleName(title);
}

void CProgressDialogEx::timerEvent(QTimerEvent *event)
{
	m_nCurValue += m_nTimeInterval;
	SetProgressValue(m_nCurValue);
}

int CProgressDialogEx::exec()
{
	if (m_nProgressType == EPT_Timing)
		m_nTimerID = startTimer(m_nTimeInterval, Qt::PreciseTimer);

	if (m_nCurValue >= m_nMaxValue)
		return QDialog::Accepted;

	return QDialog::exec();
}

void CProgressDialogEx::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_Escape:
		return;
	default:
		break;
	}
	__super::keyPressEvent(event);
}

void CProgressDialogEx::InitWidget()
{
	setFixedSize(SysDpiZoom(468), SysDpiZoom(140));

	setWindowModality(Qt::WindowModal);
	setBottomBarVisible(false);

	m_pTitleBar->SetButtonVisible(CTitleBar::ETBTMinButton, false);
	m_pTitleBar->SetButtonVisible(CTitleBar::ETBTCloseButton, false);
	m_pTitleBar->SetTitleName(CPackProjectTranslate::Packaging());

	m_pProgress = new QProgressBar(this);
	m_pProgress->setFixedSize(SysDpiZoom(352),SysDpiZoom(6));
	m_pProgress->setTextVisible(false);

	m_pProgress->setRange(m_nMinValue, m_nMaxValue);

	m_pLabelValue = new QLabel(this);
	m_pLabelValue->setText("0%");
	m_pLabelValue->setVisible((m_nMaxValue - m_nMinValue > 0));

	m_pLabelContain = new QFrame(this);

	m_pLabelContain->setFixedWidth(m_nMaxValue - m_nMinValue <= 0 ? nSizeDefine_20 : nSizeDefine_72);
	m_pLabelContain->setProperty("ShortcutTableFrame", true);
	QHBoxLayout* pLabelLayout = new QHBoxLayout(m_pLabelContain);
	pLabelLayout->setSpacing(0);
	pLabelLayout->setMargin(0);

	m_pLabelValue->setFixedWidth(nSizeDefine_72);
	m_pLabelValue->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	pLabelLayout->addWidget(m_pLabelValue);

	QHBoxLayout* layout = new QHBoxLayout;
	layout->setMargin(0);
	layout->setSpacing(0);

	layout->addSpacing(SysDpiZoom(20));
	layout->addWidget(m_pProgress, 0, Qt::AlignVCenter);
	layout->addSpacing(SysDpiZoom(4));
	layout->addWidget(m_pLabelContain);
	//layout->addStretch();

	setLayout(layout);
}

void CProgressDialogEx::CloseProgressDialog()
{
	if (m_nTimerID != -1)
		killTimer(m_nTimerID);
	emit done(QDialog::Accepted);

	SetProgressValue(0);
}

int CProgressDialogEx::GetProgressCurValue()
{
	return m_nCurValue;
}

void UICommon::CProgressDialogEx::SetCloseBtnShowState(bool bShow)
{
	m_pTitleBar->SetButtonVisible(CTitleBar::ETBTCloseButton, bShow);
}

void UICommon::CProgressDialogEx::SetMaxOffFlag(bool bFlag)
{
	m_bMaxOff = bFlag;
}

void UICommon::CProgressDialogEx::resizeEvent(QResizeEvent *event)
{
	__super::resizeEvent(event);
}

void UICommon::CProgressDialogEx::slotUpdateProgress(int nValue)
{
	if (m_nMaxValue - m_nMinValue > 0)
	{
		QString strText;
		if (m_nProgressType == EPT_RangePrecent)
		{
			double fPrecent = double(nValue) / double(m_nMaxValue - m_nMinValue) * 100.0f;
			strText = QString("%1%").arg(int(fPrecent));
		}
		else
		{
			strText = QString("%1/%2").arg(nValue).arg(m_nMaxValue - m_nMinValue);
		}
		m_pLabelValue->setText(strText);
		m_pLabelValue->update();
	}

	m_nCurValue = nValue;
	m_pProgress->setValue(nValue);
	m_pProgress->update();

	if (nValue >= m_nMaxValue && m_bMaxOff)
		CloseProgressDialog();
}

