#include "ResContentWidget.h"
#include "ResourceTreeList.h"
#include <QDebug>
#include <QTranslator>
#include <QTreeWidget>
#include "../../Model/ResourceModel/ResourceModel.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "../../Model/ResourceModel/ResourceProxyModel.h"
#include "Model/GlobalModel/GlobalModel.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/SoftwareSettingData.h"
#include <QBoxLayout>
#include <QResizeEvent>
#include <QListView>
#include <QHeaderView>
#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>

#include "UICommon/UITranslate.h"
#include "Common/Common/CommonDefine.h"
#include "ResWndUISizeDefine.h"
#include "UICommon/SvgImageButton.h"
#include "Common/ResourceDefine.h"
#include "UICommon/SearchLineEdit.h"
#include "UICommon/menuex.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

void CommonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem itemOption(option);
	itemOption.palette.setColor(QPalette::HighlightedText, index.data(Qt::TextColorRole).value<QColor>());

	// 除了第一列向右边移动12个像素
	if (index.column() != 0)
	{
		itemOption.rect.adjust(nSizeDefine_12, 0, 0, 0);
	}

	// 绘制背景色
	painter->save();

	// 设置背景色：根据当前状态决定最终的背景色
	QColor backgroundColor;
	if (option.state.testFlag(QStyle::State_Selected) && option.state.testFlag(QStyle::State_MouseOver))
	{
		backgroundColor = QColor(48, 50, 54); // 同时为selected和hover状态时，使用hover的背景色
	}
	else if (option.state.testFlag(QStyle::State_Selected))
	{
		backgroundColor = QColor(75, 78, 82); // 仅selected状态时的背景色
	}
	else if (option.state.testFlag(QStyle::State_MouseOver))
	{
		backgroundColor = QColor(48, 50, 54); // 仅hover状态时的背景色
	}

	if (option.state.testFlag(QStyle::State_Selected) || option.state.testFlag(QStyle::State_MouseOver))
	{
		QRect fullRect = option.rect;
		fullRect.adjust(0, 0, nSizeDefine_12, 0); // 扩展右侧区域以覆盖右移产生的区域
		painter->setBrush(QBrush(backgroundColor));
		painter->setPen(Qt::NoPen);
		painter->drawRect(fullRect); // 绘制包括右移区域的背景色
	}

	painter->restore();

	QStyledItemDelegate::paint(painter, itemOption, index);

	if (index.column() != index.model()->columnCount() - 1)
	{
		painter->save();
		// 绘制竖线
		QPen pen(Qt::black);
		pen.setWidth(nSizeDefine_1); // 设置竖线的宽度
		painter->setPen(pen);
		painter->drawLine(option.rect.right(), option.rect.top() + nSizeDefine_4, option.rect.right(), option.rect.bottom() - nSizeDefine_4);

		painter->restore();
	}
}

//////////////////////////////////////////////////////////////////////////

CResContentWidget::CResContentWidget(QWidget* pParent, QWidget* pMainWnd):
	CBaseWidget(pParent)
{
	m_pMainWnd = pMainWnd;
	InitWidget();
	CreateConnects();
	UpdateTranslateText();

	setFocus();
}

CResContentWidget::~CResContentWidget()
{

}

void CResContentWidget::InitResourceWndAfterLoadPrj()
{
	m_pTreelistWidget->InitResListAfterLoadPrj();
}

void CResContentWidget::InitResWndAndModelBeforeLoadPrj()
{
	m_pTreelistWidget->InitResWndAndModelBeforeLoadPrj();
}

QSize CResContentWidget::sizeHint() const 
{
	return QSize(ResWnd_Width, ResWnd_Height); /* 在这里定义dock的初始大小 */
}

void CResContentWidget::slotComboxSelectIndexChange(int nIndex)
{
	//qDebug() << "----------------Current Combox Select Index is :" << nIndex << endl;
	m_pFilterModel->SetFilterFileType(GetCurFilterFileType());
	QRegExp regExp(m_pSearchEdit->text(), Qt::CaseInsensitive);
	m_pFilterModel->setFilterRegExp(regExp);
	slotResourceCountChange();
}

void CResContentWidget::slotSearchEditTextChanged(const QString &strText)
{
	//qDebug() << "-----------------Current Search Edit Text is:" << strText << endl;
	QRegExp regExp(strText, Qt::CaseInsensitive);
	m_pFilterModel->setFilterRegExp(regExp);
}

void CResContentWidget::slotResourceSelectChange(int nResCount, int nResAndFileCount)
{
	m_pDelFileBtn->setEnabled(nResAndFileCount > 0);
	m_pLabelSelectRowCount->setText(QString::number(nResCount));
}

void CResContentWidget::slotResourceCountChange()
{
	int nResCount = g_pResourceModel->GetResNodeCount(GetCurFilterFileType());
	m_pLabelResourceCount->setText("/" + QString::number(nResCount));
}

void CResContentWidget::slotMediaResLocate(const QUuid &resId)
{
	m_pTreelistWidget->MediaResLocate(resId);
}

void CResContentWidget::InitWidget()
{
	m_pFilterModel = new CResourceProxyModel(g_pResourceModel);
	m_pFilterModel->setSourceModel(g_pResourceModel);	//设置代理模型的原模型

	m_pTreelistWidget = new CResourceTreeList(m_pFilterModel, this, m_pMainWnd);
	m_pTreelistWidget->setModel(m_pFilterModel);	//设置代理模型
	m_pTreelistWidget->setItemDelegate(new CommonDelegate(this));	// 设置委托
	m_pTreelistWidget->setTabKeyNavigation(true);
	m_pTreelistWidget->setTabOrder(m_pTreelistWidget, m_pTreelistWidget);

	m_pSelectFilterBtn = new QPushButton(this);
	m_pSelectFilterBtn->setProperty("ResWndFilterBtn", true);
	CreateFilterMenu();

	m_pSearchEdit = new CSearchLineEdit(this);
	m_pSearchEdit->setProperty("SearchEdit", true);
	m_pSearchEdit->setFocusPolicy(Qt::NoFocus);

	m_pAddFileBtn = new CSvgImageButton(":/dark/images/ResourceWnd/AddResBtn", this, false);
	m_pAddFileBtn->setFixedSize(UICommon_ImageBtnWidth, UICommon_CtrlHeight);
	m_pAddFileBtn->setProperty("transparentBkBtn", true);

	m_pDelFileBtn = new CSvgImageButton(":/dark/images/ResourceWnd/DelResBtn", this, false);
	m_pDelFileBtn->setFixedSize(UICommon_ImageBtnWidth, UICommon_CtrlHeight);
	m_pDelFileBtn->setProperty("transparentBkBtn", true);
	m_pDelFileBtn->setEnabled(false);

	m_pLabelSelectRowCount = new QLabel(this);
	m_pLabelSelectRowCount->setProperty("SelectRowCount", true);
	m_pLabelSelectRowCount->setText("0");
	m_pLabelSelectRowCount->setMargin(0);
	m_pLabelSelectRowCount->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_pLabelSelectRowCount->setFixedWidth(nSizeDefine_24);

	m_pLabelResourceCount = new QLabel(this);
	m_pLabelResourceCount->setText("/0");
	m_pLabelResourceCount->setMargin(0);
	m_pLabelResourceCount->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_pLabelResourceCount->setFixedWidth(nSizeDefine_24 + nSizeDefine_2);  //多显示一个"/"

	QFrame* pVLine = new QFrame(this);
	pVLine->setFrameShape(QFrame::VLine);									// 设置为垂直平线
	pVLine->setFrameShadow(QFrame::Plain);									// 设置阴影样式
	pVLine->setFixedSize(nSizeDefine_1, nSizeDefine_22);
	auto palette = pVLine->palette();
	palette.setColor(QPalette::WindowText, Qt::black);
	pVLine->setPalette(palette);

	QFrame* pVLine1 = new QFrame(this);
	pVLine1->setFrameShape(QFrame::VLine);									// 设置为垂直平线
	pVLine1->setFrameShadow(QFrame::Plain);									// 设置阴影样式
	pVLine1->setFixedSize(nSizeDefine_1, nSizeDefine_22);
	palette = pVLine1->palette();
	palette.setColor(QPalette::WindowText, Qt::black);
	pVLine1->setPalette(palette);

	//布局管理
	QHBoxLayout* HorLine1 = new QHBoxLayout;
	HorLine1->setSpacing(0);
	HorLine1->setContentsMargins(0,nSizeDefine_3,0, nSizeDefine_3);

	m_pSearchEdit->setFixedHeight(nSizeDefine_22);

	HorLine1->addSpacing(nSizeDefine_16);
	HorLine1->addWidget(m_pAddFileBtn);

	HorLine1->addSpacing(nSizeDefine_12);
	HorLine1->addWidget(m_pDelFileBtn);
	
	HorLine1->addSpacing(nSizeDefine_12);
	HorLine1->addWidget(m_pSearchEdit);
	HorLine1->addSpacing(nSizeDefine_12);
	HorLine1->addWidget(pVLine);
	HorLine1->addSpacing(nSizeDefine_12);
	HorLine1->addWidget(m_pSelectFilterBtn);			// 62
	HorLine1->addSpacing(nSizeDefine_12);
	HorLine1->addWidget(pVLine1);
	HorLine1->addSpacing(nSizeDefine_6);
	HorLine1->addWidget(m_pLabelSelectRowCount);
	HorLine1->addWidget(m_pLabelResourceCount);
	HorLine1->addSpacing(nSizeDefine_6);

	QFrame* pBackGrounFrame = new QFrame(this);
	pBackGrounFrame->setProperty("ResTopWmddBackground",true);
	pBackGrounFrame->setFixedHeight(nSizeDefine_30);
	pBackGrounFrame->setLayout(HorLine1);

	QVBoxLayout* VerLayout = new QVBoxLayout;
	VerLayout->setMargin(0);
	VerLayout->setSpacing(0);
	VerLayout->addWidget(pBackGrounFrame);
	VerLayout->addWidget(m_pTreelistWidget);

	setLayout(VerLayout);
}

void CResContentWidget::CreateConnects()
{
	connect(m_pAddFileBtn, SIGNAL(clicked()), m_pTreelistWidget, SLOT(slotAddNewResource()));
	connect(m_pDelFileBtn, SIGNAL(clicked()), m_pTreelistWidget, SLOT(slotDelSelectResource()));
	connect(m_pSearchEdit, SIGNAL(textChanged(const QString &)), this, SLOT(slotSearchEditTextChanged(const QString &)));
	connect(m_pTreelistWidget, &CResourceTreeList::signalResourceSelectChange, this, &CResContentWidget::slotResourceSelectChange);
	connect(m_pTreelistWidget, &CResourceTreeList::signalResourceCountChange, this, &CResContentWidget::slotResourceCountChange);
	
	connect(m_pTreelistWidget, &CResourceTreeList::signalMutedByAudioCapture, this, &CResContentWidget::signalMutedByAudioCapture);

	connect(m_pTreelistWidget, &CResourceTreeList::signalPictureTrans, this, &CResContentWidget::signalPictureTrans);

	connect(m_pTreelistWidget, &CResourceTreeList::signalPrjDemuxerFinish, this, &CResContentWidget::signalPrjDemuxerFinish);
	
	connect(m_pTreelistWidget, &CResourceTreeList::signalResDemuxerProgress, this, &CResContentWidget::signalResDemuxerProgress);
	
	connect(m_pTreelistWidget, &CResourceTreeList::signalResFileLostRecoveryState, this, &CResContentWidget::signalResFileLostRecoveryState);

	connect(m_pSelectFilterBtn, SIGNAL(clicked()), this, SLOT(slotFilterBtnClicked()));

	connect(m_pTreelistWidget, &CResourceTreeList::signalUpdateColorTextResWnd, this, &CResContentWidget::signalUpdateColorTextResWnd);

	connect(m_pTreelistWidget, &CResourceTreeList::signalUpdateResToMedia, this, &CResContentWidget::signalUpdateResToMedia);
	connect(m_pTreelistWidget, &CResourceTreeList::signalPreviewMedia, this, &CResContentWidget::signalPreviewMedia);

}

void CResContentWidget::UpdateTranslateText()
{	
	QStringList listSelectOption;
	listSelectOption << CCommonTranslate::All() << CCommonTranslate::Video() << CCommonTranslate::Audio();	// Timeline：仅视频/音频
	QList<QAction*> actions = m_pSelectFilterBtn->menu()->actions();

	for (int i = 0; i < actions.size() && i < listSelectOption.size(); i++)
	{
		actions.at(i)->setText(listSelectOption.at(i));
	}
	m_pSelectFilterBtn->setText(listSelectOption[m_nSelectedCurIndex]);

	m_pSearchEdit->setPlaceholderText(CResourceTranslate::SearchTips());

	m_pAddFileBtn->setToolTip(CCommonTranslate::Add());
	m_pDelFileBtn->setToolTip(CCommonTranslate::Delete());
	m_pTreelistWidget->UpdateTranslateText();

	if (EML_English != g_pSoftwareSetting->GetLanguage())
		m_pSelectFilterBtn->setFixedSize(nSizeDefine_52, UICommon_CtrlHeight);
	else
		m_pSelectFilterBtn->setFixedSize(nSizeDefine_66, UICommon_CtrlHeight);
}

int CResContentWidget::GetCurFilterFileType()
{
	switch (m_nSelectedCurIndex)
	{
	case 0:
		return ERT_Invalid;
	case 1:
		return ERT_Video;
	case 2:
		return ERT_Audio;
	default:
		break;
	}
	return ERT_Invalid;
}


void CResContentWidget::CreateFilterMenu()
{

	CMenuEx *pSelectFilterMenu = new CMenuEx(this, true);
	pSelectFilterMenu->setProperty("MenuResWndFilter", true);
	pSelectFilterMenu->setContentsMargins(0, nSizeDefine_2, 0, 0);
	pSelectFilterMenu->setFixedWidth(nSizeDefine_140);

	// 创建动作组
	QActionGroup* actionGroup = new QActionGroup(this);
	actionGroup->setExclusive(true);

	// Timeline：仅全部 / 视频 / 音频
	QAction* actionAll = new QAction(CCommonTranslate::All(), this);
	QAction* actionVideo = new QAction(CCommonTranslate::Video(), this);
	QAction* actionAudio = new QAction(CCommonTranslate::Audio(), this);

	actionAll->setCheckable(true);
	actionAll->setChecked(true);
	actionVideo->setCheckable(true);
	actionAudio->setCheckable(true);

	actionGroup->addAction(actionAll);
	actionGroup->addAction(actionVideo);
	actionGroup->addAction(actionAudio);

	pSelectFilterMenu->addAction(actionAll);
	pSelectFilterMenu->addAction(actionVideo);
	pSelectFilterMenu->addAction(actionAudio);

	m_pSelectFilterBtn->setMenu(pSelectFilterMenu);

	// 监听动作组的选择变化
	connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotFilterActionTriggered(QAction*)));
}


void CResContentWidget::slotFilterBtnClicked()
{
	m_pSelectFilterBtn->showMenu();

}


void CResContentWidget::slotFilterActionTriggered(QAction* action)
{
	QList<QAction*> actions = static_cast<QActionGroup*>(sender())->actions();
	m_nSelectedCurIndex = actions.indexOf(action);
	m_pSelectFilterBtn->setText(action->text());

	
	slotComboxSelectIndexChange(m_nSelectedCurIndex);
}
