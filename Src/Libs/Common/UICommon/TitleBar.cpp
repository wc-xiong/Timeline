#include "TitleBar.h"
#include "SvgImageButton.h"
#include "UICommonSizeDefine.h"

#include <QLabel>
#include <QEvent>
#include <QDebug>
#include <QStyle>
#include <QScreen>
#include <QBitmap>
#include <QAction>
#include <QPainter>
#include <QVariant>
#include <QLineEdit>
#include <QMouseEvent>
#include <QToolButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QButtonGroup>
#include <QApplication>
#include <QDesktopWidget>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

using namespace UICommon;

CTitleBar::CTitleBar(QWidget *parent): CBaseFrame(parent)
{	
	InitWidget();
	CreateConnects();
}

CTitleBar::~CTitleBar()
{
}

void CTitleBar::InitWidget()
{
	setFixedHeight(SysDpiZoom(32));
	setProperty("TitleBar", true);
	setProperty("DlgTitleBar", true);

	m_pLabelTitle = new QLabel(this);
	m_pLabelTitle->setProperty("TitleBarName", true);
	m_pLabelTitle->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);

	m_pTitleTips = new QLabel(this);
	m_pTitleTips->setProperty("TitleBarNameTips", true);
	m_pTitleTips->hide();

	m_vecButtonVisible.resize(CTitleBar::ETBTCount);
	m_vecButtonEnabled.resize(CTitleBar::ETBTCount);
	for (int i = 0; i < CTitleBar::ETBTCount; i++)
	{
		m_vecButtonEnabled[i] = true;
		m_vecButtonVisible[i] = true;
	}

	for (int idx = 0; idx < CTitleBar::ETBTMinButton; ++idx)
	{
		QToolButton* pBtn = new QToolButton(this);
		pBtn->setAutoRaise(true);
		pBtn->setAutoFillBackground(true);
		pBtn->setFocusPolicy(Qt::NoFocus);
		pBtn->setCheckable(false);
		pBtn->setProperty("TitleButton", true);

		m_vecButtons.push_back(pBtn);
	}

	m_vecButtons[CTitleBar::ETBTLogoButton]->setProperty("LogoButton", true);
	m_vecButtons[CTitleBar::ETBTLogoButton]->setFixedSize(SysDpiZoom(20), SysDpiZoom(20));
	m_vecButtons[CTitleBar::ETBTLogoButton2]->setProperty("LogoButton2", true);
	m_vecButtons[CTitleBar::ETBTLogoButton2]->setFixedSize(SysDpiZoom(100), SysDpiZoom(20));
	m_vecButtons[CTitleBar::ETBTLogoButton2]->setText("GrandMapping");
	m_vecButtons[CTitleBar::ETBTLogoButton2]->setEnabled(false);

	auto AddBtn = [&](const QString &strImagePath, const QString &strTips) {
		CSvgImageButton* pBtn = new CSvgImageButton(strImagePath, this, false);
		pBtn->setFixedSize(SysDpiZoom(24), SysDpiZoom(24));
		pBtn->setProperty("transparentBkBtn", true);
		pBtn->setToolTip(strTips);
		m_vecButtons.push_back(pBtn);
	};

	AddBtn(":/dark/images/TitleBar/Min", QStringLiteral("最小化"));
	AddBtn(":/dark/images/TitleBar/Max", QStringLiteral("最大化"));
	AddBtn(":/dark/images/TitleBar/Restore", QStringLiteral("还原"));
	AddBtn(":/dark/images/TitleBar/DlgClose", QStringLiteral("关闭"));

	m_vecButtons[CTitleBar::ETBTCloseButton]->setProperty("TitleCloseBtn", true);

	// 创建左边水平布局
	m_pLeftTitleBtnLayout = new QHBoxLayout;
	m_pLeftTitleBtnLayout->setMargin(0);
	m_pLeftTitleBtnLayout->setContentsMargins(SysDpiZoom(10), 0, 0, 0);
	m_pLeftTitleBtnLayout->setSpacing(SysDpiZoom(6));

	// 创建右边水平布局
	m_pRightTitleBtnLayout = new QHBoxLayout;
	m_pRightTitleBtnLayout->setMargin(0);
	m_pRightTitleBtnLayout->setContentsMargins(SysDpiZoom(10), 0, 0, 0);
	m_pRightTitleBtnLayout->setSpacing(0);

	auto pButtonLayout = new QHBoxLayout;
	pButtonLayout->setMargin(0);
	pButtonLayout->setSpacing(SysDpiZoom(8));
	pButtonLayout->addWidget(m_vecButtons[CTitleBar::ETBTMinButton]);
	pButtonLayout->addWidget(m_vecButtons[CTitleBar::ETBTMaxButton]);
	pButtonLayout->addWidget(m_vecButtons[CTitleBar::ETBTRestoreButton]);
	pButtonLayout->addWidget(m_vecButtons[CTitleBar::ETBTCloseButton]);
	pButtonLayout->addSpacing(SysDpiZoom(8));

	QHBoxLayout* mainLayout = new QHBoxLayout;
	mainLayout->setSpacing(0);
	mainLayout->addWidget(m_vecButtons[CTitleBar::ETBTLogoButton]);
	mainLayout->addSpacing(SysDpiZoom(2));
	mainLayout->addWidget(m_vecButtons[CTitleBar::ETBTLogoButton2]);
	
	mainLayout->addWidget(m_pLabelTitle);
	mainLayout->addSpacing(SysDpiZoom(8));
	mainLayout->addWidget(m_pTitleTips, 1, Qt::AlignLeft | Qt::AlignVCenter);
	
	mainLayout->addLayout(m_pLeftTitleBtnLayout);
	mainLayout->addStretch();

	mainLayout->addLayout(m_pRightTitleBtnLayout);
	mainLayout->addLayout(pButtonLayout);

	mainLayout->setContentsMargins(SysDpiZoom(10), 0, 0, 0);
	setLayout(mainLayout);
}

void CTitleBar::CreateConnects()
{
	connect(m_vecButtons[CTitleBar::ETBTMinButton],		&QToolButton::clicked,	this,	&CTitleBar::signalMin);
	connect(m_vecButtons[CTitleBar::ETBTMaxButton],		&QToolButton::clicked,	this,	&CTitleBar::slotSwitchMaxMin);
	connect(m_vecButtons[CTitleBar::ETBTRestoreButton], &QToolButton::clicked,	this,	&CTitleBar::slotSwitchMaxMin);
	connect(m_vecButtons[CTitleBar::ETBTCloseButton],	&QToolButton::clicked,	this,	&CTitleBar::signalClose);
}

void CTitleBar::UpdateTranslateText()
{
	m_vecButtons[CTitleBar::ETBTMinButton]->setToolTip(QStringLiteral("最小化"));
	m_vecButtons[CTitleBar::ETBTMaxButton]->setToolTip(QStringLiteral("最大化"));
	m_vecButtons[CTitleBar::ETBTRestoreButton]->setToolTip(QStringLiteral("还原"));
	m_vecButtons[CTitleBar::ETBTCloseButton]->setToolTip(QStringLiteral("关闭"));
}

void CTitleBar::SetTitleAlignment(Qt::Alignment align)
{
	m_pLabelTitle->setAlignment(align);
}

void CTitleBar::SetTitleButtonIcon(const int & buttonType, const QIcon & icon)
{
	Q_ASSERT(buttonType >= 0 && buttonType < ETBTCount);
	if (buttonType >= 0 && buttonType < ETBTCount)
	{
		m_vecButtons[buttonType]->setIcon(icon);
	}
}

QIcon CTitleBar::TitleButtonIcon(const int & buttonType)
{
	Q_ASSERT(buttonType >= 0 && buttonType < ETBTCount);
	if (buttonType >= 0 && buttonType < ETBTCount)
	{
		return m_vecButtons[buttonType]->icon();
	}

	return QIcon();
}

void CTitleBar::SetButtonVisible(const int &iButtonType, bool bVisible)
{
	Q_ASSERT(iButtonType >= 0 && iButtonType < ETBTCount);
	if (iButtonType >= 0 && iButtonType < ETBTCount)
	{
		m_vecButtons[iButtonType]->setVisible(bVisible);
		m_vecButtonVisible[iButtonType] = bVisible;
	}
	else
	{
		for (int i = 0; i < ETBTCount; i++)
		{
			m_vecButtons[i]->setVisible(bVisible);
			m_vecButtonVisible[i] = bVisible;
		}
	}
}

bool CTitleBar::IsButtonVisible(const int & buttonType)
{
	Q_ASSERT(buttonType >= 0 && buttonType < ETBTCount);
	if (buttonType >= 0 && buttonType < ETBTCount)
	{
		return m_vecButtonVisible[buttonType];
	}

	return false;
}

void CTitleBar::SetButtonEnable(const int & iButtonType, bool bEnable)
{
	Q_ASSERT(iButtonType >= 0 && iButtonType < ETBTCount);
	if (iButtonType >= 0 && iButtonType < ETBTCount)
	{
		m_vecButtons[iButtonType]->setEnabled(bEnable);
		m_vecButtonEnabled[iButtonType] = bEnable;
	}
	else
	{
		for (int i = 0; i < ETBTCount; i++)
		{
			m_vecButtons[i]->setEnabled(bEnable);
			m_vecButtonEnabled[i] = bEnable;
		}
	}
}

void CTitleBar::SetButtonFocusPolicy(const int& iButtonType, int policy)
{
	Q_ASSERT(iButtonType >= 0 && iButtonType < ETBTCount);
	if (iButtonType >= 0 && iButtonType < ETBTCount)
	{
		m_vecButtons[iButtonType]->setFocusPolicy(Qt::FocusPolicy(policy));
	}
}

bool CTitleBar::IsButtonEnable(const int & buttonType)
{
	Q_ASSERT(buttonType >= 0 && buttonType < ETBTCount);
	if (buttonType >= 0 && buttonType < ETBTCount)
	{
		return m_vecButtonEnabled[buttonType];
	}

	return false;
}

void CTitleBar::SetMaximized(bool bValue)
{
	m_bIsMaximized = bValue;

	if (m_vecButtonVisible[CTitleBar::ETBTMaxButton])
		m_vecButtons[CTitleBar::ETBTMaxButton]->setVisible(!m_bIsMaximized);
	if (m_vecButtonVisible[CTitleBar::ETBTRestoreButton])
		m_vecButtons[CTitleBar::ETBTRestoreButton]->setVisible(m_bIsMaximized);
}

void CTitleBar::SetTitleName(const QString &title)
{
	m_realTitleName = title;

	QString showText = title;
	if (m_bElideTitleName)
	{
		QFontMetrics fm(m_pLabelTitle->font());
		showText = fm.elidedText(title, Qt::ElideRight, m_pLabelTitle->width());
		m_pLabelTitle->setToolTip(title);
	}
	m_pLabelTitle->setText(showText);
}

QString CTitleBar::GetTitleName() const
{
	return m_realTitleName;
}

void CTitleBar::SetTitleNameProperty(const char* strProperty, bool bValue)
{
	m_pLabelTitle->setProperty(strProperty, bValue);
}

void CTitleBar::SetTitleFixedWidth(int nWidth)
{
	m_pLabelTitle->setFixedWidth(nWidth);
}

void UICommon::CTitleBar::SetTitleTipsVisible(bool bVisible)
{
	m_pTitleTips->setVisible(bVisible);
}

void UICommon::CTitleBar::SetTitleTipsText(const QString& strText)
{
	m_pTitleTips->setText(strText);
}

void  UICommon::CTitleBar::SetTitleNameElided(bool bElided)
{
	m_bElideTitleName = bElided;
}

void UICommon::CTitleBar::SetTitleTipsPty(const char* strProperty, const QVariant& value)
{
	m_pTitleTips->style()->unpolish(m_pTitleTips);
	m_pTitleTips->setProperty(strProperty, value);
	m_pTitleTips->style()->polish(m_pTitleTips);
}

void CTitleBar::AppendLeftTitleSplitLine()
{
	QFrame* phframe = new QFrame(this);
	phframe->setFrameShape(QFrame::HLine);      // 设置水平方向
	phframe->setFixedSize(nSizeDefine_1, nSizeDefine_16);
	phframe->setStyleSheet("QFrame{background:black}");
	m_pLeftTitleBtnLayout->addWidget(phframe);

}

QAbstractButton* CTitleBar::AppendLeftTitleButton(QAction *pAction, QSize size, bool bImageBtn, const QString &strImagePath)
{
	int cnt = m_pLeftTitleBtnLayout->count();
	return InsertLeftTitleButton(cnt, pAction, size, bImageBtn, strImagePath);
}

QAbstractButton* CTitleBar::InsertLeftTitleButton(int idx, QAction *pAction, QSize size, bool bImageBtn, const QString &strImagePath)
{
	QAbstractButton* btn = nullptr;
	if (bImageBtn)
	{
		btn = new CSvgImageButton(strImagePath, this, false);
		btn->setProperty("transparentBkBtn", true);
		btn->setFixedSize(size);
		btn->setToolTip(pAction->toolTip());
		btn->addAction(pAction);
	}
	else
	{
		QToolButton* pbtn = new QToolButton(this);
		pbtn->setProperty("TitleStageToolButton", true);
		pbtn->setDefaultAction(pAction);
		pbtn->setToolTip("");
		btn = pbtn;
		btn->setFixedHeight(SysDpiZoom(28));
	}
	
	m_pLeftTitleBtnLayout->insertWidget(idx, btn);
	return btn;
}

QAbstractButton *CTitleBar::AppendRightTitleButton(QAction *pAction, QSize size, bool bImageBtn, const QString &strImagePath)
{
	int cnt = m_pRightTitleBtnLayout->count();
	return InsertRightTitleButton(cnt, pAction, size, bImageBtn, strImagePath);
}

QAbstractButton *CTitleBar::InsertRightTitleButton(int idx, QAction *pAction, QSize size, bool bImageBtn, const QString &strImagePath)
{
	QAbstractButton* btn = nullptr;
	if (bImageBtn)
	{
		btn = new CSvgImageButton(strImagePath, this, false);
		btn->setProperty("transparentBkBtn", true);
		btn->setFixedSize(size);
		btn->setToolTip(pAction->toolTip());
	}
	else
	{
		QToolButton* pbtn = new QToolButton(this);
		pbtn->setProperty("TitleToolButton", true);
		pbtn->setDefaultAction(pAction);
		pbtn->setToolTip("");
		btn = pbtn;
		btn->setFixedHeight(SysDpiZoom(28));
	}
	btn->setIcon(pAction->icon());
	m_pRightTitleBtnLayout->insertWidget(idx, btn);
	return btn;
}

void UICommon::CTitleBar::InsertRightTitleWidget(QWidget* pWnd)
{
	m_pRightTitleBtnLayout->insertWidget(0, pWnd);
	m_pRightTitleBtnLayout->insertSpacing(1, SysDpiZoom(16));
}

QAbstractButton * CTitleBar::TitleButton(const int & buttonType)
{
	Q_ASSERT(buttonType >= 0 && buttonType < ETBTCount);
	if (buttonType >= 0 && buttonType < ETBTCount)
	{
		return m_vecButtons[buttonType];
	}

	return nullptr;
}

void CTitleBar::SetSupportDoubleClicked(bool bSupport)
{
	m_bSupportDoubleClicked = bSupport;
}

void CTitleBar::SetIconVisible(bool bValue)
{
	m_vecButtons[CTitleBar::ETBTLogoButton]->setVisible(bValue);
	m_vecButtons[CTitleBar::ETBTLogoButton2]->setVisible(bValue);
}

void CTitleBar::slotSwitchMaxMin()
 {
	clearFocus();

	if (!m_bIsMaximized)
	{
		emit signalMax();

		//还原再最大化后，还原按钮的hover状态不会退出。
		QHoverEvent hoverEvent(QEvent::HoverLeave, QPoint(10, 10), QPoint(0, 0));
		m_vecButtons[CTitleBar::ETBTRestoreButton]->setAttribute(Qt::WA_UnderMouse, false);
		QCoreApplication::sendEvent(m_vecButtons[CTitleBar::ETBTRestoreButton], &hoverEvent);
	}
	else
		emit signalNormal();

	SetMaximized(!m_bIsMaximized); 
}

void CTitleBar::slotClearChecked()
{
	for (auto &btn : m_vecButtons)
		btn->setChecked(false);
}

void CTitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
	event->accept();

	if (!m_bSupportDoubleClicked)
		return;

	slotSwitchMaxMin();

	if (!m_bIsMaximized)
	{
		if (event->globalX() < 0)
			parentWidget()->move(parentWidget()->frameGeometry().topLeft() - event->globalPos() * 0.3f);
		else
			parentWidget()->move(event->globalX() * 0.3f, 0);
	}
	m_ptLeftMouseDown = event->globalPos();
}

void CTitleBar::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton
		&& event->pos().y() >= nSizeDefine_1 
		&& event->pos().x() >= nSizeDefine_1
		&& event->pos().x() <= this->width() - nSizeDefine_1)
	{
		m_bMousePressed = true;
		m_ptLeftMouseDown = event->globalPos();
		m_ptDragPos = event->globalPos() - parentWidget()->frameGeometry().topLeft();
	}
	__super::mousePressEvent(event);
}

void CTitleBar::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton)
		m_bMousePressed = false;
	__super::mouseReleaseEvent(event);

	m_vecButtons[CTitleBar::ETBTLogoButton]->setFocus();	// 转移焦点
}

void CTitleBar::mouseMoveEvent(QMouseEvent *event)
{
	if (m_bMousePressed)
	{
		QPoint ptDis = event->globalPos() - m_ptLeftMouseDown;
		if (ptDis.manhattanLength() > nSizeDefine_5)	// 5像素的容差
		{
			if (m_bIsMaximized)
			{
				m_nOldwidth = parentWidget()->frameGeometry().width();
				slotSwitchMaxMin();	
				m_bChangeFlag = false;
				return;
			}

			if (!m_bChangeFlag)
			{
				m_bChangeFlag = true;

				float radio = m_ptDragPos.x() / (float)m_nOldwidth;
				int newwidth = parentWidget()->frameGeometry().width();
				m_ptDragPos.setX(radio * newwidth);
			}

			parentWidget()->move(event->globalPos() - m_ptDragPos);

		}
	}

	__super::mouseMoveEvent(event);
}
