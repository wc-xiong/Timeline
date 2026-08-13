#pragma once
#include <QTreeWidget>
#include "UICommon/PushButtonEx.h"
#include "UICommon/BaseWidget.h"
#include "UICommon/ComboBoxEx.h"
#include "UICommon/RenameTreeView.h"

#include <QStyledItemDelegate>

class CResourceTreeList;
class CResourceProxyModel;
class QLabel;
struct SResultData;
class CSvgImageButton;
class QLineEdit;
class CSearchLineEdit;

class CommonDelegate : public CRenameDelegate
{
	Q_OBJECT
public:
	CommonDelegate(QObject *parent = nullptr) :CRenameDelegate(parent){

	}
private:
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;	
};


class CResContentWidget: public CBaseWidget
{
	Q_OBJECT

public:
	CResContentWidget(QWidget* pParent, QWidget* pMainWnd);
	virtual ~CResContentWidget();

	void InitResourceWndAfterLoadPrj();

	void InitResWndAndModelBeforeLoadPrj();
public:
	QSize sizeHint() const;

public slots:
	//ComboBox选中发生变化
	void slotComboxSelectIndexChange(int nIndex);

	//搜索编辑框关键字发生变化
	void slotSearchEditTextChanged(const QString &strText);

	void slotResourceSelectChange(int nResCount, int nResAndFileCount);

	void slotResourceCountChange();

	void slotMediaResLocate(const QUuid &resId);

	void slotFilterBtnClicked();

	void slotFilterActionTriggered(QAction*);
signals:
	void signalMutedByAudioCapture(bool);

	void signalPrjDemuxerFinish();

	void signalPictureTrans(const QUuid& id);

	void signalResDemuxerProgress(int nValue);

	//通过小工具浮动面板创建媒体
	void signalToolsWndCreatMedia(int type);
	//通过小工具浮动面板添加媒体
	void signalToolsWndAddMedia(const QUuid &resId);

	void signalResFileLostRecoveryState(const QUuid &resId, bool bValue);

	//更新幻彩字素材属性窗
	void signalUpdateColorTextResWnd(const QVector<QString>& vecPath);

	//素材更新，通知媒体更新
	void signalUpdateResToMedia(const QUuid& resId);

	// Timeline：选中视频/音频预览
	void signalPreviewMedia(const QString& path);

protected:

	// 初始化，创建控件
	void    InitWidget();

	// 连接信号槽
	void    CreateConnects();

	void	UpdateTranslateText() override;

	int GetCurFilterFileType();

	void	CreateFilterMenu();
public:
	CResourceTreeList* m_pTreelistWidget = nullptr;	//素材库列表
	
	QPushButton* m_pSelectFilterBtn = nullptr;		//筛选类型ComboBox
	int		m_nSelectedCurIndex = 0;

	CSearchLineEdit* m_pSearchEdit = nullptr;	//关键字文本框
	CSvgImageButton* m_pAddFileBtn = nullptr;	//添加素材按钮
	CSvgImageButton* m_pDelFileBtn = nullptr;	//删除素材按钮
	QLabel* m_pLabelSelectRowCount = nullptr;	// 素材库当前选中素材个数
	QLabel* m_pLabelResourceCount = nullptr;	// 素材库当前素材总数

	CResourceProxyModel* m_pFilterModel = nullptr;	//素材库代理模型

	QWidget* m_pMainWnd = nullptr;

};

