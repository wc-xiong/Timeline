#include "ResourceTreeList.h"
#include <QDebug>
#include <QTextEdit>
#include <QMouseEvent>
#include <QMenu>
#include <QFileDialog>
#include <QMimeData>
#include <QStringList>
#include <QDrag>
#include <QDataStream>
#include <QHeaderView>
#include "Common/Common/VideoDataCommon.h"
#include "Model/ResourceModel/ResourceModel.h"
#include "Model/ResourceModel/ResourceProxyModel.h"
#include "Model/ResourceModel/ResourceNodeInfo.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "GSMedia/Demuxer/ResDemuxer.h"
#include <QApplication>
#include "UICommon/ProgressDialogEx.h"
#include "UICommon/Messagebox.h"
#include "UICommon/RenameDialog.h"
#include "UICommon/UITranslate.h"
#include <QProcess>
#include "UICommon/ShowTextDialog.h"
#include "UICommon/ShowMessageWidget.h"
#include "Common/IResourceInterface.h"

#include "Model/GlobalModel/PrjSettingData.h"

#include "Common/UICommon/LogTranslate.h"

#include "Model/GlobalModel/GlobalModel.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "UI/TimeLineWnd/TimeLineGlobalDefine.h"

#include <QMovie>
#include <QTimer>
#include <QThread>
#include "ResWndUISizeDefine.h"
#include <QDesktopServices>
#include <QScrollBar>

#include "Play/StreamMedia/StreamMediaInterface.h"
#include "Play/StreamMedia/StreamMediaExport.h"
#include <QWidgetAction>
#include <QLabel>
#include <QToolButton>
#include "UICommon/PushButtonEx.h"
#include "UICommon/menuex.h"
#include "UICommon/HeaderViewEx.h"
#include "UICommon/UICommonApi.h"

#include "UICommon/ScrollBarEx.h"
#include <QGuiApplication>

#include "Common/IDemuxerInterface.h"
#include "Common/UICommon/ProgressDialogEx.h"
#include <QPainter>

#include <QPaintEvent>

#include "Common/Common/ResourceDefine.h"
#include <mutex>
#include <QPainter>
#include <QMouseEvent>
#include <algorithm>

#include <queue>

#include "Model/ResourceModel/TextResource.h"
#include "Model/ResourceModel/VideoResource.h"
#include "Model/ResourceModel/PictureResource.h"

#include "Common/Timer.h"
#include "Common/UICommon/SuspendedScrollBar.h"
#include <memory>
#include "Common/Common/CommonFunctionFileOp.h"
#include "Common/Common/CommonDefine.h"

#include  <set>
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Common/UICommon/UICommonSizeDefine.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/LogModel.h"
#include "Model/GlobalModel/SoftwareSettingData.h"

#include "Common/SignalCenter/GSNetSignalCenter.h"

#include "Net/NetController/NetControllerExport.h"
#include "Net/NetCommon/TransDataDefine.h"
#include "Net/NetCommon/TransDataPacking.h"

#include "Net/NetMaterialTransfer/NetMaterialTransferExport.h"

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

//视频:*.mp4;*.3gp;*.m2t;*.m2ts;*.bhd;*.xv;*.kux;*.asx;*.mpe;*.vcd;*.dvd;*.dat;*.ifox;*.m4v;*.webm;*.rmvb;*.rm;*.wmv;*.asf;*.mpeg;*.mpg;*.vob;*.ts;*.tp;*.mov;*.flv;*.avi;*.mkv;
//音频:*.m4a;*.mp3;*.wav;
//照片:*.bmp;*.png;*.jpg;*.tiff;*.tga;*.webp;*.tif;

class CResourceTreeListPrivate
{
public:
	CResourceTreeListPrivate();
	~CResourceTreeListPrivate();

public:

	UICommon::CProgressDialogEx* m_pDemuxerResProgress = nullptr;

	bool m_bAfterLoadPrj = false;

	bool m_bNeedPicTran = false;

	bool m_bReplaceResDemuxer = false;	// 当前素材是否为替换素材解复用

	// 解复用总数与进度
	std::mutex m_mtxDemuxerSize;
	int m_nResNeedDemuxerCount = 0;
	int m_nResDemuxeredCount = 0;

	CResourceProxyModel* m_pFilterModel = nullptr;	//素材库代理模型（筛选）

	// 右键菜单选项
	enum ERBtnMenu
	{
		ID_AddMedia = 0,	// 添加媒体
		ID_AddFilter,	// 添加筛选器
		ID_AddFolder,	// 添加文件夹素材f
		ID_Delete,	// 素材
		ID_LocateFile,	// 定位文件
		ID_ReplaceFile,	// 替换素材
		ID_ResourceRename,	// 素材重命名
		ID_VideoCapture,
		ID_AudioCapture,
		ID_WebMedia,
		ID_Text,
		ID_NdiStream,
		ID_TestChart,
		ID_Subtitle,
		ID_ResAlwaysOpen,	//素材持续打开

		ID_Pictrans_Auto,
		ID_Pictrans_1_16,
		ID_Pictrans_1_8,
		ID_Pictrans_1_4,
		ID_Pictrans_1_2,
		ID_Pictrans_1_1,

		ID_Video_ColorRange_Full,
		ID_Video_ColorRange_limit,

		ID_Video_Decode_Hard,
		ID_Video_Decode_Soft,
		ID_Video_Decode_Auto,

		ID_Count,
	};

	// 外部控制使用
	enum EResourceErrorCode
	{
		EARE_Failed = 0,
		EARE_Success,
		EARE_InvalidType = 4000,
		EARE_RepeadRes,

		EARE_DeleteFailed,
		EARE_DeleteAllFailed,

		EARE_NoRepeat,
	} m_nResourceError = EARE_Success;

	std::map<QUuid, bool> m_mapResExpandState;

	QTimer* m_pCheckDemuxerTimeOut = nullptr;	// 检查解复用超时计时器

	std::mutex m_mtxVecNeedDemuxerIds;
	std::vector<std::pair<QUuid, __int64>> m_vecNeedDemuxerIds;	// 当前需要解复用素材，pair<素材id，开始解复用时刻>

	QWidget* m_pRightTopWgt = nullptr;  // 表格样式右上角填充

	int m_bIsDroping = -1;      //素材库移动素材标识线
	bool m_bIsHitFolder = false;

	CTimer* m_pUpdateResDemuxerState = nullptr;	// 更新素材解复用成功情况

	std::set<QUuid> m_setRecoveryResIds;	// 当前恢复素材解复用当中

	QWidget* m_pMainWnd = nullptr;

	bool IsMouseRightButtonAdd = false;  // 区分添加的方式（鼠标邮件、拖拽）
	QMovie* m_pMovie = nullptr;

	QUuid addResParentResId;	// 添加素材时点击素材id
};	

CResourceTreeListPrivate::CResourceTreeListPrivate()
	: m_pDemuxerResProgress(nullptr)
{
	m_bIsDroping = -1;
}

CResourceTreeListPrivate::~CResourceTreeListPrivate()
{
	g_pResDemuxer->Stop();

	if (m_pUpdateResDemuxerState)
	{
		delete m_pUpdateResDemuxerState;
		m_pUpdateResDemuxerState = nullptr;
	}

	if (m_pMovie)
	{
		m_pMovie->stop();
		delete m_pMovie;
		m_pMovie = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////

CResourceTreeList::CResourceTreeList(CResourceProxyModel* pFilerModel, QWidget* parent, QWidget* pMainWnd)
	: CRenameTreeList(parent), d_ptr(new CResourceTreeListPrivate)
{
	assert(pFilerModel);
	d_ptr->m_pFilterModel = pFilerModel;
	d_ptr->m_pMainWnd = pMainWnd;

	d_ptr->m_pDemuxerResProgress = new UICommon::CProgressDialogEx(this);
	d_ptr->m_pDemuxerResProgress->SetTitleName(CResourceTranslate::ResDemuxering());

	d_ptr->m_pCheckDemuxerTimeOut = new QTimer(this);
	connect(d_ptr->m_pCheckDemuxerTimeOut, &QTimer::timeout, this, &CResourceTreeList::slotCheckResDemuxerTimeOut);

	d_ptr->m_pRightTopWgt = new QWidget(this);
	d_ptr->m_pRightTopWgt->setProperty("ResHeader", true); // 和标题栏样式保持一致

	QString strOriginalPath = QApplication::applicationDirPath() + "/Animation/loading.gif";
	d_ptr->m_pMovie = new QMovie(strOriginalPath);

	InitWidget();
	CreateConnects();
	UpdateTranslateText();

	d_ptr->m_pUpdateResDemuxerState = new CTimer(std::bind(&CResourceTreeList::TrdUpdateResDemuxerState, this), 10000.0f);
	d_ptr->m_pUpdateResDemuxerState->Start();
}

CResourceTreeList::~CResourceTreeList()
{
	delete d_ptr;
	d_ptr = nullptr;
}

std::list<QUuid> CResourceTreeList::DelResources(bool isSelect, QModelIndexList delList, uint sender)
{
	QList<QPersistentModelIndex> indexes;
	//动态index转换为静态index
	foreach(auto & index, delList)
	{
		if (isSelect)
		{
			if (index.column() != 0)
				continue;
			indexes << d_ptr->m_pFilterModel->mapToSource(index);
		}
		else
			indexes << index;
	}

	std::list<QUuid> lstNotDeleteRes;
	bool bTimelineDirty = false;

	if (indexes.empty())
		return lstNotDeleteRes;

	//使用静态index删除节点
	foreach(auto & index, indexes)
	{
		auto pResource = g_pResourceModel->GetResNodeFromIndex(index);
		if (!pResource)
		{
			continue;
		}

		// 时间线上仍有引用时：先移除对应片段，再删素材（否则 UseCount>0 会一直删不掉）
		if (pResource->GetResourceUseCount() > 0 && g_pMediaModel && g_pTimeLineModel)
		{
			const auto mediaIds = g_pMediaModel->GetResMeadiaIds(pResource->GetId());
			if (g_pPlayerController)
				g_pPlayerController->Pause();
			if (g_pSelectController)
			{
				g_pSelectController->ClearTimeLineLayerSelect();
				g_pSelectController->ClearTimeLineObjectSelect();
				g_pSelectController->ClearTransFxSelect();
			}
			for (const auto& mediaId : mediaIds)
			{
				for (const auto& layerId : g_pTimeLineModel->GetAllLayerIDs())
				{
					auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
					if (!pLayer || !pLayer->IsObjectExist(mediaId))
						continue;
					if (g_pPlayerController)
						g_pPlayerController->DelMediaAndFreePlayer(mediaId);
					g_pMediaModel->DeleteMediaNode(mediaId);
					pLayer->DeleteObject(g_pTimeLineModel->GetId(), mediaId);
					bTimelineDirty = true;
					break;
				}
			}
			emit g_pSignalCenter->signalLocateResNode(QUuid());
			emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
			emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_MediaNodeSelect);
		}

		if (pResource->GetResourceUseCount() > 0)
		{
			lstNotDeleteRes.push_back(pResource->GetId());
			continue;
		}

		// 判断是否为合集
		switch (pResource->GetType())
		{
		case ERT_VideoCapture:
		case ERT_AudioCapture:
		case ERT_WebMedia:
		case ERT_Ndi:
		{
			StreamMedia_UnBindResDestroy(pResource);
			break;
		}
		default:
			break;
		}

		if (IsSupportColorTextRes(pResource->GetType()))
			UpdateColorTextResWnd(pResource);


		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CLogRecordTranslate::Delete(), pResource->GetName());

		g_pResourceModel->removeRow(index.row(), index.parent());

		// 更新播放端
		emit g_pGSNetSignal->signalResNodeOp(ETDT_DeleteResNode, pResource->GetId());
	}

	HistoryStackSave();//SAVE_PRJ_UNDO_NODE;	// 保存撤销恢复节点
	emit signalResourceCountChange();

	// 模型已删轨上片段：刷新时间线 UI（CheckObject 同步 ObjectWidget）
	if (bTimelineDirty && g_pSignalCenter)
	{
		emit g_pSignalCenter->signalRefreshWindow(
			SignalCenter::EWT_TimeLineWnd,
			WndType::WndType_LayersView | WndType::WndType_LayerLabels
			| WndType::WndType_VerScroll | WndType::WndType_NoniusView
			| WndType::WndType_TimeLineTool);
	}

	if (!lstNotDeleteRes.empty())
	{
		
		if (indexes.size() > 1)
		{
			QString strDelNumber = QString::number(indexes.size() - lstNotDeleteRes.size());
			QString strNotDelNumber = QString::number(lstNotDeleteRes.size());
			QString strTotalNumber = QString::number(indexes.size());

			CShowMessageWidget::ShowMessage(5000, CResourceTranslate::ResourcesNotDeletedTips().arg(indexes.size() - lstNotDeleteRes.size()).arg(indexes.size()).arg(lstNotDeleteRes.size()), nullptr, ESMT_Warning);
		}
		else
		{
			CShowMessageWidget::ShowMessage(5000, CResourceTranslate::ResourcesUsedTips(), nullptr, ESMT_Warning);
		}

		if (delList.size() / g_pResourceModel->columnCount() == lstNotDeleteRes.size())
			LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CLogRecordTranslate::Delete(), CLogRecordTranslate::Cancel());
	}

	return lstNotDeleteRes;
}

void CResourceTreeList::slotHeadsort(int logicalIndex, Qt::SortOrder order)
{
	header()->setSortIndicatorShown(true);
	header()->setSortIndicator(logicalIndex, order);
	d_ptr->m_pFilterModel->sort(logicalIndex, order);
	UpdateViewExpandState();
}

void CResourceTreeList::slotAddNewResource()
{
	DealResourceOperator(cursor().pos(), 0, QUuid(), true);
}

void CResourceTreeList::slotDelSelectResource()
{
	LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CLogRecordTranslate::ButtonDelete());
	DelResources(true, selectedIndexes());
}

void CResourceTreeList::slotExpanded(const QModelIndex& index)
{
	std::shared_ptr<IResourceBase> baseNode = g_pResourceModel->GetResNodeFromIndex(d_ptr->m_pFilterModel->mapToSource(index));
	if (baseNode)
		d_ptr->m_mapResExpandState[baseNode->GetId()] = true;

	QTimer::singleShot(0, this, [this]() {
		UpdateSuspendedScrollBarVisible();
	});
}

void CResourceTreeList::slotCollapsed(const QModelIndex& index)
{
	std::shared_ptr<IResourceBase> baseNode = g_pResourceModel->GetResNodeFromIndex(d_ptr->m_pFilterModel->mapToSource(index));
	if (baseNode)
		d_ptr->m_mapResExpandState[baseNode->GetId()] = false;
}

void CResourceTreeList::InitResListAfterLoadPrj()
{
	d_ptr->m_bAfterLoadPrj = true;
	DemuxerAllResource(true);

	// 需要保持打开的流媒体需要在此判断打开流
	std::map<QUuid, std::shared_ptr<IResourceBase>> mapRes = g_pResourceModel->GetAllResNode();
	for (auto& it : mapRes)
	{
		if (!IsAlwaysOpenOption(it.second->GetType()))
			continue;

		if (it.second->IsKeepOpen())
		{
			StreamMedia_DealStreamAlwaysOpen(it.second);
		}
	}
}

void CResourceTreeList::InitResWndAndModelBeforeLoadPrj()
{
	g_pResDemuxer->DelAllResDemuxer();

	d_ptr->m_bAfterLoadPrj = false;

	StreamMedia_DestroyBeforeLoadPrj(g_pResourceModel->GetAllResList());

	g_pResourceModel->InitResourceModel();
}

void CResourceTreeList::UpdateTranslateText()
{
	if (auto pHeader = dynamic_cast<CHeaderViewEx*>(header()))
	{
		pHeader->SetColNames(QStringList() << CCommonTranslate::Name()
			<< CCommonTranslate::Resolution()
			<< CCommonTranslate::Type()
			<< CCommonTranslate::Duration());
	}
	g_pResourceModel->ResDataUpdateByTranslation();

	if (d_ptr->m_pDemuxerResProgress)
		d_ptr->m_pDemuxerResProgress->SetTitleName(CResourceTranslate::ResDemuxering());
}

void CResourceTreeList::MediaResLocate(const QUuid& resId)
{
	auto resIndex = d_ptr->m_pFilterModel->mapFromSource(g_pResourceModel->GetModelIndex(resId));
	setCurrentIndex(resIndex);
	scrollTo(resIndex);
}

void CResourceTreeList::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	__super::selectionChanged(selected, deselected);

	int nSelectCount = 0;
	int nSelectAndFileCount = 0;
	auto listSelectIndex = selectedIndexes();
	for (auto it : listSelectIndex)
	{
		if (it.column() != 0)
			continue;
		std::shared_ptr<IResourceBase> resNode = g_pResourceModel->GetResNodeFromIndex(d_ptr->m_pFilterModel->mapToSource(it));
		if (resNode->GetType() != ERT_Folder)
			nSelectCount++;
		nSelectAndFileCount++;
	}

	emit signalResourceSelectChange(nSelectCount, nSelectAndFileCount);

	// Timeline：选中单个视频/音频时预览
	if (nSelectCount == 1)
	{
		for (auto it : listSelectIndex)
		{
			if (it.column() != 0)
				continue;
			std::shared_ptr<IResourceBase> resNode = g_pResourceModel->GetResNodeFromIndex(d_ptr->m_pFilterModel->mapToSource(it));
			if (resNode && (resNode->GetType() == ERT_Video || resNode->GetType() == ERT_Audio)
				&& resNode->GetDemuxerSuccessed() && !resNode->GetPath().isEmpty())
			{
				emit signalPreviewMedia(resNode->GetPath());
			}
			break;
		}
	}
}

void CResourceTreeList::setModel(QAbstractItemModel* model)
{
	__super::setModel(model);

	setColumnWidth(0, TreeList_Col1Width);
	setColumnWidth(1, TreeList_Col2Width);
	setColumnWidth(2, TreeList_Col3Width);
	setColumnWidth(3, TreeList_Col4Width);

	std::vector<int>vec;
	vec.push_back(TreeList_Col1Width);
	vec.push_back(TreeList_Col2Width);
	vec.push_back(TreeList_Col3Width);
	vec.push_back(TreeList_Col4Width);

	if (auto pHeadEx = dynamic_cast<CHeaderViewEx*>(header()))
		pHeadEx->SetColWidths(vec);
}

void CResourceTreeList::AddRealFolder(const QString& strPath, CResourceNodeInfo* parentNode, int FolderDeep)
{
	if (FolderDeep > 3)	//最多递归三层文件夹
		return;

	QDir dir(strPath);
	QFileInfoList listFileInfo = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Dirs);

	foreach(auto it, listFileInfo)
	{
		if (it.isDir())
		{
			std::shared_ptr<IResourceBase> pResource = g_pResourceModel->CreateResource(ERT_Folder);
			pResource->SetName(it.fileName());

			auto pFolder = std::dynamic_pointer_cast<IFolderResource>(pResource);
			if (!pFolder)
				continue;

			pFolder->SetFolderType(false);

			g_pResourceModel->AddResource(pResource, parentNode->GetResId());
			CResourceNodeInfo* pResNodeInfo = g_pResourceModel->GetModelItem(pResource->GetId());
			AddRealFolder(it.filePath(), pResNodeInfo, FolderDeep + 1);
		}
		else
		{
			QString path = it.filePath();
			//qDebug() << "----------------------------is File" << it.filePath() << endl;

			int type = GetFileResType(path);
			if (type == ERT_Invalid)	//过滤不支持素材
				continue;

			std::shared_ptr<IResourceBase> pResource = g_pResourceModel->CreateResource(type);
			assert(pResource);

			QString strName = path.right(path.length() - path.lastIndexOf('/') - 1);
			pResource->SetFullName(strName);
			pResource->SetName(strName.left(strName.lastIndexOf('.')));
			pResource->SetPath(path);

			g_pResourceModel->AddResource(pResource, parentNode->GetResId());
			LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CLogRecordTranslate::AddSuccess(),
				ResourceTypeString(pResource->GetType()), pResource->GetName());
		}
	}
}

void CResourceTreeList::startDrag(Qt::DropActions supportedActions)
{
	if ((QGuiApplication::keyboardModifiers() & Qt::ShiftModifier) ||	// 键盘按下这几个键时不能拖动
		(QGuiApplication::keyboardModifiers() & Qt::ControlModifier) ||
		(QGuiApplication::keyboardModifiers() & Qt::AltModifier))
		return;

	QModelIndexList listSelect = selectedIndexes();
	QModelIndexList listPreSelect = selectedIndexes();

	//剔除文件夹和文件夹子项同时选中的情况
	for (auto it = listSelect.begin(); it != listSelect.end();)
	{
		bool bDeleteSuccess = false;
		for (auto iter = listPreSelect.begin(); iter != listPreSelect.end(); iter++)
		{
			if ((*it).parent() == (*iter))
			{
				it = listSelect.erase(it);
				bDeleteSuccess = true;
				break;
			}
		}
		if (!bDeleteSuccess)
			it++;
	}

	//为了插入后顺序和原顺序相同
	std::sort(listSelect.begin(), listSelect.end(), [](QModelIndex a, QModelIndex b) {
		return a.row() > b.row();
	});

	if (listSelect.isEmpty())
		return;

	QList<QUuid>listSelectUuid;
	QList<QString>listSelectNodeName;
	for (auto it : listSelect)
	{
		if (it.column() != 0)
			continue;
		std::shared_ptr<IResourceBase> resNode = g_pResourceModel->GetResNodeFromIndex(d_ptr->m_pFilterModel->mapToSource(it));
		if (!resNode->GetResDemuxered() || !resNode->GetDemuxerSuccessed())
			continue;

		listSelectUuid.push_back(resNode->GetId());
		listSelectNodeName.push_back(resNode->GetName());;
	}

	if (listSelectUuid.isEmpty())
		return;

	QByteArray itData;
	QDataStream dataStream(&itData, QIODevice::WriteOnly);
	dataStream << listSelectUuid << listSelectNodeName;

	//qDebug() << "移动前" << listSelectUuid << endl;

	QMimeData* pMimeData = new QMimeData;
	pMimeData->setData(ResMimeType, itData);

	//如果最后一个元素是文件夹，需添加处理以方便舞台区禁止接收文件夹的拖拽
	std::shared_ptr<IResourceBase> resNode = g_pResourceModel->GetResNodeFromIndex(d_ptr->m_pFilterModel->mapToSource(listSelect.at(0)));
	if (resNode->GetType() == ERT_Folder)
	{
		pMimeData->setData("ERT_Folder", nullptr);
	}

	QDrag* pdrag = new QDrag(this);
	pdrag->setMimeData(pMimeData);

	//定义拖拽的图标为最后一个元素
	pdrag->setPixmap(resNode->GetThumbnail(true, false));
	pdrag->exec(supportedActions);

	if (pdrag->target() != this->viewport())	// 当拖拽出去时窗口不会收到MouseLeave消息，需要手动发送，不然前面Hover状态不会取消
	{
		QMouseEvent* mEvnLeave = new QMouseEvent(QEvent::Leave, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
		QApplication::postEvent(this->viewport(), mEvnLeave);

		//setProperty("disableHover", true);
		style()->polish(this);
	}
}

void CResourceTreeList::mousePressEvent(QMouseEvent* event)
{
	QApplication::sendEvent(parent(), event);	//使父窗口感知到本窗口鼠标事件（焦点框的设置）
	setFocus();
	__super::mousePressEvent(event);
}

void CResourceTreeList::mouseReleaseEvent(QMouseEvent* event)
{
	// 仅右键弹出素材菜单；左键只做选中（空库添加请用工具栏“+”）
	if (event->button() == Qt::RightButton)
	{
		auto curIndex = d_ptr->m_pFilterModel->mapToSource(indexAt(event->pos()));
		QUuid selectResIndex = QUuid();
		if (auto pNode = static_cast<CResourceNodeInfo*>(curIndex.internalPointer()))
			selectResIndex = pNode->GetResId();

		if (!curIndex.isValid())
			setCurrentIndex(QModelIndex());

		int nSelectRowCount = selectedIndexes().size() / 4;//选中会将每一列也选中，所以需要去除列的影响
		DealResourceOperator(cursor().pos(), nSelectRowCount, selectResIndex);
		return;
	}
	__super::mouseReleaseEvent(event);
}

void CResourceTreeList::mouseMoveEvent(QMouseEvent* event)
{
	if (!(event->buttons() & Qt::RightButton))
		__super::mouseMoveEvent(event);
}

void CResourceTreeList::dragMoveEvent(QDragMoveEvent* event)
{
	//qDebug() << "---CResourceTreeList::dragMoveEvent---";
	if (event->mimeData()->hasFormat(ResMimeType) || event->mimeData()->hasFormat(SysMimeType))
	{
		//如何鼠标在素材组项上则不显示标识线，因为此时放进素材组的动作优先
		QPoint pos = event->pos();
		QModelIndex index = indexAt(pos);
		std::shared_ptr<IResourceBase> resNode = g_pResourceModel->GetResNodeFromIndex(d_ptr->m_pFilterModel->mapToSource(index));
		d_ptr->m_bIsDroping = int(event->pos().y() / (nSizeDefine_30 + nSizeDefine_1)) * (nSizeDefine_30 + nSizeDefine_1);//1是像素误差
		if (resNode && resNode->GetType() != ERT_Folder)
		{
			d_ptr->m_bIsHitFolder = false;
		}
		else
		{
			if (g_pResourceModel->IsRootNode(d_ptr->m_pFilterModel->mapToSource(index)))
				d_ptr->m_bIsHitFolder = false;
			else
				d_ptr->m_bIsHitFolder = true;
		}

		update();
	}
	if (event->mimeData()->hasFormat(SysMimeType))
		event->accept();
	else
		event->ignore();
	__super::dragMoveEvent(event);
}

void CResourceTreeList::dragLeaveEvent(QDragLeaveEvent* event)
{
	//鼠标拖拽离开素材库，重置拖拽线
	if (d_ptr->m_bIsDroping != -1)
		d_ptr->m_bIsDroping = -1;

	__super::dragLeaveEvent(event);
}

void CResourceTreeList::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat(SysMimeType))
	{
		// 拖拽素材文件
		event->accept();

		QStringList listPath;
		QList<QUrl> urls = event->mimeData()->urls();
		for (auto it : urls)
			listPath.push_back(it.toLocalFile());

		QModelIndex curIndex = d_ptr->m_pFilterModel->mapToSource(indexAt(event->pos()));
		std::shared_ptr<IResourceBase> resNode = g_pResourceModel->GetResNodeFromIndex(curIndex);
		d_ptr->addResParentResId = resNode ? resNode->GetId() : QUuid();

		AddNeedDemuxerResource(listPath);

		return;
	}
	else if (event->mimeData()->hasFormat(ResourceModel::MimeType))	// 相当于拖拽改变顺序
	{
		event->ignore();
		QModelIndex curIndex = d_ptr->m_pFilterModel->mapToSource(indexAt(event->pos()));
		g_pResourceModel->DropMimeData(event->mimeData(), curIndex);

		UpdateViewExpandState();
		HistoryStackSave();//SAVE_PRJ_UNDO_NODE;

		//setProperty("disableHover", true);
		style()->polish(this);

		return;
	}
	__super::dropEvent(event);
}

void CResourceTreeList::slotResetModelChange()
{
	// 重新处理流媒体的持续打开操作
	d_ptr->m_bAfterLoadPrj = false;
	std::map<QUuid, std::shared_ptr<IResourceBase>> mapRes = g_pResourceModel->GetAllResNode();
	for (auto& it : mapRes)
	{
		if (IsAlwaysOpenOption(it.second->GetType()))
		{
			StreamMedia_DealStreamAlwaysOpen(it.second);
		}
	}

	UpdateViewExpandState();

	emit signalResourceSelectChange(0, 0);
	emit signalResourceCountChange();
}

void CResourceTreeList::slotUpdateResourceNode(const QUuid id)
{
	g_pResourceModel->UpDateResUIById(id);
}

void CResourceTreeList::slotDemuxerFinish()
{
	d_ptr->m_nResourceError = CResourceTreeListPrivate::EARE_Success;

	// 解复用完成
	if (d_ptr->m_nResNeedDemuxerCount > 0)
	{
		HistoryStackSave();//SAVE_PRJ_UNDO_NODE
	}

	if (d_ptr->m_bAfterLoadPrj)	// 加载工程解复用完成
	{
		emit signalPrjDemuxerFinish();
	}
	d_ptr->m_bAfterLoadPrj = false;
}

void CResourceTreeList::InitWidget()
{
	setProperty("ResWndTreeView", true);

	CHeaderViewEx* pHeadView = new CHeaderViewEx(this);
	pHeadView->setSectionsClickable(true);
	pHeadView->setStretchLastSection(true);

	std::vector<int> vecInterval;
	vecInterval.push_back(nSizeDefine_22);
	pHeadView->SetColLeftIntervals(vecInterval);
	pHeadView->SetTextLeftInterval(12);

	setHeader(pHeadView);
	SetTopSpacing(pHeadView->height());

	setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);  //设置多选类型

	setDragDropMode(QAbstractItemView::DragDrop);   //设置拖拽类型
	setDropIndicatorShown(false);

	setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	setDragDropOverwriteMode(false);

	setAcceptDrops(true);
	setAlternatingRowColors(true);	//交替变换背景色

	//setEditTriggers(QTreeView::DoubleClicked);

	setSortingEnabled(true);

	setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);	// 横滑动条按像素滑动较为流畅

	CSrollBarEx* pHorScrollBar = new CSrollBarEx(Qt::Orientation::Horizontal, this);
	setHorizontalScrollBar(pHorScrollBar);


	//悬浮滑动条
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollMode(ScrollPerPixel);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pVerticalScrollBar = new CSuspendedScrollBar(Qt::Vertical, this);
	m_pVerticalScrollBar->SetScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
	m_pHorizontalScrollBar = new CSuspendedScrollBar(Qt::Horizontal, this);

	setIconSize(QSize(Res_ThumbnailUIShowWidth, Res_ThumbnailUIShowHeight));
	setIndentation(nSizeDefine_22);		//设置branch图标size

	SetLeftSpacing(Res_ThumbnailUIShowWidth);
}

void CResourceTreeList::CreateConnects()
{
	connect(this, SIGNAL(expanded(const QModelIndex&)), this, SLOT(slotExpanded(const QModelIndex&)));
	connect(this, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(slotCollapsed(const QModelIndex&)));
	connect(this, &CResourceTreeList::signalUpdateResourceNode, this, &CResourceTreeList::slotUpdateResourceNode);

	connect(header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(slotHeadsort(int, Qt::SortOrder)));

	connect(this, &CResourceTreeList::signalDemuxerFinish, this, &CResourceTreeList::slotDemuxerFinish);

	connect(this, &CResourceTreeList::signalDemuxer, g_pResDemuxer, &CResDemuxer::slotDemuxer);

	qRegisterMetaType<std::shared_ptr<SResFileDemuxInfo>>("std::shared_ptr<SResFileDemuxInfo>");
	connect(g_pResDemuxer, &CResDemuxer::signalDemuxerFinish, this, &CResourceTreeList::slotDemuxerDataFinish, Qt::QueuedConnection);

	connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), m_pVerticalScrollBar, SLOT(slotValueChange(int)));
	connect(this->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), m_pVerticalScrollBar, SLOT(slotRangeChanged(int, int)));
	connect(this->horizontalScrollBar(), SIGNAL(valueChanged(int)), m_pHorizontalScrollBar, SLOT(slotValueChange(int)));
	connect(this->horizontalScrollBar(), SIGNAL(rangeChanged(int, int)), m_pHorizontalScrollBar, SLOT(slotRangeChanged(int, int)));
	connect(m_pVerticalScrollBar, &CSuspendedScrollBar::valueChanged, this, &CResourceTreeList::slotVerticalValueChange);
	connect(m_pHorizontalScrollBar, &CSuspendedScrollBar::valueChanged, this, &CResourceTreeList::slotHorizontalValueChange);

	connect(g_pSignalCenter, &CSignalCenter::signalUndoRedoRefreshResourceWnd, this, &CResourceTreeList::slotResetModelChange);

	if (d_ptr->m_pMovie->isValid())
	{
		connect(d_ptr->m_pMovie, &QMovie::updated, [&]() {
			if (g_pResourceModel->GetAllResDemuxered())
			{
				QPixmap pixmap = d_ptr->m_pMovie->currentPixmap();
				g_pResourceModel->UpdateResThumbnail(pixmap);
			}
			else
			{
				d_ptr->m_pMovie->stop();
			}
		});
		connect(d_ptr->m_pMovie, &QMovie::finished, [&]() {
			//qDebug() << "finished" << QThread::currentThreadId();
			if (g_pResourceModel->GetAllResDemuxered())
			{
				d_ptr->m_pMovie->start();
			}
		});
		connect(this, &CResourceTreeList::signalUpdateResThumbnail, this, [&]() {
			//qDebug() << "signalUpdateResThumbnail" << QThread::currentThreadId();
			if (g_pResourceModel->GetAllResDemuxered())
			{
				d_ptr->m_pMovie->start();
			}
		}, Qt::QueuedConnection);
	}

	connect(m_pRenameEdit, &QLineEdit::editingFinished, this, [&]() {
		m_pRenameEdit->hide();


		auto pResource = g_pResourceModel->GetResNodeFromIndex(d_ptr->m_pFilterModel->mapToSource(m_renameIndex));
		if (pResource)
		{
			pResource->SetName(m_pRenameEdit->text());
			HistoryStackSave();
		}
	});
}

void CResourceTreeList::slotVerticalValueChange(int nValue)
{
	this->verticalScrollBar()->setValue(nValue);
}

void CResourceTreeList::slotHorizontalValueChange(int nValue)
{
	this->horizontalScrollBar()->setValue(nValue);
}

void CResourceTreeList::resizeEvent(QResizeEvent* event) 
{
	m_pVerticalScrollBar->setGeometry(this->width() - nSizeDefine_11, nSizeDefine_28, nSizeDefine_11, this->height() - nSizeDefine_30);

	m_pHorizontalScrollBar->setGeometry(0, this->height() - nSizeDefine_11, this->width() - nSizeDefine_20, nSizeDefine_11);
	return QTreeView::resizeEvent(event);
}

void CResourceTreeList::UpdateSuspendedScrollBarVisible()
{
	if (!m_pVerticalScrollBar || !m_pHorizontalScrollBar)
		return;

	updateGeometries();

	if (!underMouse())
		return;

	m_pVerticalScrollBar->slotRangeChanged(verticalScrollBar()->minimum(), verticalScrollBar()->maximum());
	m_pVerticalScrollBar->slotValueChange(verticalScrollBar()->value());
	if (verticalScrollBar()->maximum() > 0)
		m_pVerticalScrollBar->show();
	else
		m_pVerticalScrollBar->hide();

	m_pHorizontalScrollBar->slotRangeChanged(horizontalScrollBar()->minimum(), horizontalScrollBar()->maximum());
	m_pHorizontalScrollBar->slotValueChange(horizontalScrollBar()->value());
	if (horizontalScrollBar()->maximum() > 0)
		m_pHorizontalScrollBar->show();
	else
		m_pHorizontalScrollBar->hide();
}

void CResourceTreeList::enterEvent(QEvent* event)
{
	UpdateSuspendedScrollBarVisible();
	return QTreeView::enterEvent(event);
}

void CResourceTreeList::leaveEvent(QEvent* event)
{
	m_pVerticalScrollBar->hide();
	m_pHorizontalScrollBar->hide();
	return QTreeView::leaveEvent(event);
}

void CResourceTreeList::UpdateDemuxerProgress(const QUuid& resId, bool bDemuxered, bool bDemuxerSuccess)
{
	std::lock_guard<std::mutex>locker(d_ptr->m_mtxDemuxerSize);

	// 判断是否为文件恢复重新解复用
	auto itRecovery = d_ptr->m_setRecoveryResIds.find(resId);
	if (itRecovery != d_ptr->m_setRecoveryResIds.end())
	{
		d_ptr->m_setRecoveryResIds.erase(itRecovery);
		if (bDemuxered && bDemuxerSuccess)
			emit signalResFileLostRecoveryState(resId, true);

		return;
	}

	d_ptr->m_nResDemuxeredCount++;

	if (d_ptr->m_bAfterLoadPrj)
	{
		if (d_ptr->m_nResNeedDemuxerCount > 0)
		{
			emit signalResDemuxerProgress(d_ptr->m_nResDemuxeredCount * 1.0f / d_ptr->m_nResNeedDemuxerCount * 95);
		}
	}
	else
	{
		emit signalResDemuxerProgress(d_ptr->m_nResDemuxeredCount);
	}

	if (d_ptr->m_nResDemuxeredCount == d_ptr->m_nResNeedDemuxerCount)	// 解复用完成
	{
		if (d_ptr->m_bReplaceResDemuxer)
		{
			g_pPlayerController->ReplaceRes(resId);
			HistoryStackSave();
		}
		else
			emit signalDemuxerFinish();
		d_ptr->m_bReplaceResDemuxer = false;

		d_ptr->m_pCheckDemuxerTimeOut->stop();
	}

	auto nCurTimeNode = QueryHighTickCountMS();
	{
		std::lock_guard<std::mutex> Locker(d_ptr->m_mtxVecNeedDemuxerIds);
		for (auto& it = d_ptr->m_vecNeedDemuxerIds.begin(); it != d_ptr->m_vecNeedDemuxerIds.end();)
		{
			if (it->first == resId)
			{
				it = d_ptr->m_vecNeedDemuxerIds.erase(it);
				continue;
			}
			it->second = nCurTimeNode;
			it++;
		}
	}
}

void CResourceTreeList::TrdUpdateResDemuxerState()
{
	auto mapRes = g_pResourceModel->GetAllResNode();
	for (auto& it : mapRes)
	{
		if (!IsFileResource(it.second->GetType()))
			continue;

		if (!it.second->GetResDemuxered()) // 说明当前素材正在解复用
			continue;

		bool bFileExist = QFile::exists(it.second->GetPath());
		if (it.second->GetType() == ERT_Picture)
		{
			bFileExist = bFileExist && QFile::exists(std::dynamic_pointer_cast<CPictureResource>(it.second)->GetOriginalPath());
		}

		if (it.second->GetDemuxerSuccessed() && !bFileExist)	// 说明本来存在的素材中途消失了
		{
			it.second->SetResDemuxered(true);
			it.second->SetDemuxerSuccessed(false);

			emit signalResFileLostRecoveryState(it.second->GetId(), false);
		}
		else if (!it.second->GetDemuxerSuccessed() && bFileExist)	// 说明消失的素材中途回来了
		{
			d_ptr->m_setRecoveryResIds.insert(it.second->GetId());

			it.second->SetResDemuxered(false);
			it.second->SetDemuxerSuccessed(false);

			//重新解复用时，更新缩略图为动图
			if (d_ptr->m_pMovie->state() != QMovie::Running)
			{
				qDebug() << "TrdUpdateResDemuxerState" << QThread::currentThreadId();
				emit signalUpdateResThumbnail();
			}

			switch (it.second->GetType())
			{
			case ERT_Video:
			case ERT_Audio:
			{
				emit signalDemuxer(it.second->GetId(), it.second->GetPath(), it.second->GetType());
			}
			break;
			case ERT_Picture:
			{
				if (auto pPicRes = std::dynamic_pointer_cast<IPictureResource>(it.second))
				{
					auto type = pPicRes->GetOptType();
					pPicRes->SetOptType(EPOT_1_1);

					emit signalDemuxer(it.second->GetId(), it.second->GetPath(), it.second->GetType());

					pPicRes->SetOptType(type);
				}
			}
			break;
			default:
				break;
			}
		}
	}
}

void CResourceTreeList::slotDemuxerDataFinish(const std::shared_ptr<SResFileDemuxInfo>& fileInfo)
{
	switch (fileInfo->type)
	{
	case ERT_Picture:
	case ERT_Audio:
	case ERT_Video:
	{
		std::shared_ptr<SMediaInfo>mediaInfo = std::dynamic_pointer_cast<SMediaInfo>(fileInfo);
		if (!mediaInfo)
			return;

		std::shared_ptr<IResourceBase> node = g_pResourceModel->GetResourceNode(mediaInfo->id);
		if (node == nullptr)
			return;
		node->SetResDemuxered(true);

		if (d_ptr->m_bNeedPicTran)
		{
			d_ptr->m_bNeedPicTran = false;
			//g_pPlayerController->PictureTrans(mediaInfo->id);
			return;
		}

		node->SetDemuxerSuccessed(mediaInfo->bDemuxerSuccess);
		node->SetResolution(QSize(mediaInfo->width, mediaInfo->height));
		if (node->GetType() != ERT_Picture)
			node->SetDuration(mediaInfo->duration);

		if (!mediaInfo->pixmap.isNull())
			node->SetThumbnail(mediaInfo->pixmap);

		node->SetFilesize(mediaInfo->filesize);
		node->SetAudioChannels(mediaInfo->audioChannels);
		node->SetResourceFormat(QString::fromStdString(mediaInfo->strFormat));

		node->SetAudioFormat(QString::fromStdString(mediaInfo->audiFormat));
		node->SetAudioSampleRate(mediaInfo->audioSamplerate);

		node->SetSupportHWDEC(mediaInfo->bSupportHWDEC);
		if (node->GetType() == ERT_Video)
		{
			// 入库解析：软解→软解；不可硬解→软解；自动/硬解且可硬解→硬解
			const int prjMode = g_pProjectSetting->LoadDefaultDecodingMode();
			const int mode = (!mediaInfo->bSupportHWDEC || prjMode == ED_Sort) ? ED_Sort : ED_Hard;
			node->SetDecodingMode(mode);
		}
		node->SetVideoBitRate(mediaInfo->videoBitrate);
		node->SetVideoFormat(QString::fromStdString(mediaInfo->vidioFormat));
		node->SetVideoFrameRate(mediaInfo->videoFramerate);

		if (node->GetType() == ERT_Picture)
		{
			auto pPic = std::dynamic_pointer_cast<IPictureResource>(node);
			if (pPic)
			{
				pPic->SetOptFilePaths(mediaInfo->OptFilePaths);
				pPic->SetExifOrientation(mediaInfo->nExifOrientation);
			}

			if (ToLowerCase(mediaInfo->strFormat) == "gif")		//GIF图片解复用完设置时长
			{
				node->SetDuration(mediaInfo->duration);
			}

		}
		else if (node->GetType() == ERT_Video)
		{
			auto pVideo = std::dynamic_pointer_cast<IVideoResource>(node);
			if (pVideo)
			{
				pVideo->SetSupportHDR(mediaInfo->m_bSupportHDR);
				pVideo->SetColorRange(mediaInfo->nColorRange);
			}
		}

		emit signalUpdateResourceNode(mediaInfo->id);
		UpdateDemuxerProgress(mediaInfo->id, true, mediaInfo->bDemuxerSuccess);

		MaterialTransfer::RequestTransfer(fileInfo->id, node->GetPath());
	}
		break;
	case ERT_Subtitle:
	{
		std::shared_ptr<SSubtitleInfo> info = std::dynamic_pointer_cast<SSubtitleInfo>(fileInfo);
		if (!info)
			return;

		QUuid resId = info->id;
		std::shared_ptr<IResourceBase> pRes = g_pResourceModel->GetResourceNode(resId);
		if (!pRes)
			return;

		std::shared_ptr<ISubtitleResource> pSubtitleRes = std::dynamic_pointer_cast<ISubtitleResource>(pRes);
		if (!pSubtitleRes)
			return;

		if (!info->vecSubtitleDatas.empty())
		{
			pRes->SetDuration(info->nMediaDuration);
			pSubtitleRes->SetSubtitleDatas(info->vecSubtitleDatas);
		}

		pRes->SetResDemuxered(true);
		emit signalUpdateResourceNode(resId);

		UpdateDemuxerProgress(info->id, true, info->bDemuxerSuccess);
	}
	break;
	case ERT_Teleprompter:
	{
		std::shared_ptr<STeleprompterInfo> info = std::dynamic_pointer_cast<STeleprompterInfo>(fileInfo);
		if (!info)
			return;

		QUuid resId = info->id;
		std::shared_ptr<IResourceBase> pRes = g_pResourceModel->GetResourceNode(resId);
		if (!pRes)
			return;

		std::shared_ptr<ITeleprompterResource> pTeleprompterRes = std::dynamic_pointer_cast<ITeleprompterResource>(pRes);
		if (!pTeleprompterRes)
			return;

		if (!info->vecTeleprompterDatas.empty())
		{
			pTeleprompterRes->SetTeleprompterData(info->vecTeleprompterDatas);
		}

		pRes->SetResDemuxered(true);
		emit signalUpdateResourceNode(resId);

		UpdateDemuxerProgress(info->id, true, info->bDemuxerSuccess);
	}
	break;
	default:
		break;
	}

	emit g_pGSNetSignal->signalResNodeOp(ETDT_UpdateResNode, fileInfo->id);

	
}

void CResourceTreeList::slotCheckResDemuxerTimeOut()
{
	QUuid idDemuxerFail;
	{
		std::lock_guard<std::mutex> Locker(d_ptr->m_mtxVecNeedDemuxerIds);
		for (auto it : d_ptr->m_vecNeedDemuxerIds)
		{
			if (QueryHighTickCountMS() - it.second > 30000)
			{
				std::shared_ptr<IResourceBase> pResNode = g_pResourceModel->GetResourceNode(it.first);
				if (!pResNode)
					continue;

				bool bDemuxerd = pResNode->GetResDemuxered();
				bool bDemuxerSucessed = pResNode->GetDemuxerSuccessed();
				if (!bDemuxerd && !bDemuxerSucessed)
					continue;

				pResNode->SetResDemuxered(false);
				//pResNode->SetDemuxerSuccessed(false);

				idDemuxerFail = it.first;
				break;
			}
		}
	}

	if (!idDemuxerFail.isNull())
		UpdateDemuxerProgress(idDemuxerFail, false, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CResourceTreeList::UpdateViewExpandState()
{
	auto listResNode = g_pResourceModel->GetAllResNode();
	for (auto& it : listResNode)
	{
		if (it.second->GetType() == ERT_Folder)
		{
			QModelIndex curIndex = g_pResourceModel->GetModelIndex(it.second->GetId());
			bool bExpand = d_ptr->m_mapResExpandState[it.second->GetId()];
			setExpanded(d_ptr->m_pFilterModel->mapFromSource(curIndex), bExpand);
		}
	}
}

void CResourceTreeList::AddNeedDemuxerResource(const QStringList& listPath, int nSender)
{
	QStringList listExistPath;
	QStringList listNotSupportPath;

	QStringList lstResPath;

	for (auto it : listPath)
	{
		QFileInfo fInfo(it);

		bool bNeedAdd = true;
		int nResType = ERT_Invalid;
		if (!fInfo.isDir())	//属于文件夹
		{
			nResType = GetFileResType(it);
			if (nResType == ERT_Invalid)	//过滤不支持素材
			{
				// 素材不支持，添加失败
				listNotSupportPath.append(it);
				bNeedAdd = false;
			}

			if (bNeedAdd && g_pResourceModel->IsResNodeExist(it))	// 已添加素材
			{
				listExistPath.push_back(it);
				bNeedAdd = false;
			}
		}
		
		if (bNeedAdd)
		{
			lstResPath << it;
		}
		else
		{
			LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CLogRecordTranslate::AddFailed(), it);
		}
	}

	if (!lstResPath.empty())
	{
		d_ptr->m_nResourceError = CResourceTreeListPrivate::EARE_Success;
		std::list<std::shared_ptr<IResourceBase>> list;
		for (auto it : lstResPath)
		{
			QFileInfo fInfo(it);
			if (fInfo.isDir())	//属于文件夹
			{
				std::shared_ptr<IResourceBase> pResource = g_pResourceModel->CreateResource(ERT_Folder);
				pResource->SetName(fInfo.fileName());

				std::shared_ptr<IFolderResource> pFolder = std::dynamic_pointer_cast<IFolderResource>(pResource);
				if (pFolder)
				{
					pFolder->SetFolderType(false);
					g_pSoftwareSetting->SetRecentOpenPath(it);

					g_pResourceModel->AddResource(pResource);
					CResourceNodeInfo* pResNodeInfo = g_pResourceModel->GetModelItem(pResource->GetId());
					AddRealFolder(fInfo.filePath(), pResNodeInfo, 1);
					continue;
				}
			}

			QFileInfo fileInfo(it);
			g_pSoftwareSetting->SetRecentOpenPath(fileInfo.dir().absolutePath());

			int type = GetFileResType(it);
			switch (type)
			{
			case ERT_Teleprompter:   //字幕和提词器统一处理
			{
				if (!d_ptr->IsMouseRightButtonAdd)
				{
					CSubtitleModeSelectDlg dlg(this, it, true);
					if (dlg.exec() == QDialog::Rejected)
					{
						LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CResourceTranslate::SubtitleTextSelectTitle(), CLogRecordTranslate::Cancel());
						continue;
					}

					if (dlg.GetSelectMode())
					{
						type = ERT_Subtitle;
					}
				}
				else
				{
					type = ERT_Subtitle;
				}
			}
			break;
			default:
				break;
			}

			std::shared_ptr<IResourceBase> pResource = g_pResourceModel->CreateResource(type);
			assert(pResource);

			QString strName = it.right(it.length() - it.lastIndexOf('/') - 1);
			pResource->SetFullName(strName);
			pResource->SetName(strName.left(strName.lastIndexOf('.')));
			pResource->SetPath(it);

			if (type == ERT_Subtitle || ERT_Teleprompter == type)	// 只在添加的时候需要解复用
				pResource->SetResDemuxered(false);

			list.push_back(pResource);

			LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CLogRecordTranslate::AddSuccess(),
				ResourceTypeString(pResource->GetType()), pResource->GetName());
		}
		d_ptr->IsMouseRightButtonAdd = false;

		if (!list.empty())	// 待解复用后保存撤销恢复节点
		{
			g_pResourceModel->AddResources(list, d_ptr->addResParentResId);
			// 发送数据至播放端
			for (const auto& it : list)
			{
				emit g_pGSNetSignal->signalResNodeOp(ETDT_AddResNode, it->GetId());
			}
		}

		DemuxerAllResource(false);	// 开始素材解复用
	}

	if (listExistPath.size() > 0)	// 弹窗提示重复素材
	{
		CShowTextDialog dlg(this, CResourceTranslate::ExistFileTips(), listExistPath);
		dlg.exec();
	}

	if (listNotSupportPath.size() > 0)	// 弹窗提示不支持文件
	{
		CShowTextDialog dlg(this, CResourceTranslate::NotSupportFileList(), listNotSupportPath);
		dlg.exec();
	}
}

void CResourceTreeList::AddNotDemuxerResByUndo(std::shared_ptr<IResourceBase> pNode, const QUuid& parentId)
{
	if (!pNode)
		return;

	g_pResourceModel->AddResource(pNode, parentId);

	// 发送数据至播放端
	emit g_pGSNetSignal->signalResNodeOp(ETDT_AddResNode, pNode->GetId());

	HistoryStackSave();//SAVE_PRJ_UNDO_NODE;	// 添加撤销恢复节点

	emit signalResourceCountChange();

	// 新加的素材选中
	QModelIndex newIndex = g_pResourceModel->GetModelIndex(pNode->GetId());
	setCurrentIndex(d_ptr->m_pFilterModel->mapFromSource(newIndex));
}

void CResourceTreeList::DealResourceOperator(QPoint pos, int nSelectRowCount, const QUuid& resId, bool bButtonClicked)
{
	CMenuEx* pRBtnMenu = new CMenuEx(this, true);
	pRBtnMenu->setProperty("ResourceWndMenu", true);

	auto AddMenuAction = [&](QMenu* pParentMenu, QString strText, int nData, bool bShow, QString strIcon = "", bool bChecked = false) ->QAction* {
		QAction* pAction = pParentMenu->addAction(strText);
		pAction->setData(nData);
		pAction->setVisible(bShow);
		if (!strIcon.isEmpty())
			pAction->setIcon(QIcon(strIcon));
		pAction->setCheckable(true);
		pAction->setChecked(bChecked);
		return pAction;
	};

	//判断某些项是否显示
	std::shared_ptr<IResourceBase> resNode = g_pResourceModel->GetResourceNode(resId);
	d_ptr->addResParentResId = resId;
	if (!resId.isNull() && !resNode)
	{
		delete pRBtnMenu;
		pRBtnMenu = nullptr;
		return;
	}

	int nResType = resNode ? resNode->GetType() : -1;
	bool bAddResOptionEnable = (nSelectRowCount <= 1);

	CMenuEx* pAddMenuParent = nullptr;
	if (bAddResOptionEnable) //如果右键的位置是素材则添加
	{
		if (nResType != ERT_Invalid)
		{
			CMenuEx* pResources = new CMenuEx(pRBtnMenu, true);
			pResources->setProperty("ResourceWndMenu", true);
			pResources->setIcon(QIcon(":/dark/images/ResourceWnd/Icon/AddResource.svg"));
			pResources->setTitle(CResourceTranslate::AddResource());
			pRBtnMenu->addSeparator();
			pRBtnMenu->addMenu(pResources);
			pAddMenuParent = pResources;
		}
		else
			pAddMenuParent = pRBtnMenu;

		// Timeline：仅保留本地媒体（视频/音频）与文件夹
		AddMenuAction(pAddMenuParent, CResourceTranslate::AddMedia(), CResourceTreeListPrivate::ID_AddMedia, bAddResOptionEnable, ":/dark/images/ResourceWnd/Icon/Media.svg");
		pAddMenuParent->addSeparator();
		AddMenuAction(pAddMenuParent, CResourceTranslate::AddFilter(), CResourceTreeListPrivate::ID_AddFilter, bAddResOptionEnable, ":/dark/images/ResourceWnd/Icon/VirtualFolder.svg");
		pAddMenuParent->addSeparator();
		AddMenuAction(pAddMenuParent, CResourceTranslate::AddFolder(), CResourceTreeListPrivate::ID_AddFolder, bAddResOptionEnable, ":/dark/images/ResourceWnd/Icon/RealFolder.svg");
		pAddMenuParent->addSeparator();
	}


	if (nSelectRowCount == 1 && nResType == ERT_Video)
	{
		std::shared_ptr<CVideoResource> videoNode = std::dynamic_pointer_cast<CVideoResource>(resNode);
		if (videoNode)
		{
			int colorRange = videoNode->GetColorRange();

			CMenuEx* pColorRange = new CMenuEx(pRBtnMenu, true);
			pColorRange->setTitle(CResourceTranslate::ColorRange());
			pRBtnMenu->addMenu(pColorRange);

			AddMenuAction(pColorRange, QString("0-255"), CResourceTreeListPrivate::ID_Video_ColorRange_Full, true, "", colorRange == CR_Full);
			AddMenuAction(pColorRange, QString("16-235"), CResourceTreeListPrivate::ID_Video_ColorRange_limit, true, "", colorRange == CR_Limit);

			const int decodingMode = videoNode->GetDecodingMode();
			CMenuEx* pDecodeType = new CMenuEx(pRBtnMenu, true);
			pDecodeType->setTitle(CResourceTranslate::DecodingType());
			pRBtnMenu->addMenu(pDecodeType);

			QAction* pHardAction = AddMenuAction(pDecodeType, CProjectSettingTranslate::DefaultDecodingOptions().at(1),
				CResourceTreeListPrivate::ID_Video_Decode_Hard, true, "", decodingMode == ED_Hard);
			pHardAction->setEnabled(videoNode->GetbSupportHWDEC());
			AddMenuAction(pDecodeType, CProjectSettingTranslate::DefaultDecodingOptions().at(0),
				CResourceTreeListPrivate::ID_Video_Decode_Soft, true, "", decodingMode == ED_Sort);
			AddMenuAction(pDecodeType, CProjectSettingTranslate::AutoSelectOption(),
				CResourceTreeListPrivate::ID_Video_Decode_Auto, true, "", decodingMode == ED_Auto);

			pRBtnMenu->addSeparator();
		}
	}

	if (nSelectRowCount == 1 && nResType == ERT_Picture)
	{
		std::shared_ptr<CPictureResource> pictureNode = std::dynamic_pointer_cast<CPictureResource>(resNode);
		if (pictureNode)
		{
			int nMinOpt = pictureNode->GetMinOptType();

			if (nMinOpt > EPOT_1_1)
			{
				CMenuEx* pPicTrans = new CMenuEx(pRBtnMenu, true);
				pPicTrans->setTitle(CResourceTranslate::PicTrans());
				pRBtnMenu->addMenu(pPicTrans);

				int nOpt = pictureNode->GetOptType();

				AddMenuAction(pPicTrans, CResourceTranslate::PicTransAuto(), CResourceTreeListPrivate::ID_Pictrans_Auto, true, "", nOpt == EPOT_Auto);
				if (nMinOpt >= EPOT_1_16)
					AddMenuAction(pPicTrans, QString("1:16"), CResourceTreeListPrivate::ID_Pictrans_1_16, true, "", nOpt == EPOT_1_16);
				if (nMinOpt >= EPOT_1_8)
					AddMenuAction(pPicTrans, QString("1:8"), CResourceTreeListPrivate::ID_Pictrans_1_8, true, "", nOpt == EPOT_1_8);
				if (nMinOpt >= EPOT_1_4)
					AddMenuAction(pPicTrans, QString("1:4"), CResourceTreeListPrivate::ID_Pictrans_1_4, true, "", nOpt == EPOT_1_4);
				if (nMinOpt >= EPOT_1_2)
					AddMenuAction(pPicTrans, QString("1:2"), CResourceTreeListPrivate::ID_Pictrans_1_2, true, "", nOpt == EPOT_1_2);
				AddMenuAction(pPicTrans, CResourceTranslate::PicTransNone(), CResourceTreeListPrivate::ID_Pictrans_1_1, true, "", nOpt == EPOT_1_1);

				pRBtnMenu->addSeparator();
			}
		}
	}

	// Timeline：不提供文字 / 测试图 / 字幕等小工具

	bool bShowAlwaysOpenOption = (nSelectRowCount == 1 && IsAlwaysOpenOption(nResType));
	if (bShowAlwaysOpenOption)	// 判断Check状态
	{
		bool bAlwaysOpen = resNode->IsKeepOpen();
		QString strIcon = bAlwaysOpen ? ":/dark/images/Menu/Checked.svg" : "";
		AddMenuAction(pRBtnMenu, CResourceTranslate::AlwaysOpen(), CResourceTreeListPrivate::ID_ResAlwaysOpen, bShowAlwaysOpenOption, strIcon, bAlwaysOpen);
		pRBtnMenu->addSeparator();
	}

	AddMenuAction(pRBtnMenu, CResourceTranslate::LocateFile(), CResourceTreeListPrivate::ID_LocateFile, resNode && resNode->GetDemuxerSuccessed() &&
		nSelectRowCount == 1 && (IsFileResource(nResType)));
	AddMenuAction(pRBtnMenu, CCommonTranslate::Rename(), CResourceTreeListPrivate::ID_ResourceRename, nSelectRowCount == 1 || nResType == ERT_Folder);
	AddMenuAction(pRBtnMenu, CResourceTranslate::ReplaceRes(), CResourceTreeListPrivate::ID_ReplaceFile, nSelectRowCount == 1 &&
		(nResType == ERT_Video || nResType == ERT_Audio));
	AddMenuAction(pRBtnMenu, CCommonTranslate::Delete(), CResourceTreeListPrivate::ID_Delete, nSelectRowCount > 0);// 删除按UI无图标
	
	QAction* pSelect = pRBtnMenu->exec(pos);
	if (pSelect == nullptr)
	{
		delete pRBtnMenu;
		pRBtnMenu = nullptr;
		return;
	}

	auto ChangeVideoColorRange = [=](int type)->void {
		auto res = g_pResourceModel->GetResourceNode(resId);
		if (auto pVideo = std::dynamic_pointer_cast<IVideoResource>(res))
		{
			pVideo->SetColorRange(type);
		}
	};

	auto ChangeVideoDecodingMode = [=](int mode)->void {
		auto res = g_pResourceModel->GetResourceNode(resId);
		if (res)
			res->SetDecodingMode(mode);
	};

	auto DoPicTrans = [=](int type)->void {
		auto res = g_pResourceModel->GetResourceNode(resId);
		if (res)
		{
			auto pPic = std::dynamic_pointer_cast<IPictureResource>(res);
			if (pPic)
			{
				pPic->SetOptType(type);

				QFileInfo file(res->GetPath());
				if (!file.isFile())
				{
					pPic->SetOptType(EPOT_1_1);

					d_ptr->m_bNeedPicTran = true;
					emit signalDemuxer(res->GetId(), res->GetPath(), res->GetType());

					pPic->SetOptType(type);
				}

				if (!d_ptr->m_bNeedPicTran)
				{
					//g_pPlayerController->PictureTrans(res->GetId());
				}
			}
		}
	};

	int nSelectType = pSelect->data().toInt();
	switch (nSelectType)
	{
	case CResourceTreeListPrivate::ID_AddMedia:
	{
		QStringList resType;
		resType << CResourceTranslate::AddMedia();
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), (bButtonClicked ? CLogRecordTranslate::TopbarButtonAdd() : CLogRecordTranslate::RButtonAdd()),
			resType.at(nSelectType));

		QString strFileFilter;
		// Timeline：仅视频 / 音频
		QString strVideoFilter = CResourceTranslate::ResVideoFilter().arg(strVideoFormat);
		QString strAudioFilter = CResourceTranslate::ResAudioFilter().arg(strAudioFormat);
		strFileFilter = QStringLiteral("Media Files(%1 %2)\n%3\n%4")
			.arg(strVideoFormat).arg(strAudioFormat).arg(strVideoFilter).arg(strAudioFilter);

		QFileDialog fileDlg(this);
		fileDlg.setWindowTitle(u8"Timeline");
		fileDlg.setNameFilter(strFileFilter);
		fileDlg.setFileMode(QFileDialog::ExistingFiles);
		fileDlg.setDirectory(g_pSoftwareSetting->GetRecentOpenPath());

		if (fileDlg.exec() == QDialog::Accepted)
		{
			QStringList beforeListFileName = fileDlg.selectedFiles();
			QStringList listFileName;
			for (auto it : beforeListFileName)
			{
				QFileInfo fInfo(it);
				if (!fInfo.isDir())	//不属于文件夹
					listFileName.push_back(it);
			}
			LOG_OPT_PC(CResourceTranslate::ResourceWnd(), (bButtonClicked ? CLogRecordTranslate::TopbarButtonAdd() : CLogRecordTranslate::RButtonAdd()), CLogRecordTranslate::OK());

			AddNeedDemuxerResource(listFileName);
		}
		else
		{
			LOG_OPT_PC(CResourceTranslate::ResourceWnd(), (bButtonClicked ? CLogRecordTranslate::TopbarButtonAdd() : CLogRecordTranslate::RButtonAdd()), CLogRecordTranslate::Cancel());
		}
	}
	break;
	case CResourceTreeListPrivate::ID_AddFilter:
	{
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), (bButtonClicked ? CLogRecordTranslate::TopbarButtonAdd() : CLogRecordTranslate::RButtonAdd()), CResourceTranslate::AddFilter());
		std::shared_ptr<IResourceBase> pResource = g_pResourceModel->CreateResource(ERT_Folder);
		pResource->SetName(g_pResourceModel->GetNewResSuitableName(ERT_Folder));

		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CLogRecordTranslate::AddSuccess(), ResourceTypeString(pResource->GetType()), pResource->GetName());
		AddNotDemuxerResByUndo(pResource, resId);
	}
	break;
	case CResourceTreeListPrivate::ID_AddFolder:
	{
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), (bButtonClicked ? CLogRecordTranslate::TopbarButtonAdd() : CLogRecordTranslate::RButtonAdd()), CResourceTranslate::AddFolder());
		QFileDialog file;
		file.setFileMode(QFileDialog::Directory);
		if (file.exec() == QDialog::Accepted)
		{
			QStringList listSelectFiles = file.selectedFiles();
			if (listSelectFiles.isEmpty())
				break;

			LOG_OPT_PC(CResourceTranslate::ResourceWnd(), (bButtonClicked ? CLogRecordTranslate::TopbarButtonAdd() : CLogRecordTranslate::RButtonAdd()), CLogRecordTranslate::OK(), listSelectFiles[0]);
			AddNeedDemuxerResource(listSelectFiles);
		}
		else
		{
			LOG_OPT_PC(CResourceTranslate::ResourceWnd(), (bButtonClicked ? CLogRecordTranslate::TopbarButtonAdd() : CLogRecordTranslate::RButtonAdd()), CLogRecordTranslate::Cancel());
		}
	}
	break;
	case CResourceTreeListPrivate::ID_VideoCapture:
	{
		AddDlgSettingRes(resId, ERT_VideoCapture, bButtonClicked);
	}
	break;
	case CResourceTreeListPrivate::ID_AudioCapture:
	{
		AddDlgSettingRes(resId, ERT_AudioCapture, bButtonClicked);
	}
	break;
	case CResourceTreeListPrivate::ID_NdiStream:
	{
		AddDlgSettingRes(resId, ERT_Ndi, bButtonClicked);
	}
	break;
	case CResourceTreeListPrivate::ID_WebMedia:
	{
		AddDlgSettingRes(resId, ERT_WebMedia, bButtonClicked);
	}
	break;
	case CResourceTreeListPrivate::ID_Text:
	{
		AddToolRes(ERT_Text, bButtonClicked);
	}
	break;
	case CResourceTreeListPrivate::ID_TestChart:
	{
		AddToolRes(ERT_TestChart, bButtonClicked);
	}
	break;
	case CResourceTreeListPrivate::ID_Subtitle:
	{
		AddSubtitleOrTeleprompter(resId, bButtonClicked);
	}
	break;
	case CResourceTreeListPrivate::ID_Delete:
	{
		int nSelectSize = selectedIndexes().size() / g_pResourceModel->columnCount();
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CLogRecordTranslate::RButtonDelete(),
			nSelectSize == 1 ? CLogRecordTranslate::SingleChoice() : CLogRecordTranslate::MultipleChoice());
		DelResources(true, selectedIndexes());
		break;
	}
	case CResourceTreeListPrivate::ID_LocateFile:
	{
		QString strPath;
		if (resNode->GetType() == ERT_Picture)
		{
			if (auto pPicRes = std::dynamic_pointer_cast<IPictureResource>(resNode))
				strPath = pPicRes->GetOriginalPath();
		}
		else
			strPath = resNode->GetPath();

		QStringList param;
		if (!QFileInfo(strPath).isDir())
			param << QLatin1String("/select,");
		param << QDir::toNativeSeparators(strPath);
		QProcess::startDetached("explorer", param);
	}
	break;
	case CResourceTreeListPrivate::ID_ResAlwaysOpen:
	{
		auto pResNode = g_pResourceModel->GetResourceNode(resId);
		if (!pResNode)
			break;

		bool bAlwaysOpen = pResNode->IsKeepOpen();
		pResNode->SetKeepOpen(!bAlwaysOpen);

		StreamMedia_DealStreamAlwaysOpen(pResNode);

		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CResourceTranslate::AlwaysOpen(), pResNode->GetName(), bAlwaysOpen ? CLogRecordTranslate::Open() : CLogRecordTranslate::Close());

		HistoryStackSave();//SAVE_PRJ_UNDO_NODE;
	}
	break;
	case CResourceTreeListPrivate::ID_ReplaceFile:
	{
		// Timeline：仅视频 / 音频可替换
		QString strVideoFilter = CResourceTranslate::ResVideoFilter().arg(strVideoFormat);
		QString strAudioFilter = CResourceTranslate::ResAudioFilter().arg(strAudioFormat);
		QString strFileFilter = QStringLiteral("Media Files(%1 %2)\n%3\n%4")
			.arg(strVideoFormat).arg(strAudioFormat).arg(strVideoFilter).arg(strAudioFilter);

		switch (nResType)
		{
		case ERT_Video:
			strFileFilter = strVideoFilter;
			break;
		case ERT_Audio:
			strFileFilter = strAudioFilter;
			break;
		default:
			break;
		}

		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CCommonTranslate::Replace(), resNode->GetName());
		QString strFileName = QFileDialog::getOpenFileName(this, u8"Timeline", g_pSoftwareSetting->GetRecentOpenPath(), strFileFilter);
		if (strFileName.isEmpty())
		{
			LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CCommonTranslate::Replace(), CLogRecordTranslate::Cancel());
			break;
		}

		QFileInfo fileInfo(strFileName);
		g_pSoftwareSetting->SetRecentOpenPath(fileInfo.dir().absolutePath());

		QString strBeforePath = resNode->GetPath();
		if (g_pResourceModel->IsResNodeExist(strFileName))
		{
			UICommon::CMessageBox::information(this, CCommonTranslate::MessageBoxTip(), CResourceTranslate::ReplaceExistResTips());
			LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CLogRecordTranslate::ReplaceFailed(), QString("%1 -> %2").arg(strBeforePath).arg(strFileName));
			break;
		}

		auto pResource = g_pResourceModel->GetResourceNode(resId);
		pResource->SetPath(strFileName);
		QString strName = strFileName.right(strFileName.length() - strFileName.lastIndexOf('/') - 1);
		pResource->SetName(strName.left(strName.lastIndexOf('.')));
		pResource->SetFullName(strName);
		pResource->SetResDemuxered(false);


		d_ptr->m_bReplaceResDemuxer = true;
		DemuxerAllResource(false);

		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CLogRecordTranslate::ReplaceSuccess(), QString("%1 -> %2").arg(strBeforePath).arg(strFileName));
	}
	break;
	case CResourceTreeListPrivate::ID_ResourceRename:
	{
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CCommonTranslate::Rename(), resNode->GetName());
		UICommon::CRenameDialog dlg(this, resNode->GetName());
		int ret = dlg.exec();
		if (ret == QDialog::Accepted)
		{
			QString logStr = resNode->GetName() + " -> " + dlg.GetText();
			LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CCommonTranslate::Rename(), CLogRecordTranslate::OK(), logStr);
			g_pResourceModel->RenameResourcesByUndo(resNode->GetId(), dlg.GetText());

			HistoryStackSave();//SAVE_PRJ_UNDO_NODE;
		}
		else
		{
			LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CCommonTranslate::Rename(), CLogRecordTranslate::Cancel());
		}
	}
	break;
	case CResourceTreeListPrivate::ID_Pictrans_Auto:
	{
		DoPicTrans(EPOT_Auto);
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CResourceTranslate::PicTrans(), resNode->GetName(), pSelect->text());
	}
	break;
	case CResourceTreeListPrivate::ID_Pictrans_1_16:
	{
		DoPicTrans(EPOT_1_16);
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CResourceTranslate::PicTrans(), resNode->GetName(), pSelect->text());
	}
	break;
	case CResourceTreeListPrivate::ID_Pictrans_1_8:
	{
		DoPicTrans(EPOT_1_8);
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CResourceTranslate::PicTrans(), resNode->GetName(), pSelect->text());
	}
	break;
	case CResourceTreeListPrivate::ID_Pictrans_1_4:
	{
		DoPicTrans(EPOT_1_4);
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CResourceTranslate::PicTrans(), resNode->GetName(), pSelect->text());
	}
	break;
	case CResourceTreeListPrivate::ID_Pictrans_1_2:
	{
		DoPicTrans(EPOT_1_2);
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CResourceTranslate::PicTrans(), resNode->GetName(), pSelect->text());
	}
	break;
	case CResourceTreeListPrivate::ID_Pictrans_1_1:
	{
		DoPicTrans(EPOT_1_1);
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CResourceTranslate::PicTrans(), resNode->GetName(), pSelect->text());
	}
	break;
	case CResourceTreeListPrivate::ID_Video_ColorRange_Full:
	{
		ChangeVideoColorRange(CR_Full);
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CResourceTranslate::ColorRange(), resNode->GetName(), pSelect->text());
	}
	break;
	case CResourceTreeListPrivate::ID_Video_ColorRange_limit:
	{
		ChangeVideoColorRange(CR_Limit);
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CResourceTranslate::ColorRange(), resNode->GetName(), pSelect->text());
	}
	break;
	case CResourceTreeListPrivate::ID_Video_Decode_Hard:
	{
		ChangeVideoDecodingMode(ED_Hard);
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CResourceTranslate::DecodingType(), resNode->GetName(), pSelect->text());
	}
	break;
	case CResourceTreeListPrivate::ID_Video_Decode_Soft:
	{
		ChangeVideoDecodingMode(ED_Sort);
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CResourceTranslate::DecodingType(), resNode->GetName(), pSelect->text());
	}
	break;
	case CResourceTreeListPrivate::ID_Video_Decode_Auto:
	{
		ChangeVideoDecodingMode(ED_Auto);
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CResourceTranslate::DecodingType(), resNode->GetName(), pSelect->text());
	}
	break;
	default:
		qWarning() << "CResourceTreeList::DealResourceOperator: unknown menu action id" << nSelectType;
		break;
	}
	delete pRBtnMenu;
	pRBtnMenu = nullptr;
}

void CResourceTreeList::DemuxerAllResource(bool bLoadPrj)
{
	emit signalResourceCountChange();
	emit signalResourceSelectChange(0, 0);

	d_ptr->m_nResNeedDemuxerCount = 0;
	d_ptr->m_nResDemuxeredCount = 0;

	if (d_ptr->m_pMovie->state() != QMovie::Running)
	{
		d_ptr->m_pMovie->start();
	}

	{
		std::lock_guard<std::mutex> Locker(d_ptr->m_mtxVecNeedDemuxerIds);
		d_ptr->m_vecNeedDemuxerIds.clear();
	}

	std::map<QUuid, std::shared_ptr<IResourceBase>> mapRes = g_pResourceModel->GetAllResNode();
	for (auto& it : mapRes)
	{
		if (it.second->GetResDemuxered() || !it.second->GetDemuxerSuccessed())	// 当前解复用不解析已经解析失败的素材
			continue;

		switch (it.second->GetType())
		{
		case ERT_Video:
		case ERT_Audio:
		case ERT_Subtitle:
		case ERT_Teleprompter:
		{
			emit signalDemuxer(it.second->GetId(), it.second->GetPath(), it.second->GetType(), it.second->GetName());
		}
		break;
		case ERT_Picture:
		{
			if (auto pPicRes = std::dynamic_pointer_cast<IPictureResource>(it.second))
			{
				auto type = pPicRes->GetOptType();
				pPicRes->SetOptType(EPOT_1_1);

				emit signalDemuxer(it.second->GetId(), it.second->GetPath(), it.second->GetType());

				pPicRes->SetOptType(type);
			}
		}
		break;
		default:
			// 非预期类型不参与本次解复用统计，避免错误计数
			qWarning() << "CResourceTreeList::DemuxerAllResource: unexpected resource type" << static_cast<int>(it.second->GetType());
			continue;
		}

		{
			std::lock_guard<std::mutex> Locker(d_ptr->m_mtxVecNeedDemuxerIds);
			d_ptr->m_vecNeedDemuxerIds.push_back({ it.second->GetId(),QueryHighTickCountMS() });
		}

		d_ptr->m_nResNeedDemuxerCount++;
	}

	if (d_ptr->m_nResNeedDemuxerCount > 0)
		d_ptr->m_pCheckDemuxerTimeOut->start(1000);	// 一秒检查一次

	if (d_ptr->m_nResNeedDemuxerCount == 0)	// 若没有需要解复用
	{
		emit signalDemuxerFinish();
		HistoryStackSave();
	}
	else if (!bLoadPrj) // 不是加载工程单独出进度条
	{
		if (d_ptr->m_pDemuxerResProgress)
		{
			delete d_ptr->m_pDemuxerResProgress;
			d_ptr->m_pDemuxerResProgress = nullptr;
		}
		d_ptr->m_pDemuxerResProgress = new UICommon::CProgressDialogEx(d_ptr->m_pMainWnd);
		d_ptr->m_pDemuxerResProgress->SetTitleName(CResourceTranslate::ResDemuxering());

		connect(this, &CResourceTreeList::signalResDemuxerProgress, d_ptr->m_pDemuxerResProgress, &UICommon::CProgressDialogEx::slotSetValue);
		d_ptr->m_pDemuxerResProgress->SetMinMaxRange(0, d_ptr->m_nResNeedDemuxerCount, EPT_RangeInt);
		d_ptr->m_pDemuxerResProgress->SetProgressValue(0);
		d_ptr->m_pDemuxerResProgress->show();
	}
}

std::shared_ptr<IResourceBase> CResourceTreeList::AddToolRes(int nType, bool bButtonClicked, uint sender)
{
	QString strTypeText = ResourceTypeString(nType);
	LOG_OPT_PC(CResourceTranslate::ResourceWnd(), (bButtonClicked ? CLogRecordTranslate::TopbarButtonAdd() : CLogRecordTranslate::RButtonAdd()), strTypeText);

	std::shared_ptr<IResourceBase> pRes = g_pResourceModel->CreateResource(nType);
	if (!pRes)
		return pRes;

	pRes->SetName(g_pResourceModel->GetNewResSuitableName(nType));

	LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CLogRecordTranslate::AddSuccess(), strTypeText, pRes->GetName());

	AddNotDemuxerResByUndo(pRes, d_ptr->addResParentResId);

	return pRes;
}


bool CResourceTreeList::AddSubtitleOrTeleprompter(const QUuid& resId, bool bButtonClicked)
{
	QStringList strSubtitleFilePath;
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
			return false;

		QFileInfo fInfo(listFileName.front());
		g_pSoftwareSetting->SetRecentOpenPath(fInfo.dir().absolutePath());

		strSubtitleFilePath = listFileName;

		LOG_OPT_PC(CLogRecordTranslate::LogProperty(), CPropertyTranslate::ResuorcePropertyWnd(),
			CCommonTranslate::Subtitle(), CCommonTranslate::Subtitle(), CSubtitleTranslate::ImprotFile(), CLogRecordTranslate::Save());
	}
	else
	{
		LOG_OPT_PC(CLogRecordTranslate::LogProperty(), CPropertyTranslate::ResuorcePropertyWnd(),
			CCommonTranslate::Subtitle(), CCommonTranslate::Subtitle(), CSubtitleTranslate::ImprotFile(), CLogRecordTranslate::Close());
	}

	d_ptr->IsMouseRightButtonAdd = true;
	LOG_OPT_PC(CResourceTranslate::ResourceWnd(), (bButtonClicked ? CLogRecordTranslate::TopbarButtonAdd() : CLogRecordTranslate::RButtonAdd()), CCommonTranslate::Subtitle());


	AddNeedDemuxerResource(strSubtitleFilePath);


	/*CSubtitleModeSelectDlg dlg(this);
	if (dlg.exec() == QDialog::Rejected || (dlg.GetSelectMode() && dlg.GetSubtitleFilePath().isEmpty()))
	{
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), CResourceTranslate::SubtitleTextSelectTitle(), CLogRecordTranslate::Cancel());
		return false;
	}

	if (dlg.GetSelectMode())
	{
		d_ptr->IsMouseRightButtonAdd = true;
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), (bButtonClicked ? CLogRecordTranslate::TopbarButtonAdd() : CLogRecordTranslate::RButtonAdd()), CCommonTranslate::Subtitle());

		const QStringList strListPath = dlg.GetSubtitleFilePath();

		AddNeedDemuxerResource(strListPath);
	}
	else
	{
		LOG_OPT_PC(CResourceTranslate::ResourceWnd(), (bButtonClicked ? CLogRecordTranslate::TopbarButtonAdd() : CLogRecordTranslate::RButtonAdd()), CCommonTranslate::Subtitle());

		std::shared_ptr<IResourceBase> pRes = g_pResourceModel->CreateResource(ERT_Teleprompter);
		pRes->SetName(g_pResourceModel->GetNewResSuitableName(ERT_Teleprompter));
		pRes->SetDemuxerSuccessed(true);
		pRes->SetResDemuxered(true);

		AddNotDemuxerResByUndo(pRes, resId);
	}
*/
	return true;
}

void CResourceTreeList::AddDlgSettingRes(const QUuid& resId, int nType, bool bBtnClicked)
{
	Q_UNUSED(resId);
	Q_UNUSED(nType);
	Q_UNUSED(bBtnClicked);
	// Timeline：不支持采集 / NDI / 流媒体对话框
	return;
}

void CResourceTreeList::keyPressEvent(QKeyEvent* event)
{
	bool bmodifiersflag = event->modifiers() == Qt::NoModifier;
	int ikey = event->key();
	if (bmodifiersflag && (ikey == Qt::Key_PageUp || ikey == Qt::Key_PageDown || ikey == Qt::Key_Home || ikey == Qt::Key_End))
	{
		bool bflag = event->isAutoRepeat();
		if (!bflag)
		{
			return;
		}
	}
	else if (ikey == Qt::Key_Delete)
	{
		slotDelSelectResource();
	}
	else if (ikey == Qt::Key_Backtab)
	{
		//qDebug() << currentIndex().data(Qt::UserRole) << " " <<model()->index(0,0).data(Qt::UserRole);

		if (currentIndex().data(Qt::UserRole) == model()->index(0, 0).data(Qt::UserRole))
		{
			QModelIndex childIndex = QModelIndex();
			do
			{
				// 获取子项的数量
				int numRows = d_ptr->m_pFilterModel->rowCount(childIndex) - 1;

				childIndex = d_ptr->m_pFilterModel->index(numRows, 0, childIndex); // 假设树是单列的

			} while (isExpanded(childIndex));

			if (childIndex.isValid())
				setCurrentIndex(childIndex);

			event->accept();
			return;
		}
	}
	else if (ikey == Qt::Key_Tab)
	{
		if (!this->selectionModel()->hasSelection())
			return;
		
		//qDebug() << currentIndex().row() << " " << ikey;
		bool bIsend = false;
		if (GetCurItemRow(QModelIndex(), bIsend) == countTreeItems(QModelIndex(), d_ptr->m_pFilterModel))
		{
			setCurrentIndex(model()->index(0, 0));
			event->accept();
			return;
		}
	}
	return __super::keyPressEvent(event);
}

//计算项的总数，不包含被折叠的
int CResourceTreeList::countTreeItems(const QModelIndex& parentIndex, QAbstractItemModel* model)
{
	int itemCount = 0;

	// 获取子项的数量
	int numRows = model->rowCount(parentIndex);

	// 遍历子项并递归计算
	for (int row = 0; row < numRows; ++row) {
		QModelIndex childIndex = model->index(row, 0, parentIndex); // 假设树是单列的
		itemCount++; // 计算当前项
		if (isExpanded(childIndex))
			itemCount += countTreeItems(childIndex, model);
	}

	return itemCount;
}

void CResourceTreeList::paintEvent(QPaintEvent* event)
{
	__super::paintEvent(event);

	
	if(g_pResourceModel->rowCount() == 0)
	{
		QPainter painter(viewport());
		painter.setRenderHint(QPainter::Antialiasing);  // 开启抗锯齿，绘制更平滑
		painter.setRenderHint(QPainter::SmoothPixmapTransform);
		QPixmap pixmap;
		int pixmapY = 0;  // 图标Y坐标
		if (pixmap.load(":/dark/images/ResourceWnd/Empty.svg"))
		{
			// 加载成功，缩放图片并保持宽高比
			QPixmap scaledPixmap = pixmap.scaled(SysDpiZoom(100),
				SysDpiZoom(84),
				Qt::KeepAspectRatio,
				Qt::SmoothTransformation);

			// 计算图标绘制位置（居中显示）
			int pixmapX = (viewport()->width() - scaledPixmap.width()) / 2;
			pixmapY = (viewport()->height() - scaledPixmap.height() - nSizeDefine_60) / 2;  // 预留文字空间
			// 绘制图标
			painter.drawPixmap(pixmapX, pixmapY, scaledPixmap);
		}

		QFont font("Microsoft Yahei");
		font.setPixelSize(nSizeDefine_12);
		font.setWeight(QFont::Bold);
		painter.setFont(font);
		painter.setPen(QColor(204, 208, 215));

		QRect textRect1 = viewport()->rect();
		textRect1.setTop(pixmapY + SysDpiZoom(84));
		textRect1.setBottom(textRect1.top() + nSizeDefine_20);  // 文字高度
		painter.drawText(textRect1, Qt::AlignHCenter | Qt::AlignTop, CResourceTranslate::ResUIPlaceHolderText());
	}
	else if (d_ptr->m_pFilterModel->rowCount() == 0)
	{
		QPainter painter(viewport());
		painter.setRenderHint(QPainter::Antialiasing);  // 开启抗锯齿，绘制更平滑
		painter.setRenderHint(QPainter::SmoothPixmapTransform);
		QPixmap pixmap;
		int pixmapY = 0;  // 图标Y坐标
		if (pixmap.load(":/dark/images/SliceWnd/NoSearchSlice.png"))
		{
			// 加载成功，缩放图片并保持宽高比
			QPixmap scaledPixmap = pixmap.scaled(SysDpiZoom(100),
				SysDpiZoom(84),
				Qt::KeepAspectRatio,
				Qt::SmoothTransformation);

			// 计算图标绘制位置（居中显示）
			int pixmapX = (viewport()->width() - scaledPixmap.width()) / 2;
			pixmapY = (viewport()->height() - scaledPixmap.height() - nSizeDefine_60) / 2;  // 预留文字空间
			// 绘制图标
			painter.drawPixmap(pixmapX, pixmapY, scaledPixmap);
		}

		QFont font("Microsoft Yahei");
		font.setPixelSize(nSizeDefine_12);
		font.setWeight(QFont::Bold);
		painter.setFont(font);
		painter.setPen(QColor(204, 208, 215));

		QRect textRect1 = viewport()->rect();
		textRect1.setTop(pixmapY + SysDpiZoom(84));
		textRect1.setBottom(textRect1.top() + nSizeDefine_20);  // 文字高度
		painter.drawText(textRect1, Qt::AlignHCenter | Qt::AlignTop, CSliceWndTranslate::NoSearchSlice());

	}
	
	if (verticalScrollBar()->isVisible() && header()->isVisible())
	{
		d_ptr->m_pRightTopWgt->setGeometry(this->viewport()->width(), 0,
			this->verticalScrollBar()->width(), this->header()->height());
		d_ptr->m_pRightTopWgt->setVisible(true);

		// 更新垂直滑动条位置
		int nNewHeight = this->height() - this->header()->height();

		if (this->horizontalScrollBar()->isVisible())
			nNewHeight -= horizontalScrollBar()->height();

		this->verticalScrollBar()->setGeometry(
			0, this->header()->height(), verticalScrollBar()->width(), nNewHeight);
	}
	else
		d_ptr->m_pRightTopWgt->setVisible(false);

	if (d_ptr->m_bIsDroping != -1)
	{
		const int nDropIndicatorMaxY = countTreeItems(QModelIndex(), d_ptr->m_pFilterModel) * (nSizeDefine_30 + 1);
		if (d_ptr->m_bIsDroping < nDropIndicatorMaxY)
		{
			QPainter painter(viewport());
			QPen pen;
			pen.setColor(QColor(255, 139, 24));  // 设置颜色为 RGBA (65, 127, 249, 1)
			pen.setWidth(nSizeDefine_1);  // 设置线条宽度
			painter.setPen(pen);

			if (d_ptr->m_bIsHitFolder)
				painter.drawRect(QRect(0, d_ptr->m_bIsDroping, viewport()->rect().width() - 1, nSizeDefine_30));
			else
				painter.drawLine(0, d_ptr->m_bIsDroping, viewport()->rect().width(), d_ptr->m_bIsDroping);
			d_ptr->m_bIsDroping = -1;
		}
	}
}

int CResourceTreeList::GetCurItemRow(const QModelIndex& parentIndex, bool& bIsFind)
{
	int itemCount = 0;

	// 获取子项的数量
	int numRows = d_ptr->m_pFilterModel->rowCount(parentIndex);

	// 遍历子项并递归计算
	for (int row = 0; row < numRows; ++row) {
		QModelIndex childIndex = d_ptr->m_pFilterModel->index(row, 0, parentIndex); // 假设树是单列的
		itemCount++; // 计算当前项

		if (currentIndex().row() == childIndex.row())
		{
			bIsFind = true;
			return itemCount;
		}

		if (isExpanded(childIndex)/* && currentIndex().parent() != childIndex*/)
		{
			itemCount += GetCurItemRow(childIndex, bIsFind);
			if (bIsFind)
				return itemCount;
		}
	}

	return itemCount;
}

void CResourceTreeList::UpdateColorTextResWnd(std::shared_ptr<IResourceBase> pNode)
{
	QVector<QString> vecPath;
	if (std::shared_ptr <ITextResource> pTextNode = std::dynamic_pointer_cast<ITextResource>(pNode))
		vecPath = pTextNode->GetSColorTextFillParam().m_vecImportTexturePath;

	if (vecPath.size() == 0)
		return;

	emit signalUpdateColorTextResWnd(vecPath);
}
