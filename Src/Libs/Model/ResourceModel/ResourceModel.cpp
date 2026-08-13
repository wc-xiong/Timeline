#include "ResourceModel.h"
#include "ResourceBase.h"
#include "VideoResource.h"
#include "AudioResource.h"
#include "PictureResource.h"
#include "FolderResource.h"
#include "AudioCaptureResource.h"
#include "VideoCaptureResource.h"
#include "TeleprompterResource.h"
#include <QDebug>
#include <QMimeData>
#include <QUrl>
#include "Common/CommonFunctionApi.h"
#include "Common/Timer.h"
#include "UICommon/UITranslate.h"
#include "ResourceBase.h"
#include "WebMediaResource.h"
#include "TextResource.h"
#include "TestChartResource.h"
#include "NdiStreamResource.h"
#include "SubtitleResource.h"
#include "ResourceNodeInfo.h"
#include <QPainter>
#include <QIcon>
#include "Common/UICommon/UICommonSizeDefine.h"
#include "Common/ResourceDefine.h"
#include "Common/Common/IResourceInterface.h"

#include "Model/GlobalModel/GlobalModel.h"
#include <QThread>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CResourceModel::CResourceModel(QObject* pParent) :
	QAbstractItemModel(pParent)
{
	m_bCurResourceModel = true;
	m_pRoot = new CResourceNodeInfo(this);

	std::shared_ptr<IResourceBase> rootRes = CreateResource(ERT_Folder);

	if (rootRes)
	{
		if(std::shared_ptr<IFolderResource> rootFolder = std::dynamic_pointer_cast<IFolderResource>(rootRes))
			rootFolder->SetRootFolder();
		m_pRoot->SetResNode(rootRes);//创建根节点
	}

	qRegisterMetaType<QVector<int> >("QVector<int>");

	m_RWMutex = new QReadWriteLock(QReadWriteLock::Recursive);

	m_pMtxResDirty = new QReadWriteLock();
	m_pMtxSlicePage = new QReadWriteLock();

	m_loadBasePix = QPixmap(":/dark/images/ResourceWnd/Thumbmail/Normal/Base.svg");
	m_loadfailPix = QPixmap(":/dark/images/ResourceWnd/Thumbmail/Normal/LoadFail.svg");
}

CResourceModel::~CResourceModel()
{
	ResetResModel();

	delete m_pRoot;
	m_pRoot = nullptr;

	delete m_pMtxResDirty;
	m_pMtxResDirty = nullptr;

	delete m_pMtxSlicePage;
	m_pMtxSlicePage = nullptr;

	delete m_RWMutex;
	m_RWMutex = nullptr;
}

ICopyInterface * CResourceModel::Copy()
{
	CResourceModel* other = new CResourceModel(*this);

	if (other->m_pRoot)
	{
		delete other->m_pRoot;
		other->m_pRoot = nullptr;
	}

	other->m_pRoot = CopyResourceModel();
	
	return other;
}

void CResourceModel::CopyTo(ICopyInterface * p)
{
	auto pOtherResourceModel = dynamic_cast<CResourceModel*>(p);
	if (pOtherResourceModel == nullptr)
		return;

	pOtherResourceModel->ResetModel(m_pRoot);
}

Qt::ItemFlags CResourceModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags theFlags = QAbstractItemModel::flags(index);
	theFlags |= Qt::ItemIsDropEnabled;
	if (index.isValid()) 
		theFlags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
	return theFlags;
}

QModelIndex CResourceModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const
{
	if (!m_pRoot || row < 0 || column < 0 || column >= ResourceModel::ECT_Count || (parent.isValid() && parent.column() != 0))
		return QModelIndex();

	QReadLocker lock(m_RWMutex);
	CResourceNodeInfo *parentItem = itemForIndex(parent);
	if (CResourceNodeInfo *item = parentItem->ChildAt(row))
		return createIndex(row, column, item);
	return QModelIndex();
}

QModelIndex CResourceModel::parent(const QModelIndex &child) const
{
	if (!child.isValid())
		return QModelIndex();

	QReadLocker lock(m_RWMutex);
	CResourceNodeInfo* childItem = static_cast<CResourceNodeInfo*>(child.internalPointer());
	if (!childItem)
		return QModelIndex();

	CResourceNodeInfo *parentItem = childItem->GetParent();
	if (!parentItem|| parentItem == m_pRoot)
		return QModelIndex();

	if (CResourceNodeInfo *grandParentItem = parentItem->GetParent())
	{
		int row = grandParentItem->RowOfChild(parentItem);
		return createIndex(row, 0, parentItem);
	}
	
	return QModelIndex();
}

std::shared_ptr<IResourceBase> CResourceModel::GetResNodeFromIndex(const QModelIndex &index) const
{
	if (auto pModelNode = itemForIndex(index))
		return pModelNode->GetResNode();
	return nullptr;
}


QVariant CResourceModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
	if (!m_pRoot || !index.isValid() || index.column() < 0 || index.column() >= ResourceModel::ECT_Count)
		return QVariant();

	QReadLocker lock(m_RWMutex);
	std::shared_ptr<IResourceBase> item = GetResNodeFromIndex(index);
	if (!item)
		return QVariant();
	//if (role == Qt::FontRole && index.column() == 0 && !item->GetDemuxerSuccessed())
	//{
	//	QFont font;
	//	font.setStrikeOut(true);//删除线
	//	return font;
	//}
	//else if (role == Qt::TextColorRole)
	if (role == Qt::TextColorRole)
	{
		if (!item->GetDemuxerSuccessed())
			return QColor(100, 104, 109);
		return QColor(Qt::white);
	}
	else if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		switch (index.column())
		{
		case ResourceModel::ECT_Name:
			return item->GetName();
		case ResourceModel::ECT_Resolution:
		{
			if(IsToolResource(item->GetType()))
				return u8"—";

			if (IsResWndShowReslution(item->GetType()))
			{
				if (!item->GetDemuxerSuccessed())
					return u8"—";
				return QString("%1 x %2").arg(item->GetResolution().width()).arg(item->GetResolution().height());
			}
			else
			{
				return u8"—";
			}
		}
		case ResourceModel::ECT_Duration:
		{
			if (IsSupportShowDuration(item->GetType()) || item->GetResourceFormat().toLower() == "gif")
				return FormatMsTime2String(item->GetDuration());
			else
			{
				return u8"—";
			}
		}
		case ResourceModel::ECT_Type:
		{
			return ResourceTypeString(item->GetType());
		}
		default:
			return QVariant();
		}
	}

	else if (role == Qt::DecorationRole && index.column() == ResourceModel::ECT_Name)
	{
		QPixmap Pix;
		bool bIsUsed = (item->GetResourceUseCount()>0);
		if (!item->GetResDemuxered()) //是否完成解复用
		{
			Pix = GetLoadingThumbnail(true, bIsUsed);
		}
		else if (!item->GetDemuxerSuccessed()) //解复用失败
		{
			Pix = GetLoadingThumbnail(false, bIsUsed);
		}
		else //解复用成功
		{
			Pix = item->GetThumbnail(true, true);
		}

		QIcon icon = QIcon(Pix);
		icon.addPixmap(Pix, QIcon::Selected);
		return icon;
	}

	else if (role == Qt::ToolTipRole && item->GetType() != ERT_Invalid)
	{
		QString strTips = CCommonTranslate::Name() + ": " + item->GetName() + "\n";
		switch (item->GetType())
		{
		case ERT_Invalid:
			break;
		case ERT_Video:
		{
			strTips += CCommonTranslate::Type() + ": " + item->GetResourceFormat() + "\n";
			strTips += CCommonTranslate::Duration() + ": " + FormatMsTime2String(item->GetDuration()) + "\n";
			strTips += CCommonTranslate::Resolution() + ": " + QString("%1 x %2").arg(item->GetResolution().width()).arg(item->GetResolution().height()) + "\n";
			strTips += CCommonTranslate::AudioChannels() + ": " + QString::number(item->GetAudioChannels()) + "\n";
			strTips += CCommonTranslate::Size() + ": " + QString::number(item->GetFilesize() / 1024.0f / 1024.0f, 'f', 2) + " MB\n";
			strTips += CCommonTranslate::Path() + ": " + item->GetPath();
		}
			break;
		case ERT_Audio:
		{
			strTips += CCommonTranslate::Type() + ": " + item->GetAudioFormat() + "\n";
			strTips += CCommonTranslate::Duration() + ": " + FormatMsTime2String(item->GetDuration()) + "\n";
			strTips += CCommonTranslate::AudioChannels() + ": " + QString::number(item->GetAudioChannels()) + "\n";
			strTips += CCommonTranslate::Size() + ": " + QString::number(item->GetFilesize() / 1024.0f / 1024.0f, 'f', 2) + " MB\n";
			strTips += CCommonTranslate::Path() + ": " + item->GetPath();
		}
			break;
		case ERT_Picture:
		{
			strTips += CCommonTranslate::Type() + ": " + item->GetResourceFormat() + "\n";
			strTips += CCommonTranslate::Size() + ": " + QString::number(item->GetFilesize() / 1024.0f / 1024.0f, 'f', 2) + " MB\n";
			if (std::shared_ptr<CPictureResource> pBaseNode = std::dynamic_pointer_cast<CPictureResource>(std::dynamic_pointer_cast<CResourceBase>(item)))
			{
				//auto resNode = *pBaseNode;
				strTips += CCommonTranslate::Path() + ": " + pBaseNode->GetOriginalPath();
			}
		}
			break;
		case ERT_Folder:
		{
			strTips += CCommonTranslate::Type() + ": " + ResourceTypeString(item->GetType());
		}
			break;
		case ERT_VideoCapture:
		{
			strTips += CCommonTranslate::Video() + ": " + item->GetPath() + "\n";
			if (auto pCapNode = std::dynamic_pointer_cast<IVideoCaptureResource>(item))
			{
				strTips += CCommonTranslate::Format() + ": " + QString("%1 x %2").arg(item->GetResolution().width()).arg(item->GetResolution().height()) + " "
					+ QString::number(pCapNode->GetFps(), 'f', 2) + "fps " + pCapNode ->GetColotFmt();
			}
		}
			break;
		case ERT_AudioCapture:
		{
			strTips += CCommonTranslate::Audio() + ": " + item->GetPath() + "\n";
			if (auto pCapNode = std::dynamic_pointer_cast<IAudioCaptureResource>(item))
			{
				strTips += CCommonTranslate::Format() + ": " + QString("%1 x %2").arg(pCapNode->GetChannels()).arg(pCapNode->GetSampleRate()) + "HZ";
			}
		}
			break;
		case ERT_WebMedia:
		{
			strTips += CCommonTranslate::Type() + ": Stream\n";
			strTips += CCommonTranslate::WebPage() + ": " + item->GetPath() + "\n";
			strTips += CCommonTranslate::Duration() + ": " + FormatMsTime2String(item->GetDuration()) + "\n";
			strTips += CCommonTranslate::Resolution() + ": " + QString("%1 x %2").arg(item->GetResolution().width()).arg(item->GetResolution().height());
		}
			break;
		case ERT_Ndi:
		{
			strTips += CCommonTranslate::Type() + ": " + ResourceTypeString(item->GetType()) + "\n";
			strTips += CNdiTranslate::SourceName() + ": " + item->GetPath() + "\n";
			if (auto pNode = std::dynamic_pointer_cast<INdiStreamResource>(item))
			{
				strTips += QString("IP: ") + pNode->GetIp() + "\n";
				strTips += CCommonTranslate::AudioChannels() + ": " + QString::number(pNode->GetChannel()) + "\n";
			}
			strTips += CCommonTranslate::Resolution() + ": " + QString("%1 x %2").arg(item->GetResolution().width()).arg(item->GetResolution().height());
		}
			break;
		case ERT_TestChart:
		case ERT_Text:
		case ERT_Subtitle:
		case ERT_Teleprompter:
		{
			strTips += CCommonTranslate::Type() + ": " + ResourceTypeString(item->GetType());
		}
		break;
		case ERT_Count:
			break;
		default:
			break;
		}
		return strTips;
	}
	else if (role == Qt::UserRole)
	{
		return item->GetId();
	}
	return QVariant();
}

int CResourceModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
	if (parent.column() > 0)
		return 0;

	QReadLocker lock(m_RWMutex);
	auto *parentItem = itemForIndex(parent);
	return parentItem ? parentItem->GetChildCount() : 0;
}

int CResourceModel::columnCount(const QModelIndex &) const
{
	return ResourceModel::ECT_Count;
}

bool CResourceModel::hasChildren(const QModelIndex &parent /*= QModelIndex()*/) const
{
	QReadLocker lock(m_RWMutex);
	if(auto pItem = itemForIndex(parent))
		return pItem->GetChildCount() > 0;
	return false;
}

QVariant CResourceModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	return QVariant();
}

bool CResourceModel::removeRows(int row, int count, const QModelIndex &parent /*= QModelIndex()*/)
{
	if (row == -1)
		return false;

	beginRemoveRows(parent, row, row + count - 1);
	{
		QWriteLocker lock(m_RWMutex);
		CResourceNodeInfo *item = parent.isValid() ? static_cast<CResourceNodeInfo*>(parent.internalPointer()) : m_pRoot;
		if (!item)
			return false;
		std::vector<QUuid> vecDelResIds;
		for (int i = 0; i < count; ++i)
			item->DelChildResource(row, vecDelResIds);

		for (auto it : vecDelResIds)
			m_mapResNodeInfo.erase(it);
	}
	endRemoveRows();
	return true;
}

QStringList CResourceModel::mimeTypes() const
{
	return QStringList() << ResourceModel::MimeType;
}

bool CResourceModel::canDropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
	if (action == Qt::IgnoreAction)
		return false;

	if (mimeData->hasFormat(ResourceModel::MimeType))
		return true;

	if (mimeData->hasFormat(ResourceModel::SysMimeType))
		return true;
	return false;
}

bool CResourceModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	return __super::dropMimeData(mimeData, action, row, column, parent);	// 不用Qt自带的
}

CResourceNodeInfo* CResourceModel::itemForIndex(const QModelIndex &index) const
{
	QReadLocker lock(m_RWMutex);

	if (auto item = static_cast<CResourceNodeInfo*>(index.internalPointer()))
		return item;

	return m_pRoot;
}

void CResourceModel::sort(int column, Qt::SortOrder order /*= Qt::AscendingOrder*/)
{
	QWriteLocker lock(m_RWMutex);
	m_pRoot->SortChild(column, order == Qt::AscendingOrder);
}

QString CResourceModel::GetNewResSuitableName(int nType, bool bFirst)
{
	std::vector<QString> vecNames = GetAllTypeResName(nType);

	for (int i = 1; i <= Max_CustomizedResource_Num; i++)
	{
		QString strName;
		switch (nType)
		{
		case ERT_Text:
		{
			strName = CResourceTranslate::TextName().arg(i);
		}
			break;
		case ERT_TestChart:
			strName = CResourceTranslate::TestChartName().arg(i);
			break;
		case ERT_Subtitle:
			strName = CCommonTranslate::Subtitle() + QString(" %1").arg(i);
			break;
		case ERT_Teleprompter:
			strName = CCommonTranslate::Teleprompter() + QString(" %1").arg(i);
			break;
		case ERT_Folder:
			strName = CResourceTranslate::NewFolder() + QString(" %1").arg(i);
			break;
		default:
			assert(0);
			return "";
		}
		auto it = std::find(vecNames.begin(), vecNames.end(), strName);
		if (it == vecNames.end())
			return strName;
	}
	return "";
}

std::vector<QString> CResourceModel::GetAllCaptureName(bool bAudioCapture)
{
	QReadLocker lock(m_RWMutex);

	std::vector<QString> vecNames;
	for (auto it = m_mapResNodeInfo.begin(); it != m_mapResNodeInfo.end(); ++it)
	{
		if ((it->second->GetType() == ERT_VideoCapture && !bAudioCapture) || (it->second->GetType() == ERT_AudioCapture && bAudioCapture))
		{
			vecNames.push_back(it->second->GetPath());
		}
	}

	return vecNames;
}

std::vector<QString> CResourceModel::GetAllStreamMediaUrl()
{
	QReadLocker lock(m_RWMutex);

	std::vector<QString> vecUrls;
	for (auto it = m_mapResNodeInfo.begin(); it != m_mapResNodeInfo.end(); ++it)
	{
		if (it->second->GetType() == ERT_WebMedia)
		{
			vecUrls.push_back(it->second->GetPath());
		}
	}

	return vecUrls;
}

bool CResourceModel::GetNdiResolutionByName(const QString& strName, QSize& size)
{
	QReadLocker lock(m_RWMutex);

	for (auto it = m_mapResNodeInfo.begin(); it != m_mapResNodeInfo.end(); ++it)
	{
		if (it->second->GetType() != ERT_Ndi)
			continue;

		if (it->second->GetPath() == strName)
		{
			size = it->second->GetResolution();
			return true;
		}
	}

	return false;
}

std::vector<QString> CResourceModel::GetAllNdiDeviceName()
{
	QReadLocker lock(m_RWMutex);

	std::vector<QString> vecIps;
	for (auto it = m_mapResNodeInfo.begin(); it != m_mapResNodeInfo.end(); ++it)
	{
		if (it->second->GetType() == ERT_Ndi)
			vecIps.push_back(it->second->GetPath());
	}
	return vecIps;
}

std::vector<QString> CResourceModel::GetAllTypeResName(int nType)
{
	std::vector<QString> vecNames;
	for (auto it = m_mapResNodeInfo.begin(); it != m_mapResNodeInfo.end(); ++it)
	{
		if (it->second->GetType() == nType)
		{
			vecNames.push_back(it->second->GetName());
		}
	}

	return vecNames;
}

CResourceNodeInfo* CResourceModel::GetModelItem(QUuid Guid, CResourceNodeInfo* pParent /*= nullptr*/)
{
	QReadLocker lock(m_RWMutex);

	if (!pParent)
		pParent = m_pRoot;

	CResourceNodeInfo* node = nullptr;
	std::list<CResourceNodeInfo*> listChildNode = pParent->GetChildNodeList();

	for (auto it = listChildNode.begin(); it != listChildNode.end(); it++)
	{
		if ((*it)->GetResId() == Guid)
		{
			node = *it;
			break;
		}

		node = GetModelItem(Guid, (*it));
		if (node)
			break;
	}
	return node;
}

QModelIndex CResourceModel::GetModelIndex(const QUuid &resId)
{
	if(resId.isNull())
		return QModelIndex();

	QReadLocker lock(m_RWMutex);

	CResourceNodeInfo* pItem = GetModelItem(resId);
	return GetModelIndex(pItem);
}

QModelIndex CResourceModel::GetModelIndex(CResourceNodeInfo* pItem)
{
	QReadLocker lock(m_RWMutex);

	if (!pItem || pItem == m_pRoot)
		return QModelIndex();

	if (auto pParentItem = pItem->GetParent())
	{
		int row = pParentItem->RowOfChild(pItem);
		return createIndex(row, 0, pItem);
	}
	return QModelIndex();
}

std::shared_ptr<IResourceBase> CResourceModel::CreateResource(int nType, QUuid& resId)
{
	IResourceBase* pResource = nullptr;

	switch (nType)
	{
	case ERT_Invalid:
		assert(0);
		break;
	case ERT_Video:
	{
		pResource = new CVideoResource;
		QPixmap pix(":/dark/images/ResourceWnd/Thumbmail/Normal/Video.svg");
		pix = pix.scaled(Res_ThumbnailWidth, Res_ThumbnailHeight);
		pResource->SetThumbnail(pix);          
		break;
	}
	case ERT_Picture:
	{
		pResource = new CPictureResource;
		QPixmap pix(":/dark/images/ResourceWnd/Thumbmail/Normal/Picture.svg");
		pix = pix.scaled(Res_ThumbnailWidth, Res_ThumbnailHeight);
		pResource->SetThumbnail(pix);
		break;
	}
	case ERT_Audio:
	{
		pResource = new CAudioResource;
		QPixmap pix(":/dark/images/ResourceWnd/Thumbmail/Normal/Audio.svg");
		pix = pix.scaled(Res_ThumbnailWidth, Res_ThumbnailHeight);
		pResource->SetThumbnail(pix);
		break;
	}
	case ERT_Folder:
	{
		pResource = new CFolderResource;
		QPixmap pix(":/dark/images/ResourceWnd/Thumbmail/Normal/Folder.svg");
		pix = pix.scaled(Res_ThumbnailWidth, Res_ThumbnailHeight);
		pResource->SetThumbnail(pix);
		break;
	}
	case ERT_VideoCapture:
	{
		pResource = new CVideoCaptureResource;
		QPixmap pix(":/dark/images/ResourceWnd/Thumbmail/Normal/VideoCapture.svg");
		pix = pix.scaled(Res_ThumbnailWidth, Res_ThumbnailHeight);
		pResource->SetThumbnail(pix);
		break;
	}
	case ERT_AudioCapture:
	{
		pResource = new CAudioCaptureResource;
		QPixmap pix(":/dark/images/ResourceWnd/Thumbmail/Normal/AudioCapture.svg");
		pix = pix.scaled(Res_ThumbnailWidth, Res_ThumbnailHeight);
		pResource->SetThumbnail(pix);
		break;
	}
	case ERT_WebMedia:
	{
		pResource = new CWebMediaResource;
		QPixmap pix(":/dark/images/ResourceWnd/Thumbmail/Normal/live.svg");
		pix = pix.scaled(Res_ThumbnailWidth, Res_ThumbnailHeight);
		pResource->SetThumbnail(pix);
		break;
	}
	case ERT_Text:
	{
		pResource  = new CTextResource;
		QPixmap pix(":/dark/images/ResourceWnd/Thumbmail/Normal/TXT.svg");
		pix = pix.scaled(Res_ThumbnailWidth, Res_ThumbnailHeight);
		pResource->SetThumbnail(pix);
		break;
	}
	break;
	case ERT_TestChart:
	{
		pResource = new CTestChartResource;
		QPixmap pix(":/dark/images/ResourceWnd/Thumbmail/Normal/TestChart.svg");
		pix = pix.scaled(Res_ThumbnailWidth, Res_ThumbnailHeight);
		pResource->SetThumbnail(pix);
		break;
	}
	case ERT_Ndi:
	{
		pResource = new CNdiStreamResource;
		QPixmap pix(":/dark/images/ResourceWnd/Thumbmail/Normal/NDI.svg");
		pix = pix.scaled(Res_ThumbnailWidth, Res_ThumbnailHeight);
		pResource->SetThumbnail(pix);
		break;
	}
	case ERT_Subtitle:
	{
		pResource = new CSubtitleResource;
		break;
	}
	case ERT_Teleprompter:
	{
		pResource = new CTeleprompterResource;
		break;
	}
	default:
		return nullptr;
	}

	pResource->SetId(resId);
	return std::shared_ptr<IResourceBase>(pResource);
}

CResourceNodeInfo* CResourceModel::CopyResource(CResourceNodeInfo* node, CResourceNodeInfo* newParent)
{
	//QReadLocker lock(m_RWMutex);
	if (!node)
		return nullptr;

	CResourceNodeInfo* pResourceInfo = new CResourceNodeInfo();

	auto pPreResNode = node->GetResNode();
	if (pPreResNode)
	{
		auto pPreCopyResNode = pPreResNode->Copy();
		if (auto pRes = dynamic_cast<IResourceBase*>(pPreCopyResNode))
			pResourceInfo->SetResNode(std::shared_ptr<IResourceBase>(pRes));
		else
		{
			delete pResourceInfo;
			return nullptr;
		}
	}

	auto lstChild = node->GetChildNodeList();
	for (auto child : lstChild)
	{
		pResourceInfo->AddChildResource(CopyResource(child, pResourceInfo), -1);
	}
	pResourceInfo->SetParent(newParent);

	return pResourceInfo;
}

void CResourceModel::AddResources(std::list<std::shared_ptr<IResourceBase>> list, const QUuid &parentId)
{
	QModelIndex parentIndex = QModelIndex();
	CResourceNodeInfo* pParentItem = nullptr;
	QModelIndex  CurItem = GetModelIndex(parentId);
	int nRow = CurItem.row();
	int nIndex = nRow;
	{
		QReadLocker lock(m_RWMutex);
		if (CurItem.isValid())
		{
			pParentItem = itemForIndex(CurItem);
			if (pParentItem->GetResType() != ERT_Folder)
			{
				pParentItem = pParentItem->GetParent();
				parentIndex = CurItem.parent();
			}
			else
			{
				parentIndex = GetModelIndex(pParentItem);
				nRow = pParentItem->GetChildCount();
				nIndex = -1;
			}
		}
		else
		{
			pParentItem = m_pRoot;
			nRow = m_pRoot->GetChildCount();
			nIndex = -1;
			parentIndex = QModelIndex();
		}
	}

	beginInsertRows(parentIndex, nRow, nRow + (int)list.size() - 1);
	{
		QWriteLocker lock(m_RWMutex);
		foreach(std::shared_ptr<IResourceBase> pItem, list)
		{
			auto modelItem = new CResourceNodeInfo(this);
			modelItem->SetResNode(pItem);

			pParentItem->AddChildResource(modelItem, nIndex);
			modelItem->SetParent(pParentItem);

			m_mapResNodeInfo[pItem->GetId()] = pItem;
		}
	}
	endInsertRows();
}

void CResourceModel::AddResource(std::shared_ptr<IResourceBase> pNode, const QUuid &parentId)
{
	std::list<std::shared_ptr<IResourceBase>> list;
	list.push_back(pNode);
	AddResources(list, parentId);
}

std::shared_ptr<IResourceBase> CResourceModel::GetResourceNode(QUuid Guid)
{
	auto it = m_mapResNodeInfo.find(Guid);
	if (it == m_mapResNodeInfo.end())
		return nullptr;
	return it->second;
}

void CResourceModel::SetResDirty(const QUuid& id, bool bDirty)
{
	if (id.isNull())
		return;
	QWriteLocker lock(m_pMtxResDirty);
	m_mapResDirty[id] = bDirty;
}

bool CResourceModel::GetResDirty(const QUuid &id)
{
	if (id.isNull())
		return false;
	{
		QReadLocker lock(m_pMtxResDirty);
		auto it = m_mapResDirty.find(id);
		if (it != m_mapResDirty.end())
			return it->second;
	}
	// 首次查询：记为 dirty，促使小工具补绘
	QWriteLocker lock(m_pMtxResDirty);
	auto it = m_mapResDirty.find(id);
	if (it != m_mapResDirty.end())
		return it->second;
	m_mapResDirty[id] = true;
	return true;
}


void CResourceModel::SetNeedToSlicePage(const QUuid & id, bool bFlag)
{
	if (id.isNull())
		return;
	QWriteLocker lock(m_pMtxSlicePage);
	m_mapResNeedToSlicePage[id] = bFlag;
}

bool CResourceModel::GetNeedToSlicePage(const QUuid & id)
{
	if (id.isNull())
		return false;
	QWriteLocker lock(m_pMtxSlicePage);
	auto it = m_mapResNeedToSlicePage.find(id);
	if (it == m_mapResNeedToSlicePage.end())
	{
		m_mapResNeedToSlicePage[id] = true;
		return true;
	}
	return it->second;
}


void CResourceModel::UpdateResUIByPtr(CResourceNodeInfo* pItem)
{
	QReadLocker lock(m_RWMutex);

	assert(pItem && pItem != m_pRoot);
	if (CResourceNodeInfo *pParentItem = pItem->GetParent())
	{
		int row = pParentItem->RowOfChild(pItem);
		QModelIndex beiginIndex = createIndex(row, 0, pItem);
		QModelIndex endIndex = createIndex(row, 3, pItem);
		emit dataChanged(beiginIndex, endIndex);
	}
}

void CResourceModel::UpDateResUIById(const QUuid &resId)
{
	QReadLocker lock(m_RWMutex);
	UpdateResUIByPtr(GetModelItem(resId));
}

bool CResourceModel::IsResNodeExist(const QString &strPath)
{
	QReadLocker lock(m_RWMutex);

	for (auto it = m_mapResNodeInfo.begin(); it != m_mapResNodeInfo.end(); ++it)
	{
		if (!IsFileResource(it->second->GetType()))
			continue;

		QString strResPath = it->second->GetPath();
		if (it->second->GetType() == ERT_Picture)
		{
			if (auto pPicRes = std::dynamic_pointer_cast<IPictureResource>(it->second))
				strResPath = pPicRes->GetOriginalPath();
		}

		if (strResPath == strPath)
			return true;
	}

	return false;
}

void CResourceModel::slotUpdateResDirty(const QUuid &resId, bool bValue)
{
	SetResDirty(resId, bValue);
}

void CResourceModel::ResDataUpdateByTranslation()
{
	QReadLocker lock(m_RWMutex);

	for (auto it = m_mapResNodeInfo.begin(); it != m_mapResNodeInfo.end(); ++it)
	{
		if (IsNeedDrawDeafultPic(it->second->GetType()))
		{
			QWriteLocker lock(m_pMtxResDirty);
			m_mapResDirty[it->second->GetId()] = true;
		}
	}
}

void CResourceModel::UpdateResourceUsedCount(const QUuid &id, int nCount)
{
	QReadLocker lock(m_RWMutex);

	CResourceNodeInfo* pResourceInfo = GetModelItem(id);

	if (!pResourceInfo)
		return;

	std::shared_ptr<IResourceBase> pResource = pResourceInfo->GetResNode();
	if (!pResource)
		return;

	bool bUsed = pResource->GetResourceUseCount();

	while (pResourceInfo != m_pRoot)
	{
		pResource->UpdateResourceUseCount(nCount);
		if (pResource->GetType() != ERT_Folder)
		{
			QModelIndex index = GetModelIndex(id);
			emit dataChanged(index, index);
		}
		
		pResourceInfo = pResourceInfo->GetParent();
		pResource = pResourceInfo->GetResNode();
	}
}

std::map<QUuid, std::shared_ptr<IResourceBase>> CResourceModel::GetAllResNode()
{
	QReadLocker lock(m_RWMutex);
	return m_mapResNodeInfo;
}

bool CResourceModel::IsRootNode(const QModelIndex& index)
{
	if (m_pRoot == itemForIndex(index))
		return true;
	return false;
}

std::vector<std::pair<QUuid, QString>> CResourceModel::GetAllNeedSyncRes()
{
	std::list<std::shared_ptr<IResourceBase>> listAllRes = GetAllResList();

	std::vector<std::pair<QUuid, QString>> lst;
	for (const auto& pRes : listAllRes)
	{
		if (!pRes || !IsNeedsFileTransfer(pRes->GetType()))
			continue;

		QString path;
		const auto pPic = std::dynamic_pointer_cast<IPictureResource>(pRes);
		if (pPic != nullptr)
			path = pPic->GetOriginalPath();
		else
			path = pRes->GetPath();

		if (path.isEmpty() || !QFileInfo::exists(path))
			continue;
		lst.push_back({ pRes->GetId(),path });
	}
	return lst;
}

bool CResourceModel::GetAllResDemuxered()
{
	QReadLocker lock(m_RWMutex);

	for (auto it = m_mapResNodeInfo.begin(); it != m_mapResNodeInfo.end(); ++it)
	{
		if (!it->second->GetResDemuxered())
		{
			return true; //解复用未完成
		}
	}
	return false;
}

void CResourceModel::UpdateResThumbnail(const QPixmap& pix)
{
	//qDebug() << "UpdateResThumbnail" << QThread::currentThreadId();
	m_loadingPix = pix;

	QReadLocker lock(m_RWMutex);

	for (auto it = m_mapResNodeInfo.begin(); it != m_mapResNodeInfo.end(); ++it)
	{
		if (!it->second->GetResDemuxered())
		{
			CResourceNodeInfo* pResourceInfo = GetModelItem(it->first);
			if (CResourceNodeInfo* pParentItem = pResourceInfo->GetParent())
			{
				int row = pParentItem->RowOfChild(pResourceInfo);
				QModelIndex Index = createIndex(row, 0, pResourceInfo);
				emit dataChanged(Index, Index);
			}
		}
	}
}

void CResourceModel::SaveResourceModelToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator, bool bPacketed, bool bOnlyUsed)
{
	QReadLocker lock(m_RWMutex);
	SaveResourceNodeToRapidJson(root, allocator, m_pRoot, bPacketed, bOnlyUsed);
}

void CResourceModel::LoadResourceModelFromRapidJson(const rapidjson::Value &root)
{
	beginResetModel();

	{
		QWriteLocker lock(m_RWMutex);
		m_mapResDirty.clear();
	}

	{
		QWriteLocker lock(m_pMtxSlicePage);
		m_mapResNeedToSlicePage.clear();
	}

	auto pNewRoot = LoadResourceFromRapidJson(root);
	if(pNewRoot)
	{
		QWriteLocker lock(m_RWMutex);

		m_pRoot->DeleteAllChild();
		m_pRoot->deleteLater();
		//delete m_pRoot;
		m_pRoot = pNewRoot;
	}

	endResetModel();
}

void CResourceModel::SaveResourceNodeToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator, CResourceNodeInfo* pResourceInfo, bool bPacketed, bool bOnlyUsed)
{
	QReadLocker lock(m_RWMutex);

	if (!pResourceInfo)
		return;

	auto pResource = pResourceInfo->GetResNode();

	rapidjson::Value obj(rapidjson::kObjectType);
	pResource->SaveDataToRapidJson(obj, allocator);
	if (bPacketed)
	{
		QString strPrePath;

		if (pResource->GetType() == ERT_Picture)
		{
			strPrePath = std::dynamic_pointer_cast<IPictureResource>(pResource)->GetOriginalPath();
		}
		else
		{
			strPrePath = pResource->GetPath();
		}

		if (IsFileResource(pResource->GetType()))
		{
			// 打包时将绝对路径设置为相对路径
			QString strResId = pResource->GetId().toString();
			pResource->SetPath(QString("/Resource/%1_%2").arg(strResId.right(5).left(4)).arg(pResource->GetFullName()));	
		}
			
		obj.SetObject();
		pResource->SaveDataToRapidJson(obj, allocator);
		pResource->SetPath(strPrePath);
	}
	
	std::list<CResourceNodeInfo*> listChildNode = pResourceInfo->GetChildNodeList();

	if (!listChildNode.empty())
	{
		rapidjson::Value arResource(rapidjson::kArrayType);
		foreach(auto it, listChildNode)
		{
			if (bPacketed && bOnlyUsed && it->GetResNode()->GetResourceUseCount() == 0)
				continue;
		
			rapidjson::Value childRapid(rapidjson::kObjectType);
			SaveResourceNodeToRapidJson(childRapid, allocator, it, bPacketed, bOnlyUsed);
			arResource.PushBack(childRapid, allocator);
		}
		obj.AddMember("ChildNodeList", arResource, allocator);
	}
	root.CopyFrom(obj, allocator);
}

CResourceNodeInfo* CResourceModel::LoadResourceFromRapidJson(const rapidjson::Value &root)
{
	QWriteLocker lock(m_RWMutex);
	if (!root.IsObject())
		return nullptr;

	if (!root.HasMember(Xml_ResourceModel_ResourceType) || !root.HasMember(Xml_ResourceModel_ResourceID))
		return nullptr;
	int nType = root[Xml_ResourceModel_ResourceType].GetInt();
	QUuid resId = QUuid(QString::fromUtf8(root[Xml_ResourceModel_ResourceID].GetString())); 

	CResourceNodeInfo* pResourceInfo = new CResourceNodeInfo(this);

	std::shared_ptr<IResourceBase> pResource = CreateResource(nType, resId);
	//AddResource(pResource);

	assert(pResource);
	pResource->LoadDataFromRapidJson(root);
	if (pResource)
	{
		if (std::shared_ptr<IFolderResource> rootFolder = std::dynamic_pointer_cast<IFolderResource>(pResource))
			rootFolder->SetRootFolder();
		pResourceInfo->SetResNode(pResource);//创建根节点
	}

	m_mapResNodeInfo[pResource->GetId()] = pResource;

	if (root.HasMember("ChildNodeList") && root["ChildNodeList"].IsArray())
	{
		const rapidjson::Value& arChildNode = root["ChildNodeList"];
		for (rapidjson::SizeType i = 0; i < arChildNode.Size(); ++i)
		{
			const rapidjson::Value& childObj = arChildNode[i];
			if (!childObj.IsObject())
				continue;
			CResourceNodeInfo* pChildResourceInfo = LoadResourceFromRapidJson(childObj);
			if (!pChildResourceInfo)
				continue;
			pResourceInfo->AddChildResource(pChildResourceInfo, -1);
			pChildResourceInfo->SetParent(pResourceInfo);
		}
	}

	return pResourceInfo;
}

void CResourceModel::InitResourceModel()
{
	{
		QWriteLocker lock(m_RWMutex);
		m_mapResDirty.clear();
	}

	{
		QWriteLocker lock(m_pMtxSlicePage);
		m_mapResNeedToSlicePage.clear();
	}

	beginResetModel();
	{
		QWriteLocker lock(m_RWMutex);

		// 当前存在的素材则不需要创建
		m_mapTempResNodeInfo.clear();
		m_mapResNodeInfo.swap(m_mapTempResNodeInfo);
		m_mapTempResNodeInfo.clear();
		m_pRoot->DeleteAllChild();
	}
	endResetModel();
}

CResourceNodeInfo * CResourceModel::CopyResourceModel()
{
//	QReadLocker lock(m_RWMutex);
	auto pRoot = CopyResource(m_pRoot, nullptr);
	return pRoot;
}

CResourceNodeInfo * CResourceModel::GetRootNode()
{
	return m_pRoot;
}

QPixmap CResourceModel::GetLoadingThumbnail(bool bDemuxerSuccessed, bool bIsUsed) const
{
	QPixmap thumbnail(":/dark/images/ResourceWnd/Thumbmail/Normal/Base.svg");

	// 缩小 QPixmap 的内容，留出一个像素的边框空间
	QPixmap pix = thumbnail.scaled(Res_ThumbnailUIShowWidth - nSizeDefine_2, Res_ThumbnailUIShowHeight - nSizeDefine_2);

	// 创建一个稍大的 QPixmap 以容纳缩小后的图像和边框
	QPixmap result(Res_ThumbnailUIShowWidth, Res_ThumbnailUIShowHeight);
	result.fill(Qt::transparent);

	QPainter painter(&result);
	// 创建一个圆角矩形路径
	QPainterPath path;
	QRect rc = result.rect().adjusted(nSizeDefine_1, nSizeDefine_1, -nSizeDefine_1, -nSizeDefine_1); // 为留出边框调整矩形位置和大小
	int nRadius = nSizeDefine_2; // 圆角半径
	path.addRoundedRect(rc, nRadius, nRadius);

	// 设置剪切路径，这样绘制时只会在圆角矩形内绘制
	painter.setClipPath(path);

	// 将缩小后的 pix 绘制到圆角矩形内部
	painter.drawPixmap(nSizeDefine_1, nSizeDefine_1, pix);
	painter.setRenderHint(QPainter::Antialiasing);

	if (bDemuxerSuccessed)
	{
		pix = m_loadingPix.scaled(nSizeDefine_18, nSizeDefine_18);
	}
	else
	{
		pix = m_loadfailPix.scaled(nSizeDefine_18, nSizeDefine_18);
	}
	
	int xOffset = (rc.width() - pix.width()) / 2;
	int yOffset = (rc.height() - pix.height()) / 2;
	painter.drawPixmap(rc.x() + xOffset, rc.y() + yOffset, pix);

	// 移除剪切路径以绘制边框
	painter.setClipping(false);

	// 绘制边框
	painter.setPen(QPen(QColor(0, 0, 0)));
	painter.drawRoundedRect(result.rect().adjusted(0, 0, -nSizeDefine_1, -nSizeDefine_1), nRadius, nRadius);

	// 如果需要绘制 "已使用" 图标
	if (bIsUsed)
	{
		QRect rc = result.rect();
		rc.setTop(rc.top() + nSizeDefine_1);
		rc.setLeft(rc.left() + nSizeDefine_1);
		rc.setRight(rc.left() + nSizeDefine_13);
		rc.setBottom(rc.top() + nSizeDefine_13);
		painter.drawPixmap(rc, QPixmap(":/dark/images/ResourceWnd/ResourceUsed.png"));
	}

	return result;
}

CResourceNodeInfo* CResourceModel::ResetResNodeInfo(CResourceNodeInfo* pPreNode, CResourceNodeInfo* pParent)
{
	if (!pPreNode)
		return nullptr;

	CResourceNodeInfo* pResourceInfo = new CResourceNodeInfo(this);

	if (auto pPreResNode = pPreNode->GetResNode())
	{
		std::shared_ptr<IResourceBase> pCurRes = nullptr;
		if (pPreResNode->GetType() == ERT_Folder && std::dynamic_pointer_cast<IFolderResource>(pPreResNode)->IsRootFolder())
		{
			pCurRes = pPreResNode;
		}
		else
		{
			auto it = m_mapResNodeInfo.find(pPreResNode->GetId());
			if (it == m_mapResNodeInfo.end())
				pCurRes = CreateResource(pPreResNode->GetType());
			else
				pCurRes = it->second;
			assert(pCurRes);

			pPreResNode->CopyTo(pCurRes.get());

			m_mapTempResNodeInfo[pCurRes->GetId()] = pCurRes;
		}

		pResourceInfo->SetResNode(pCurRes);

	}

	auto lstChild = pPreNode->GetChildNodeList();
	for (auto child : lstChild)
	{
		pResourceInfo->AddChildResource(ResetResNodeInfo(child, pResourceInfo), -1);
	}

	pResourceInfo->SetParent(pParent);

	return pResourceInfo;
}

void CResourceModel::ResetModel(CResourceNodeInfo * pRoot)
{
	QAbstractItemModel::beginResetModel();

	// 当前存在的素材则不需要创建
	m_mapTempResNodeInfo.clear();

	auto pNewRoot = ResetResNodeInfo(pRoot, nullptr);

	m_mapResNodeInfo.swap(m_mapTempResNodeInfo);
	m_mapTempResNodeInfo.clear();

	m_pRoot->DeleteAllChild();
	delete m_pRoot;

	m_pRoot = pNewRoot;

	QAbstractItemModel::endResetModel();

	// 小工具全部重绘（撤销/恢复后参数已变，需强制刷新纹理；不能只依赖历史 dirty 表项）
	{
		QWriteLocker lock(m_pMtxResDirty);
		for (auto itTool = m_mapResDirty.begin(); itTool != m_mapResDirty.end();)
		{
			auto baseNode = GetResourceNode(itTool->first);
			if (!baseNode)
			{
				itTool = m_mapResDirty.erase(itTool);
				continue;
			}

			itTool->second = true;
			++itTool;
		}

		for (const auto& itRes : m_mapResNodeInfo)
		{
			if (itRes.second && IsToolResource(itRes.second->GetType()))
				m_mapResDirty[itRes.first] = true;
		}
	}
}

int CResourceModel::GetResNodeCount(int nFilterType, bool bAddFolder)
{
	QReadLocker lock(m_RWMutex);
	int nCount = 0;

	for (auto it = m_mapResNodeInfo.begin(); it != m_mapResNodeInfo.end(); ++it)
	{
		if (it->second->GetType() == ERT_Folder)
		{
			if (bAddFolder)
				nCount++;
		}
		else if (nFilterType == ERT_Invalid || it->second->GetType() == nFilterType)
			nCount++;
		else if (nFilterType == RES_OtherType && IsOtherRes(it->second->GetType()) )
			nCount++;
	}

	return nCount;
}

void CResourceModel::RenameResourcesByUndo(const QUuid& resId, const QString& name)
{
	QReadLocker lock(m_RWMutex);

	CResourceNodeInfo* node = GetModelItem(resId);
	if(!node)
		return;

	auto resNode = node->GetResNode();
	if (resNode)
	{
		resNode->SetName(name);
		UpdateResUIByPtr(node);
	}
}

void CResourceModel::DropMimeData(const QMimeData *mimeData, const QModelIndex &parent)
{
	if (!mimeData->hasFormat(ResourceModel::MimeType))
		return;
	QUuid insertBelowId = QUuid();
	QModelIndex parentIndex = parent;
	CResourceNodeInfo* pParentInfo = itemForIndex(parent);

	auto pParentResNode = pParentInfo->GetResNode();

	// 寻找插入位置
	{
		QReadLocker lock(m_RWMutex);

		insertBelowId = pParentResNode->GetId();
		if (pParentResNode->GetType() != ERT_Folder)
		{
			parentIndex = parent.parent();
			if (pParentInfo->GetParent())
			{
				pParentInfo = pParentInfo->GetParent();
				pParentResNode = pParentInfo->GetResNode();
			}
		}
	}

	QByteArray itData = mimeData->data(ResourceModel::MimeType);
	QDataStream dataStream(&itData, QIODevice::ReadOnly);

	QList<QUuid>listItemId;
	QList<QString> resourceName;
	dataStream >> listItemId >> resourceName;

	beginResetModel();

	for (auto it : listItemId)
	{
		CResourceNodeInfo* pItemInfo = GetModelItem(it);
		std::shared_ptr<IResourceBase> pItem = pItemInfo->GetResNode();

		UpdateResourceUsedCount(pItemInfo->GetParent()->GetResNode()->GetId(), -pItem->GetResourceUseCount());
		{
			QWriteLocker lock(m_RWMutex);
			pItemInfo->GetParent()->DelChildResource(pItemInfo->GetResId(), false);
			assert(pItemInfo->GetParent());
			pParentInfo->AddChildResource(pItemInfo, insertBelowId);
			pItemInfo->SetParent(pParentInfo);
		}
		UpdateResourceUsedCount(pParentResNode->GetId(), pItem->GetResourceUseCount());
		insertBelowId = pItem->GetId();
	}

	endResetModel();
}

void CResourceModel::ResetResModel()
{
	QReadLocker lock(m_RWMutex);

	m_mapTempResNodeInfo.clear();
	m_mapResNodeInfo.swap(m_mapTempResNodeInfo);
	m_mapTempResNodeInfo.clear();

	m_pRoot->DeleteAllChild();
}


//返回素材列表
std::list<std::shared_ptr<IResourceBase>> CResourceModel::GetAllResList()
{
	std::list<std::shared_ptr<IResourceBase>> listAllRes;
	
	QReadLocker lock(m_RWMutex);
	GetResNodeList(m_pRoot, listAllRes);

	return listAllRes;
}


void CResourceModel::GetResNodeList(CResourceNodeInfo* pItme, std::list<std::shared_ptr<IResourceBase>>& listRes)
{
	QReadLocker lock(m_RWMutex);
	auto listChilds = pItme->GetChildNodeList();
	for (auto it : listChilds)
	{
		if (it->GetResType() == ERT_Folder)
			GetResNodeList(it, listRes);
		else
			listRes.push_back(it->GetResNode());
	}
}
