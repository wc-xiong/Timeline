#define _USE_MATH_DEFINES		//解决qmath.h和math.h的头文件冲突
#include "LayerLabelsWidget.h"
#include "LayersWidget.h"
#include "GSMedia/ThreadManager/ThreadManagerExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Common/SignalCenter/GSNetSignalCenter.h"
#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"

#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/LogModel.h"
#include "Model/GlobalModel/SoftwareSettingData.h"
#include "Model/MediaModel/MediaModelExport.h"

#include "UICommon/menuex.h"
#include "UICommon/Messagebox.h"
#include "UICommon/RenameDialog.h"
#include "UICommon/Colordialog.h"
#include "UICommon/UITranslate.h"

#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QDebug>
#include <QPainter>
#include <cassert>
#include <algorithm>
#include <QGridLayout>
#include <QMessageBox>
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


CTimeLineLayerLabelWidget::CTimeLineLayerLabelWidget(QWidget* pParent)
	:QWidget(pParent)
{
	InitWidget();
	CreateConnects();
	setAcceptDrops(true);
}

CTimeLineLayerLabelWidget::~CTimeLineLayerLabelWidget()
{
}

void CTimeLineLayerLabelWidget::UpdateView(const QUuid& LayerId, int index)
{
	m_id = LayerId;
	m_index = index;

	auto pLayer = g_pTimeLineModel->GetLayer(m_id).lock();
	if (pLayer)
	{
		m_pColorButton->SetIdAndColor(m_id, pLayer->GetColor());
		m_pName->SetIdAndText(m_id, pLayer->GetName());
		m_pBtnLock->setChecked(pLayer->IsLock());
		m_pBtnProcess->setChecked(pLayer->IsShowPlayProgress());
		m_pBtnHide->setChecked(pLayer->IsHide());
		m_pBtnMute->setChecked(pLayer->IsMute());
		m_pBtnHide->setEnabled(!pLayer->IsLock());
		m_pBtnMute->setEnabled(!pLayer->IsLock());
	}

	update();
}

void CTimeLineLayerLabelWidget::SetLayerHeight(int h)
{
	if (h < SingleLayerHeight)
		h = SingleLayerHeight;
	if (height() == h)
		return;
	setFixedHeight(h);
	if (m_pColorButton)
		m_pColorButton->setFixedSize(nSizeDefine_10, h - nSizeDefine_1);
}

void CTimeLineLayerLabelWidget::InitWidget()
{
	setFixedSize(QSize(nSizeDefine_200, SingleLayerHeight));

	m_pName = new CTimeLineLayerLabelName(this);
	m_pName->setFixedWidth(nSizeDefine_90);

	m_pBtnProcess = new CSvgImageButton(":/dark/images/TimeLineWnd/Process", this);
	m_pBtnProcess->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnProcess->setProperty("transparentBkBtn", true);
	m_pBtnProcess->setCheckable(true);

	m_pBtnLock = new CSvgImageButton(":/dark/images/TimeLineWnd/Lock", this);
	m_pBtnLock->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnLock->setProperty("transparentBkBtn", true);
	m_pBtnLock->setCheckable(true);

	m_pBtnHide = new CSvgImageButton(":/dark/images/TimeLineWnd/Hide", this);
	m_pBtnHide->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnHide->setProperty("transparentBkBtn", true);
	m_pBtnHide->setCheckable(true);

	m_pBtnMute = new CSvgImageButton(":/dark/images/TimeLineWnd/Mute", this);
	m_pBtnMute->setFixedSize(UICommon_ImageBtnSize);
	m_pBtnMute->setProperty("transparentBkBtn", true);
	m_pBtnMute->setCheckable(true);

	m_pColorButton = new CTimeLineLayerLabelColor(this);
	m_pColorButton->setFixedSize(nSizeDefine_10, SingleLayerHeight - nSizeDefine_1);

	QHBoxLayout* pLayout = new QHBoxLayout(this);
	pLayout->setMargin(0);
	pLayout->setSpacing(0);
	pLayout->addSpacing(nSizeDefine_12);
	pLayout->addWidget(m_pName);
	pLayout->addStretch();
	pLayout->addWidget(m_pBtnProcess);
	pLayout->addWidget(m_pBtnLock);
	pLayout->addWidget(m_pBtnHide);
	pLayout->addWidget(m_pBtnMute);
	pLayout->addSpacing(nSizeDefine_6);
}

void CTimeLineLayerLabelWidget::CreateConnects()
{
	connect(m_pBtnLock, &CSvgImageButton::clicked, this, [&]() {
		auto pLayer = g_pTimeLineModel->GetLayer(m_id).lock();
		if (pLayer)
		{
			pLayer->SetLock(m_pBtnLock->isChecked());
			auto objectIDs = pLayer->GetAllObjectIDs();
			for (const auto& objectID : objectIDs)
				g_pSelectController->DeleteTimeLineObjectSelect(objectID);

			m_pBtnHide->setEnabled(!pLayer->IsLock());
			m_pBtnMute->setEnabled(!pLayer->IsLock());

			emit signalUpdateView(WndType::WndType_LayersTool |
				WndType::WndType_LayersView);

			emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
			emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);

			HistoryStackSave();
			LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLayers(),
				pLayer->GetName(), CTimeLineWndTranslate::LockUnlock(), CCommonTranslate::Click());
		}
	});
	connect(m_pBtnProcess, &CSvgImageButton::clicked, this, [&]() {
		auto pLayer = g_pTimeLineModel->GetLayer(m_id).lock();
		if (pLayer)
		{
			pLayer->SetShowPlayProgress(m_pBtnProcess->isChecked());

			emit signalUpdateView(WndType::WndType_LayersTool |
				WndType::WndType_LayersView);

			HistoryStackSave();
			emit g_pSignalCenter->signalRefreshWindow(
				SignalCenter::EWT_Property | SignalCenter::EWT_TimeLineWnd,
				SignalCenter::EDDT_LayerDataChange);
			LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLayers(),
				pLayer->GetName(), CPropertyTranslate::ShowPlayProgress(), CCommonTranslate::Click());
		}
	});
	connect(m_pBtnHide, &CSvgImageButton::clicked, this, [&]() {
		auto pLayer = g_pTimeLineModel->GetLayer(m_id).lock();
		if (pLayer)
		{
			pLayer->SetHide(m_pBtnHide->isChecked());
			pLayer->SetMute(m_pBtnHide->isChecked());

			m_pBtnMute->setChecked(m_pBtnHide->isChecked());

			emit signalUpdateView(WndType::WndType_LayersTool |
				WndType::WndType_LayersView);

			HistoryStackSave();
			if (g_pGSNetSignal)
				emit g_pGSNetSignal->signalModifyTimelineLayerHideMute(QList<QUuid>{ m_id });
			LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLayers(),
				pLayer->GetName(), CTimeLineWndTranslate::HideShow(), CCommonTranslate::Click());
		}
	});
	connect(m_pBtnMute, &CSvgImageButton::clicked, this, [&]() {
		auto pLayer = g_pTimeLineModel->GetLayer(m_id).lock();
		if (pLayer)
		{
			pLayer->SetMute(m_pBtnMute->isChecked());

			emit signalUpdateView(WndType::WndType_LayersTool);

			HistoryStackSave();
			if (g_pGSNetSignal)
				emit g_pGSNetSignal->signalModifyTimelineLayerHideMute(QList<QUuid>{ m_id });
			LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLayers(),
				pLayer->GetName(), CTimeLineWndTranslate::MuteUnmute(), CCommonTranslate::Click());
		}
	});
}

void CTimeLineLayerLabelWidget::UpdateTranslateText()
{
	m_pBtnLock->setToolTip(CTimeLineWndTranslate::LockUnlock());
	m_pBtnProcess->setToolTip(CPropertyTranslate::ShowPlayProgress());
	m_pBtnHide->setToolTip(CTimeLineWndTranslate::HideShow());
	m_pBtnMute->setToolTip(CTimeLineWndTranslate::MuteUnmute());
}

void CTimeLineLayerLabelWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);

	QRect rc = rect();
	painter.fillRect(rc, g_pSelectController->IsTimeLineLayerSelected(m_id) ? QColor("#3E4044") : QColor("#303236"));

	rc = rect();
	rc.setTop(rc.height() - nSizeDefine_1);
	painter.fillRect(rc, Qt::black);

	__super::paintEvent(event);
}

void CTimeLineLayerLabelWidget::mousePressEvent(QMouseEvent* e)
{
	__super::mousePressEvent(e);

	g_pSelectController->ClearTransFxSelect();
	g_pSelectController->AddTimeLineLayerSelect(m_id, true);
	g_pSelectController->ClearTimeLineObjectSelect();

	QUuid layerId = m_id;
	emit g_pSignalCenter->signalLayerSelectChanged(layerId);
	emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_TimeLineTool |
		WndType::WndType_LayersView);

	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_LayerSelect);

	if (e->button() == Qt::LeftButton) 
	{
		m_dragStart = e->pos();
	}

	e->accept();
}

void CTimeLineLayerLabelWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (!(event->buttons() & Qt::LeftButton)) return;

	int distance = (event->pos() - m_dragStart).manhattanLength();
	if (distance < nSizeDefine_10) return;

	QDrag* drag = new QDrag(this);

	QMimeData* mimeData = new QMimeData;
	mimeData->setData("TimelineLayerId", m_id.toByteArray());
	mimeData->setData("TimelineLayerIndex", QString::number(m_index).toUtf8());
	drag->setMimeData(mimeData);

	QPixmap pixmap(size());
	render(&pixmap);
	QPixmap transparentPixmap(pixmap.size());
	transparentPixmap.fill(Qt::transparent); 
	QPainter painter(&transparentPixmap);
	painter.setOpacity(0.7);
	painter.drawPixmap(0, 0, pixmap);
	drag->setPixmap(transparentPixmap);

	drag->setHotSpot(m_dragStart);
	drag->exec(Qt::MoveAction);

	emit signalUpdateDragLine(-1);
}

void CTimeLineLayerLabelWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat("TimelineLayerId"))
	{
		event->setDropAction(Qt::MoveAction);
		event->accept();
	}
}

void CTimeLineLayerLabelWidget::dragMoveEvent(QDragMoveEvent* event)
{
	if (event->mimeData()->hasFormat("TimelineLayerId"))
	{
		if (event->pos().y() > height() / 2)
			emit signalUpdateDragLine(m_index + 1);
		else
			emit signalUpdateDragLine(m_index);

		QUuid id = QUuid::fromString(QString::fromUtf8(event->mimeData()->data("TimelineLayerId")));
		int index = QString::fromUtf8(event->mimeData()->data("TimelineLayerIndex")).toInt();
		const int diff = index - m_index;
		if (diff == 0 || 
			(diff == 1 && event->pos().y() > height() / 2) || 
			(diff == -1 && event->pos().y() < height() / 2))
		{
			event->ignore();
			return;
		}
		event->accept();
	}
}

void CTimeLineLayerLabelWidget::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat("TimelineLayerId"))
	{
		QUuid id = QUuid::fromString(QString::fromUtf8(event->mimeData()->data("TimelineLayerId")));
		int index = QString::fromUtf8(event->mimeData()->data("TimelineLayerIndex")).toInt();

		int newindex = m_index;
		if (m_index < index)
		{
			if (event->pos().y() > height() / 2)
			{
				newindex = m_index + 1;
			}
		}
		else if (m_index > index)
		{
			if (event->pos().y() < height() / 2)
			{
				newindex = m_index - 1;
			}
		}
		
		g_pTimeLineModel->MoveLayer(id, newindex);
		emit signalUpdateView(WndType::WndType_LayerLabels |
			WndType::WndType_LayersView);

		HistoryStackSave();
		LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLayers(),
			CCommonTranslate::Drag(), QString(QString::number(m_index) + QString("->") + QString::number(newindex)));
	}
}

void CTimeLineLayerLabelWidget::mouseReleaseEvent(QMouseEvent* e)
{
	__super::mouseReleaseEvent(e);

	if (e->button() == Qt::RightButton)
	{
		if (!rect().contains(e->pos())) return;

		//右键菜单
		CMenuEx* pRBtnMenu = new CMenuEx(this);
		int nWidth = g_pSoftwareSetting->GetLanguage() == EML_Chinese ? SysDpiZoom(148) : SysDpiZoom(200);
		pRBtnMenu->setFixedWidth(nWidth);

		auto pAddPre = pRBtnMenu->addAction(CTimeLineWndTranslate::InsertLastLayer());
		auto pAddNext = pRBtnMenu->addAction(CTimeLineWndTranslate::InsertNextLayer());
		auto pClear = pRBtnMenu->addAction(CTimeLineWndTranslate::ClearLayerObjects());
		auto pDelete = pRBtnMenu->addAction(CCommonTranslate::Delete());

		pClear->setEnabled(false);
		auto pLayer = g_pTimeLineModel->GetLayer(m_id).lock();
		if (pLayer)
		{
			pClear->setEnabled(0 != pLayer->GetObjectCount());
		}

		QAction* pSelect = pRBtnMenu->exec(cursor().pos());

		if (pSelect== pAddPre)
		{
			auto p = g_pTimeLineModel->CreateLayer();
			p->SetName(CTimeLineWndTranslate::Layer() + QString::number(g_pTimeLineModel->GetNewLayerIndex()));
			g_pTimeLineModel->AddLayer(p, m_index);
			g_pSelectController->AddTimeLineLayerSelect(p->GetId(), true);

			emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_VerScroll |
				WndType::WndType_LayersView);

			HistoryStackSave();
			LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLayers(),
				CTimeLineWndTranslate::InsertLastLayer(), QString::number(m_index));
		}
		else if (pSelect == pAddNext)
		{
			auto p = g_pTimeLineModel->CreateLayer();
			p->SetName(CTimeLineWndTranslate::Layer() + QString::number(g_pTimeLineModel->GetNewLayerIndex()));
			g_pTimeLineModel->AddLayer(p, m_index + 1);
			g_pSelectController->AddTimeLineLayerSelect(p->GetId(), true);

			emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_VerScroll |
				WndType::WndType_LayersView);

			HistoryStackSave();
			LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLayers(),
				CTimeLineWndTranslate::InsertNextLayer(), QString::number(m_index));
		}
		else if (pSelect == pClear)
		{
			if (pLayer)
			{
				auto objectIds = pLayer->GetAllObjectIDs();
				for (const auto& objectId : objectIds)
				{
					auto pObject = pLayer->GetObject(objectId).lock();
					if (pObject->GetType() == ITimeLineObject::ObjectType_Media)
					{
						g_pPlayerController->DelMediaAndFreePlayer(objectId);
					}
				}
				emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);

				pLayer->ClearObjects();

				emit signalUpdateView(WndType::WndType_LayersView);

				HistoryStackSave();
				LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLayers(),
					CTimeLineWndTranslate::ClearLayerObjects(), QString::number(m_index));
			}
		}
		else if (pSelect == pDelete)
		{
			if (pLayer)
			{
				auto allLayerIds = g_pTimeLineModel->GetAllLayerIDs();
				int index = g_pTimeLineModel->GetLayerIndex(m_id);
				bool bDelete = false;

				if (0 != pLayer->GetObjectCount())
				{
					auto res = UICommon::CMessageBox::warning(this, "GrandMapping", CTimeLineWndTranslate::DeleteTip(),
						QMessageBox::Ok | QMessageBox::Cancel);
					if (QMessageBox::Ok == res)
					{
						auto objectIds = pLayer->GetAllObjectIDs();
						for (const auto& objectId : objectIds)
						{
							auto pObject = pLayer->GetObject(objectId).lock();
							if (pObject->GetType() == ITimeLineObject::ObjectType_Media)
							{
								g_pPlayerController->DelMediaAndFreePlayer(objectId);
							}
						}

						g_pTimeLineModel->DeleteLayer(m_id);
						g_pSelectController->DeleteTimeLineLayerSelect(m_id);

						emit g_pSignalCenter->signalLocateResNode(QUuid());
						emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
						emit g_pSignalCenter->signalUpdatePropertyShowType();

						if (g_pPlayerController->IsPlaying())
							g_pPlayerController->Pause();
						HistoryStackSave();
						LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLayers(),
							CCommonTranslate::Delete(), pLayer->GetName());

						bDelete = true;
					}
				}
				else
				{
					g_pTimeLineModel->DeleteLayer(m_id);
					g_pSelectController->DeleteTimeLineLayerSelect(m_id);

					HistoryStackSave();
					LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLayers(),
						CCommonTranslate::Delete(), pLayer->GetName());

					bDelete = true;
				}

				if (bDelete)
				{
					int next = index + 1;
					int pre = index - 1;
					if (next >= 0 && next < (int)allLayerIds.size())
						g_pSelectController->AddTimeLineLayerSelect(allLayerIds[next]);
					else if (pre >= 0 && pre < (int)allLayerIds.size())
						g_pSelectController->AddTimeLineLayerSelect(allLayerIds[pre]);

					emit signalUpdateView(WndType::WndType_LayerLabels |
						WndType::WndType_TimeLineTool |
						WndType::WndType_VerScroll |
						WndType::WndType_LayersView);
				}
			}
		}

		pRBtnMenu->deleteLater();
	}
}

void CTimeLineLayerLabelWidget::wheelEvent(QWheelEvent*e)
{
	__super::wheelEvent(e);
	emit signalWheel(-e->delta() / 12);
}

CTimeLineLayerLabelName::CTimeLineLayerLabelName(QWidget* parent) :QFrame(parent)
{
	m_pLabel = new QLabel(this);
	m_pEdit = new CLineEditEx(this);

	QGridLayout* playout = new QGridLayout(this);
	playout->setMargin(0);
	playout->setSpacing(0);
	playout->addWidget(m_pLabel, 0, 0);
	playout->addWidget(m_pEdit, 0, 0);

	m_pEdit->setVisible(false);
	m_pLabel->setVisible(true);

	connect(m_pEdit, &CLineEditEx::editingFinished, this, [&]() {

		QString strElideText = m_pLabel->fontMetrics().elidedText(m_pEdit->GetRealText(), Qt::ElideRight, m_pLabel->width());
		m_pLabel->setText(strElideText);
		m_pLabel->setToolTip(m_pEdit->GetRealText());
		auto pLayer = g_pTimeLineModel->GetLayer(m_id).lock();
		if (pLayer)
		{
			auto oldName = pLayer->GetName();
			if (strElideText.isEmpty())
			{
				m_pLabel->setText(oldName);
				m_pEdit->setVisible(false);
				m_pLabel->setVisible(true);
				return;
			}
			pLayer->SetName(m_pEdit->GetRealText());

			if (oldName != m_pEdit->GetRealText())
			{
				HistoryStackSave();
				LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLayers(),
					CCommonTranslate::Rename(), QString(oldName + "->" + m_pEdit->GetRealText()));
			}
		}

		m_pEdit->setVisible(false);
		m_pLabel->setVisible(true);
	});
}

void CTimeLineLayerLabelName::mouseDoubleClickEvent(QMouseEvent* e)
{
	__super::mouseDoubleClickEvent(e);

	auto pLayer = g_pTimeLineModel->GetLayer(m_id).lock();
	if (pLayer)
	{
		m_pEdit->setText(pLayer->GetName());
	}
	m_pEdit->setVisible(true);
	m_pLabel->setVisible(false);
	m_pEdit->setFocus();
}

void CTimeLineLayerLabelColor::mousePressEvent(QMouseEvent* e)
{
	CColorDialog ColorDialog(this);
	ColorDialog.InitColor(m_color);
	connect(&ColorDialog, &CColorDialog::signalNewColor, this, [&](QColor color) {
		m_color = color;
		auto pLayer = g_pTimeLineModel->GetLayer(m_id).lock();
		if (pLayer)
		{
			pLayer->SetColor(color);
			LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewLayers(),
				pLayer->GetName(), QString("(%1,%2,%3)").arg(m_color.red()).arg(m_color.green()).arg(m_color.blue()));
		}
	});
	ColorDialog.exec();
	disconnect(&ColorDialog, &CColorDialog::signalNewColor, 0, 0);
	HistoryStackSave();
}

void CTimeLineLayerLabelColor::paintEvent(QPaintEvent* event)
{
	QPainter p(this);
	p.fillRect(QRect(0,0,nSizeDefine_3,height()), m_color);
}


CLayerLabelsWidget::CLayerLabelsWidget(QWidget* pParent)
	:QWidget(pParent)
{
	InitWidget();
	CreateConnects();

	setAcceptDrops(true);
}

CLayerLabelsWidget::~CLayerLabelsWidget()
{
}

void CLayerLabelsWidget::UpdateView()
{
	const auto vecLayerIDs = g_pTimeLineModel->GetAllLayerIDs();
	int size = (int)vecLayerIDs.size();
	std::vector<int> heights;
	const auto layersWidgets = parentWidget()
		? parentWidget()->findChildren<CLayersWidget*>()
		: QList<CLayersWidget*>();
	if (!layersWidgets.isEmpty())
		heights = layersWidgets.first()->GetLayerHeights();
	if (heights.empty())
		heights.assign(size, SingleLayerHeight);
	else if ((int)heights.size() < size)
		heights.resize(size, SingleLayerHeight); // 右侧尚未同步时补齐，避免新行无 geometry
	else if ((int)heights.size() > size)
		heights.resize(size);

	//增加或删除图层widget
	int diff = (int)vecLayerIDs.size() - (int)m_vecPLayers.size();
	if (diff > 0)
	{
		while (diff--)
		{
			auto p = new CTimeLineLayerLabelWidget(m_pFrameView);
			connect(p, &CTimeLineLayerLabelWidget::signalUpdateView, this, &CLayerLabelsWidget::signalUpdateView);
			connect(p, &CTimeLineLayerLabelWidget::signalWheel, this, &CLayerLabelsWidget::signalWheel);
			connect(p, &CTimeLineLayerLabelWidget::signalUpdateDragLine, this, &CLayerLabelsWidget::signalUpdateDragLine);
			m_vecPLayers.push_back(p);
			p->show();
		}
	}
	else if (diff < 0)
	{
		diff = abs(diff);
		while (diff--)
		{
			CTimeLineLayerLabelWidget* p = m_vecPLayers.back();
			m_vecPLayers.pop_back();
			if (p)
				p->deleteLater();
		}
	}

	assert(size == (int)m_vecPLayers.size());

	// 绝对坐标排版（与 RelayoutLayersGeometry 同一套，消除行列间隙）
	RelayoutLayersGeometry(heights);
	for (int i = 0; i < size; i++)
		m_vecPLayers[i]->UpdateView(vecLayerIDs[i], i);
}

bool CLayerLabelsWidget::IsCanScroll()
{
	return m_pFrameView->height() > height();
}

int CLayerLabelsWidget::GetScrollMax()
{
	return std::max(m_pFrameView->height() - height(), 0);
}

int CLayerLabelsWidget::GetScrollCur()
{
	return abs(m_pFrameView->y());
}

void CLayerLabelsWidget::SetScrollCur(int v)
{
	m_pFrameView->move(0, -v);
}

void CLayerLabelsWidget::RelayoutLayersGeometry(const std::vector<int>& heights)
{
	if (!m_pFrameView)
		return;

	// 绝对坐标堆叠（与右侧 Layers 一致），并先改行高再改 frame，避免中间瞬时空缝
	m_pFrameView->setUpdatesEnabled(false);
	int totalH = 0;
	int cumY = 0;
	const int rowW = m_pFrameView->width() > 0 ? m_pFrameView->width() : nSizeDefine_200;
	for (size_t i = 0; i < m_vecPLayers.size(); ++i)
	{
		int h = (i < heights.size()) ? heights[i] : SingleLayerHeight;
		if (h < SingleLayerHeight)
			h = SingleLayerHeight;
		auto* pRow = m_vecPLayers[i];
		if (pRow)
		{
			pRow->SetLayerHeight(h);
			pRow->setGeometry(0, cumY, rowW, h);
		}
		cumY += h;
		totalH += h;
	}
	if (m_pFrameView->height() != totalH)
		m_pFrameView->setFixedHeight(totalH);
	m_pFrameView->setUpdatesEnabled(true);
}

void CLayerLabelsWidget::InitWidget()
{
	m_pFrameView = new QFrame(this);
	m_pFrameView->setFixedWidth(nSizeDefine_200);
}

void CLayerLabelsWidget::CreateConnects()
{
}

void CLayerLabelsWidget::mousePressEvent(QMouseEvent* e)
{
	__super::mousePressEvent(e);

	g_pSelectController->ClearTimeLineLayerSelect();
	g_pSelectController->ClearTimeLineObjectSelect();
	emit signalUpdateView(WndType::WndType_LayerLabels | WndType::WndType_TimeLineTool |
		WndType::WndType_LayersView);

	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
}

void CLayerLabelsWidget::wheelEvent(QWheelEvent*e)
{
	__super::wheelEvent(e);

	emit signalWheel(-e->delta() / 12);
}

void CLayerLabelsWidget::resizeEvent(QResizeEvent* e)
{
	__super::resizeEvent(e);

	emit signalUpdateView(WndType::WndType_VerScroll);
}

void CLayerLabelsWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat("TimelineLayerId"))
	{
		event->setDropAction(Qt::MoveAction);
		event->accept();
	}
}

void CLayerLabelsWidget::dragMoveEvent(QDragMoveEvent* event)
{
	if (event->mimeData()->hasFormat("TimelineLayerId"))
	{
		emit signalUpdateDragLine((int)m_vecPLayers.size());

		event->accept();
	}
}

void CLayerLabelsWidget::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat("TimelineLayerId"))
	{
		QUuid id = QUuid::fromString(QString::fromUtf8(event->mimeData()->data("TimelineLayerId")));

		g_pTimeLineModel->MoveLayer(id, g_pTimeLineModel->GetLayerCount() - 1);
		emit signalUpdateView(WndType::WndType_LayerLabels |
			WndType::WndType_LayersView);

		emit signalUpdateDragLine(-1);

		HistoryStackSave();
	}
}

void CLayerLabelsWidget::UpdateTranslateText()
{
	for (auto pWnd : m_vecPLayers)
	{
		pWnd->UpdateTranslateText();
	}
}
