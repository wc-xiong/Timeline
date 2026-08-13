#include "FramelessDialog.h"
#include "Framelesshelper.h"
#include "Titlebar.h"
#include "UITranslate.h"

#include <QLayout>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QPalette>
#include <QIcon>
#include <QLayout>
#include <QGraphicsDropShadowEffect>
#include <QVariant>
#include <QDesktopWidget>
#include <QApplication>
#include <QPushButton>
#include <QAbstractButton>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <Windows.h>
#include <WinUser.h>
#include <QStyle>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include "UICommonSizeDefine.h"
#include <QKeyEvent>

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

CFramelessDialog::CFramelessDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f | Qt::X11BypassWindowManagerHint)
    , m_pContentHolder(nullptr)
    , m_pBottomWidget(nullptr)
    , m_pCentralLayout(nullptr)
{
	m_clrBackground = QColor(39, 41, 44);

	m_nDlgShadowWidth = nSizeDefine_10;
	m_nDlgRadius = nSizeDefine_5;
    createWidgets();
    createConnects();
    setMinButtonVisible(false);
    setMaxButtonVisible(false);
}

CFramelessDialog::~CFramelessDialog()
{

}

void CFramelessDialog::setTitleButtonIcon(const int &buttonType, const QIcon &icon)
{
    m_pTitleBar->SetTitleButtonIcon(buttonType, icon);
}

void CFramelessDialog::setDialogMovable(bool b)
{
    m_pFramelessHelper->SetMovable(false);  //设置窗体可移动
}

void CFramelessDialog::setRubberBandOnMove(bool b)
{
    m_pFramelessHelper->SetRubberBandOnMove(false);  //设置橡皮筋效果-可移动
}

void CFramelessDialog::setDialogResizable(bool b)
{
    m_pFramelessHelper->SetResizable(b);  //设置窗体可缩放
}

void CFramelessDialog::setRubberBandOnResize(bool b)
{
    m_pFramelessHelper->SetRubberBandOnResize(b);
}

void CFramelessDialog::setLayout(QLayout *layout)
{
	if (m_pCentralLayout)
	{
		m_pMainLayout->removeItem(m_pCentralLayout);
		delete m_pCentralLayout;
	}
    QMargins margins = layout->contentsMargins();
    m_pCentralLayout = layout;
    m_pCentralLayout->setContentsMargins(margins.left(), margins.top(),
        margins.right(), margins.bottom());
    m_pMainLayout->insertLayout(1, m_pCentralLayout);
}

QLayout *CFramelessDialog::layout() const
{
    if (!m_pCentralLayout && m_pContentHolder)
        return m_pContentHolder->layout();
    else
        return m_pCentralLayout;
}

QWidget *CFramelessDialog::getContentHolder()
{
    if (!m_pContentHolder)
    {
        m_pContentHolder = new QWidget(this);
        m_pContentHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_pMainLayout->insertWidget(2, m_pContentHolder);
    }
    return m_pContentHolder;
}
void CFramelessDialog::setWindowTitle(const QString &title)
{
    m_pTitleBar->SetTitleName(title);
}

void CFramelessDialog::setWindowTitleAlignment(Qt::Alignment align /*= Qt::AlignCenter*/)
{
    m_pTitleBar->SetTitleAlignment(align);
}

QString CFramelessDialog::windowTitle() const
{
    return m_pTitleBar->GetTitleName();
}

void CFramelessDialog::setWindowIcon(const QIcon &icon)
{
    m_pTitleBar->SetTitleButtonIcon(CTitleBar::ETBTLogoButton, icon);
    QDialog::setWindowIcon(icon);
}

QIcon CFramelessDialog::windowIcon() const
{
    return m_pTitleBar->TitleButtonIcon(CTitleBar::ETBTLogoButton);
}

QVBoxLayout* CFramelessDialog::getMainLayout()
{
    return  m_pMainLayout;
}

void CFramelessDialog::createWidgets()
{
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    setAttribute(Qt::WA_TranslucentBackground, true);

    m_pTitleBar = new CTitleBar(this);
    m_pTitleBar->SetTitleAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_pTitleBar->SetButtonFocusPolicy(CTitleBar::ETBTCloseButton, Qt::NoFocus);
	m_pTitleBar->SetIconVisible(false);
    m_pTitleBarLayout = new QHBoxLayout();
    m_pTitleBarLayout->setSpacing(0);
	m_pTitleBarLayout->setMargin(0);
    m_pTitleBarLayout->addWidget(m_pTitleBar);

    QDesktopWidget* desktopWidget = QApplication::desktop();
    setMaximumSize(desktopWidget->availableGeometry().size());

    m_pMainLayout = new QVBoxLayout();
    m_pMainLayout->setSpacing(0);
	int nMargin = m_nDlgShadowWidth + nSizeDefine_1;  // 内外边距的距离
	m_pMainLayout->setContentsMargins(nMargin, nMargin, nMargin - nSizeDefine_1, nMargin);

    m_pMainLayout->addLayout(m_pTitleBarLayout);

    m_pBottomWidget = new QWidget(this);
    m_pBottomLayout = new QHBoxLayout(m_pBottomWidget);
    m_pBottomLayout->setSpacing(nSizeDefine_12);
    m_pBottomLayout->setContentsMargins(SysDpiZoom(18), SysDpiZoom(5), SysDpiZoom(18), SysDpiZoom(16));
   
    m_pOKButton = new QPushButton(QStringLiteral("确定"), m_pBottomWidget);
	m_pOKButton->setProperty("BrandBlue", true);
	m_pOKButton->setFocusPolicy(Qt::NoFocus);
    m_pOKButton->setDefault(true);
	m_pOKButton->setFixedSize(UICommon_SmallBtnWidth, UICommon_CtrlHeight);

    m_pCancelButton = new QPushButton(QStringLiteral("取消"), m_pBottomWidget);
	m_pCancelButton->setFocusPolicy(Qt::NoFocus);
	m_pCancelButton->setFixedSize(UICommon_SmallBtnWidth, UICommon_CtrlHeight);
   
    m_pApplyButton  = new QPushButton(QStringLiteral("应用"), m_pBottomWidget);
	m_pApplyButton->setFixedSize(UICommon_SmallBtnWidth, UICommon_CtrlHeight);
    setApplyButtonVisible(false);
	m_pBottomLayout->addStretch();
    m_pBottomLayout->addWidget(m_pCancelButton, Qt::AlignHCenter | Qt::AlignRight);
    m_pBottomLayout->addWidget(m_pApplyButton, Qt::AlignHCenter | Qt::AlignRight);
	m_pBottomLayout->addWidget(m_pOKButton, Qt::AlignHCenter | Qt::AlignRight);
    m_pMainLayout->addWidget(m_pBottomWidget);
    QDialog::setLayout(m_pMainLayout);

    m_pFramelessHelper = new CFramelessHelper(this);
    m_pFramelessHelper->ActiveOn(this);  //激活当前窗体
	m_pFramelessHelper->SetBorderWidth(m_nDlgShadowWidth);
    m_pFramelessHelper->SetTitleHeight(m_pTitleBar->height());  //设置窗体的标题栏高度
	m_pFramelessHelper->SetResizable(true);
}

void CFramelessDialog::createConnects()
{
    connect(m_pTitleBar, &CTitleBar::signalClose, this, &CFramelessDialog::Close);
    connect(m_pTitleBar, &CTitleBar::signalMin, this, &CFramelessDialog::showMinimized);
    connect(m_pTitleBar, &CTitleBar::signalMax, this, &CFramelessDialog::swithMaxNormal);
    connect(m_pTitleBar, &CTitleBar::signalNormal, this, &CFramelessDialog::swithMaxNormal);

	connect(m_pOKButton, &QAbstractButton::clicked, this, &QDialog::accept);
	connect(m_pCancelButton, &QAbstractButton::clicked, this, &QDialog::reject);
}

void CFramelessDialog::swithMaxNormal()
{
	if (m_bDlgMax)	// 改变边界间隙是为了最大化时界面填充满整个屏幕
	{
		int nMargin = m_nDlgShadowWidth + nSizeDefine_2;  // 内外边距的距离
		m_pMainLayout->setContentsMargins(nMargin, nMargin, nMargin - nSizeDefine_1, nMargin);
		QDialog::showNormal();
		m_bDlgMax = false;
	}
	else
	{
		m_pMainLayout->setContentsMargins(0, 0, 0, 0);
		QDialog::showMaximized();
		m_bDlgMax = true;
	}
	this->update();
}

void CFramelessDialog::show()
{
    QDialog::show();
	m_pTitleBar->SetMaximized(isMaximized());
	m_bDlgMax = isMaximized();
}

void CFramelessDialog::showNormal()
{
    QDialog::showNormal();
	if (!parentWidget())
	{
		QSize szScreen = QGuiApplication::primaryScreen()->size();
		move((szScreen.width() - QDialog::width()) / 2, (szScreen.height() - QDialog::height()) / 2);
	}
	else
	{
		QRect rcGeometry = QApplication::desktop()->screenGeometry(parentWidget());
		move(rcGeometry.left() + (rcGeometry.width() - QDialog::width()) / 2, rcGeometry.top() + (rcGeometry.height() - QDialog::height()) / 2);
	}
}

int CFramelessDialog::exec()
{
	__super::setVisible(true);	// 必须先show，不然移动不生效

	if (!parentWidget())
	{
		QSize szScreen = QGuiApplication::primaryScreen()->size();
		move((szScreen.width() - QDialog::width()) / 2, (szScreen.height() - QDialog::height()) / 2);
	}
	else
	{
		QRect rcGeometry = QApplication::desktop()->screenGeometry(parentWidget());
		move(rcGeometry.left() + (rcGeometry.width() - QDialog::width()) / 2, rcGeometry.top() + (rcGeometry.height() - QDialog::height()) / 2);
	}

	__super::setVisible(false);

    return QDialog::exec();
}

int UICommon::CFramelessDialog::exec(const QPoint &point)
{
	__super::setVisible(true);	// 必须先show，不然移动不生效
	auto ptReal = point - QPoint(m_nDlgShadowWidth, m_nDlgShadowWidth);
	move(ptReal);
	__super::setVisible(false);

	return QDialog::exec();
}

void CFramelessDialog::setMinButtonVisible(bool b)
{
    m_pTitleBar->SetButtonVisible(CTitleBar::ETBTMinButton, b);
}

void UICommon::CFramelessDialog::SetTitleName(const QString &strName)
{
	m_pTitleBar->SetTitleName(strName);
}

void UICommon::CFramelessDialog::SetTitleBarVisible(bool bShow)
{
	m_pTitleBar->setVisible(false);
}

void UICommon::CFramelessDialog::setMinButtonEnable(bool b)
{
	m_pTitleBar->SetButtonEnable(CTitleBar::ETBTMinButton, b);
}

void CFramelessDialog::setMaxButtonVisible(bool b)
{
    m_pTitleBar->SetButtonVisible(CTitleBar::ETBTMaxButton, b);
    m_pTitleBar->SetButtonVisible(CTitleBar::ETBTRestoreButton, b);
    if (!b)
    {
        setDialogResizable(b);
    }
    else 
    {
        m_pTitleBar->SetMaximized(isMaximized());
    }
}

void UICommon::CFramelessDialog::setMaxButtonEnabled(bool b)
{
	m_pTitleBar->SetButtonEnable(CTitleBar::ETBTMaxButton, b);
}

void CFramelessDialog::setCloseButtonVisible(bool b)
{
    m_pTitleBar->SetButtonVisible(CTitleBar::ETBTCloseButton, b);
}

bool CFramelessDialog::getCloseButtonVisible() const
{
    if(m_pTitleBar)
        return m_pTitleBar->IsButtonVisible(CTitleBar::ETBTCloseButton);
    return false;
}

void CFramelessDialog::setApplyButtonVisible(bool b)
{
    m_pApplyButton->setVisible(b);
}

void CFramelessDialog::setOKButtonVisible(bool b)
{
    m_pOKButton->setVisible(b);
}

void CFramelessDialog::setOKButtonDefault(bool b)
{
    m_pOKButton->setDefault(b);
}

void CFramelessDialog::setCancelButtonVisible(bool b)
{
    m_pCancelButton->setVisible(b);
}

void CFramelessDialog::appendExtendTitleButton(QAction *pAction,
	QSize size,
	bool bImage,
	const QString &strImagePath)
{
    m_pTitleBar->AppendLeftTitleButton(pAction, size, bImage,strImagePath);
}

void CFramelessDialog::insertExtendTitleButton(int idx,
    QAction *pAction,
	QSize size,
	bool bImage,
	const QString &strImagePath)
{
    m_pTitleBar->InsertLeftTitleButton(idx, pAction, size, bImage, strImagePath);
}

QAbstractButton* CFramelessDialog::appendRightTitleButton(QAction *pAction,
    QSize size,
    bool bImage,
    const QString &strImagePath)
{
    return m_pTitleBar->AppendRightTitleButton(pAction, size, bImage, strImagePath);
}

void CFramelessDialog::setBottomBarVisible(bool b)
{
    m_pBottomWidget->setVisible(b);
}

void CFramelessDialog::SetTitleBarHeight(int height)
{
	m_pTitleBar->setFixedHeight(height);
	m_pFramelessHelper->SetTitleHeight(m_pTitleBar->height());
}

void CFramelessDialog::SetBackGroundColor(QColor color)
{
	m_clrBackground = color;
}

void CFramelessDialog::UpdateTranslateText()
{
	m_pOKButton->setText(CCommonTranslate::OK());
	m_pCancelButton->setText(CCommonTranslate::Cancel());
	m_pApplyButton->setText(CCommonTranslate::Apply());
}

void CFramelessDialog::changeEvent(QEvent *event)
{
    switch(event->type())
    {
    case QEvent::LanguageChange:
        UpdateTranslateText();
        break;
    default:
        break;
    }
	QDialog::changeEvent(event);
}

bool CFramelessDialog::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	MSG *msg = static_cast<MSG *>(message);
	if (msg->message == WM_NCACTIVATE)
	{
		m_bDialogFocus = (bool)(msg->wParam);
		update();
	}
	return __super::nativeEvent(eventType, message, result);
}

void UICommon::CFramelessDialog::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape)
	{
		event->accept();
		return;
	}
	return __super::keyPressEvent(event);
}

void UICommon::CFramelessDialog::paintEvent(QPaintEvent *event)
{
	__super::paintEvent(event);

	int nBorderMargin = m_bDlgMax ? 0 : m_nDlgShadowWidth;
	int nDlgRadius = m_bDlgMax ? 0 : m_nDlgRadius;

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	painter.setRenderHints(QPainter::TextAntialiasing, true);
	//TODO: 先填充背景，原来是透明的，用背景颜色进行填充
	QPainterPath path;
	path.setFillRule(Qt::WindingFill);
	path.addRoundedRect(nBorderMargin, nBorderMargin, QDialog::width() - nBorderMargin * 2, QDialog::height() - nBorderMargin * 2, nDlgRadius, nDlgRadius);
	painter.fillPath(path, QBrush(m_clrBackground));

	if (!m_bDlgMax)
	{
		// 绘制内外边框
		QPen BorderPen;
		BorderPen.setColor(QColor(100, 104, 109));
		BorderPen.setWidth(nSizeDefine_1);
		painter.setPen(BorderPen);
		painter.drawRoundedRect(nBorderMargin, nBorderMargin, QDialog::width() - nBorderMargin * 2, QDialog::height() - nBorderMargin * 2, nDlgRadius, nDlgRadius);

		//painter.setRenderHint(QPainter::Antialiasing, true);
		//painter.setPen(Qt::black);
		//painter.drawRoundedRect(nBorderMargin, nBorderMargin, QDialog::width() - nBorderMargin * 2, QDialog::height() - nBorderMargin * 2, nDlgRadius, nDlgRadius);
		//nBorderMargin++;	
		//painter.setPen(m_bDialogFocus ? QColor(70, 124, 212) : QColor(66, 70, 74));
		//painter.drawRoundedRect(nBorderMargin, nBorderMargin, QDialog::width() - nBorderMargin * 2, QDialog::height() - nBorderMargin * 2, nDlgRadius, nDlgRadius);

		painter.setRenderHint(QPainter::Antialiasing, true);
		QColor color(0, 0, 0, 50);
		for (int i = 1; i <= m_nDlgShadowWidth; i++)
		{
			color.setAlpha(std::max(int(120 - qSqrt(i) * 40), 0));
			painter.setPen(color);
			// drawRoundedRect 绘制带原角
			painter.drawRoundedRect(m_nDlgShadowWidth - i, m_nDlgShadowWidth - i, QDialog::width() - (m_nDlgShadowWidth - i) * 2, QDialog::height() - (m_nDlgShadowWidth - i) * 2, nDlgRadius * 2, nDlgRadius * 2);
		}
	}
}


void UICommon::CFramelessDialog::resizeEvent(QResizeEvent *event)
{
	__super::resizeEvent(event);

	// qDebug() << "----------------Framelessdialog Size is:" <<event->size();
}

void UICommon::CFramelessDialog::resize(const QSize &s)
{
	return QDialog::resize(s + QSize(m_nDlgShadowWidth * 2, m_nDlgShadowWidth * 2));
}

void UICommon::CFramelessDialog::setFixedHeight(int h)
{
	return QDialog::setFixedHeight(h + m_nDlgShadowWidth * 2);
}

void UICommon::CFramelessDialog::setFixedSize(const QSize &s)
{
	return QDialog::setFixedSize(s + QSize(m_nDlgShadowWidth * 2, m_nDlgShadowWidth * 2));
}

void UICommon::CFramelessDialog::setFixedWidth(int w)
{
	return QDialog::setFixedWidth(w + m_nDlgShadowWidth * 2);
}

int UICommon::CFramelessDialog::height() const
{
	return QDialog::height() - m_nDlgShadowWidth * 2;
}

int UICommon::CFramelessDialog::width() const
{
	return QDialog::width() - m_nDlgShadowWidth * 2;
}

QSize UICommon::CFramelessDialog::size() const
{
	return QDialog::size() - QSize(m_nDlgShadowWidth * 2, m_nDlgShadowWidth * 2);
}

void UICommon::CFramelessDialog::setFixedSize(int w, int h)
{
	return QDialog::setFixedSize(w + m_nDlgShadowWidth * 2, h + m_nDlgShadowWidth * 2);
}

void UICommon::CFramelessDialog::resize(int w, int h)
{
	return QDialog::resize(w + m_nDlgShadowWidth * 2, h + m_nDlgShadowWidth * 2);
}

void UICommon::CFramelessDialog::Close()
{
	__super::close();
}
