#pragma once

#include "ResourceWndGlobal.h"
#include "UICommon/DockWidgetEx.h"

struct SResultData;
class CResContentWidget;

class RESOURCEWND_EXPORT CResourceWnd : public CDockWidgetEx
{
    Q_OBJECT
public:
    CResourceWnd(QWidget *parent, QWidget* pMainWnd);
	virtual ~CResourceWnd();

	void InitWndAfterLoadPrj(bool bNewPrj) override;

	void InitResourceWndAfterLoadPrj();

	void CreateConnects();

	void InitResWndAndModelBeforeLoadPrj();

protected:
	void UpdateTranslateText() override;
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

protected slots:
	void slotRefreshWindow(int eWindowType, int eOperatorType);

protected:
	CResContentWidget* m_pContentWidget;
};
