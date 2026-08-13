#pragma once
#include <QAbstractItemModel>
#include <rapidjson/document.h>
#include <QModelIndex>
#include <QUuid>
#include <mutex>
#include <QObject>
#include <QReadWriteLock>
#include "ResourceModelGlobal.h"
#include "Common/Common/IResourceInterface.h"
#include <QPixmap>
#include <shared_mutex>

class CTimer;
class CResourceNodeInfo;
class IResourceBase;

namespace ResourceModel {
	const QString MimeType = "ResourceItem";	//素材库内部拖拽
	const QString SysMimeType = "text/uri-list";	//直接从系统中拖拽文件进入素材库

	enum EColumnType {
		ECT_Name = 0,
		ECT_Resolution,
		ECT_Type,
		ECT_Duration,
		ECT_Count,
	};
}

class RESOURCEMODEL_EXPORT CResourceModel : public QAbstractItemModel,public ICopyInterface
{
	Q_OBJECT

public:
	CResourceModel(QObject* pParent = 0);
	virtual ~CResourceModel();

	CResourceModel(const CResourceModel& other) {}

public:
	virtual ICopyInterface* Copy();
	virtual void CopyTo(ICopyInterface* p);

public:
	void InitResourceModel();
	void ResetResModel();	// 此处是为了释放所有素材node，无撤销恢复，慎用
	void ResetModel(CResourceNodeInfo* pRoot);
	CResourceNodeInfo* CopyResourceModel();
	CResourceNodeInfo* GetRootNode();

public: //查询
	//通过素材数据指针获取对应界面ModelIndex
	QModelIndex GetModelIndex(const QUuid &resId);
	QModelIndex GetModelIndex(CResourceNodeInfo* node);

	//通过界面ModelIndex获取素材数据指针
	std::shared_ptr<IResourceBase> GetResNodeFromIndex(const QModelIndex &index) const;
	CResourceNodeInfo* itemForIndex(const QModelIndex &index) const;

	CResourceNodeInfo* GetModelItem(QUuid Guid, CResourceNodeInfo* pParent = nullptr);

	std::list<std::shared_ptr<IResourceBase>> GetAllResList();	//返回素材列表,不包含目录
	std::map<QUuid, std::shared_ptr<IResourceBase>> GetAllResNode();
	std::shared_ptr<IResourceBase> GetResourceNode(QUuid Guid);	//通过GUID获取数据节点指针
	bool IsResNodeExist(const QString &strPath);	// 判断当前素材素材是否添加过
	bool IsRootNode(const QModelIndex& index);	// 索引项是否为根节点

	// 获取所有需要同步到播放端的素材信息
	std::vector<std::pair<QUuid, QString>> GetAllNeedSyncRes();

public:
	//更新小工具素材的重绘标志
	void SetResDirty(const QUuid& id, bool bDirty);
	bool GetResDirty(const QUuid &id);

	void SetNeedToSlicePage(const QUuid &id, bool bFlag);
	bool GetNeedToSlicePage(const QUuid &id);

	std::vector<QString> GetAllCaptureName(bool bAudioCapture);
	std::vector<QString> GetAllStreamMediaUrl();
	std::vector<QString> GetAllNdiDeviceName();
	std::vector<QString> GetAllTypeResName(int nType);
	bool GetNdiResolutionByName(const QString& strName, QSize& size);
	QString GetNewResSuitableName(int nType, bool bFirst = false);	// 根据类型获取新素材名称，小工具，合集等
	int GetResNodeCount(int nFilterType, bool bAddFolder = false);
	bool GetAllResDemuxered();	// 当前所有的素材是否还存在正在解复用
	void ResDataUpdateByTranslation();	// 翻译更新，处理需要更新的素材

public:
	//通过素材类型，创建素材数据对象
	std::shared_ptr<IResourceBase> CreateResource(int nType, QUuid& resId = QUuid::createUuid());

	//添加素材资源
	void AddResources(std::list<std::shared_ptr<IResourceBase>> list, const QUuid &parentId = QUuid());
	void AddResource(std::shared_ptr<IResourceBase> pNode, const QUuid &parentId = QUuid());

public:
	void DropMimeData(const QMimeData *data, const QModelIndex &parent);

	//更新Model界面
	void UpdateResUIByPtr(CResourceNodeInfo* pItem);
	void UpDateResUIById(const QUuid &resId);

	void UpdateResourceUsedCount(const QUuid &id, int nCount);		// 更新素材使用次数
	void RenameResourcesByUndo(const QUuid& resId, const QString& name);

	void UpdateResThumbnail(const QPixmap &pix);	//更新缩略图

public://Model数据保存与加载
	void SaveResourceModelToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator, bool bPacketed, bool bOnlyUsed = false);
	void LoadResourceModelFromRapidJson(const rapidjson::Value &root);
	void SaveResourceNodeToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator, CResourceNodeInfo* pResource, bool bPacketed, bool bOnlyUsed = false);
	CResourceNodeInfo*  LoadResourceFromRapidJson(const rapidjson::Value &root);

protected:
	CResourceNodeInfo* CopyResource(CResourceNodeInfo* node, CResourceNodeInfo* newParent);
	CResourceNodeInfo* ResetResNodeInfo(CResourceNodeInfo* pRoot, CResourceNodeInfo* pParent);
	void GetResNodeList(CResourceNodeInfo* pItme, std::list<std::shared_ptr<IResourceBase>>& listRes);
	QPixmap GetLoadingThumbnail(bool bDemuxerSuccessed, bool bIsUsed) const;

public:
	Qt::ItemFlags flags(const QModelIndex &index) const override;

	virtual QModelIndex index(int row, int column,
		const QModelIndex &parent = QModelIndex()) const override;
	virtual QModelIndex parent(const QModelIndex &child) const override;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

	//Drag、Drop
	virtual QStringList mimeTypes() const override;
	virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action,
		int row, int column, const QModelIndex &parent) const override;
	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
		int row, int column, const QModelIndex &parent) override;

	virtual Qt::DropActions supportedDropActions() const override
	{
		return Qt::DropAction::MoveAction | Qt::DropAction::CopyAction;
	}
	virtual Qt::DropActions supportedDragActions() const override
	{
		return Qt::DropAction::MoveAction | Qt::DropAction::CopyAction;
	}

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

protected slots:
	void slotUpdateResDirty(const QUuid &resId, bool bValue);

signals:
	void signalExControlResThumbnailChanged(const QUuid & resId);

private:
	CResourceNodeInfo* m_pRoot = nullptr;	//素材库根节点指针

	std::map<QUuid, std::shared_ptr<IResourceBase>> m_mapResNodeInfo;
	std::map<QUuid, std::shared_ptr<IResourceBase>> m_mapTempResNodeInfo;

	QReadWriteLock* m_RWMutex = nullptr;//读写锁

	QReadWriteLock* m_pMtxResDirty = nullptr;
	std::map<QUuid, bool> m_mapResDirty; // 素材数据是否被修改标志（目前用于小工具）

	QReadWriteLock* m_pMtxSlicePage = nullptr;
	std::map<QUuid, bool> m_mapResNeedToSlicePage; // 素材是否需要切页


	QPixmap m_loadBasePix;
	QPixmap m_loadingPix;
	QPixmap m_loadfailPix;

	bool m_bCurResourceModel = false;
};
