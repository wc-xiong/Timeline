#pragma once
#include "SignalCenterGlobal.h"
#include <QObject>
#include <QUuid>
#include <QRect>
#include <set>

namespace SignalCenter {
	enum EWindowType
	{
		EWT_None = 0,
		EWT_Grid = 1,
		EWT_Property = 2,
		EWT_ModelGrid = 4,
		EWT_SliceWnd = 8,					//输入输出窗口
		EWT_ResourceWnd = 1 << 4,
		EWT_ScreenMgrWnd = 1 << 5,
		EWT_SliceManageWnd = 1 << 6,		//切片管理窗口
		EWT_TimeLineWnd = 1 << 7,
		EWT_Title = 1 << 8,
		EWT_ResProperty = 1 << 9,
		EWT_CueListWnd = 1 << 10,
		EWT_All = EWT_Grid | EWT_Property | EWT_ModelGrid | EWT_SliceWnd |
		EWT_ResourceWnd | EWT_ScreenMgrWnd | EWT_SliceManageWnd | EWT_TimeLineWnd | EWT_Title | EWT_CueListWnd,
	};

	enum EDirtyDataType
	{
		EDDT_None = 0,
		EDDT_ModelNodeSelect = 1,
		EDDT_ModelNodeDataChange = 2,
		EDDT_SliceTreeNodeSelect = 4,
		EDDT_SliceTreeNodeDataChange = 8,
		EDDT_MediaNodeSelect = 16,
		EDDT_MediaNodeDataChange = 32,
		EDDT_ScreenSelect = 1 << 8,
		EDDT_ScreenDataChange = 1 << 9,
		EDDT_ViewDataChange = 1 << 10,
		EDDT_AppOpTypeChange = 1 << 11,
		EDDT_SectionSelect = 1 << 12,
		EDDT_SectionDataChange = 1 << 13,
		EDDT_TagSelect = 1 << 14,
		EDDT_TagDataChange = 1 << 15,
		EDDT_NoteSelect = 1 << 16,
		EDDT_NoteDataChange = 1 << 17,
		EDDT_LayerSelect = 1 << 18,
		EDDT_LayerDataChange = 1 << 19,
		EDDT_TransFxSelect = 1 << 20,
		EDDT_TransFxDataChange = 1 << 21,
		EDDT_All = EDDT_SliceTreeNodeSelect | EDDT_ModelNodeSelect| EDDT_ModelNodeDataChange | EDDT_SliceTreeNodeDataChange |
					 EDDT_ViewDataChange | EDDT_AppOpTypeChange,
	};

	enum EDataType
	{
		EDT_None,//无
		EDT_Select,//从选中管理获取被选中切片ID
		EDT_Param,//从信号参数获取携带的ID
		EDT_All,//从数据层获取所有切片ID
	};

	enum E2DDirtyType
	{
		E2DDT_None = 0,
		E2DDT_UnSelectSlice = 1,
		E2DDT_SelectSlice = 2,
		E2DDT_Slice = E2DDT_UnSelectSlice | E2DDT_SelectSlice,
		E2DDT_Media = 4,
		E2DDT_Screen = 8,
		EDDT_PreviewSlice = 16,
		E2DDT_All = E2DDT_Slice | E2DDT_Media | E2DDT_Screen | EDDT_PreviewSlice,
	};

	enum EViewUpdateType
	{
		EVUT_None,
		EVUT_Input = 1,
		EVUT_Output = 2,
		EVUT_AllView = EVUT_Input | EVUT_Output,
	};

	// 相机录制结束结果（signalCameraRecordFinished）
	enum ECameraRecordFinishResult
	{
		ECRFR_Success = 0,		// 有成片且编码成功
		ECRFR_Failed = 1,		// 准备/编码异常失败
		ECRFR_Cancelled = 2,	// 预热取消等：仅恢复 UI，不提示
	};

	enum EChangeModeRole
	{
		ECMR_Button,
		ECMR_SliceWnd,
		ECMR_3DViewWnd
	};

	enum EModelCreateWndType
	{
		EMCWT_Dialog = 1,
		EMCWT_PrimaryView = 2,
		EMCWT_PropertyView = 4,
	};
}

class SIGNALCENTER_EXPORT CSignalCenter :public QObject
{
	Q_OBJECT
public:
	CSignalCenter() {}
	~CSignalCenter() {}

public:
	virtual bool eventFilter(QObject *watched, QEvent *event);

	bool IsCtrl();
	bool IsShift();
	bool IsAlt();
	bool IsWin();
	bool IsW();
	bool IsE();
	bool IsR();
	bool IsS();

signals:
	void signalRefreshWindow(int eWindowType = SignalCenter::EWT_None, int eOperatorType = SignalCenter::EDDT_None);
	void signalSwitchFocusTo(int eWindowType = SignalCenter::EWT_None);

	void signalSwitchDockWnd(int eWnd);

	void signalLoadDataFromXml();
	void signalResetDataByHistoryStack();

	//模型库
	/** 即将删除模型（DeleteChild 之前）；用于同步清理路径相机时间线片段等 */
	void signalPrepareDeleteModel(const QUuid& id);
	void signalDeleteModelTo3DView(const QUuid& id);
	void signalUpdateModelTo3DView(const QUuid& id);
	void signalAddModelTo3DView(const QUuid& resourceId, const QUuid& modelId = QUuid());
	/** 对象列表：从 ViewDataManager 剪贴板粘贴物体（由 Primary3DView 处理 GPU/数据层） */
	void signalPasteObjectFromClipboard();
	void signalLightChange(const QUuid& id);								// 灯光属性修改
	void signalLightDelete(int modelType, int LightModelIndex);				// 灯光删除
	void signalLightInFolderchange();										// 集合中的灯光属性修改
	void signalEditPresetModel(const QUuid& id);
	void signalAmbientLightUpdate();										//环境光更新

	//相机录制相关
	void signalCameraModelChange();											//相机模型创建/删除/名称修改，发去相机录制窗口更新
	void signalCameraRecordStart(QString& outputFilePath, int width, int height, bool bTimelineMode, bool bApplyColorlightWatermark, bool bApplyTimestamp); 	// 相机视图：准备开始相机录制
	void signalCameraScreenshot(QString& outputFilePath, int width, int height, bool bApplyColorlightWatermark, bool bApplyTimestamp);	// 相机截图
	void signalCameraRecordStop();											// 手动结束/时间线模式达到结束时间时，发送执行停止录制、播放的指令的信号
	void signalCameraRecordFinished(int nResult);							// 相机录制结束（ECameraRecordFinishResult），用于恢复 UI / 提示
	void signalCameraRecordWarmupComplete();								// 正式录制前的 3s 倒计时（预热）结束，供 UI 启动秒表
	void signalCameraTimelineRecordPrepare(qint64 startMs);					// 时间线录制额外信号：Seek 到起点并锁定时间线操作
	void signalCameraTimelineRecordPlay();									// 时间线录制额外信号：时间线模型开始播放
	void signalCameraTimelineRecordRelease();								// 时间线录制额外信号：解锁时间线操作

	//切片视图
	void signalUpdate2DView(int e2DDirtyType = SignalCenter::E2DDT_All, int nUpdateViewType = SignalCenter::EVUT_AllView);

	void signalMediaDirty(bool bSelected);									// 媒体边框绘制更新	
	void signalChangeOperatorMode(int nModeType, SignalCenter::EChangeModeRole role = SignalCenter::ECMR_Button);			// 操作模式切换(媒体/切片)

	

	//素材库
	void signalLocateResNode(const QUuid& id);

	//属性区
	void signalShotrcut();											// 快捷键表
	void signalShotrcutResize();									// 快捷键表位置大小更新

	//时间线
	void signalPlayPause();
	void signalFindMedia(const QUuid&);
	void signalAddNewMediaByView(QUuid resourceId, QRect rcScreen);
	void signalSectionSelectChanged(QUuid& ids);
	void signalTagSelectChanged(QUuid& ids);
	void signalTransFxSelectChanged(QUuid& ids);
	void signalNoteSelectChanged(QUuid& ids);
	void signalPathCameraClipSelectChanged(QUuid& ids);
	void signalLayerSelectChanged(QUuid& ids);
	void signalClearTimeLienWidgets();								// 清除时间线上的控件对象
	void signalTimeLineFpsChange(float fFps);						// 时间线帧率变更
	void signalNoniusTimeChanged(qint64 timelineMs);									// 时间线游标位置变更
	void signalSeekTimelineAt(qint64 timelineMs);										// 请求时间线 Seek（同步游标与播放器）
	void signalFinishSeekTimeline(qint64 timelineMs);									// Seek 出帧收尾
	void signalLocateNonius();											// 定位时间线游标
	//切片列表
	void signalDeleteSliceTree(QUuid& id);

	//更新相机可观察对象列表
	void signalUpdateCameraObservableObjs();

	void signalBindCameraViewChange(const QUuid& id);
	void signalBindLightChange(const QUuid& id);


	void signalSetOrSwitchMainCamera();
	void signalWakeUpCameraView();

	void signalModelChangedForCameraView();

	void signalRefreshMainCameraUI(bool bShowCameraView = true);

	void signalStopTimeLindPause();

	//vr相机
	void signalResetBaseCamera();
	void signalSwitchBaseCameraInSequence();

	// 屏幕相关
	void signalRefreshScreenItem(bool bMask = false);
	void signalAudioOutputDevicesChanged(const QUuid& machineId);

	void signalScreenAdd(const QUuid& screenId);
	void signalScreenDelete(const QUuid& screenId);
	void signalScreenUpdate(const QUuid&);
	void signalScreenUpdateConnTestChartFile(const QUuid&);

	void signalFindScreen(const QUuid&);
	void signalUpdateScreenOpenState(std::vector<QUuid> vecIds, bool bOpen);

	//素材属性窗
	void signalUpdatePropertyShowType();
	void signalShowResPropertyWnd(bool bShow);

	void signalTestChartResolutionChange(const QUuid& mediaid);
	void signalUndoRedoRefreshResourceWnd();
	void signalScrollEnable(bool bEnable);
	void signalUpdateMediaProperty(const QUuid& mediaid);
	// ClipFx Plot 拖拽/选中后同步属性面板（nPropId=EMediaPropertyId，clipLocalMs<0 用播放头时刻）
	void signalSyncClipFxActiveProperty(const QUuid& mediaId, int nPropId, qint64 clipLocalMs);
	// 属性面板编辑滑条/SpinBox 后通知 ClipFx 刷新 Plot（游标处更新或新增关键帧）
	void signalClipFxPropertyKeyframeChanged(const QUuid& mediaId, int nPropId, qint64 clipLocalMs);
	void signalBtnRadioChanged(int nDockType, int nIndex);
	void signalWheelRadioChanged(int nDockType, int nIndex);

	void signalFocusModel(const QUuid& id);

	void signalRefreshModelEditDialog(int eWndType);
};
