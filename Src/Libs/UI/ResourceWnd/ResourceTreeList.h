#pragma once

#include <QWidget>
#include <QTreeView>
#include <QUuid>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include "SubtitleModeSelectDlg.h"
#include "UICommon/RenameTreeView.h"

class IResourceBase;
class CResourceProxyModel;
class QMenu;
class QAction;
class QUndoCommand;
class CResourceTreeListPrivate;
struct SExMsgData;
class IMediaStream;
struct SMediaInfo;
class IResourceBase;
class CResourceNodeInfo;
class CSuspendedScrollBar;
struct SResFileDemuxInfo;
class CResViewTranslate;

class CResourceTreeList : public CRenameTreeList
{
    Q_OBJECT

public:
    CResourceTreeList(CResourceProxyModel* pFilerModel, QWidget *parent, QWidget* pMainWnd);
	virtual ~CResourceTreeList();

public:
	//删除选中素材，返回当前未删除素材
	std::list<QUuid> DelResources(bool isSelect, QModelIndexList delList, uint sender = 0);

	// 加载文件
	void InitResListAfterLoadPrj();

	// 切换工程
	void InitResWndAndModelBeforeLoadPrj();

	void UpdateTranslateText();

	void MediaResLocate(const QUuid &resId);

public slots:

	//按名称排序
	void slotHeadsort(int logicalIndex, Qt::SortOrder order);

	//通过素材库按钮添加新素材
	void slotAddNewResource();

	//通过素材库按钮删除选中素材
	void slotDelSelectResource();

	//素材库文件夹筛选器展开
	void slotExpanded(const QModelIndex &index);

	//素材库文件夹筛选器收起
	void slotCollapsed(const QModelIndex &index);

public slots:
	void slotVerticalValueChange(int nValue);
	void slotHorizontalValueChange(int nValue);

signals:
	void signalResourceSelectChange(int nResCount, int nResAndFileCount);

	void signalResourceCountChange();

	void signalMutedByAudioCapture(bool);

	void signalPrjDemuxerFinish();

	void signalUpdateResourceNode(const QUuid id);

	void signalDemuxerFinish();

	void signalPictureTrans(const QUuid& id);

	void signalDemuxer(const QUuid& uuid, const QString& strPath, int type = -1,const QString & strName = "");

	void signalResDemuxerProgress(int nValue);

	void signalResFileLostRecoveryState(const QUuid &resId, bool bValue);

	// Timeline：选中视频/音频预览
	void signalPreviewMedia(const QString& path);

	//更新幻彩字素材属性窗
	void signalUpdateColorTextResWnd(const QVector<QString>& vecPath);

	//更新动图
	void signalUpdateResThumbnail();

	//更新素材-通知舞台区媒体更新(当前仅网页窗口模式用到)
	void signalUpdateResToMedia(const QUuid& resId);

public:
	//开始拖拽，设置鼠标所带图片，以及Drag所带数据
	void startDrag(Qt::DropActions supportedActions) override;

	void mousePressEvent(QMouseEvent* event) override;

	void mouseReleaseEvent(QMouseEvent* event) override;
	
	void mouseMoveEvent(QMouseEvent* event) override;

	void dragMoveEvent(QDragMoveEvent *event) override;

	void dragLeaveEvent(QDragLeaveEvent* event) override;

	void dropEvent(QDropEvent *event) override;
	
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

	void setModel(QAbstractItemModel *model) override;

	void keyPressEvent(QKeyEvent *event) override;

	int countTreeItems(const QModelIndex &parentIndex, QAbstractItemModel *model);
	void paintEvent(QPaintEvent *event) override;

	void resizeEvent(QResizeEvent* event) override;

	void enterEvent(QEvent* event) override;

	void leaveEvent(QEvent* event) override;

public slots:
	void slotResetModelChange();

protected slots:
	void slotUpdateResourceNode(const QUuid id);

	void slotDemuxerFinish();

	//解复用模块回调接口
	void slotDemuxerDataFinish(const std::shared_ptr<SResFileDemuxInfo>& fileInfo);

	void slotCheckResDemuxerTimeOut();
protected:
	//初始化界面设置
	void InitWidget();	

	//创建信号槽连接
	void CreateConnects();	

	//在排序，拖拽后更新原文件展开状态
	void UpdateViewExpandState();

	// 添加素材
	void AddNeedDemuxerResource(const QStringList &listPath, int nSender = 0);

	std::shared_ptr<IResourceBase> AddToolRes(int nType, bool bButtonClicked, uint sender = 0);
	
	//添加字幕 或提词器
	bool AddSubtitleOrTeleprompter(const QUuid &resId, bool bButtonClicked);
	
	void AddDlgSettingRes(const QUuid& resId, int nType, bool bButtonClicked);

	//添加文件夹（真实路径文件夹）
	void AddRealFolder(const QString &strPath, CResourceNodeInfo* parentNode, int FolderDeep);
	
	// 添加素材，记录堆栈
	void AddNotDemuxerResByUndo(std::shared_ptr<IResourceBase> pNode, const QUuid &parentId);

	void DealResourceOperator(QPoint pos, int nSelectRowCount, const QUuid &resId, bool bButtonClicked = false);	// resId为当前选中素材Id，未有选中则为空

	void DemuxerAllResource(bool bLoadPrj);

	// 更新当前解复用进度（解复进度+1）
	void UpdateDemuxerProgress(const QUuid &resId, bool bDemuxered, bool bDemuxerSuccess);

	// 线程更新素材解复用情况
	void TrdUpdateResDemuxerState();

	//获取当前项在素材库的行号
	int GetCurItemRow(const QModelIndex& parentIndex,bool& bIsFind);

	//更新幻彩字素材属性窗
	void UpdateColorTextResWnd(std::shared_ptr<IResourceBase> pNode);

	void UpdateSuspendedScrollBarVisible();

private:
	CResourceTreeListPrivate* d_ptr;

	CSuspendedScrollBar *m_pVerticalScrollBar = nullptr;
	CSuspendedScrollBar *m_pHorizontalScrollBar = nullptr;
};
