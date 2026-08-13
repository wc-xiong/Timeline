#include "ShowMessageWidget.h"
#include "UICommonSizeDefine.h"
#include "UICommonApi.h"

#include <QVariant>
#include <QHBoxLayout>
#include <QTimer>
#include <QList>

#include <QApplication>
#include <QMainWindow>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

namespace
{
const int kMaxVisibleMessageCount = 3;
const int kMessageSpacing = nSizeDefine_4;
const int kMessageMaxWidth = 480;
const int kMessageMinHeight = 28;
QList<CShowMessageWidget*> g_messageQueue;

void RemoveMessageFromQueue(CShowMessageWidget* pMessage)
{
	if (pMessage == nullptr)
	{
		return;
	}

	g_messageQueue.removeAll(pMessage);
}

int CountMessagesForHost(QWidget* pHost)
{
	int nCount = 0;
	for (CShowMessageWidget* pMessageWidget : g_messageQueue)
	{
		if (pMessageWidget != nullptr && pMessageWidget->parentWidget() == pHost)
			++nCount;
	}
	return nCount;
}

void RelayoutMessageQueue(QWidget* pHost)
{
	if (pHost == nullptr)
	{
		return;
	}

	// 仅清理空指针；其他 host 的 tip 保留在队列中，避免混用时误删主窗口 tip
	for (int nIndex = g_messageQueue.count() - 1; nIndex >= 0; --nIndex)
	{
		if (g_messageQueue.at(nIndex) == nullptr)
			g_messageQueue.removeAt(nIndex);
	}

	const QRect localRect = pHost->rect();
	int localY = 0;
	for (int nIndex = 0; nIndex < g_messageQueue.count(); ++nIndex)
	{
		CShowMessageWidget* pMessageWidget = g_messageQueue.at(nIndex);
		if (pMessageWidget == nullptr || pMessageWidget->parentWidget() != pHost)
			continue;

		pMessageWidget->adjustSize();
		const int localX = (localRect.width() - pMessageWidget->width()) / 2;
		pMessageWidget->move(localX, localY);
		pMessageWidget->raise();
		localY += pMessageWidget->height() + kMessageSpacing;
	}
}

void TrimMessageQueueToLimit(QWidget* pHost)
{
	if (pHost == nullptr)
		return;

	while (CountMessagesForHost(pHost) >= kMaxVisibleMessageCount)
	{
		CShowMessageWidget* pFirstForHost = nullptr;
		int nFirstIndex = -1;
		for (int nIndex = 0; nIndex < g_messageQueue.count(); ++nIndex)
		{
			CShowMessageWidget* pMessageWidget = g_messageQueue.at(nIndex);
			if (pMessageWidget != nullptr && pMessageWidget->parentWidget() == pHost)
			{
				pFirstForHost = pMessageWidget;
				nFirstIndex = nIndex;
				break;
			}
		}

		if (nFirstIndex < 0 || pFirstForHost == nullptr)
			break;

		g_messageQueue.removeAt(nFirstIndex);
		pFirstForHost->deleteLater();
	}
}
}

CShowMessageWidget::CShowMessageWidget(QWidget* pParent):
	QWidget(pParent)
{
	setAttribute(Qt::WA_StyledBackground, true);
	setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	

	InitWidget();
	InitLayout();
}

CShowMessageWidget::~CShowMessageWidget()
{
	
}

inline QWidget* getMainWindow()
{
	// 1. 获取程序中所有顶层窗口（无父窗口的部件）
	QList<QWidget*> topLevelWins = qApp->allWidgets();
	if (topLevelWins.isEmpty()) {
		return nullptr; // 无任何顶层窗口，直接返回空
	}

	// 2. 遍历顶层窗口，筛选真正的主窗口（核心过滤规则）
	for (QWidget* win : topLevelWins) 
	{
		if (win->objectName() == "MainWindow")
			return win;
	}

	// 3. 兼容处理：若未筛选出（如所有顶层窗口都是弹窗），返回第一个顶层窗口
	return topLevelWins.first();
}

void CShowMessageWidget::ShowMessage(int nShowTime, const QString & strText, QWidget * pParent, EShowMessageType type, const QString& strBtnText)
{
	Q_UNUSED(pParent);

	// 必须挂在主窗口下：若 parent 为树视图等子控件，move 的坐标系是父控件局部坐标，
	// 原先用主窗口全局矩形去 move，会把提示移到错误位置（常见为屏幕外，看起来像没显示）。
	ShowMessageOnHost(nShowTime, strText, getMainWindow(), type, strBtnText);
}

void CShowMessageWidget::ShowMessageInWidget(int nShowTime, const QString& strText, QWidget* pHost, EShowMessageType type, const QString& strBtnText)
{
	if (!pHost)
		return;

	ShowMessageOnHost(nShowTime, strText, pHost, type, strBtnText);
}

void CShowMessageWidget::ShowMessageOnHost(int nShowTime, const QString& strText, QWidget* pHost, EShowMessageType type, const QString& strBtnText)
{
	if (!pHost)
	{
		assert(0);
		return;
	}

	TrimMessageQueueToLimit(pHost);

	CShowMessageWidget* pMessageBox = new CShowMessageWidget(pHost);
	// 非主窗口挂载时用普通子控件，避免 SubWindow/StaysOnTop 在自绘区域销毁后留下黑底
	if (pHost != getMainWindow())
	{
		pMessageBox->setParent(pHost);
		pMessageBox->setWindowFlags(Qt::Widget);
		pMessageBox->setAttribute(Qt::WA_StyledBackground, true);
	}
	pMessageBox->SetTypeProperty(type);
	pMessageBox->SetShowTime(nShowTime);
	pMessageBox->SetText(strText);
	if (!strBtnText.isEmpty())
	{
		pMessageBox->SetBtnText(strBtnText);
	}
	pMessageBox->adjustSize();

	QObject::connect(pMessageBox, &QObject::destroyed, pHost, [pHost](QObject* pObject)
	{
		CShowMessageWidget* pDestroyedMessageWidget = static_cast<CShowMessageWidget*>(pObject);
		RemoveMessageFromQueue(pDestroyedMessageWidget);
		RelayoutMessageQueue(pHost);
		if (pHost)
			pHost->update();
	});

	g_messageQueue.push_back(pMessageBox);
	RelayoutMessageQueue(pHost);
	pMessageBox->show();
	pHost->update();
}

void CShowMessageWidget::SetShowTime(int nShowTime)
{
	m_nShowTime = nShowTime;
}

void CShowMessageWidget::SetText(const QString & strText)
{
	m_pMessageTxtLabel->setText(strText);
	UpdateWidgetSizeByContent();
}

void CShowMessageWidget::SetBtnText(const QString& strText)
{
	m_pToolBtn->setText(strText);
	m_pToolBtn->setVisible(true);
	UpdateWidgetSizeByContent();
}

void CShowMessageWidget::InitWidget()
{
	setMaximumWidth(SysDpiZoom(kMessageMaxWidth));
	setMinimumHeight(SysDpiZoom(kMessageMinHeight));

	m_pMessageTxtLabel = new QLabel(this);
	m_pMessageTxtLabel->setMinimumWidth(SysDpiZoom(128));
	m_pMessageTxtLabel->setWordWrap(true);
	m_pMessageTxtLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	m_pMessageTxtLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	m_pMessageIconLabel = new QLabel(this);
	m_pMessageIconLabel->setFixedSize(SysDpiZoom(16), SysDpiZoom(16));

	m_pToolBtn = new QPushButton(this);
	m_pToolBtn->setProperty("ShowMessageBtn", true);
	m_pToolBtn->setFixedHeight(SysDpiZoom(16));
	m_pToolBtn->setVisible(false);

	QSizePolicy sizePolicy = this->sizePolicy();
	sizePolicy.setVerticalPolicy(QSizePolicy::Minimum);
	sizePolicy.setHorizontalPolicy(QSizePolicy::Maximum);
	setSizePolicy(sizePolicy);
}

void CShowMessageWidget::UpdateWidgetSizeByContent()
{
	const int nMaxWidgetWidth = SysDpiZoom(kMessageMaxWidth);
	const int nMinTextWidth = m_pMessageTxtLabel->minimumWidth();
	const int nTextSafetyPadding = SysDpiZoom(12);

	int nReservedWidth = 0;
	QLayout* pLayout = layout();
	if (pLayout)
	{
		const QMargins margins = pLayout->contentsMargins();
		const int nSpacing = pLayout->spacing();
		nReservedWidth += margins.left() + margins.right();
		nReservedWidth += m_pMessageIconLabel->width();
		nReservedWidth += nSpacing; // 图标与文本之间的间距
		nReservedWidth += nSpacing; // 文本与伸缩项之间的间距

		if (m_pToolBtn->isVisible())
		{
			nReservedWidth += nSpacing; // 伸缩项与按钮之间的间距
			nReservedWidth += m_pToolBtn->sizeHint().width();
		}
	}
	else
	{
		nReservedWidth = SysDpiZoom(44);
	}

	int nMaxTextWidth = std::max(nMinTextWidth, nMaxWidgetWidth - nReservedWidth);

	int nSingleLineTextWidth = 0;
	const QStringList lineList = m_pMessageTxtLabel->text().split('\n');
	for (const QString& line : lineList)
	{
		const int nLineWidth = m_pMessageTxtLabel->fontMetrics().size(Qt::TextSingleLine, line).width();
		nSingleLineTextWidth = std::max(nLineWidth, nSingleLineTextWidth);
	}

	nSingleLineTextWidth += nTextSafetyPadding;
	nSingleLineTextWidth = std::max(nMinTextWidth, nSingleLineTextWidth);
	nSingleLineTextWidth = std::min(nMaxTextWidth, nSingleLineTextWidth);

	const int nWidgetWidth = std::min(nMaxWidgetWidth, nReservedWidth + nSingleLineTextWidth);
	const int nLabelWidth = std::max(nMinTextWidth, nWidgetWidth - nReservedWidth);

	setFixedWidth(nWidgetWidth);
	m_pMessageTxtLabel->setFixedWidth(nLabelWidth);
	m_pMessageTxtLabel->updateGeometry();
	adjustSize();
	updateGeometry();
}

void CShowMessageWidget::InitLayout()
{
	QHBoxLayout* pMessageLayout = new QHBoxLayout(this);
	pMessageLayout->setContentsMargins(nSizeDefine_12, nSizeDefine_6, nSizeDefine_12, nSizeDefine_6);
	pMessageLayout->setSpacing(nSizeDefine_4);
	pMessageLayout->addWidget(m_pMessageIconLabel);
	pMessageLayout->addWidget(m_pMessageTxtLabel);
	pMessageLayout->addStretch();
	pMessageLayout->addWidget(m_pToolBtn);
}

void CShowMessageWidget::show()
{
	QTimer::singleShot(m_nShowTime, this, &QWidget::deleteLater);
	__super::show();
}

void CShowMessageWidget::SetTypeProperty(EShowMessageType type)
{
	switch (type)
	{
	case ESMT_Succ:
	{
		setProperty("LayoutMessageBoxSucc", true);
		QPixmap pixmap = GetPixmapFromSvg(
			":/dark/images/TitleWnd/messageIconSucc.svg",
			QSize(SysDpiZoom(16), SysDpiZoom(16))
		);
		m_pMessageIconLabel->setPixmap(pixmap);

		m_pMessageTxtLabel->setProperty("LayoutMessageTxtSucc", true);
	}
		break;
	case ESMT_Fail:
	{
		setProperty("LayoutMessageBoxFail", true);
		QPixmap pixmap = GetPixmapFromSvg(
			":/dark/images/TitleWnd/messageIconFail.svg",
			QSize(SysDpiZoom(16), SysDpiZoom(16))
		);
		m_pMessageIconLabel->setPixmap(pixmap);

		m_pMessageTxtLabel->setProperty("LayoutMessageTxtFail", true);
	}
		break;
	case ESMT_Warning:
	{
		setProperty("LayoutMessageBoxWarning", true);
		QPixmap pixmap = GetPixmapFromSvg(
			":/dark/images/TitleWnd/messageIconWarning.svg",
			QSize(SysDpiZoom(16), SysDpiZoom(16))
		);
		m_pMessageIconLabel->setPixmap(pixmap);

		m_pMessageTxtLabel->setProperty("LayoutMessageTxtWarning", true);
	}
		break;
	case ESMT_Normal:
	{
		setProperty("LayoutMessageBoxNormal", true);
		QPixmap pixmap = GetPixmapFromSvg(
			":/dark/images/TitleWnd/messageIconNormal.svg",
			QSize(SysDpiZoom(16), SysDpiZoom(16))
		);
		m_pMessageIconLabel->setPixmap(pixmap);

		m_pMessageTxtLabel->setProperty("LayoutMessageTxtNormal", true);
	}
		break;
	default:
		break;
	}

	m_pMessageIconLabel->setProperty("LayoutMessageIcon", true);

}
