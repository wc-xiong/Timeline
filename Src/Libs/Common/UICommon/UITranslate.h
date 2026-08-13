#pragma once

#include <QObject>
#include <QString>

class CCommonTranslate : public QObject
{
	Q_OBJECT
public:
	static QString Brightness() { return QStringLiteral("亮度"); }
	static QString Red() { return QStringLiteral("红色"); }
	static QString Green() { return QStringLiteral("绿色"); }
	static QString Blue() { return QStringLiteral("蓝色"); }
	static QString Transparent() { return QStringLiteral("不透明度"); }
	static QString Contrast() { return QStringLiteral("对比度"); }
	static QString Hue() { return QStringLiteral("色相"); }
	static QString Saturation() { return QStringLiteral("饱和度"); }
	static QString GlobalVolume() { return QStringLiteral("全局音量"); }

	static QString ColorPicker() { return QStringLiteral("拾色器"); }
	static QString SoftwareRunning() { return QStringLiteral("软件正在运行"); }
	static QString Lastsessionnotsaved() { return QStringLiteral("检测到上次使用时未正常保存工程，是否恢复上次的工程？"); }

	static QString PropertyModification() { return QStringLiteral("属性修改"); }
	static QString PleaseWait() { return QStringLiteral("请等待"); }
	static QString Loading() { return QStringLiteral("加载中..."); }
	static QString Input() { return QStringLiteral("输入"); }
	static QString Output() { return QStringLiteral("输出"); }
	static QString EmDashPlaceholder() { return QStringLiteral("—"); }

	static QString Name() { return QStringLiteral("名称"); }
	static QString Type() { return QStringLiteral("类型"); }
	static QString Path() { return QStringLiteral("路径"); }
	static QString Size() { return QStringLiteral("大小"); }
	static QString Duration() { return QStringLiteral("时长"); }
	static QString Resolution() { return QStringLiteral("分辨率"); }
	static QString AudioChannels() { return QStringLiteral("声道"); }

	static QString Rename() { return QStringLiteral("重命名"); }
	static QString Unnamed() { return (QStringLiteral("未命名")); }
	static QString RenameEmpty() { return QStringLiteral("输入名称不能为空!"); }

	static QString Device() { return QStringLiteral("设备"); }
	static QString Screen() { return QStringLiteral("屏幕"); }

	static QString W() { return QStringLiteral("宽"); }
	static QString H() { return QStringLiteral("高"); }
	static QString Row() { return QStringLiteral("行"); }
	static QString Column() { return QStringLiteral("列"); }
	static QString Cancel() { return QStringLiteral("取消"); }
	static QString Width() { return QStringLiteral("宽"); }
	static QString Height() { return QStringLiteral("高"); }
	static QString Position() { return QStringLiteral("位置"); }

	static QString OK() { return QStringLiteral("确定"); }
	static QString Apply() { return QStringLiteral("应用"); }
	static QString ApplySuccess() { return QStringLiteral("应用成功"); }
	static QString Click() { return QStringLiteral("点击"); }
	static QString Reset() { return QStringLiteral("重置"); }
	static QString ClearAll() { return QStringLiteral("清除所有"); }
	static QString LeftMouseButton() { return QStringLiteral("鼠标左键"); }
	static QString RightMouseButton() { return QStringLiteral("鼠标右键"); }

	static QString Redo() { return QStringLiteral("恢复"); }
	static QString Undo() { return QStringLiteral("撤销"); }

	static QString Drag() { return QStringLiteral("拖动"); }
	static QString MoveUp() { return QStringLiteral("上移"); }
	static QString MoveDown() { return QStringLiteral("下移"); }
	static QString RootFolder() { return QStringLiteral("根目录"); }
	static QString HideWidget() { return QStringLiteral("隐藏窗口"); }
	static QString ShowWidget() { return QStringLiteral("显示窗口"); }

	static QString Cut() { return QStringLiteral("分割"); }
	static QString Merge() { return QStringLiteral("合并"); }

	static QString Add() { return QStringLiteral("添加"); }
	static QString Edit() { return QStringLiteral("编辑"); }
	static QString Import() { return QStringLiteral("导入"); }
	static QString Export() { return QStringLiteral("导出"); }
	static QString Delete() { return QStringLiteral("删除"); }
	static QString Replace() { return QStringLiteral("替换"); }
	static QString Copy() { return QStringLiteral("复制"); }
	static QString Paste() { return QStringLiteral("粘贴"); }
	static QString Copypaste() { return QStringLiteral("重复"); }

	static QString About() { return (QStringLiteral("关于")); }
	static QString ColorLightName() { return (QStringLiteral("卡莱特云科技股份有限公司")); }
	static QString ColorlightWebSite() { return (QStringLiteral("https://colorlightinside.com/?language=cn")); }

	static QString MessageBoxTip() { return QStringLiteral("提示"); }
	static QString DontShowAgain() { return QStringLiteral("不再提示"); }
	static QString GetMonthText(int nMonth)
	{
		switch (nMonth)
		{
		case 1:
			return QStringLiteral("一月");
		case 2:
			return QStringLiteral("二月");
		case 3:
			return QStringLiteral("三月");
		case 4:
			return QStringLiteral("四月");
		case 5:
			return QStringLiteral("五月");
		case 6:
			return QStringLiteral("六月");
		case 7:
			return QStringLiteral("七月");
		case 8:
			return QStringLiteral("八月");
		case 9:
			return QStringLiteral("九月");
		case 10:
			return QStringLiteral("十月");
		case 11:
			return QStringLiteral("十一月");
		case 12:
			return QStringLiteral("十二月");
		default:
			break;
		}
		return "";
	}

	//计算器
	static QString Calculator() { return QStringLiteral("计算器"); }
	static QString errorTip() { return QStringLiteral("除数不能为0"); }
	static QString UseTip() { return QStringLiteral("输入“=”号开启计算器/按下“Enter”键应用"); }
	static QString Retry() { return QStringLiteral("重试"); }

	//导出数据
	static QString NoDonle() { return QStringLiteral("当前设备未检测到加密狗"); }
	static QString DonleState() { return QStringLiteral("状态"); }
	static QString DonleSerial() { return QStringLiteral("加密锁序列号"); }
	static QString DonleNoRight() { return QStringLiteral("加密狗无GrandMapping权限"); }
	static QString MappingDataName() { return QStringLiteral("数据包名称"); }
	static QString DonleStateNormal() { return QStringLiteral("加密狗权限正常"); }
	static QString ExportMappingData() { return QStringLiteral("导出Mapping数据"); }
	static QString ExportMappingDataError() { return QStringLiteral("Mapping数据导出失败，请检查后重新导出"); }
	static QString ExportMappingDataSuccess() { return QStringLiteral("Mapping数据导出成功，请导入EE进行后续操作"); }
	static QString ExportMappingDongleScope() { return QStringLiteral("加密锁"); }
	static QString ExportMappingLocalDongleOnly() { return QStringLiteral("当前加密锁"); }
	static QString ExportMappingAllDongles() { return QStringLiteral("所有加密锁"); }
	static QString ExportMappingNoDongleDetected() { return QStringLiteral("未检测到加密锁"); }

	static QString PleaseChoose() { return QStringLiteral("请选择"); }
	static QString Connnected() { return QStringLiteral("已连接"); }
	static QString Unconnnected() { return QStringLiteral("未连接"); }
	static QString Refresh() { return QStringLiteral("刷新"); }

	static QString VideoFrameRate() { return QStringLiteral("帧率"); }
	static QString VideoFormat() { return QStringLiteral("视频格式"); }
	static QString VideoBitRate() { return QStringLiteral("码率"); }
	static QString VideoHardwareDecode() { return QStringLiteral("解码类型: 硬解码"); }
	static QString VideoSoftwareDecode() { return QStringLiteral("解码类型: 软解码"); }
	static QString AudioFormat() { return QStringLiteral("音频格式"); }
	static QString AudioSampleRate() { return QStringLiteral("采样率"); }

	static QString ColorText() { return QStringLiteral("幻彩字"); }
	static QString Video() { return QStringLiteral("视频"); }
	static QString Audio() { return QStringLiteral("音频"); }
	static QString Picture() { return QStringLiteral("图片"); }
	static QString All() { return QStringLiteral("全部"); }
	static QString Folder() { return QStringLiteral("素材组"); }
	static QString CaptureCard() { return QStringLiteral("采集卡"); }
	static QString Text() { return QStringLiteral("文本"); }
	static QString TestChart() { return QStringLiteral("测试图"); }
	static QString Subtitle() { return QStringLiteral("字幕"); }
	static QString Teleprompter() { return QStringLiteral("提词器"); }
	static QString WebPage() { return QStringLiteral("网页"); }

	static QString Cache() { return QStringLiteral("缓存"); }
	static QString Preview() { return QStringLiteral("预览"); }
	static QString Configure() { return QStringLiteral("配置"); }
	static QString Used() { return QStringLiteral("已使用"); }
	static QString NotUsed() { return QStringLiteral("未使用"); }
	static QString ResName() { return QStringLiteral("资源名称"); }
	static QString Format() { return QStringLiteral("格式"); }
	static QString Default() { return QStringLiteral("默认"); }

	static QString Volume() { return QStringLiteral("音量"); }

	static QString Solo() {return  QStringLiteral("独立显示");}
	static QString CancelSolo() { return  QStringLiteral("取消独立显示"); }
	static QString SoloTip() { return QStringLiteral("仅显示选中"); }

	//幻彩风格
	static QStringList ShowStyle()
	{
		return QStringList() << QStringLiteral("一般风格") << QStringLiteral("幻彩风格");
	}
	static QString ChangeType() { return QStringLiteral("切换显示风格"); }

	static QString Vertical() { return QStringLiteral("竖"); }
	static QString Horizontal() { return QStringLiteral("横"); }
	static QString Pixel() { return QStringLiteral("像素"); }
};

class CTitleTranslate : public QObject
{
	Q_OBJECT
public:
	static QString File() { return QStringLiteral("文件"); }
	static QString FileMenu() { return QStringLiteral("文件菜单"); }
	static QString WidgetManage() { return QStringLiteral("窗口"); }
	static QString Setting() { return QStringLiteral("设置"); }
	static QString Help() { return QStringLiteral("帮助"); }
	static QString Redo() { return QStringLiteral("恢复(Ctrl+Y)"); }
	static QString Undo() { return QStringLiteral("撤销(Ctrl+Z)"); }
	static QString Open() { return QStringLiteral("打开"); }
	static QString Save() { return QStringLiteral("保存"); }
	static QString LockScreen() { return QStringLiteral("锁定"); }

	static QString ProjectSetting() { return QStringLiteral("工程设置"); }
	static QString SoftwareSetting() { return QStringLiteral("软件设置"); }

	static QString NewProject() { return QStringLiteral("新建工程"); }
	static QString OpenProject() { return QStringLiteral("打开工程"); }
	static QString SaveProject() { return QStringLiteral("保存工程"); }
	static QString ReNameProject() { return QStringLiteral("重命名项目"); }
	static QString Language() { return QStringLiteral("语言(Language)"); }
	static QString English() { return QStringLiteral("英文(English)"); }
	static QString Chinese() { return QStringLiteral("中文(Chinese)"); }
	static QString RecentProject() { return QStringLiteral("最近工程"); }
	static QString SaveProjectAs() { return QStringLiteral("另存为..."); }
	static QString ResetLayout() { return QStringLiteral("重置为默认布局"); }
	static QString PackProject() { return QStringLiteral("打包"); };
	static QString Log() { return QStringLiteral("日志"); }
	static QString UserGuide() { return QStringLiteral("使用手册"); }
	//
	static QString ShortCutList() { return QStringLiteral("快捷键列表"); }
	static QString About() { return QStringLiteral("关于"); }
	static QString SoftWidgetMax() { return QStringLiteral("软件窗口最大化"); }
	static QString SoftWidgetMin() { return QStringLiteral("软件窗口最小化"); }
	static QString SoftWidgetReset() { return QStringLiteral("软件窗口还原"); }
	static QString SoftWidgetClose() { return QStringLiteral("软件关闭"); }
	static QString MappingMap() {return QStringLiteral("Mapping映射");}

	static QString History() { return QStringLiteral("历史记录"); }
	static QString Apply() { return QStringLiteral("按下“Enter”键应用"); }

	static QString PreviewStage() { return  QStringLiteral("阶段—预览"); }
	static QString DebugStage() { return  QStringLiteral("阶段—调试"); }
	static QString PlayControlStage() { return  QStringLiteral("阶段—播控"); }
	static QString SaveWndLayout() { return  QStringLiteral("保存当前布局"); }
	static QString ResetWndLayoutMessageBoxTip() { return  QStringLiteral("继续操作将会重置所有界面并恢复默认，是否继续？"); }
	static QString ResetWndLayoutMessageBoxTitle() { return  QStringLiteral("重置为默认布局"); }
	static QString SaveWndLayoutMessageBoxTitle() { return  QStringLiteral("保存当前阶段"); }
	static QString SavePreviewWndLayoutMessageBoxTip() { return  QStringLiteral("继续操作将会替换原【阶段—预览】界面，是否继续？"); }
	static QString SaveDebugWndLayoutMessageBoxTip() { return  QStringLiteral("继续操作将会替换原【阶段—调试】界面，是否继续？"); }
	static QString SavePlayCtrolWndLayoutMessageBoxTip() { return  QStringLiteral("继续操作将会替换原【阶段—播控】界面，是否继续？"); }
	static QString DebugSnapshotTip() { return  QStringLiteral("已退出调试，预览快照已保留"); }
	static QString WndLayoutResetSuc() { return  QStringLiteral("界面重置成功"); }
	static QString WndLayoutSaveSuc() { return  QStringLiteral("界面保存成功"); }

	static QString SaveDebugSliceTip() { return  QStringLiteral("该操作会影响当前现场调试数据。请选择保存方式"); }
	static QString SaveDebugSlice() { return  QStringLiteral("保存为新工程"); }
	static QString NoSaveDebugSlice() { return  QStringLiteral("不保存"); }
	static QString SaveDebugSliceDesc() { return  QStringLiteral("（含输入/输出切片与算法设置/引用信息）"); }
	static QString NoSaveDebugSliceDesc() { return  QStringLiteral("（直接回到预览，清除调试切片）"); }

	static QString Preview(){return QStringLiteral("预览");}
	static QString Debug(){return QStringLiteral("调试");}
	static QString PlayControl(){return QStringLiteral("播控");}

	static QString PreviewToDebugNoModelTip() { return QStringLiteral("暂无屏幕模型，无法进入调试阶段，请先创建屏幕模型！"); }
	static QString PreviewToDebugNoSlice() { return QStringLiteral("缺少输入预览切片，请在「模型属性 - 切片」中生成后重试！"); }
	static QString PreviewToDebugTip() { return QStringLiteral("输入 / 输出预览切片已锁定，现场调试方案已生成！"); }
	static QString DebugToPlayControlTip() { return QStringLiteral("当前现场调试切片已作为播控底图，切片与模型已设为只读，仅允许时间线与媒体播控操作！"); }
	static QString PlayControlToDebugTip() { return QStringLiteral("已退出播控，时间线已暂停，调试切片编辑已恢复。"); }

	static QString PreviewToPlayControlTip() { return QStringLiteral("预览阶段需完成调试，方可进入播控"); }
	static QString PlayControlToPreviewTip() { return QStringLiteral("播控阶段需完成调试，方可进入预览"); }
};

class CSliceViewTranslate : public QObject
{
	Q_OBJECT
public:
	static QString InputView() { return QStringLiteral("输入视图"); }
	static QString OutputView() { return QStringLiteral("输出视图"); }
	static QString LocateFile() { return QStringLiteral("定位素材"); }
	static QString Slice() { return QStringLiteral("切片"); }
	static QString Slices() { return QStringLiteral("多个切片"); }
	static QString PreviewUnit() { return QStringLiteral("预览单元"); }


	//QuadrilateralSlicingOptimization-QSOptimization
	static QString QSOptimization() { return QStringLiteral("四边形切片优化"); }
	static QStringList OptimizationList()
	{
		return  QStringList() << QStringLiteral("无优化")
			<< QStringLiteral("透视模式")
			<< QStringLiteral("拉伸模式");
	}

	static QString SyncToInput() { return QStringLiteral("同步至输入"); }
	static QString SyncToOutput() { return QStringLiteral("同步至输出"); }

	static QString StageOffset() { return QStringLiteral("舞台偏移量"); }

	static QString MediaMode() { return QStringLiteral("媒体模式"); }
	static QString ScreenMode() { return QStringLiteral("屏幕模式"); }
	static QString SliceMode() { return QStringLiteral("切片模式"); }

	static QString Mesh() { return QStringLiteral("网格"); }
	static QString VertexEdit() { return QStringLiteral("顶点编辑"); }

	static QString VertexEditAdsorption() { return QStringLiteral("吸附"); }
	static QString VertexEditLinkage() { return QStringLiteral("联动"); }

	static QString Combination() { return QStringLiteral("组合"); }
	static QString UndoCombination() { return QStringLiteral("取消组合"); }
	static QString Demarcate() { return QStringLiteral("标定"); }
	static QString CancelDemarcate() { return QStringLiteral("取消标定"); }
	static QString Flip() { return QStringLiteral("翻转"); }
	static QString FlipUp() { return QStringLiteral("向上翻折"); }
	static QString FlipDown() { return QStringLiteral("向下翻折"); }
	static QString FlipLeft() { return QStringLiteral("向左翻折"); }
	static QString FlipRight() { return QStringLiteral("向右翻折"); }

	static QString FlipVer() { return QStringLiteral("垂直翻转"); }
	static QString FlipHor() { return QStringLiteral("水平翻转"); }

	static QString CutMerge() { return  QStringLiteral("分割"); }
	static QString SliceCut() { return QStringLiteral("分割"); }
	static QString SliceMerge() { return QStringLiteral("合并"); }

	static QString Align() { return QStringLiteral("对齐"); }
	static QString AlignLeft() { return QStringLiteral("左对齐"); }
	static QString AlignHorizontalCenter() { return QStringLiteral("水平居中对齐"); }
	static QString AlignRight() { return QStringLiteral("右对齐"); }
	static QString AlignTop() { return QStringLiteral("顶部对齐"); }
	static QString AlignVerticalCenter() { return QStringLiteral("垂直居中对齐"); }
	static QString AlignBottom() { return QStringLiteral("底部对齐"); }
	static QString HorizontalDistribution() { return QStringLiteral("水平分布"); }
	static QString VerticalDistribution() { return QStringLiteral("垂直分布"); }

	static QString Grid_InputGrid() { return QStringLiteral("输入网格"); }
	static QString Grid_OutputGrid() { return QStringLiteral("输出网格"); }

	static QString Grid_Show() { return QStringLiteral("显示网格"); }
	static QString Grid_Hor_Line() { return QStringLiteral("横线"); }
	static QString Grid_Ver_Line() { return QStringLiteral("竖线"); }
	static QString Grid_Right_Line() { return QStringLiteral("右斜线"); }
	static QString Grid_Left_Line() { return QStringLiteral("左斜线"); }
	static QString Grid_Interval() { return QStringLiteral("间距"); }
	static QString Grid_LineWidth() { return QStringLiteral("线宽"); }
	static QString Grid_Slider_Name() { return QStringLiteral("网格移动速度"); }
	static QString Grid_Slider_Slow() { return QStringLiteral("慢"); }
	static QString Grid_Slider_Fast() { return QStringLiteral("快"); }
	static QString Grid_Btn_Pause() { return QStringLiteral("暂停"); }
	static QString Grid_Btn_Start() { return QStringLiteral("继续"); }
	static QString Grid_Move_Tip1() { return QStringLiteral("键盘"); }
	static QString Grid_Move_Tip2() { return QStringLiteral("移动网格"); }

	static QString MoveUp() { return QStringLiteral("上移"); }
	static QString MoveDown() { return QStringLiteral("下移"); }
	static QString MoveLeft() { return QStringLiteral("左移"); }
	static QString MoveRight() { return QStringLiteral("右移"); }
	static QString Hierarchy() { return QStringLiteral("层级"); }
	static QString BringToFront() { return QStringLiteral("置顶"); }
	static QString SendToBack() { return QStringLiteral("置底"); }

	static QString Rotate() { return QStringLiteral("旋转"); }
	static QString Roam() { return QStringLiteral("漫游"); }
	static QString Zoom() { return QStringLiteral("缩放"); }
	static QString Reset() { return QStringLiteral("复位"); }

	static QString AccordingRowCol() { return QStringLiteral("按行列"); }
	static QString Cols() { return QStringLiteral("列数"); }
	static QString Rows() { return QStringLiteral("行数"); }
	static QString AccordingWidthHeight() { return QStringLiteral("按宽高"); }
	static QString Width() { return QStringLiteral("宽度"); }
	static QString Height() { return QStringLiteral("高度"); }
	static QString ScreenConnRelation() { return QStringLiteral("屏幕连接关系"); }
	static QString AccordingScreenConnRelation() { return QStringLiteral("按所在区域的屏幕连接关系进行分割"); }
	static QString NoConnRelationInThisArea() { return QStringLiteral("当前区域未存在连接关系"); }
	static QString ManualCut() { return QStringLiteral("手动分割"); }
	static QString AutoCut() { return QStringLiteral("自动分割"); }
	static QString CanceCut() { return QStringLiteral("取消分割"); }

	static QString MultiSliceCutTips() { return QStringLiteral("多选时，分割效果均对每个切片独立进行"); }
	static QString AccordingSelfPoints() { return QStringLiteral("按多边形点分割"); }
	static QString AccordingSelfPointsTips() { return QStringLiteral("按切片自身点水平竖直分割"); }
	static QString SelSliceCountTips() { return QStringLiteral("已选中%1个切片"); }

	static QString SliceForbidCut() { return QStringLiteral("切片宽或高小于1像素，不支持分割"); }
	static QString SliceCutResultTips() { return QStringLiteral("已分割切片:%1个，已跳过:%2个"); }
	static QString SliceSplitFailTips() { return QStringLiteral("切片分割失败！"); }

	static QString PreviewStage() { return QStringLiteral("预览"); }
	static QString DebugStage() { return QStringLiteral("调试"); }
	static QString PlayControlStage() { return QStringLiteral("播控"); }

	static QString SliceOptimizeGroup() { return QStringLiteral("优化切片"); }
	static QString OptimizeGroupName() { return QStringLiteral("优化组%1"); }
	static QString ModelSlice() { return QStringLiteral("模型切片"); }
	static QString CustomGroup() { return QStringLiteral("切片"); }

	static QString AddNewSlice() { return QStringLiteral("新增切片"); }
	static QString SliceNonDeleteTip() { return QStringLiteral("不可删除"); }
	static QString SliceModelNormalFlip() { return QStringLiteral("选区面翻转"); }
	static QString SliceModelNormalFlipTooltip() { return QStringLiteral("翻转选中区域所对应的模型表面"); }
	static QString SliceOptimizeMenu() { return QStringLiteral("切片优化"); }
	static QString OptimizeSelectedItems() { return QStringLiteral("优化选中项"); }
	static QString OptimizeUnselectedItems() { return QStringLiteral("优化未选项"); }
	static QString SliceOptimizeMenuTooltip() { return QStringLiteral("优化后切片隐藏，不可编辑"); }
	static QString SliceOptimizeAction() { return QStringLiteral("切片优化"); }
	static QString CancelSliceOptimize() { return QStringLiteral("取消优化"); }
	static QString PixelMappingShowHide() { return QStringLiteral("像素映射（显示/隐藏）"); }
	static QString ShowAllHiddenItems() { return QStringLiteral("显示全部隐藏项"); }
	static QString ShowLastHiddenItems() { return QStringLiteral("显示上一步隐藏项"); }
	static QString HideSelectedItems() { return QStringLiteral("隐藏选中项"); }
	static QString HideUnselectedItems() { return QStringLiteral("隐藏未选中项"); }
	static QString ShowHiddenOverLimitTip() { return QStringLiteral("当前隐藏切片数量过多，一次性全部显示可能导致程序卡顿或异常，请先对切片取消分割后再操作。"); }

	static QString SliceMultiEditTip() { return QStringLiteral("切片数量超出限制（100）"); }
	static QString CancelSplitContainsPixmapSliceTip() { return QStringLiteral("取消分割的切片中涉及切片优化的切片，是否仍然取消？"); }
	static QString DeleteSummaryTip() { return QStringLiteral("已删除%1/%2个切片，%3个模型切片和%4个切片优化组切片无法删除！"); }
	static QString DeleteOnlyTip() { return QStringLiteral("已删除 %1 个切片。"); }
	static QString DeletePartialModelSkipTip() { return QStringLiteral("已删除 %1/%2 个切片，%3 个模型切片无法删除！"); }
	static QString DeleteForbiddenInOptimizeOrModelTip() { return QStringLiteral("模型切片/切片优化组切片无法删除！"); }
};

class CCueListTranslate
{
public:
	static QString CueList() { return QStringLiteral("Cue列表"); }
	static QString Note() { return QStringLiteral("注释"); }
	static QString Tag() { return QStringLiteral("时间码标签"); }
	static QString Pos() { return QStringLiteral("位置"); }
	static QString Remain() { return QStringLiteral("剩余时间"); }
	static QString TimeCodeInvalid() { return QStringLiteral("时间码无效"); }
};

class CStageTranslate : public QObject
{
	Q_OBJECT
public:
	static QString ReturnToOri() { return QStringLiteral("回到原点"); }
	static QString CD_PrimarySwapChain_CrossSelectTip() { return QStringLiteral("不允许组合内交叉选中"); }
	static QString CD_PrimarySwapChain_MoveVertex() { return QStringLiteral("顶点位置非法"); }
	static QString CD_PrimarySwapChain_ForcedPointSyn() { return QStringLiteral("强制开启点编辑同步"); }
	static QString CD_PrimarySwapChain_ForcedPointSynClose() { return QStringLiteral("强制点编辑同步关闭"); }
	static QString CD_PrimarySwapChain_InvalidSlice() { return QStringLiteral("无效分片"); }
	static QString CD_PrimarySwapChain_NoScreenAdded() { return QStringLiteral("没有添加屏幕，请从屏幕列表中添加屏幕"); }
	static QString CD_PrimarySwapChain_SliceCutInvalid() { return QStringLiteral("无法分割"); }
	static QString CD_PrimarySwapChain_QuickCutTip() { return QStringLiteral("Alt+W/A/S/D 向上/左/下/右分割"); }
	static QString CD_PrimarySwapChain_QuickAddPointTip() { return QStringLiteral("Contrl+左键 进行加点"); }
	static QString CD_PrimarySwapChain_VertexeMinCount() { return QStringLiteral("删除后顶点数应大于2"); }
	static QString CD_PrimarySwapChain_VertexeCross() { return QStringLiteral("删除后应不相交"); }
	static QString CD_PrimarySwapChain_NoSharedMergeSide() { return QStringLiteral("请确保合并时存在一条重叠的边"); }
	static QString CD_PrimarySwapChain_MergeSliceOverlapped() { return QStringLiteral("请确保两切片重叠面积为零"); }
	static QString CD_PrimarySwapChain_SliceMergeInvalid() { return QStringLiteral("无法合并"); }

	static QString CD_PrimarySwapChain_SliceSnapEditTip() { return QStringLiteral("单切片·吸附操作"); }
	static QString CD_PrimarySwapChain_SliceLinkEditTip() { return QStringLiteral("单切片·联动操作"); }
	static QString CD_PrimarySwapChain_SliceMulitEditTip() { return QStringLiteral("切片组·点编辑操作"); }

	static QString VRCameraAddLimitedTips() { return QStringLiteral("当前场景已存在VR相机！"); }
	static QString VR_ResetBaseCamera() { return QStringLiteral("重置基准相机位置"); }
	static QString VR_EndVRConnected() { return QStringLiteral("停止VR连接"); }
};

class CObjectListTranslate {
public:
	static QString ObjectList() { return QStringLiteral("对象列表"); }
	static QString Baselight() { return QStringLiteral("基础光源"); }
};

class CModelWndTranslate {
public:
	static QString Model() { return QStringLiteral("模型"); }
	static QString DeleteModel() { return QStringLiteral("删除模型"); }
	static QString ModelLibrary() { return QStringLiteral("模型库"); }
	static QString Search() { return QStringLiteral("搜索"); }
	static QString NothingTip() { return QStringLiteral("未找到相关内容"); }
	static QString ImportProject() { return QStringLiteral("屏幕模型"); }
	static QString PreConfiguredProject() { return QStringLiteral("预设模型"); }
	static QString Cube() { return QStringLiteral("立方体"); }
	static QString Cylinder() { return QStringLiteral("圆柱体"); }
	static QString Sphere() { return QStringLiteral("球体"); }
	static QString Plane() { return QStringLiteral("平面"); }
	static QString Cone() { return QStringLiteral("圆锥体"); }
	static QString ImmersiveSpace() { return QStringLiteral("沉浸式空间屏幕"); }
	static QString IrregularPrism() { return QStringLiteral("柱体沉浸式屏幕"); }
	static QString RotationSolid() { return QStringLiteral("旋转体屏幕"); }
	static QString Camera() { return QStringLiteral("相机"); }
	static QString UnExistModel() { return QStringLiteral("模型文件不存在"); }
	static QString IsDelModel() { return QStringLiteral("是否删除模型？"); }
	static QString ModelInUse() { return QStringLiteral("模型使用中，无法删除"); }
	static QString Inport() { return QStringLiteral("导入"); }
	static QString ViewOf3D() { return QStringLiteral("3D视图"); }

	static QString ModelFileErrorTip() { return QStringLiteral("文件格式错误，请检查后重新导入。"); }
	static QString ModelBigTip() { return QStringLiteral("模型面数过多，可能导致异常，请简化后重新导入。"); }
	static QString ModelImport() { return QStringLiteral("模型导入设置"); }
	static QString ModelAnchorPosition() { return QStringLiteral("锚点位置"); }
	static QString ModelFileSize() { return QStringLiteral("模型大小"); }
	static QString OriginalAnchor() { return QStringLiteral("模型原始中心"); }
	static QString BottomAnchor() { return QStringLiteral("模型底部中心"); }
	static QString GeometryCenterAnchor() { return QStringLiteral("几何中心"); }

	static QString RetryImport() { return QStringLiteral("重新导入"); }

	static QString DirectionalLight() { return QStringLiteral("平行光"); }
	static QString PointLight() { return QStringLiteral("点光"); }
	static QString SpotLight() { return QStringLiteral("聚光"); }
	static QString SightModel() { return QStringLiteral("静物模型"); }
	static QString HelperModel() { return QStringLiteral("辅助模型"); }
	static QString ScreenModel() { return QStringLiteral("屏幕模型"); }
	static QString InferenceModel() { return QStringLiteral("AI推理模型"); }
	static QString AIPanoramaModel() { return QStringLiteral("AI全景模型"); }
	static QString VRCamrea() { return QStringLiteral("VR 相机"); }


	static QString ImportScreenModel() { return QStringLiteral("导入屏幕模型"); }
	static QString ImportSightModel() {	return QStringLiteral("导入静物模型");	}
	static QString ImportModel() { return QStringLiteral("导入模型"); }
	static QString NoSupportedFiles() { return QStringLiteral("没有受支持的文件"); }

	// Import dialog (multi-file)
	static QString Upload() { return QStringLiteral("上传"); }
	static QString OnlySupportFbxObj() { return QStringLiteral("仅支持上传FBX/ OBJ格式文件"); }

	static QString BackgroundImage() { return QStringLiteral("背景图片"); }
	static QString ImportBackgroundImage() { return QStringLiteral("导入背景图片"); }
	static QString ImageSightImportTitle() { return QStringLiteral("图片导入设置"); }
	static QString ImageSightDisplay() { return QStringLiteral("显示"); }
	static QString ImageSightSingleSided() { return QStringLiteral("单面"); }
	static QString ImageSightDoubleSided() { return QStringLiteral("双面"); }
	static QString ImageSightCreate() { return QStringLiteral("创建"); }
	static QString BackgroundImageLostTip() { return QStringLiteral("当前场景存在背景图片丢失，请检查！"); }
};

class CGridWndTranslate {
public:
	static QString ObjectList() { return QStringLiteral("对象列表"); }
	static QString Search() { return QStringLiteral("搜索"); }
	static QString Hide() { return QStringLiteral("隐藏"); }
	static QString Show() { return QStringLiteral("显示"); }
	static QString Lock() { return QStringLiteral("锁定"); }
	static QString UnLock() { return QStringLiteral("解除锁定"); }
	static QString ScenceSet() { return QStringLiteral("场景集合"); }
	static QString ParentChange() { return QStringLiteral("父目录改变"); }
	static QString IndexChange() { return QStringLiteral("索引改变"); }

	static QString SetMainCamera() { return QStringLiteral("设为主相机"); }
	static QString DelTip() { return QStringLiteral("确认执行删除操作？删除后，所选内容及关联的历史配置将同步清除，是否继续？"); }

	static QString CopyObject() { return QStringLiteral("复制物体"); }
	static QString PasteObject() { return QStringLiteral("粘贴物体"); }
	static QString GuideSliceSourceExport() { return QStringLiteral("指导片源数据导出"); }
	static QString ImportSliceFile() { return QStringLiteral("CGridWndTranslate_ImportSliceFile"); }
	static QString CopyTransform() { return QStringLiteral("复制变换"); }
	static QString PasteTransform() { return QStringLiteral("粘贴变换"); }
	static QString CopyTransform_All() { return QStringLiteral("复制全部变换"); }
	static QString CopyTransform_Position() { return QStringLiteral("复制位置"); }
	static QString CopyTransform_Rotation() { return QStringLiteral("复制旋转"); }
	static QString CopyTransform_Scale() { return QStringLiteral("复制缩放"); }
	static QString PasteTransform_All() { return QStringLiteral("粘贴全部变换"); }
	static QString PasteTransform_Position() { return QStringLiteral("粘贴位置"); }
	static QString PasteTransform_Rotation() { return QStringLiteral("粘贴旋转"); }
	static QString PasteTransform_Scale() { return QStringLiteral("粘贴缩放"); }
	static QString ToastCopiedOneObject() { return QStringLiteral("已复制一个选中物体。"); }
	static QString ToastCopiedTransformToClipboard() { return QStringLiteral("已复制变换参数 (XYZ) 至剪贴板。"); }
	static QString ToastTransformParamError() { return QStringLiteral("参数错误，请重试！"); }
	static QString ToastAuxiliaryModelNotSupportScale() { return QStringLiteral("辅助模型不支持缩放。"); }

	static QString FocusModelTip() { return QStringLiteral("快速定位聚焦模型"); }
};

class CSliceWndTranslate {
public:
	static QString SliceList() { return QStringLiteral("切片列表"); }
	static QString EmptyText1() { return QStringLiteral("暂无切片"); }
	static QString EmptyText2() { return QStringLiteral("当前模型未生成切片或未选中切片"); }
	static QString EmptyText3() { return QStringLiteral("此阶段暂无切片生成"); }
	static QString ModelDefaultExpand() { return QStringLiteral("内置展开"); }
	static QString ModelCameraExpand() { return QStringLiteral("基于相机通用展开"); }
	static QString ModelSpecifyExpand() { return QStringLiteral("定制展开"); }
	static QString NoSearchSlice() { return QStringLiteral("暂无搜索结果"); }
};

class CPrimary3DViewTranslate {
public:

	static QString Select() { return QStringLiteral("选中"); }
	static QString Move() { return QStringLiteral("移动"); }
	static QString Rotate() { return QStringLiteral("旋转"); }
	static QString Zoom() { return QStringLiteral("缩放"); }
	static QString ResetView() { return QStringLiteral("重置视角"); }
	static QString PerspectiveView() { return QStringLiteral("透视视角"); }
	static QString OrthogonalView() { return QStringLiteral("正交视角"); }
	static QString Roam() { return QStringLiteral("漫游"); }
	static QString CancelRoam() { return QStringLiteral("取消漫游"); }
	static QString ReturnToOrigin() { return QStringLiteral("回到原点"); }
	static QString RotateCameraAngle() { return QStringLiteral("旋转相机视角"); }
	static QString AdjustDistanceCameraToModel() { return QStringLiteral("调整相机距离"); }
	static QString PerspectiveCube() { return QStringLiteral("视角立方体"); }
	static QString PerspectiveCubeButton() { return QStringLiteral("视角立方体按钮"); }
	static QString GlobalCsys() { return QStringLiteral("世界坐标系"); }
	static QString LocalCsys() { return QStringLiteral("局部坐标系"); }
	static QString ViewScale() { return QStringLiteral("视图缩放"); }
	static QString FirstPerson() { return QStringLiteral("第一人称相机"); }
	static QString ThirdPerson() { return QStringLiteral("第三人称相机"); }
	static QString CreateCameraByCurrentView() { return QStringLiteral("当前视角生成相机"); }
	static QString ViewingCameraView() { return QStringLiteral("查看相机视角"); }
	static QString Fold() { return QStringLiteral("折叠"); }
	static QString Unfold() { return QStringLiteral("展开"); }

	static QString AuxObjectsVisible() { return QStringLiteral("辅助对象可见"); }
	static QString AuxObjectsInvisible() { return QStringLiteral("辅助对象不可见"); }
	static QString ShowAllObjectNames() { return QStringLiteral("显示所有对象名称"); }
	static QString HideAllObjectNames() { return QStringLiteral("隐藏所有对象名称"); }
	static QString ShowAllObjects() { return QStringLiteral("显示所有对象"); }

	static QString ModelCorrupted() {return QStringLiteral("模型文件损坏，请检查！");}
	static QString ModelOrTextureCorrupted() {return QStringLiteral("模型文件或纹理文件损坏，请检查：");}
	static QString ModelCreationLimit() {return QStringLiteral("模型创建个数达到上限！");}
	static QString ProjectMissingModel() {	return QStringLiteral("项目中使用了模型文件不存在的模型:");}
	static QString ProjectCorruptedModel() {return QStringLiteral("项目中使用了模型文件已损坏的模型:");}
	static QString PleaseCheckModel() { return QStringLiteral("请在模型库中检查"); }

	static QString DropMediaError() { return QStringLiteral("该模型无切片生成，无法显示！"); }

	static QString ToastCameraDistanceClosest() { return QStringLiteral("离观察目标已达最近"); }
	static QString ToastCameraDistanceFarthest() { return QStringLiteral("离观察目标已达最远"); }
	static QString ToastCameraCreatedByCurrentView() { return QStringLiteral("已根据当前视角创建新相机！"); }
};

class CPresetModelEditTranslate
{
public:

	static QString CameraViewTip() { return QStringLiteral("创建模型时同步生成相机"); }

	//沉浸式编辑弹窗
	static QString ImmersiveCreationWizard() { return QStringLiteral("沉浸式创建向导"); }
	static QString ImmersiveEdit() { return QStringLiteral("沉浸式编辑"); }
	static QString SelectImmersiveType() { return QStringLiteral("选择沉浸式类型"); }
	static QString SelectScreenForImmersive() { return QStringLiteral("选择屏幕以创建沉浸式"); }
	static QString SetParameters() { return QStringLiteral("设置参数"); }
	static QString ModelParameterSettings() { return QStringLiteral("模型参数设置"); }
	static QString RestoreDefaultValue() { return QStringLiteral("恢复默认值"); }
	static QString SetScreenParameters() { return QStringLiteral("设置屏幕参数"); }
	static QString AdjustSingleScreen() { return QStringLiteral("调整单面屏"); }
	static QString SelectAndAdjustScreenPositionSize() { return QStringLiteral("选择屏幕调整屏幕位置大小"); }
	static QString AdvancedSettings() { return QStringLiteral("高级设置"); }
	static QString BasicSettings() { return QStringLiteral("基础设置"); }
	static QString Depth() { return QStringLiteral("深"); }
	static QString ScreenPicture() { return QStringLiteral("画面"); }
	static QString InnerOuterFlip() { return QStringLiteral("内外翻转"); }
	static QString FreeView() { return QStringLiteral("自由视角"); }
	static QString CameraView() { return QStringLiteral("相机视角"); }
	static QString ScreenDotSpacingConsistent() { return QStringLiteral("屏幕点间距一致"); }
	static QString DotSpacing() { return QStringLiteral("点间距"); }
	static QString Up() { return QStringLiteral("上"); }
	static QString Down() { return QStringLiteral("下"); }
	static QString Left() { return QStringLiteral("左"); }
	static QString Right() { return QStringLiteral("右"); }
	static QString Create() { return QStringLiteral("创建"); }
	static QString AtLeastSelectScreenTip() { return QStringLiteral("至少选择一个屏幕"); }
	static QString SelectScreenTip() { return QStringLiteral("请从预览模型中选择屏幕以调整屏幕位置大小"); }
	static QString CurrentResolution() { return QStringLiteral("当前分辨率"); }

	static QString GetFaceText(int n)
	{
		switch (n)
		{
		case 0:
			return QStringLiteral("前面屏");
		case 1:
			return QStringLiteral("后面屏");
		case 2:
			return QStringLiteral("左面屏");
		case 3:
			return QStringLiteral("右面屏");
		case 4:
			return QStringLiteral("上面屏");
		case 5:
			return QStringLiteral("下面屏");
		default:
			break;
		}

		return "";
	}
	
	static QString Subdivision() { return QStringLiteral("细分"); }
	static QString Segment() { return QStringLiteral("分段"); }
	static QString ReturnBasicClearAdvancedTip() { return QStringLiteral("返回基础设置，高级配置将全部清除"); }

	// 沉浸式预览步右栏
	static QString PreviewModelGroup() { return QStringLiteral("模型"); }
	static QString PivotAxis() { return QStringLiteral("轴心"); }
	static QString OverallCenterAnchor() { return QStringLiteral("整体中心"); }
	static QString TopCenterAnchor() { return QStringLiteral("顶部中心"); }
	static QString LeftCenterAnchor() { return QStringLiteral("左侧中心"); }
	static QString RightCenterAnchor() { return QStringLiteral("右侧中心"); }
	static QString FrontCenterAnchor() { return QStringLiteral("前侧中心"); }
	static QString BackCenterAnchor() { return QStringLiteral("后侧中心"); }

	static QString GetImmersiveAnchorText(int type)
	{
		switch (type)
		{
		case 0:
			return CModelWndTranslate::OriginalAnchor();
		case 1:
			return OverallCenterAnchor();
		case 2:
			return TopCenterAnchor();
		case 3:
			return CModelWndTranslate::BottomAnchor();
		case 4:
			return LeftCenterAnchor();
		case 5:
			return RightCenterAnchor();
		case 6:
			return FrontCenterAnchor();
		case 7:
			return BackCenterAnchor();
		default:
			return CModelWndTranslate::OriginalAnchor();
		}
	}

	//柱体沉浸式编辑
	static QString CreateCylinderWizard() { return QStringLiteral("柱体创建向导"); }
	static QString ImmersiveCylinderEditing() { return QStringLiteral("柱体沉浸式编辑"); }
	static QString BaseEditing() { return QStringLiteral("底面编辑"); }
	static QString EditCylinderBaseShape() { return QStringLiteral("进行柱体底面形状编辑"); }
	static QString Preview() { return QStringLiteral("预览"); }
	static QString Cylinder3DPreview() { return QStringLiteral("进行柱体模型3D预览"); }
	static QString SelectTool() { return QStringLiteral("选择模式"); }
	static QString PenTool() { return QStringLiteral("绘制模式"); }
	static QString ScaleRatio() { return QStringLiteral("缩放比例"); }
	static QString DrawFromUnclosedVertex() { return QStringLiteral("必须从前一段图形的端点开始绘制"); }
	static QString SplineEditing() { return QStringLiteral("样条编辑"); }
	static QString PointEditing() { return QStringLiteral("点编辑"); }
	static QString EdgeEditing() { return QStringLiteral("边编辑"); }
	static QString CornerPoint() { return QStringLiteral("角点"); }
	static QString Smooth() { return QStringLiteral("平滑"); }
	static QString GenerateCylinderSurface() { return QStringLiteral("生成柱面"); }
	static QString Steps() { return QStringLiteral("步数"); }
	static QString Base() { return QStringLiteral("底面"); }
	static QString Top() { return QStringLiteral("顶面"); }
	static QString Height() { return QStringLiteral("高度"); }
	static QString GraphCross() { return QStringLiteral("图形存在线交叉或点重合，请重新编辑"); }
	static QString FaceGenerateTip() { return QStringLiteral("当前无柱面生成"); }
	static QString GenerateBasesTip() { return QStringLiteral("当前底面形状未封闭，不支持生成顶面和底面"); }

	//旋转体屏幕
	static QString RotateExtrudeCreationWizard() { return QStringLiteral("旋转体创建向导"); }
	static QString RotateExtrudeEditor() { return QStringLiteral("旋转体编辑"); }
	static QString ProfileEditor() { return QStringLiteral("轮廓编辑"); }
	static QString SideProfileEditor() { return QStringLiteral("进行旋转体侧面轮廓编辑"); }
	static QString RotateExtrude3DPreviewer() { return QStringLiteral("进行旋转体模型3D预览"); }

	//按键翻译
	static QString Previous() { return QStringLiteral("上一步"); }
	static QString Next() { return QStringLiteral("下一步"); }

	//沉浸式
	static QString Margin() { return QStringLiteral("边距"); }

	//柱体
	static QString EdgeSize() { return QStringLiteral("尺寸"); }
	static QString EdgeLength() { return QStringLiteral("长度"); }
	static QString GenerateFace() { return QStringLiteral("生面"); }

	//AI模型推理
	static QString InvalidSamplingTableTip() { return QStringLiteral("抽点表无效，请重新选择后导入"); }
	static QString ModelInferenceView() { return QStringLiteral("推理模型编辑界面"); }
	static QString LoadLinkRelationship() { return QStringLiteral("加载连接关系"); }
	static QString ScreenFile() { return QStringLiteral("屏幕文件"); }
	static QString ScreenFileTip() { return QStringLiteral("目前仅支持导入连接关系.mapping格式文件"); }
	static QString LinkRelationFile() { return QStringLiteral("连接关系文件"); }
	static QString ChoseFile() { return QStringLiteral("请选择文件"); }
	static QString LinkRelationFileFailTip() { return QStringLiteral("连接关系文件无法读取/不匹配，请检查文件"); }
	static QString LedLinkRelationFileDesc() { return QStringLiteral("通过LED屏幕的连接关系文件，可以推断出屏幕模型"); }

	static QString ModuleProperty() { return QStringLiteral("模组属性"); }
	static QString ModuleTip() { return QStringLiteral("当前版本的模组仅保存参数文件数据，不影响位置和大小"); }
	static QString Module() { return QStringLiteral("模组"); }
	static QString ModuleNumber() { return QStringLiteral("编号"); }
	static QString ModuleLoc() { return QStringLiteral("位置"); }
	static QString ModuleSize() { return QStringLiteral("大小"); }
	static QString ModuleWidth() { return QStringLiteral("宽"); }
	static QString ModuleHeight() { return QStringLiteral("高"); }
	static QString ModuleFile() { return QStringLiteral("参数文件"); }
	static QString ModuleSamplingTable() { return QStringLiteral("抽点表"); }

	static QString AdjacentWiringProperty() { return QStringLiteral("邻接线属性"); }
	static QString AdjacentWiringTip() { return QStringLiteral("模组的参数文件会影响模组间的邻接关系"); }
	static QString RecalcAdjacentWiring() { return QStringLiteral("全部重新计算"); }
	static QString RecalcAdjacentWiringSucTip() { return QStringLiteral("重新计算成功"); }
	static QString ClearAdjacentWiring() { return QStringLiteral("清空邻接线"); }
	static QString DelAdjacentWiring() { return QStringLiteral("删除邻接线"); }
	static QString RelationMoudle() { return QStringLiteral("关系模组"); }
	static QString AdjacentWiring() { return QStringLiteral("邻接线"); }
	static QString CoverMouldeDataTip() { return QStringLiteral("检测到已配置参数，是否覆盖？"); }
	static QString ChooseSamplingTableFile() { return QStringLiteral("选择抽点表文件"); }
	static QString SamplingTableFileType() { return QStringLiteral("抽点表文件 (*.csv);;所有文件 (*.*)"); }
	static QString CannotOpenFile() { return QStringLiteral("无法打开文件:"); }
	static QString ClearAdjacentWiringTip() { return QStringLiteral("继续操作将会清空所有的邻接线，是否清空？"); }
	static QString CalcAllAdjacentWiringTip() { return QStringLiteral("继续操作将会重新计算所有的信息，是否继续？"); }
	static QString SelectedMoudle1() { return QStringLiteral("已选中"); }
	static QString SelectedMoudle2() { return QStringLiteral("个模组"); }

	static QString EditInferenceModel() { return QStringLiteral("编辑推理模型"); }
	static QString ReCalcTip() { return QStringLiteral("正在重新计算，请稍候..."); }
	static QString InferenceModelCreateGuide() { return QStringLiteral("推理模型创建向导"); }
	static QString AIPanoramaCreateGuide() { return QStringLiteral("AI全景模型创建向导"); }
	static QString AIPanoramaLoading() { return QStringLiteral("加载中……"); }
	static QString AIPanoramaLoadFailTip() { return QStringLiteral("加载失败，请检查后重试！"); }
	static QString FileUpload() { return QStringLiteral("文件上传"); }
	static QString ImageProcess() { return QStringLiteral("图片处理"); }
	static QString PanoramaFileTitle() { return QStringLiteral("全景图文件"); }
	static QString PanoramaDropHint() { return QStringLiteral("将图片拖到此处，或点击上传"); }
	static QString PanoramaDropSubHint() { return QStringLiteral("仅支持2:1 JPG格式全景图片"); }
	static QString PanoramaRatioInvalid() { return QStringLiteral("图片分辨率不符合要求"); }
	static QString PanoramaFormatInvalid() { return QStringLiteral("图片格式不符合要求"); }
	static QString ReplaceImage() { return QStringLiteral("替换图片"); }
	static QString SpatialFloorPlan() { return QStringLiteral("空间平面图"); }
	static QString DrawWallLine() { return QStringLiteral("绘制墙线"); }
	static QString SetWallHeight() { return QStringLiteral("设置墙高"); }
	static QString ResetWallData() { return QStringLiteral("重置墙面数据"); }
	static QString FileImport() { return QStringLiteral("文件导入"); }
	static QString DataProcess() { return QStringLiteral("数据处理"); }
	static QString Preview3DModel() { return QStringLiteral("模型3D预览"); }
	static QString CalcAdjacentRelationTip() { return QStringLiteral("邻接关系计算中..."); }
	static QString DataProcessTip() { return QStringLiteral("数据处理中..."); }
	static QString ModelInferenceTip() { return QStringLiteral("模型推理中..."); }
	static QString ModelInferenceSucTip() { return QStringLiteral("模型推理中..."); }
	static QString ModelInferenceFailTip() { return QStringLiteral("模型推理失败，请检查后重试！"); }
	static QString DataLoadSucTip() { return QStringLiteral("加载成功！"); }
	static QString DataLoadFailTip() { return QStringLiteral("数据加载失败，请重新上传文件！"); }

	//R角
	static QString RCorner() { return QStringLiteral("R角"); }
	static QString RCornerSizeTip() { return QStringLiteral("宽度、深度和高度不包含R角部分"); }
	static QString RCornerEdgeLength() { return QStringLiteral("边长"); }
	static QString Value() { return QStringLiteral("值"); }

	//球体
	static QString SphereCreationWizard() { return QStringLiteral("球体创建向导"); }
	static QString EditSphereModel() { return QStringLiteral("球体模型编辑"); }
	static QString Size() { return QStringLiteral("尺寸"); }
	static QString Range() { return QStringLiteral("范围"); }
	static QString StartAngle() { return QStringLiteral("起始"); }
	static QString EndAngle() { return QStringLiteral("结束"); }
	static QString RotateAngle() { return QStringLiteral("旋转"); }
	static QString SphereImageShow() { return QStringLiteral("显示"); }

	//
	static QString ImmersionPoint() { return QStringLiteral("端点"); }
	static QString ImmersionPointLoc() { return QStringLiteral("位置"); }

	static QString DebugStageDisableEditModelTip() { return QStringLiteral("调试阶段禁止模型编辑"); }
	static QString DebugStageDisableDelModelTip() { return QStringLiteral("调试阶段禁止删除模型"); }
	static QString PlayControlStageDisableEditModelTip() { return QStringLiteral("播控阶段禁止模型编辑"); }
	static QString PlayControlStageDisableDelModelTip() { return QStringLiteral("播控阶段禁止删除模型"); }

	// 展开优化类型
	static QString UnfoldOptType() { return QStringLiteral("展开优化类型"); }

	// 法向共平面阈值
	static QString NormalCoplanarThresh() { return QStringLiteral("法向共平面阈值"); }

	// 阈值越大，判定越宽松，合并越“激进”，切片数量可能更少
	static QString ThreshDesc() { return QStringLiteral("阈值越大，判定越宽松，合并越“激进”，切片数量可能更少"); }

	// 仅合并共平面
	static QString MergeOnlyCoplanar() { return QStringLiteral("仅合并共平面"); }

	// UV均匀展开
	static QString UVUniformUnfold() { return QStringLiteral("UV均匀展开"); }

	// UV十字平铺展开
	static QString UVCrossTileUnfold() { return QStringLiteral("UV十字平铺展开"); }

	// 非四边形面分离
	static QString SeparateNonQuadFace() { return QStringLiteral("非四边形面分离"); }

	// 四边形切片矩形化
	static QString QuadSliceRectify() { return QStringLiteral("四边形切片矩形化"); }

	// 可能导致切片重叠
	static QString SliceOverlapTip() { return QStringLiteral("可能导致切片重叠"); }

	static QString NoOptimization() { return QStringLiteral("无优化"); }
	static QString Recommend() { return QStringLiteral("推荐"); }
	static QString ModelImportTo3D() { return QStringLiteral("模型导入"); }
	static QString ModelLoading() { return QStringLiteral("模型加载中"); }
	static QString NormalTip() { return QStringLiteral("阈值越大，判定越宽松，合并越“激进”，切片数量可能更少"); }
	static QString ImportSuccees() { return QStringLiteral("导入成功"); }
	static QString ModelEdit() { return QStringLiteral("模型编辑"); }

	static QString CurvatureDegree() { return QStringLiteral("卷曲程度"); }
	static QString ScreenDisplay() { return QStringLiteral("画面显示"); }
	static QString ScreenSelection() { return QStringLiteral("屏幕选择"); }

	static QString CurvatureAdjustForbidden() { return QStringLiteral("该连接关系推断为封闭曲面，卷曲程度不可调节。"); }
	static QString CurvatureValueDesc() { return QStringLiteral("数值越大，闭合程度越高；当数值过大时可能出现叠面。"); }
};

class CScreenTranslate : public QObject
{
	Q_OBJECT
public:
	static QString ScreenClose() { return QStringLiteral("关闭"); }
	static QString ScreenOpend() { return QStringLiteral("打开"); }
	static QString ScreenLost() { return QStringLiteral("无效"); }
	static QString ScreenNotAdded() { return QStringLiteral("未添加"); }
	static QString ScreenAdded() { return QStringLiteral("已添加"); }
	static QString ScreenMgr() { return QStringLiteral("屏幕管理"); }
	static QString AddScreen() { return QStringLiteral("添加屏幕"); }
	static QString DeleteScreen() { return QStringLiteral("删除屏幕"); }
	static QString ExportSliceFile(){ return QStringLiteral("导出Mapping数据"); }
	static QString OpenScreen() { return QStringLiteral("打开屏幕"); }
	static QString CloseScreen() { return QStringLiteral("关闭屏幕"); }
	static QString MaskScreen() { return QStringLiteral("标识屏幕"); }
	static QString UnMaskScreen() { return QStringLiteral("关闭屏幕标识"); }
	static QString OpenAllScreen() { return QStringLiteral("打开所有屏幕"); }
	static QString CloseAllScreen() { return QStringLiteral("关闭所有屏幕"); }
	static QString DeleteScreenTip() { return QStringLiteral("请先关闭屏幕!"); }
 
	static QString ScreenProperty() { return QStringLiteral("屏幕属性"); }

	static QString VirtualScreen() { return QStringLiteral("虚拟屏幕"); }
	static QString AddVirtualScreen() { return QStringLiteral("添加虚拟屏幕"); }

	static QString ScreenMap() { return QStringLiteral("映射屏幕"); }
	static QString ScreenUnmap() { return QStringLiteral("取消映射"); }

	static QString AddLayoutReceiverFile() { return QStringLiteral("添加连接关系"); }
	static QString DeleteLayoutReceiverFile() { return QStringLiteral("删除连接关系"); }

	static QString ScreenFind() { return QStringLiteral("定位屏幕"); }

	static QString PleaseSelect() { return QStringLiteral("请选择"); }

	static QString SizeUnequalTip() { return QStringLiteral("连接关系大小(%1x%2)和屏幕大小(%3x%4)不一致"); }
	static QString LayoutReceiverFileErrorTip() { return QStringLiteral("数据解析错误，请检测后重新导入。"); }

	static QString NoMapScreenErrorTip() { return QStringLiteral("屏幕分辨率不一致，无法映射。"); }

	static QString OutputViewPosition() { return QStringLiteral("位置"); }
	static QString ScreenSize() { return QStringLiteral("屏幕分辨率"); }

	static QString Used() { return QStringLiteral("已使用"); }
	static QString PrimaryTip() { return QStringLiteral("(主屏幕，不建议使用)"); }

	static QString ScreenDelTip() { return QStringLiteral("确认执行删除操作？删除后，所选屏幕及其关联的内容将受到影响，是否继续？"); }
	static QString InputSyncToOutput() { return QStringLiteral("输入同步到输出"); }
	static QString OutputSyncToInput() { return QStringLiteral("输出同步到输入"); }

	static QString OpenCloseAllScreensTip() { return QStringLiteral("打开/关闭所有屏幕"); }
	static QString ShowCloseTestChartTip() { return QStringLiteral("显示/关闭屏幕测试图"); }
	static QString OpenCloseFTBTip() { return QStringLiteral("开启/关闭 FTB"); }
	static QString ShowScreenTestChart() { return QStringLiteral("显示屏幕测试图"); }
	static QString CloseScreenTestChart() { return QStringLiteral("关闭屏幕测试图"); }
	static QString OpenFTB() { return QStringLiteral("开启 FTB"); }
	static QString CloseFTB() { return QStringLiteral("关闭 FTB"); }
	static QString FTBDuration() { return QStringLiteral("渐变时长"); }
	static QString TestChartFileInvalid() { return QStringLiteral("测试图文件无效"); }
	static QString TestChartLoadFailed() { return QStringLiteral("测试图加载失败"); }
	static QString RestoreDefaultValue() { return QStringLiteral("恢复默认值"); }
};

class CResourceTranslate : public QObject
{
	Q_OBJECT
public:
	static QString ResourceWnd() { return QStringLiteral("素材库"); }
	static QString ExistFileTips() { return QStringLiteral("已存在素材列表"); }
	static QString NotSupportFileList() { return QStringLiteral("不支持文件列表"); }
	static QString ReplaceRes() { return QStringLiteral("素材替换"); }
	static QString QuoteQuery() { return QStringLiteral("引用查询"); }
	static QString AddResource() { return QStringLiteral("添加素材"); }
	static QString LocateFile() { return QStringLiteral("打开所在文件夹..."); }
	static QString AddFolder() { return QStringLiteral("文件夹素材"); }
	static QString AddFilter() { return QStringLiteral("新建素材组"); }
	static QString AddMedia() { return QStringLiteral("视频/音频/图片"); }
	static QString ResourcesNotDeletedTips() { return QStringLiteral("已删除%1/%2个素材，%3个使用中素材无法删除！"); }
	static QString ResourcesDeletedTips() { return QStringLiteral("删除提示"); }
	static QString ResourcesUsedTips() { return QStringLiteral("素材使用中无法删除！"); }

	static QString Video() { return QStringLiteral("视频"); }
	static QString ImportVideo() { return QStringLiteral("导入视频"); }
	static QString Picture() { return QStringLiteral("图片"); }
	static QString ImportImage() { return QStringLiteral("导入图片"); }
	static QString TestChart() { return QStringLiteral("测试图"); }
	static QString MaterialReuse() { return QStringLiteral("素材解复用"); }
	static QString Fail() { return QStringLiteral("失败"); }
	static QString Success() { return QStringLiteral("成功"); }
	static QString Cancel() { return QStringLiteral("取消"); }
	static QString DoubleClick() { return QStringLiteral("双击"); }
	static QString RightClickToEditor() { return QStringLiteral("菜单编辑"); }
	static QString WhetherDeleteResource() { return QStringLiteral("是否确定删除素材？"); }


	static QString SearchTips() { return QStringLiteral("搜索..."); }
	static QString AddAudioCap() { return QStringLiteral("音频采集"); }
	static QString AddVideoCap() { return QStringLiteral("视频采集"); }
	static QString AddTools() { return QStringLiteral("小工具"); }
	static QString AddText() { return QStringLiteral("文本"); }
	static QString AddColorText() { return QStringLiteral("幻彩风格"); }
	static QString AddTestChart() { return QStringLiteral("测试图"); }
	static QString AddTimerRes() { return QStringLiteral("计时器"); }
	static QString AddEnvironmentInfoRes() { return QStringLiteral("环境信息"); }
	static QString AddAnalogClockRes() { return QStringLiteral("模拟时钟"); }
	static QString AddDigitalClockRes() { return QStringLiteral("数字时钟"); }
	static QString AddCalendarRes() { return QStringLiteral("日历"); }
	static QString AddDataBaseRes() { return QStringLiteral("数据库"); }
	static QString TextName() { return QStringLiteral("文本 %1"); }
	static QString ColorTextName() { return QStringLiteral("幻彩字 %1"); }
	static QString TestChartName() { return QStringLiteral("测试图 %1"); }
	static QString TimerName() { return QStringLiteral("计时器 %1"); }
	static QString AnalogClockName() { return QStringLiteral("模拟时钟 %1"); }
	static QString DigitalClockName() { return QStringLiteral("数字时钟 %1"); }
	static QString CalendarName() { return QStringLiteral("日历 %1"); }
	static QString DataBaseName() { return QStringLiteral("数据库 %1"); }
	static QString EnvironmentInfoName() { return QStringLiteral("环境信息 %1"); }
	static QString NewFolder() { return QStringLiteral("素材组"); }
	static QString AddStreamMedia() { return QStringLiteral("流媒体"); }
	static QString AddNDIStream() { return QStringLiteral("NDI视频源"); }
	static QString AddPPT() { return QStringLiteral("PPT"); }
	static QString AddExcel() { return QStringLiteral("Excel"); }
	static QString AddPdf() { return QStringLiteral("PDF"); }
	static QString AddWord() { return QStringLiteral("Word"); }
	static QString AlwaysOpen() { return QStringLiteral("持续打开"); }
	static QString PicTrans() { return QStringLiteral("图片优化"); }
	static QString ColorRange() { return QStringLiteral("颜色范围"); }
	static QString DecodingType() { return QStringLiteral("解码类型"); }
	static QString PicTransAuto() { return QStringLiteral("自动"); }
	static QString PicTransNone() { return QStringLiteral("无优化"); }

	static QString PPTModeSelectDlgTitle() { return QStringLiteral("PPT模式选择"); }
	static QString PPTResolution() { return QStringLiteral("显示分辨率"); }
	static QString PPTFilePath() { return QStringLiteral("文件路径"); }
	static QString PPTWidth() { return QStringLiteral("宽度:"); }
	static QString PPTHeight() { return QStringLiteral("高度:"); }
	static QString PPTAnimationMode() { return QStringLiteral("动画模式"); }
	static QString Recommended() { return QStringLiteral("(推荐使用)"); }
	static QString PPTPictureMode() { return QStringLiteral("图片模式"); }
	static QString PPTAnimationModeExplain() { return QStringLiteral("纹理实时渲染，实现流畅的动画效果，通过工程设置—素材设置修改渲染引擎及动画技术"); }
	static QString PPTPictureModeExplain() { return QStringLiteral("将PPT提前解析并转换为图片，兼容性好，<br>不显示原PPT的动画效果"); }
	static QString PPTHostMode() { return QStringLiteral("主持人模式"); }
	static QString PPTExportNotes() { return QStringLiteral("自动读取PPT备注并生成提词器"); }
	static QString UninstallOffice() { return QStringLiteral("服务器未安装office，请检查."); }
	static QString UninstallWPS() { return QStringLiteral("服务器未安装WPS，请检查."); }
	static QString UninstallOfficeAndWPS() { return QStringLiteral("服务器未安装office与WPS，请检查."); }
	static QString PPTPreloadFailed() { return QStringLiteral("PPT预加载失败"); }

	static QString ReplaceExistResTips() { return QStringLiteral("选中素材已被添加进素材库，请重新选择!"); }
	static QString ResDemuxering() { return QStringLiteral("素材解复用中..."); }
	static QString ResUIPlaceHolderText() { return QStringLiteral("导入素材以开始"); }

	static QString ResMediaFilter() { return QStringLiteral("全部(%1 %2 %3)\n\t\t%4\n\t\t%5\n\t\t%6"); }
	static QString ResVideoFilter() { return QStringLiteral("视频(%1)"); }
	static QString ResAudioFilter() { return QStringLiteral("音频(%1)"); }
	static QString ResPictureFilter() { return QStringLiteral("图片(%1)"); }
	static QString ResPPTFilter() { return QStringLiteral("PPT(%1)"); }
	static QString ResExcelFilter() { return QStringLiteral("Excel(%1)"); }
	static QString ResPdfFilter() { return QStringLiteral("PDF(%1)"); }
	static QString ResWordFilter() { return QStringLiteral("Word(%1)"); }

	static QString VideoFile() { return QStringLiteral("视频文件"); }
	static QString AudioFile() { return QStringLiteral("音频文件"); }
	static QString PictureFile() { return QStringLiteral("图片文件"); }
	static QString FailedReplaceResByInGroupRes() { return QStringLiteral("素材[%1]已被添加到合集[%2]中,请检查!"); }

	static QString DataBaseLoginDlg() { return QStringLiteral("数据库登录窗口"); }
	static QString DataBaseTableSelectDlg() { return QStringLiteral("数据库表格选择窗口"); }

	static QString OfficeUninstallTips() { return QStringLiteral("文件加载失败:\n          本机未安装Office-PPT.\n          本机未安装WPS.\n          Office未激活."); }
	static QString TableModeSelectDlgTitle() { return QStringLiteral("表格模式选择"); }
	static QString TableEditorModeSelectText() { return QStringLiteral("编辑模式"); }
	static QString TableEditorModeDescText() { return QStringLiteral("自研引擎实时绘制,可自由调节内容,颜色和边框等属性,支持导入csv和gstf文件"); }
	static QString TablePictureModeSelectText() { return QStringLiteral("图片模式"); }
	static QString TablePictureModeDescText() { return QStringLiteral("将表格提前解析并转换为图片,兼容性好,支持Excel和WPS表格"); }

	static QString SubtitleSelectText() { return QStringLiteral("通过导入lrc、srt和txt字幕文件，将字幕与视频素材绑定以实现音画同步"); }
	static QString TeleprompterSelectText() { return QStringLiteral("自由编辑时间轴和文本内容，随心翻页、精准控场"); }
	static QString SubtitleTextSelectTitle() { return QStringLiteral("字幕模式选择"); }

	static QString SubtitleOrTeleprompter() { return QStringLiteral("字幕/提词器"); }

	static QString SearchByFileNameTips() { return QStringLiteral("请输入文件名搜索..."); }
	static QString FileContentIsEmpty() { return QStringLiteral("文件内容为空"); }
};

class CTestChartTranslate : public QObject
{
	Q_OBJECT
public:
	static QString Number() { return QStringLiteral("数字"); }
	static QString Letter() { return QStringLiteral("字母"); }
	static QString Row() { return QStringLiteral("行"); }
	static QString Col() { return QStringLiteral("列"); }
	static QString GrandShow() { return QStringLiteral("GrandShow"); }
	static QString NumbmerMode() { return QStringLiteral("编号"); }

	static QString LocationSizeProperty() { return QStringLiteral("大小"); }
	static QString Train() { return QStringLiteral("行列"); }
	static QString ModeSelectProperty() { return QStringLiteral("模式选择"); }
	static QString OrderProperty() { return QStringLiteral("序号"); }
	static QString SplitLineProperty() { return QStringLiteral("分割线"); }
	static QString CheekFrameProperty() { return QStringLiteral(" TCT_Margin"); }
	static QString CircleProperty() { return QStringLiteral(" TCT_Cirlcle"); }
	static QString DiagonalProperty() { return QStringLiteral(" TCT_Diagonal"); }
	static QString ResolutionProperty() { return QStringLiteral(" TCT_Resolution"); }
	static QString BoxNumProperty() { return QStringLiteral(" TCT_BoxNum"); }
	static QString LogoProperty() { return QStringLiteral(" TCT_Logo Settings"); }
	static QString TextInfoProperty() { return QStringLiteral(" TCT_Text Info"); }

	static QString Color() { return QStringLiteral("颜色"); }
	static QString Mode() { return QStringLiteral("模式"); }
	static QString FontSize() { return QStringLiteral("字号"); }
	static QString WireFrame() { return QStringLiteral("线宽"); }
	static QString Radius() { return QStringLiteral("半径"); }
	static QString LogoMode() { return QStringLiteral("Logo"); }
	static QString Zoom() { return QStringLiteral("缩放"); }
	static QString Opacity() { return QStringLiteral("不透明度"); }
	static QString Transparency() { return QStringLiteral("不透明度"); }
	static QString Identifier() { return QStringLiteral("编号"); }
	static QString PureColorMode() { return QStringLiteral("纯色模式"); }
	static QString GradientMode() { return QStringLiteral("渐变模式"); }
	static QString ChessMode() { return QStringLiteral("棋盘模式"); }
	static QString TestCardMode() { return QStringLiteral("色卡模式"); }
	static QString Text() { return QStringLiteral("文本"); }
	static QString ExceedingResolutionTips() { return QStringLiteral("超过最大宽高8192，属性编辑不生效"); }

	static QString ChartTypeGroup() { return QStringLiteral("测试图"); }
	static QString ChartType() { return QStringLiteral("类型"); }
	static QString ResolutionMarkProperty() { return QStringLiteral("分辨率标识"); }
	static QString PlaneGrid() { return QStringLiteral("平面网格"); }
	static QString SingleFisheye() { return QStringLiteral("单鱼眼"); }
	static QString DualFisheye() { return QStringLiteral("双鱼眼"); }
	static QString Skybox() { return QStringLiteral("天空盒"); }
	static QString Width() { return QStringLiteral("宽"); }
	static QString Height() { return QStringLiteral("高"); }
	static QString FisheyeBoundary() { return QStringLiteral("鱼眼边界"); }
	static QString AngleScaleCircle() { return QStringLiteral("角度刻度圆"); }
	static QString FullFov() { return QStringLiteral("完整视场角"); }
	static QString HalfFov() { return QStringLiteral("半视场角"); }
	static QString TickInterval() { return QStringLiteral("刻度间隔"); }
	static QString GridSubdiv() { return QStringLiteral("细分"); }
	static QString RadialLines() { return QStringLiteral("放射线"); }
	static QString RadialCount() { return QStringLiteral("数量"); }
	static QString SubdivRadial() { return QStringLiteral("细分放射线"); }
	static QString CenterRefCircle() { return QStringLiteral("中心参考圆"); }
	static QString BackgroundColor() { return QStringLiteral("背景颜色"); }
	static QString GridColor() { return QStringLiteral("网格颜色"); }
	static QString LineWidth() { return QStringLiteral("线宽"); }
	static QString Show() { return QStringLiteral("显示"); }
	static QString Hide() { return QStringLiteral("隐藏"); }
	static QString View() { return QStringLiteral("视图"); }
	static QString DualLayout() { return QStringLiteral("双鱼眼模式"); }	// 双鱼眼模式（展开组名）
	static QString DualLayoutMethod() { return QStringLiteral("方式"); }	// 方式
	static QString LayoutLR() { return QStringLiteral("左右排列"); }
	static QString LayoutTB() { return QStringLiteral("上下排列"); }
	static QString FaceName() { return QStringLiteral("面名称"); }
	static QString FaceUp() { return QStringLiteral("上"); }
	static QString FaceDown() { return QStringLiteral("下"); }
	static QString FaceLeft() { return QStringLiteral("左"); }
	static QString FaceRight() { return QStringLiteral("右"); }
	static QString FaceFront() { return QStringLiteral("前"); }
	static QString FaceBack() { return QStringLiteral("后"); }
};

class CPropertyTranslate : public QObject
{
	Q_OBJECT
public:

	static QString PropertyWidget() { return QStringLiteral("属性窗"); }
	static QString ModelProperty() { return QStringLiteral("模型属性"); }
	static QString MediaPropertyWnd() { return QStringLiteral("媒体属性"); }
	static QString SliceProperty() { return QStringLiteral("切片属性"); }
	static QString CameraProperty() { return QStringLiteral("相机属性"); }
	static QString MediaProperty() { return QStringLiteral("媒体属性"); }
	static QString TimeLineProperty() { return QStringLiteral("时间线属性"); }
	static QString PreviewUnitProperty() { return QStringLiteral("预览单元属性"); }
	static QString VRCameraProperty() { return QStringLiteral("VR 相机属性"); }
	static QString ResuorcePropertyWnd() { return QStringLiteral("素材属性"); }
	static QString VertexProperty() { return QStringLiteral("顶点属性"); }
	static QString GlobalProperty() { return QStringLiteral("全局属性"); }
	static QString SectionProperty() { return QStringLiteral("Section属性"); }
	static QString TagProperty() { return QStringLiteral("Tag属性"); }
	static QString NoteProperty() { return QStringLiteral("Note属性"); }
	static QString PathCameraClipProperty() { return QStringLiteral("相机动画片段属性"); }
	static QString PathCameraClipEnd() { return QStringLiteral("截止位置"); }
	static QString PathCamera3DPath() { return QStringLiteral("三维路径"); }
	static QString PathCameraFovGroup() { return QStringLiteral("视野范围"); }
	static QString PathCameraTargetGroup() { return QStringLiteral("目标物体"); }
	static QString PathCameraObject() { return QStringLiteral("对象"); }
	static QString LayerProperty() { return QStringLiteral("图层属性"); }
	static QString TransFxProperty() { return QStringLiteral("转场属性"); }
	static QString State() { return QStringLiteral("状态"); }
	static QString ShowPlayProgress() { return QStringLiteral("显示进度"); }
	static QString Server() { return QStringLiteral("服务器"); }
	static QString Device() { return QStringLiteral("设备"); }
	static QString LocalServer() { return QStringLiteral("本地服务器"); }
	static QString DefaultDevice() { return QStringLiteral("默认设备"); }
	static QString DeviceDisconnectedSuffix() { return QStringLiteral("(已断开)"); }
	static QString NoAvailableAudioDevice() { return QStringLiteral("无可用音频设备"); }

	static QString BaseProperty() { return QStringLiteral("基础属性"); }
	static QString AudioProperty() { return QStringLiteral("音频属性"); }
	static QString AudioType() { return QStringLiteral("当前素材音频类型"); }
	static QString AudioChannel() { return QStringLiteral("声道"); }
	static QString UsedChannelMapTip() { return QStringLiteral("使用声道映射"); }
	static QStringList AudioChannelName()
	{
		return QStringList() << QStringLiteral("前左") << QStringLiteral("前右") << QStringLiteral("前中")
			<< QStringLiteral("低音") << QStringLiteral("后左") << QStringLiteral("后右")
			<< QStringLiteral("侧左") << QStringLiteral("侧右");
	}

	static QString Angle() { return QStringLiteral("角度"); }
	static QString PleaseSelect() { return QStringLiteral("请选择"); }

	static QString SetXAxisPosition() { return QStringLiteral("设置X轴坐标"); }
	static QString SetYAxisPosition() { return QStringLiteral("设置Y轴坐标"); }
	static QString SetZAxisPosition() { return QStringLiteral("设置Z轴坐标"); }

	static QString SetXAxisAngle() { return QStringLiteral("设置X轴角度"); }
	static QString SetYAxisAngle() { return QStringLiteral("设置Y轴角度"); }
	static QString SetZAxisAngle() { return QStringLiteral("设置Z轴角度"); }

	static QString SetXAxisScale() { return QStringLiteral("设置X轴缩放"); }
	static QString SetYAxisScale() { return QStringLiteral("设置Y轴缩放"); }
	static QString SetZAxisScale() { return QStringLiteral("设置Z轴缩放"); }

	static QString SetWidth() { return QStringLiteral("设置宽度"); }
	static QString SetHeight() { return QStringLiteral("设置高度"); }
	static QString SetAngle() { return QStringLiteral("设置角度"); }

	static QString CopyXYZCoordinate() { return QStringLiteral("复制XYZ坐标"); }
	static QString PasteXYZCoordinate() { return QStringLiteral("粘贴XYZ坐标"); }
	static QString ToastCopiedValueToClipboard() { return QStringLiteral("已复制参数至剪贴板。"); }

	static QString RectPoint() { return QStringLiteral("左上角矩阵点"); }
	static QString CenterPoint() { return QStringLiteral("中心点"); }
	static QString VertexPoint() { return QStringLiteral("顶点"); }
	static QString Slice() { return QStringLiteral("切片"); }
	
	static QString Fov() { return QStringLiteral("视野范围"); }
	static QString CameraType() { return QStringLiteral("相机类型"); }
	static QString PerspectiveCamera() { return QStringLiteral("透视相机"); }
	static QString OrthographicCamera() { return QStringLiteral("正交相机"); }
	static QString PanoramicCamera() { return QStringLiteral("全景相机"); }

	static QString BuiltInExpansion() { return QStringLiteral("内置展开"); }
	static QString CameraExpansion() { return QStringLiteral("基于相机的通用展开"); }
	static QString CustomExpansion() { return QStringLiteral("定制展开"); }
	static QString ModelSize() { return QStringLiteral("尺寸"); }
	static QString GeneratingSlices(){ return QStringLiteral("生成切片"); }
	static QString SliceExpansion() {	return QStringLiteral("切片展开");	}
	static QString SpecifyingCameras() {return QStringLiteral("指定相机");}
	static QString CustomFile() { return QStringLiteral("定制文件"); }
	static QString GeneratingSucceed() { return QStringLiteral("切片数据已生成，可在输入视图和切片列表中查看。"); }
	static QString GeneratingFail() { return QStringLiteral("切片数据生成失败，请检查后重试。"); }
	static QString Param() { return QStringLiteral("参数"); }

	static QString Direction() {return QStringLiteral("方向");}
	static QString Lightintensity() {return QStringLiteral("光强");}
	static QString Range() {return QStringLiteral("范围");}
	static QString ConvergenceFactor() {return QStringLiteral("汇聚因子");}
	static QString LightProperties() {return QStringLiteral("光源属性");}
	static QString LightColor() { return QStringLiteral("颜色"); }

	static QString ToCameraView() { return QStringLiteral("主视角转至相机视角"); }
	static QString ToMainView() { return QStringLiteral("相机视角移动至主视角"); }

	static QString LinkFile() { return QStringLiteral("链接关系文件"); }

	static QString PlayStart(){ return QStringLiteral("播放位置"); }
	static QString PlayDuration() { return QStringLiteral("播放时长"); }
	static QString ClipBegin() { return QStringLiteral("截取起点"); }
	static QString ClipEnd() { return QStringLiteral("截取终点"); }
	static QString Volumn() { return QStringLiteral("音量"); }

	static QString PlayEndAcction() { return QStringLiteral("播放结束动作"); }
	static QString CycleToPlay() { return QStringLiteral("循环播放"); }
	static QString StopToPlay() { return QStringLiteral("停止播放"); }
	static QString PauseAtLastFrame() { return QStringLiteral("定格在最后一帧"); }
	static QString DeleteTail() { return QStringLiteral("删除无素材结尾"); }
	static QString ObserveObj() { return QStringLiteral("目标物体"); }
	static QString LightTrackObj() { return QStringLiteral("目标物体"); }
	static QString None() { return QStringLiteral("无"); }


	//相机
	static QString CameraViewShift() { return QStringLiteral("视角转换"); }
	static QString MainCamera() { return QStringLiteral("主相机"); }
	static QString IsMainCamera() { return QStringLiteral("是"); }
	static QString NotMainCamera() { return QStringLiteral("否"); }
	
	//生成切片
	static QString Generate() { return QStringLiteral("生成"); }
	static QString GenerateSliceTip() { return QStringLiteral("对象将按设置生成切片，请谨慎操作"); }
	static QString OutputSlice() { return QStringLiteral("输出切片"); }
	static QString AppointedCamera() { return QStringLiteral("指定相机"); }
	static QString SliceCustomFile() { return QStringLiteral("定制文件"); }
	static QString InputSliceUnwrapAlgor() { return QStringLiteral("输入切片展开算法"); }
	static QString CoverSliceData() { return QStringLiteral("根据模型内置UV展开并覆盖原有切片数据"); }
	static QString SliceSplit() { return QStringLiteral("切片分割"); }
	static QString SliceSplitDesc() { return QStringLiteral("允许自动分割输出切片获得最佳展开效果"); }
	static QString AccordingInputPreviewSlice() { return QStringLiteral("根据输入预览切片生成"); }
	static QString AccordingInputPreviewSliceAndCoverData() { return QStringLiteral("根据预览阶段的数据生成并覆盖原有切片数据"); }
	static QString AccordingFileAndCoverData() { return QStringLiteral("根据文件覆盖原有切片数据"); }
	static QString FailAlgorFile() { return QStringLiteral("算法文件无法读取/不匹配，请检查文件"); }


	//模型属性
	static QString PreviewSlice() { return QStringLiteral("预览切片底图"); }
	static QString UnfoldType() { return QStringLiteral("展开类型"); }
	static QString PerspUnfold() { return QStringLiteral("透视展开"); }
	static QString OrthoUnfold() { return QStringLiteral("正交展开"); }
	static QString VR21Unfold() { return QStringLiteral("2:1VR全景展开"); }
	static QString SingleFisheyeUnfold() { return QStringLiteral("单鱼眼全景展开"); }
	static QString DualFisheyeUnfold() { return QStringLiteral("双鱼眼全景展开"); }
	static QString SkyboxUnfold() { return QStringLiteral("天空盒全景展开"); }

	//屏幕属性
	static QString ScreenConnectRel() { return QStringLiteral("连接关系"); }
	static QString ScreenState() { return QStringLiteral("屏幕状态"); }
	static QString ScreenFileLoadFail() { return QStringLiteral("数据解析错误，请检查后重新添加"); }

	//切片属性
	static QString SliceRotationCenter() { return QStringLiteral("旋转中心"); }
	static QString SliceSource() { return QStringLiteral("切片来源"); }
	static QString AlgorithmGenerated() { return QStringLiteral("算法生成"); }
	static QString ManualGenerated() { return QStringLiteral("手动生成"); }
	

	// VR 相机
	static QString VR_DeviceState() { return QStringLiteral("设备状态"); }
	static QString VR_SessionState() { return QStringLiteral("会话状态"); }
	static QString VR_BaseCamrea() { return QStringLiteral("基准相机"); }
	static QString VR_CamreaMoveType() { return QStringLiteral("移动方式"); }
	static QString VR_CamreaMoveStep() { return QStringLiteral("灵敏度"); }
	static QString VR_CamreaHorMove() { return QStringLiteral("水平移动"); }
	static QString VR_CamreaFreeMove() { return QStringLiteral("自由移动"); }
	static QString VR_VRCameraState() { return QStringLiteral("VR状态"); }
	static QString VR_VRCameraStart() { return QStringLiteral("启动"); }
	static QString VR_VRCameraEnd() { return QStringLiteral("停止"); }
	static QString VR_VRCameraDirection() { return QStringLiteral("VR方向"); }
	static QString VR_VRCameraReset() { return QStringLiteral("重置"); }
	static QString VR_VRCameraConnecting() { return QStringLiteral("连接中..."); }
	static QString VR_VRCameraReConnTip() { return QStringLiteral("请先启动 BusinessStreaming，重新连接"); }
	static QString VR_VRCameraPosLimit() { return QStringLiteral("位置限制"); }
	static QString VR_VRCameraPosLimitTip() { return QStringLiteral("激活坐标轴，将限制 VR 相机的可移动范围"); }
	static QString VR_VRCameraBaseCameraTip() { return QStringLiteral("基准相机的坐标需在位置限制范围内"); }

	// 环境属性
	static QString EnvProperty() { return QStringLiteral("环境属性"); }
	// 环境光色温
	static QString EnvLightColorTemperature() { return QStringLiteral("环境光色温"); }
	// 环境光强度
	static QString EnvLightIntensity() { return QStringLiteral("环境光强度"); }
	// 目标亮度一致
	static QString TargetBrightnessConsistent() { return QStringLiteral("目标亮度一致"); }
	// 强度
	static QString Intensity() { return QStringLiteral("强度"); }

	//媒体属性
	static QString EffectProperty() { return QStringLiteral("效果属性"); }
	static QString FillType() { return QStringLiteral("填充模式"); }
	static QString FullWindow() { return QStringLiteral("全屏"); }
	static QString KeepRatio() { return QStringLiteral("保持比例"); }
	static QString NoScale() { return QStringLiteral("无缩放"); }
	static QString ScaleAspectLock() { return QStringLiteral("锁定宽高比"); }
	static QString ScaleAspectUnlock() { return QStringLiteral("解锁宽高比"); }

	static QString PPTPlayingMode() { return QStringLiteral("Playback mode"); }

	static QString TimePos() { return QStringLiteral("位置"); }
	static QString TimeCode() { return QStringLiteral("时间码"); }
	static QString TagSelect() { return QStringLiteral("Tag选择"); }

	static QString ETFT_Fade() { return QStringLiteral("淡入淡出"); }
	static QString ETFT_LeftMove() { return QStringLiteral("左移"); }
	static QString ETFT_RightMove() { return QStringLiteral("右移"); }
	static QString ETFT_UpMove() { return QStringLiteral("上移"); }
	static QString ETFT_DownMove() { return QStringLiteral("下移"); }
	static QString ETFT_LeftTopMove() { return QStringLiteral("左上移"); }
	static QString ETFT_LeftBottomMove() { return QStringLiteral("左下移"); }
	static QString ETFT_RightTopMove() { return QStringLiteral("Right Top Move"); }
	static QString ETFT_RightBottomMove() { return QStringLiteral("Right Bottom Move"); }
	static QString ETFT_LeftCover() { return QStringLiteral("Left Cover"); }
	static QString ETFT_TopCover() { return QStringLiteral("Top Cover"); }
	static QString ETFT_RightCover() { return QStringLiteral("Right Cover"); }
	static QString ETFT_BottomCover() { return QStringLiteral("Bottom Cover"); }
	static QString ETFT_LeftTopCover() { return QStringLiteral("Left Top Cover"); }
	static QString ETFT_RightTopCover() { return QStringLiteral("Right Top Cover"); }
	static QString ETFT_LeftBottomCover() { return QStringLiteral("Left Bottom Cover"); }
	static QString ETFT_RightBottomCover() { return QStringLiteral("Right Bottom Cover"); }
	static QString ETFT_LeftTopObliqueCover() { return QStringLiteral("Left Top Oblique Cover"); }
	static QString ETFT_RightTopObliqueCover() { return QStringLiteral("Right Top Oblique Cover"); }
	static QString ETFT_LeftBottomObliqueCover() { return QStringLiteral("Left Bottom Oblique Cover"); }
	static QString ETFT_RightBottomObliqueCover() { return QStringLiteral("Right Bottom Oblique Cover"); }
	static QString ETFT_GrillesHorizontal() { return QStringLiteral("Horizontal Blinds"); }
	static QString ETFT_GrillesVertical() { return QStringLiteral("Vertical Blinds"); }
	static QString ETFT_LeftRightOpen() { return QStringLiteral("Left Right Open"); }
	static QString ETFT_TopBottomOpen() { return QStringLiteral("Top Bottom Open"); }
	static QString ETFT_LeftRightClose() { return QStringLiteral("Left Right Close"); }
	static QString ETFT_TopBottomClose() { return QStringLiteral("Top Bottom Close"); }
	static QString ETFT_MosaicLarge() { return QStringLiteral("Mosaic Large"); }
	static QString ETFT_MosaicMiddle() { return QStringLiteral("Mosaic Middle"); }
	static QString ETFT_MosaicSmall() { return QStringLiteral("Mosaic Small"); }
	static QString ETFT_RightRotFull() { return QStringLiteral("Rotate Right 360"); }
	static QString ETFT_RightRotHalf() { return QStringLiteral("Rotate Right 180"); }
	static QString ETFT_RightRotQuarter() { return QStringLiteral("Rotate Right 90"); }
	static QString ETFT_LeftRotFull() { return QStringLiteral("Rotate Left 360"); }
	static QString ETFT_LeftRotHalf() { return QStringLiteral("Rotate Left 180"); }
	static QString ETFT_LeftRotQuarter() { return QStringLiteral("Rotate Left 90"); }
	static QString ETFT_SmallToLargeCenter() { return QStringLiteral("Small To Large Center"); }
	static QString ETFT_SmallToLargeLeftTop() { return QStringLiteral("Small To Large Left Top"); }
	static QString ETFT_SmallToLargeLeftBottom() { return QStringLiteral("Small To Large Left Bottom"); }
	static QString ETFT_SmallToLargeRightTop() { return QStringLiteral("Small To Large Right Top"); }
	static QString ETFT_SmallToLargeRightBottom() { return QStringLiteral("Small To Large Right Bottom"); }
	static QString ETFT_CenterToBorderRect() { return QStringLiteral("Center To Border Rect"); }
	static QString ETFT_BorderToCenterRect() { return QStringLiteral("Border To Center Rect"); }
	static QString ETFT_CenterToBorderLozenge() { return QStringLiteral("Center To Border Lozenge"); }
	static QString ETFT_BorderToCenterLozenge() { return QStringLiteral("Border To Center Lozenge"); }
	static QString ETFT_CenterToBorderCross() { return QStringLiteral("Center To Border Cross"); }
	static QString ETFT_BorderToCenterCross() { return QStringLiteral("Border To Center Cross"); }
	static QString ETFT_PerspectiveRotationLeft() { return QStringLiteral("Perspective Rotation Left"); }
	static QString ETFT_PerspectiveRotationRight() { return QStringLiteral("Perspective Rotation Right"); }
	static QString ETFT_PerspectiveRotationTop() { return QStringLiteral("Perspective Rotation Top"); }
	static QString ETFT_PerspectiveRotationBottom() { return QStringLiteral("Perspective Rotation Bottom"); }
};

class CCameraRecordTranslate : public QObject
{
	Q_OBJECT
public:
	static QString CameraRecordWnd() { return QStringLiteral("相机录制"); }
	static QString StartRecording() { return QStringLiteral("开始录制"); }
	static QString StopRecording() { return QStringLiteral("停止录制"); }
	static QString Screenshot() { return QStringLiteral("截图"); }
	static QString RecordingMode() { return QStringLiteral("录制模式"); }
	static QString Manual() { return QStringLiteral("手动"); }
	static QString TimelineSettings() { return QStringLiteral("时间线设置"); }
	static QString VideoSettings() { return QStringLiteral("画面设置"); }
	static QString OutputSettings() { return QStringLiteral("输出设置"); }
	static QString OpenRecordedFile() { return QStringLiteral("打开文件夹"); }
	static QString NoCameraAvailableTip() { return QStringLiteral("当前无可用相机，请先创建相机！"); }
	static QString SavePathInvalidTip() { return QStringLiteral("保存路径无效，请重新设置"); }
	static QString NoDongleWatermarkTip() { return QStringLiteral("系统未检测到加密狗，所有截图和录制内容将自动加水印。请连接加密狗以移除水印！"); }
	static QString ScreenshotSaved() { return QStringLiteral("截图已保存"); }
	static QString RecordingSaved() { return QStringLiteral("录制已保存"); }
	static QString TimelineRecordLockedTip() { return QStringLiteral("当前正在时间线录制中，为保证录制完整，禁止操作时间线"); }
	static QString RecordingLockedTip() { return QStringLiteral("当前正在相机录制中，请先手动停止录制再操作！"); }
	static QString Recording() { return QStringLiteral("录制中"); }
	static QString ManualMode() { return QStringLiteral("手动模式"); }
	static QString TimelineMode() { return QStringLiteral("时间线模式"); }
	static QString RecordingFailed() { return QStringLiteral("录制失败！"); }
	static QString RecordingFailedDiskTip() { return QStringLiteral("录制失败，请检查保存路径或磁盘空间！"); }
	static QString ScreenshotFailed() { return QStringLiteral("截图失败！"); }
	static QString ScreenshotFailedDiskTip() { return QStringLiteral("截图失败，请检查保存路径或磁盘空间！"); }
	static QString Timestamp() { return QStringLiteral("时间戳"); }
	static QString Show() { return QStringLiteral("显示"); }
	static QString Hide() { return QStringLiteral("不显示"); }
};

class CNetworkStatusTranslate : public QObject
{
	Q_OBJECT
public:
	static QString NetworkStatusWnd() { return QStringLiteral("网络状态"); }
	static QString NoDevice() { return QStringLiteral("暂无连接设备"); }
	static QString AutoSyncAllMaterials() { return QStringLiteral("自动同步所有素材"); }
	static QString ClearRedundantCache() { return QStringLiteral("清除冗余缓存"); }
};

class CTimeCodeRecvTranslate : public QObject
{
	Q_OBJECT
public:
	static QString TimeCodeRecvWnd() { return QStringLiteral("时间码接收"); }
	static QString NoDevice() { return QStringLiteral("无设备"); }
	static QString NotReceiving() { return QStringLiteral("未接收"); }
	static QString FollowingTimeCode() { return QStringLiteral("追随时间码"); }
	static QString NotControlled() { return QStringLiteral("未控制"); }
	static QString Disconnected() { return QStringLiteral("脱离"); }
	static QString Interactive() { return QStringLiteral("互动"); }
	static QString TimeCode() { return QStringLiteral("时间码"); }
	static QString LTC() { return QStringLiteral("LTC"); }
	static QString MTC() { return QStringLiteral("MTC"); }
	static QString Monitor() { return QStringLiteral("监听"); }
	static QString Control() { return QStringLiteral("控制"); }
	static QString InputDevice() { return QStringLiteral("输入设备"); }
	static QString NoneDevice() { return QStringLiteral("无"); }
	static QString FrameRate() { return QStringLiteral("帧率"); }
	static QString FpsMismatch() { return QStringLiteral("帧率不一致"); }
	static QString TimeCodeOffset() { return QStringLiteral("时间码偏移"); }
	static QString PlaybackMode() { return QStringLiteral("播放模式"); }
	static QString TimeCodeLoss() { return QStringLiteral("时间码丢失"); }
	static QString LoopSection() { return QStringLiteral("循环Section"); }
	static QString SingleSection() { return QStringLiteral("播放到Section末尾"); }
	static QString ContinuePlayback() { return QStringLiteral("继续播放"); }
	static QString StopOnLoss() { return QStringLiteral("停止播放"); }
};

class CTimeLineWndTranslate :public QObject
{
	Q_OBJECT
public:
	
	static QString TimeLineWnd() { return QStringLiteral("时间线"); }

	static QString LocateNonius() { return QStringLiteral("定位到游标"); }
	static QString DeleteTip() { return QStringLiteral("删除后，该图层内的媒体将被移除"); }
	static QString TimeType() { return QStringLiteral("格式"); }
	static QString PreKeyFrame() { return QStringLiteral("上一帧"); }
	static QString NextKeyFrame() { return QStringLiteral("下一帧"); }
	static QString ZoomSamll() { return QStringLiteral("缩小"); }
	static QString ZoomBig() { return QStringLiteral("放大"); }

	static QString ResourecLoacl() { return QStringLiteral("素材库定位"); }
	static QString ViewLoacl() { return QStringLiteral("视图区定位"); }

	static QString LockUnlock(){ return QStringLiteral("锁定/解锁"); }
	static QString HideShow() { return QStringLiteral("隐藏/显示"); }
	static QString MuteUnmute() { return QStringLiteral("静音/取消静音"); }

	static QString InsertLastLayer() { return QStringLiteral("向上插入图层"); }
	static QString InsertNextLayer() { return QStringLiteral("向下插入图层"); }
	static QString ClearLayerObjects() { return QStringLiteral("清空当前图层媒体"); }

	static QString Layer() { return QStringLiteral("图层"); }

	static QString AddLayer() { return QStringLiteral("添加图层"); }

	static QString PlayPause() { return QStringLiteral("播放/暂停"); }
	static QString Stop() { return QStringLiteral("停止"); }
	static QString StopByStageChange() { return QStringLiteral("阶段切换"); }

	static QString Seek() { return QStringLiteral("跳转"); }

	static QString TimeLineWndPlayControl() { return QStringLiteral("播控工具栏"); }
	static QString TimeLineWndTop() { return QStringLiteral("时间线工具栏"); }
	static QString TimeLineWndViewLeftTop() { return QStringLiteral("播控工具栏"); }
	static QString TimeLineWndViewLayers() { return QStringLiteral("图层标题栏"); }
	static QString TimeLineWndViewLeftBottom() { return QStringLiteral("图层操作栏"); }
	static QString TimeLineWndViewTimeLineBar() { return QStringLiteral("时间刻度条"); }
	static QString TimeLineWndViewContent() { return QStringLiteral("媒体轨道栏"); }
	static QString TimeLineWndTag() { return QStringLiteral("Tag栏"); }

	static QString MoveObject() { return QStringLiteral("移动媒体"); }
	static QString ScaleObject() { return QStringLiteral("缩放媒体"); }

	static QString SplitSection() { return QStringLiteral("拆分"); }
	static QString MergeSection() { return QStringLiteral("合并"); }
	static QString InsertTime() { return QStringLiteral("插入时间"); }
	static QString RemoveTime() { return QStringLiteral("移除时间"); }
	static QString InsertTimeDlgTitle() { return QStringLiteral("插入时间"); }
	static QString RemoveTimeDlgTitle() { return QStringLiteral("移除时间"); }
	static QString TimelinePosition() { return QStringLiteral("时间线位置"); }
	static QString InsertLength() { return QStringLiteral("插入长度"); }
	static QString RemoveLength() { return QStringLiteral("移除长度"); }
	static QString InsertMode() { return QStringLiteral("插入模式"); }
	static QString RemoveMode() { return QStringLiteral("移除模式"); }
	static QString TimeEditLayersOnly() { return QStringLiteral("仅图层"); }
	static QString TimeEditLayersAndSection() { return QStringLiteral("图层和Section"); }
	static QString TimeEditNoClipAtPosition() { return QStringLiteral("插入位置没有素材，无法执行仅图层插入。"); }
	static QString TimeEditInvalidParam() { return QStringLiteral("时间参数无效，请检查位置和长度。"); }
	static QString ScaleSection() { return QStringLiteral("缩放Section"); }
	static QString ScaleSectionTip() { return QStringLiteral("按下Alt键拖动"); }
	static QString InvalidPos() { return QStringLiteral("位置不合理"); }
	static QString PathClipOverlapTip() { return QStringLiteral("相机动画片段禁止重叠"); }

	static QString EditTagPos() { return QStringLiteral("修改Tag位置"); }
	static QString EditTagTCPos() { return QStringLiteral("修改Tag时间码"); }

	// 播控按钮
	static QString Play() { return QStringLiteral("播放"); }
	static QString SectionEndPlay() { return QStringLiteral("播放到Section末尾"); }
	static QString SectionLoopPlay() { return QStringLiteral("循环Section"); }
	static QString Pause() { return QStringLiteral("停止/暂停播放"); }
	static QString PreSection() { return QStringLiteral("上一Section"); }
	static QString NextSection() { return QStringLiteral("下一Section"); }
	static QString BackToStart() { return QStringLiteral("返回起始点"); }

	static QString CtrlAlignSection(){ return QStringLiteral("Ctrl: 对齐Section"); }
	static QString AltNewLayerTip(){ return QStringLiteral("Alt: 新建图层"); }

	static QString LinearKeyFrameCurve(){ return QStringLiteral("直线"); }
	static QString AutoBezierKeyFrameCurve() { return QStringLiteral("自动曲线"); }
	static QString FreeBezierKeyFrameCurve() { return QStringLiteral("自由曲线"); }

	static QString ShowKeyFrame() { return QStringLiteral("显示关键帧"); }
	static QString HideKeyFrame() { return QStringLiteral("隐藏关键帧"); }
};

class CShortcutTranslate : public QObject
{
	Q_OBJECT
public:
	static QString Shortcut() { return QStringLiteral("快捷键列表"); }

	static QString Interface() { return QStringLiteral("界面"); }
	static QString Perspective() { return QStringLiteral("视角"); }
	static QString Object() { return QStringLiteral("对象"); }
	static QString Slice() { return QStringLiteral("切片"); }
	static QString TimeLine() { return QStringLiteral("时间线"); }

	static QString OpenSetting() { return QStringLiteral("打开设置"); }
	static QString NewProject() { return QStringLiteral("新建工程"); }
	static QString OpenProject() { return QStringLiteral("打开工程"); }
	static QString SaveProject() { return QStringLiteral("保存工程"); }
	static QString Redo() { return QStringLiteral("撤销"); }
	static QString Undo() { return QStringLiteral("恢复"); }
	static QString OpenScreen() { return QStringLiteral("开屏"); }
	static QString CloseScreen() { return QStringLiteral("关屏"); }


	static QString Grid() { return QStringLiteral("舞台网格"); }
	static QString ResetViewport() { return QStringLiteral("重置视角"); }
	static QString TranslateViewport() { return QStringLiteral("平移视角"); }
	static QString RotateViewport() { return QStringLiteral("旋转视角"); }
	static QString ZoomViewport() { return QStringLiteral("缩放视角"); }
	static QString SwitchViewMode() { return QStringLiteral("切换透视/正交视图"); }
	static QString TopView() { return QStringLiteral("顶视图"); }
	static QString BottomView() { return QStringLiteral("底视图"); }
	static QString FrontView() { return QStringLiteral("前视图"); }
	static QString BackView() { return QStringLiteral("后视图"); }
	static QString LeftView() { return QStringLiteral("左视图"); }
	static QString RightView() { return QStringLiteral("右视图"); }
	static QString ZoomInView() { return QStringLiteral("放大视图"); }
	static QString ZoomOutView() { return QStringLiteral("缩小视图"); }
	static QString MoveLeftViewport() { return QStringLiteral("右移视角"); }
	static QString MoveRightViewport() { return QStringLiteral("左移视角"); }
	static QString MoveTopViewport() { return QStringLiteral("上移视角"); }
	static QString MoveBottonViewport() { return QStringLiteral("下移视角"); }
	static QString RedoView() { return QStringLiteral("恢复视图"); }
	static QString UndoView() { return QStringLiteral("撤销视图"); }

	static QString SelectionTool() { return QStringLiteral("选择工具"); }
	static QString TranslationTool() { return QStringLiteral("移动工具"); }
	static QString RotationTool() { return QStringLiteral("旋转工具"); }
	static QString ScaleTool() { return QStringLiteral("缩放工具"); }
	static QString Cursor() { return QStringLiteral("鼠标光标"); }
	static QString CheckAll() { return QStringLiteral("全选"); }
	static QString Move_along_the_X_axis() { return QStringLiteral("沿X轴移动"); }
	static QString Move_along_the_Y_axis() { return QStringLiteral("沿Y轴移动"); }
	static QString Move_along_the_Z_axis() { return QStringLiteral("沿Z轴移动"); }
	static QString Rotate_along_the_X_axis() { return QStringLiteral("沿X轴旋转"); }
	static QString Rotate_along_the_Y_axis() { return QStringLiteral("沿Y轴旋转"); }
	static QString Rotate_along_the_Z_axis() { return QStringLiteral("沿Z轴旋转"); }
	static QString Scale_along_the_X_axis() { return QStringLiteral("沿X轴缩放"); }
	static QString Scale_along_the_Y_axis() { return QStringLiteral("沿Y轴缩放"); }
	static QString Scale_along_the_Z_axis() { return QStringLiteral("沿Z轴缩放"); }

	static QString AddPoint() { return QStringLiteral("增加顶点"); }
	static QString QuickCutUp() { return QStringLiteral("快速向上分割"); }
	static QString QuickCutDown() { return QStringLiteral("快速向下分割"); }
	static QString QuickCutLeft() { return QStringLiteral("快速向左分割"); }
	static QString QuickCutRight() { return QStringLiteral("快速向右分割"); }
	static QString SliceCut() {return QStringLiteral("切片分割");}
	static QString SliceMerge() { return QStringLiteral("切片合并"); }
	static QString PartialMultiSelectSlices() { return QStringLiteral("局部多选切片"); }
	static QString ModelRotationSnap() { return QStringLiteral("模型旋转吸附"); }
	static QString XYZAnchorRotation() { return QStringLiteral("X/Y/Z锚点旋转"); }
	static QString LeftMouseButtonBoxSelect() { return QStringLiteral("鼠标左键框选"); }

	static QString MiddleMouseButton() { return QStringLiteral("鼠标中键"); }
	static QString RightMouseButton() { return QStringLiteral("鼠标右键"); }
	static QString LeftMouseButton() { return QStringLiteral("鼠标左键"); }
	static QString MouseWheel() { return QStringLiteral("鼠标滚轮"); }
	static QString Keypad() { return QStringLiteral("小键盘"); }

	static QString MoveTimeLineObject() { return QStringLiteral("移动素材"); }

	static QString PlayPause() { return QStringLiteral("播放/暂停"); }
	static QString DeleteTimeLineObject() { return QStringLiteral("删除素材"); }
	static QString VerMoveTimeLineObject() { return QStringLiteral("纵向移动素材"); }
	static QString ZoomTimeLineView() { return QStringLiteral("放缩视图"); }
	static QString VerMoveTimeLineView() { return QStringLiteral("垂直移动视图"); }
	static QString HorMoveTimeLineView() { return QStringLiteral("水平移动视图"); }

	static QString AddTagAtCurTime() { return QStringLiteral("在当前时间添加标记"); }
	static QString SplitSectionAtCurTime() { return QStringLiteral("在当前时间拆分章节"); }
	static QString MergeSectionAtCurTime() { return QStringLiteral("合并章节"); }
	static QString SeekTimelinePrevFrame() { return QStringLiteral("上一帧"); }
	static QString SeekTimelineNextFrame() { return QStringLiteral("下一帧"); }
	static QString LocateNoniusAtThird() { return QStringLiteral("定位游标到可视区域"); }
	static QString SeekToSelectedObjectHead() { return QStringLiteral("跳转到选中素材头部"); }
	static QString SeekToSelectedObjectTail() { return QStringLiteral("跳转到选中素材尾部"); }

	static QString Others() { return QStringLiteral("其他"); }
};

class CInitPrjDialogTranslate : public QObject
{
	Q_OBJECT
public:
	static QString EixstSame() { return QStringLiteral("当前路径下存在相同名称的工程！"); }
	static QString LoadPrjing() { return QStringLiteral("工程加载中..."); }
	static QString PrjNotExistTips() { return QStringLiteral("当前文件被删除或者重命名,请检查文件!"); }
	static QString AppExitPrjNotSaveTips() { return QStringLiteral("当前工程未保存,是否需要保存?"); }
	static QString AppExitScreenOpenTips() { return QStringLiteral("关闭软件前请先关闭屏幕!"); }
	static QString WhetherSaveProjectTips() { return QStringLiteral("当前工程文件未保存,是否需要保存?"); }

	static QString ProjectVersionErrorTips() { return QStringLiteral("当前文件版本（V%1）过低，V%2 版本暂不支持直接加载。请切换至对应的版本软件打开！"); }
	static QString ProjectErrorTips() { return QStringLiteral("文件错误！"); }

	static QString StartPage() { return QStringLiteral("启动页"); }
	static QString NewPrj() { return QStringLiteral("新建工程"); }
	static QString RecentProject() { return QStringLiteral("最近工程"); }
	static QString OpenFile() { return QStringLiteral("打开工程文件"); }
	static QString NotShowStartPage() { return QStringLiteral("不再显示启动页"); }
	static QString CreateProject() { return QStringLiteral("创建工程"); }
	static QString ProjectName() { return QStringLiteral("名称"); }
	static QString FrameRate() { return QStringLiteral("帧率"); }
	static QString PalceHolderTip() { return QStringLiteral("暂无最近打开的工程"); }
	static QString NoProjectFile() { return QStringLiteral("无工程文件"); }
	static QString OpenPrjFolder() { return QStringLiteral("打开所在文件夹"); }
	static QString DelPrjFile() { return QStringLiteral("删除工程文件"); }
	static QString DelPrjRecord() { return QStringLiteral("从列表中移除"); }
	static QString DelPrjFileTips() { return QStringLiteral("确定删除该工程文件吗？"); }
	static QString DelPrjRecordTips() { return QStringLiteral("确定从最近工程列表中移除吗？"); }

	static QString AppLoadingIntro() { return QStringLiteral("三维映射软件"); }
};

class CLogTranslate : public QObject
{
	Q_OBJECT
public:
	static QString OperationLog() { return QStringLiteral("操作日志"); }
	static QString Today() { return QStringLiteral("本日"); }
	static QString NearlyOneDay() { return QStringLiteral("近一日"); }
	static QString NearlyThreeDay() { return QStringLiteral("近三日"); }
	static QString NearlySevenDay() { return QStringLiteral("近七日"); }
	static QString Export() { return QStringLiteral("导出"); }
	static QString View() { return QStringLiteral("查看"); }
	static QString Clear() { return QStringLiteral("清空"); }
	static QString SummaryLine() { return QStringLiteral("合计%1条, 每页%2条"); }
	static QString FrontPage() { return QStringLiteral("首页"); }
	static QString LastPage() { return QStringLiteral("尾页"); }
	static QString PreviousPage() { return QStringLiteral("上一页"); }
	static QString NextPage() { return QStringLiteral("下一页"); }
	static QString Time() { return QStringLiteral("时间"); }
	static QString Module() { return QStringLiteral("模块"); }
	static QString Function() { return QStringLiteral("功能"); }
	static QString Describe() { return QStringLiteral("描述"); }
	static QString Controller() { return QStringLiteral("控制端"); }
	static QString ExportLog() { return QStringLiteral("日志导出"); }
	static QString LogFileFilter() { return QStringLiteral("日志(*.csv)"); }
	static QString SerialNumber() { return QStringLiteral("序号"); }
	static QString ClearDataEmpty() { return QStringLiteral("查询的日志记录为空!"); }
	static QString ClearDataWarning() { return QStringLiteral("是否要清空本次查询到的日志记录?清空记录后不可恢复!"); }
	static QString ViewLog() { return QStringLiteral("查看"); }
	static QString ImportDataErrorTips() { return QStringLiteral("导入数据为空,请检查文件是否有效!"); }

	static QString PCController() { return QStringLiteral("PC"); }
	static QString APPController() { return QStringLiteral("APP"); }
	static QString FPController() { return QStringLiteral("前面板"); }
	static QString CenterController() { return QStringLiteral("中控"); }
	static QString CloudController() { return QStringLiteral("云控"); }

	static QString LogViewPlaceHolderText() { return QStringLiteral("当前无内容"); }
};

class CDongleTranslate :public QObject
{
	Q_OBJECT
public:
	static QString Title() { return (QStringLiteral("授权许可异常")); }
	static QString Retry() { return (QStringLiteral("重试")); }
	static QString TryOut() { return (QStringLiteral("试用")); }
	static QString OverLoad() { return (QStringLiteral("实际带载大于授权带载!")); }
	static QString OverLoadTip() { return (QStringLiteral("实际带载: %1\n授权带载: %2")); }
	static QString Nofound() { return (QStringLiteral("本机没有找到加密锁!")); }
	static QString TryTip() { return (QStringLiteral("加密锁没有正常授权!")); }
	static QString Tip() { return (QStringLiteral("需要加密锁正确授权才能正常使用本软件.")); }
	static QString Dongle() { return (QStringLiteral("加密锁:")); }
	static QString None() { return (QStringLiteral("无")); }
	static QString Normal() { return (QStringLiteral("正常")); }
	static QString Overload() { return (QStringLiteral("超载")); }
};

class CPackProjectTranslate : public QObject
{
	Q_OBJECT
public:
	static QString ProjectName() { return QStringLiteral("工程包名称"); }
	static QString Content() { return QStringLiteral("打包内容"); }
	static QString Mode() { return QStringLiteral("打包方式"); }
	static QString EmptyProNameTip() { return QStringLiteral("工程包名称不能为空!"); }
	static QString PackSuccessTip() { return QStringLiteral("打包成功"); }
	static QString PackFailTip() { return QStringLiteral("打包失败"); }
	static QString InvalidProNameTip() { return QStringLiteral("工程包名不合法!"); }
	static QString MkdirError() { return QStringLiteral("创建文件夹失败,请重试!"); }
	static QString Copy() { return QStringLiteral("拷贝"); }
	static QString Move() { return QStringLiteral("移动"); }
	static QString OnlyUse() { return QStringLiteral("仅已使用媒体和模型"); }
	static QString UseAll() { return QStringLiteral("所有媒体和模型"); }
	static QString StopPlayErrorTip() { return QStringLiteral("打包前请先停止播放!"); }
	static QString SameFilePackFailTip() { return QStringLiteral("打包失败，存在同名素材"); }

	static QString Packaging() { return QStringLiteral("打包中"); }
};

class CStreamMediaTranslate :public QObject
{
	Q_OBJECT
public:
	static QString Title() { return (QStringLiteral("流媒体")); }
	static QString PutInUrlTips() { return (QStringLiteral("请输入流媒体地址!")); }
	static QString Loading() { return (QStringLiteral("加载中")); }
	static QString LoadFailed() { return (QStringLiteral("加载失败!")); }
	static QString NoVideo() { return (QStringLiteral("无视频资源!")); }
	static QString NoVideoAudio() { return (QStringLiteral("无视频、音频资源!")); }
	static QString ValidUrlTips() { return (QStringLiteral("请输入正确的流媒体地址!")); }
	static QString AddExistUrlTips() { return (QStringLiteral("该流媒体已经被添加至素材库中!")); }
	static QString AddSportUrlTips() { return (QStringLiteral("支持rtsp、rtmp、http、https格式")); }

};

class CNdiTranslate :public QObject
{
	Q_OBJECT
public:
	static QString Title() { return (QStringLiteral("NDI视频源")); }
	static QString NdiExistSelectTips() { return (QStringLiteral("请选择:")); }
	static QString NoNdiExistSelectTips() { return (QStringLiteral("请选择(无NDI视频源):")); }
	static QString NoNdiBkTips() { return (QStringLiteral("无NDI源!")); }
	static QString NdiAdressInvalidTips() { return (QStringLiteral("NDI视频源地址有误,请检查!")); }
	static QString SourceName() { return (QStringLiteral("源名称")); }
};

class CCaptureTranslate : public QObject
{
	Q_OBJECT
public:
	static QString AudioCapture() { return QStringLiteral("音频采集"); }
	static QString VideoCapture() { return QStringLiteral("视频采集"); }
	static QString VideoDevice() { return QStringLiteral("视频设备"); }
	static QString AudioDevice() { return QStringLiteral("音频设备"); }
	static QString Audition() { return QStringLiteral("试听"); }
	static QString SelectOneDevice() { return QStringLiteral("请选择一个设备!"); }
	//static QString WindowStream() { return QStringLiteral("Capture_Window Streaming"); }
};

class CSubtitleTranslate : public QObject
{
	Q_OBJECT
public:
	static QString BeginTime() { return QStringLiteral("开始时间"); }
	static QString EndTime() { return QStringLiteral("结束时间"); }
	static QString TextShowDir() { return QStringLiteral("显示方向"); }
	static QString ImprotFile() { return QStringLiteral("导入文件"); }
	static QString ImprotFileTips() { return QStringLiteral("请导入字幕文件"); }
	static QString SupprtFileFormat() { return QStringLiteral("支持格式：lrc，srt，txt"); }
	static QString TextHor() { return QStringLiteral("横排"); }
	static QString TextVer() { return QStringLiteral("竖排"); }
	static QString Serial() { return QStringLiteral("序号"); }
	static QString Content() { return QStringLiteral("内容"); }
	static QString SubtitleDataLoading() { return QStringLiteral("字幕数据加载中"); }
	static QString TeleprompterDataLoading() { return QStringLiteral("提词器数据加载中"); }
};

//幻彩字属性界面翻译
class CColorTextPropertyFrameTranslate : public QObject
{
public:
	CColorTextPropertyFrameTranslate(QObject *parent = nullptr) : QObject(parent) {}
	//共用名称：颜色、不透明度
	static QString ColorShareName() { return QStringLiteral("颜色"); }
	static QString OpacityShareName() { return QStringLiteral("不透明度"); }
	//文本
	static QString TextPropertyGroupName() { return QStringLiteral("文本"); }
	static QString SelectFontText() { return QStringLiteral("字体"); }
	static QString TextEffectText() { return QStringLiteral("风格"); }
	static QStringList FontStyleBtnToolTip()
	{
		QStringList tipList;
		return tipList << QStringLiteral("粗体") << QStringLiteral("斜体")
			<< QStringLiteral("下划线") << QStringLiteral("左对齐") << QStringLiteral("居中对齐")
			<< QStringLiteral("右对齐") << QStringLiteral("顶对齐") << QStringLiteral("居中对齐")
			<< QStringLiteral("底对齐");
	}
	static QString AlignMentText() { return QStringLiteral("对齐方式"); }

	//幻彩字
	static QString ColorTextPropertyGroupName() { return QStringLiteral("幻彩效果"); }
	/*static QStringList ColorTextBtnToolTip()
	{

		QStringList tipList;
		return tipList << QStringLiteral("粗体") << QStringLiteral("斜体")<< QStringLiteral("下划线")
			<< QStringLiteral("左对齐") << QStringLiteral("居中对齐")<< QStringLiteral("右对齐")
			<< QStringLiteral("顶对齐") << QStringLiteral("居中对齐")<< QStringLiteral("底对齐")
			<< QStringLiteral("左对齐") << QStringLiteral("居中对齐") << QStringLiteral("右对齐")
			<< QStringLiteral("顶对齐") << QStringLiteral("居中对齐") << QStringLiteral("底对齐");

	}*/

	//填充
	static QString FillPropertyGroupName() { return QStringLiteral("填充"); }
	static QString FillModeComboBoxName() { return QStringLiteral("填充类型"); }
	static QString FillDirectionComboBoxName() { return QStringLiteral("方向"); }
	static QString FillRollComboBoxName() { return QStringLiteral("动画"); }
	static QString FillTextureComboBoxName() { return QStringLiteral("填充方式"); }
	static QString FillSpeedSliderName() { return QStringLiteral("速度"); }
	static QStringList FillModeItems()
	{
		QStringList scrollModeList;
		return 	scrollModeList << QStringLiteral("纯色填充")
			<< QStringLiteral("渐变填充")
			<< QStringLiteral("纹理填充");
	}
	static QStringList FillDirectionItems()
	{
		QStringList scrollModeList;
		return 	scrollModeList
			<< QStringLiteral("水平")
			<< QStringLiteral("竖直");
	}
	static QStringList FillGradientHSrollItems()
	{
		QStringList scrollModeList;
		return 	scrollModeList << QStringLiteral("静止")
			<< QStringLiteral("左移")
			<< QStringLiteral("右移")
			<< QStringLiteral("左右对开")
			<< QStringLiteral("左右闭合")
			<< QStringLiteral("顺时针")
			<< QStringLiteral("逆时针");

	}
	static QStringList FillGradientVSrollItems()
	{
		QStringList scrollModeList;
		return 	scrollModeList << QStringLiteral("静止")
			<< QStringLiteral("上移")
			<< QStringLiteral("下移")
			<< QStringLiteral("上下对开")
			<< QStringLiteral("上下闭合")
			<< QStringLiteral("顺时针")
			<< QStringLiteral("逆时针");
	}
	static QStringList FillTextureSrollItems()
	{
		QStringList scrollModeList;
		return 	scrollModeList << QStringLiteral("静止")
			<< QStringLiteral("左移")
			<< QStringLiteral("右移")
			<< QStringLiteral("上移")
			<< QStringLiteral("下移")
			<< QStringLiteral("左右对开")
			<< QStringLiteral("左右闭合")
			<< QStringLiteral("上下对开")
			<< QStringLiteral("上下闭合");
	}
	static QStringList FillTextureItems()
	{
		QStringList scrollModeList;
		return 	scrollModeList
			<< QStringLiteral("平铺")
			<< QStringLiteral("拉伸");
	}
	static QString FillGradientName() { return QStringLiteral("渐变"); }
	static QString FillChooseTexture() { return QStringLiteral("选择文件"); }
	static QString FillPreGradientName() { return QStringLiteral("纹理预设"); }
	static QString FillImportGradientName() { return QStringLiteral("导入纹理"); }
	static QString FillOpenTextrueFileBtnName() { return QStringLiteral("打开纹理文件"); }
	static QString FillTextrueWarning() { return QStringLiteral("纹理警告"); }
	static QString FillTip() { return QStringLiteral("动态特效开启时，播放模式仅支持静止模式"); }
	static QString FillImportTextip() { return QStringLiteral("纹理分辨率过高将导致卡顿"); }
	//描边
	static QString OutlinePropertyGroupName() { return QStringLiteral("描边"); }
	static QString OutlineThicknessLSpinboxName() { return QStringLiteral("粗细"); }

	//阴影
	static QString ShadePropertyGroupName() { return QStringLiteral("阴影"); }
	static QString ShadeAngleLSpinboxName() { return QStringLiteral("角度"); }
	static QString ShadeDistanceLSpinboxName() { return QStringLiteral("距离"); }
	static QString ShadeAmbiguitySliderName() { return QStringLiteral("模糊度"); }

	//背景
	static QString BackgroundTypeComboBoxName() { return QStringLiteral("类型"); }
	static QString BackgroundPropertyGroupName() { return QStringLiteral("背景"); }
	static QString BackgroundFilletSliderName() { return QStringLiteral("圆角"); }
	static QString BackgroundWidthSliderName() { return QStringLiteral("宽度"); }
	static QString BackgroundHeightSliderName() { return QStringLiteral("高度"); }
	static QStringList BackgroundTypeItems()
	{
		QStringList scrollModeList;
		return 	scrollModeList
			<< QStringLiteral("静态背景")
			<< QStringLiteral("动态背景1")
			<< QStringLiteral("动态背景2")
			<< QStringLiteral("动态背景3")
			<< QStringLiteral("动态背景4")
			<< QStringLiteral("动态背景5")
			<< QStringLiteral("动态背景6");

	}
	static QString BackgroundTip() { return QStringLiteral("动态背景开启时，播放模式仅支持静止模式"); }
	//滚动
	static QString ScrollGroupName() { return QStringLiteral("滚动"); }
	static QStringList ScrollModelItems()
	{
		QStringList scrollModeList;
		return 	scrollModeList << QStringLiteral("静止") << QStringLiteral("从左到右")
			<< QStringLiteral("从上到下")
			<< QStringLiteral("从右到左") << QStringLiteral("从下到上");
	}
	static QString EteBtnTip() { return QStringLiteral("首尾相接"); }
	static QString InterValText() { return QStringLiteral("间隔"); }
	static QString ScrollSpeedText() { return QStringLiteral("速度"); }
};

class CFxPropertyTranslate : public QObject
{
	Q_OBJECT
public:
	static QString FxProperty() { return QStringLiteral("特效属性"); }
	static QString FxWnd() { return QStringLiteral("特效库"); }

	static QString DeleteFx() { return QStringLiteral("删除特效"); }

	static QString Effect() { return QStringLiteral("特效库"); }

	static QString Reset() { return QStringLiteral("重置"); }

	static QString Visiable() { return QStringLiteral("启用"); }
	static QString Unvisiable() { return QStringLiteral("关闭"); }

	static QString Filter() { return QStringLiteral("滤色"); }

	static QString Nightvision() { return QStringLiteral("夜视"); }
	static QString NightvisionNoise() { return QStringLiteral("添加噪声"); }

	static QString Sharpen() { return QStringLiteral("锐化"); }
	static QString Degree() { return QStringLiteral("程度"); }
	static QString Frequency() { return QStringLiteral("频率"); }
	static QString Direction() { return QStringLiteral("方向"); }

	static QString Feather() { return QStringLiteral("羽化"); }
	static QString FeatherRadius() { return QStringLiteral("半径"); }

	static QString Mosaic() { return QStringLiteral("马赛克"); }
	static QString MosaicSize() { return QStringLiteral("单元格大小"); }
	static QString MosaicWidth() { return QStringLiteral("宽度"); }
	static QString MosaicLeft() { return QStringLiteral("左"); }
	static QString MosaicRight() { return QStringLiteral("右"); }
	static QString MosaicTop() { return QStringLiteral("上"); }
	static QString MosaicBottom() { return QStringLiteral("下"); }

	static QString Width() { return QStringLiteral("宽度"); }
	static QString Height() { return QStringLiteral("高度"); }

	static QString Denoise() { return QStringLiteral("降噪"); }
	static QString DenoiseType() { return QStringLiteral("降噪类型"); }
	static QString DenoiseGaussian() { return QStringLiteral("高斯降噪"); }
	static QString DenoiseSalt() { return QStringLiteral("椒盐降噪"); }

	static QString Blur() { return QStringLiteral("模糊"); }
	static QString Complement() { return QStringLiteral("反色"); }
	static QString HSV() { return QStringLiteral("色轮"); }
	static QString Emboss() { return QStringLiteral("浮雕"); }
	static QString LeftTop() { return QStringLiteral("左上"); }
	static QString Top() { return QStringLiteral("上"); }
	static QString RightTop() { return QStringLiteral("右上"); }
	static QString Right() { return QStringLiteral("右"); }
	static QString RightBottom() { return QStringLiteral("右下"); }
	static QString Bottom() { return QStringLiteral("下"); }
	static QString LeftBottom() { return QStringLiteral("左下"); }
	static QString Left() { return QStringLiteral("左"); }

	static QString Spherize() { return QStringLiteral("鱼眼"); }
	static QString Radius() { return QStringLiteral("半径"); }
	static QString ClaerOutSize() { return QStringLiteral("去除特效外部无效区域"); }
	static QString XOffset() { return QStringLiteral("X轴偏移量"); }
	static QString YOffset() { return QStringLiteral("Y轴偏移量"); }



	static QString Twirl() { return QStringLiteral("漩涡"); }
	static QString Rotate() { return QStringLiteral("角度"); }
	static QString Clockwise() { return QStringLiteral("顺时针"); }
	static QString Anticlockwise() { return QStringLiteral("逆时针"); }


	static QString Mirror() { return QStringLiteral("万花筒"); }
	static QString Cycle() { return QStringLiteral("周期"); }
	static QString EdgeOffset() { return QStringLiteral("边缘偏移"); }


	static QString Shadow() { return QStringLiteral("投影"); }
	static QString Transparency() { return QStringLiteral("不透明度"); }
	static QString Range() { return QStringLiteral("投影范围"); }
	static QString Zoomout() { return QStringLiteral("缩放"); }
	static QString Distance() { return QStringLiteral("距离"); }


	static QString Cut() { return QStringLiteral("裁剪"); }
	static QString CutType() { return QStringLiteral("裁剪方式"); }
	static QString ByRadio() { return QStringLiteral("按比例"); }
	static QString ByPixel() { return QStringLiteral("按像素"); }
	static QString Padding() { return QStringLiteral("填充"); }


	static QString Mask() { return QStringLiteral("蒙版"); }
	static QString None() { return QStringLiteral("无"); }
	static QString CustomLayer() { return QStringLiteral("自定义图层"); }
	static QString Triangle() { return QStringLiteral("三角形"); }
	static QString Flower() { return QStringLiteral("花形"); }
	static QString Lozenge() { return QStringLiteral("菱形"); }
	static QString Heart() { return QStringLiteral("心形"); }
	static QString Hexagon() { return QStringLiteral("六边形"); }
	static QString Ellipse() { return QStringLiteral("椭圆"); }
	static QString Pentagram() { return QStringLiteral("五角星"); }
	static QString RoundedRectangle() { return QStringLiteral("圆角矩形"); }
	static QString Circle() { return QStringLiteral("圆形"); }
	static QString MaskType() { return QStringLiteral("蒙版选项"); }
	static QString Reverse() { return QStringLiteral("反向"); }
	static QString Select() { return QStringLiteral("选择文件"); }


	static QString Keying() { return QStringLiteral("抠像"); }
	static QString Threshold() { return QStringLiteral("容差"); }

	static QString Frame() { return QStringLiteral("边框"); }
	static QString Marquees() { return QStringLiteral("跑马灯"); }
	static QString Scroll() { return QStringLiteral("循环滚动"); }
	static QString Speed() { return QStringLiteral("速度"); }
	static QString RightToLeft() { return QStringLiteral("从右往左"); }
	static QString LeftToRight() { return QStringLiteral("从左往右"); }
	static QString TopToBottom() { return QStringLiteral("从上往下"); }
	static QString BottomToTop() { return QStringLiteral("从下往上"); }

	static QString Shake() { return QStringLiteral("抖动"); }
	static QString Blink() { return QStringLiteral("闪屏"); }
	static QString Duration() { return QStringLiteral("持续时间"); }
	static QString Gradation() { return QStringLiteral("渐变"); }

	static QString DelAll() { return QStringLiteral("删除所有"); }
};

class CTextPropertyFrameTranslate : public QObject
{
public:
	CTextPropertyFrameTranslate(QObject *parent = nullptr) : QObject(parent) {}

	static QStringList FontStyleBtnToolTip()
	{
		QStringList tipList;
		return tipList << QStringLiteral("粗体") << QStringLiteral("斜体")
			<< QStringLiteral("下划线") << QStringLiteral("左对齐") << QStringLiteral("居中对齐")
			<< QStringLiteral("右对齐") << QStringLiteral("两端对齐") << QStringLiteral("顶对齐") << QStringLiteral("居中对齐")
			<< QStringLiteral("底对齐");
	}

	static QStringList WordsSpacingItems()
	{
		QStringList wordsSpacingList;
		return wordsSpacingList << QStringLiteral("默认");
	}

	static QStringList LineSpacingItems()
	{
		QStringList lineSpacingList;
		return lineSpacingList << QStringLiteral("单倍行距")
			<< QStringLiteral("1.5x行距") << QStringLiteral("双倍行距")
			<< QStringLiteral("最小值") << QStringLiteral("固定值") << QStringLiteral("多倍行距");
	}

	static QStringList ScrollModelItems()
	{
		QStringList scrollModeList;
		return 	scrollModeList << QStringLiteral("静止") << QStringLiteral("从左到右")
			<< QStringLiteral("从上到下")
			<< QStringLiteral("从右到左") << QStringLiteral("从下到上");
	}

	static QString FontColorBtnTip() { return QStringLiteral("字体颜色"); }
	static QString OutLineColorBtnTip() { return QStringLiteral("下划线颜色"); }
	static QString BackGroundBtnTip() { return QStringLiteral("背景颜色"); };
	static QString TransparentBtnTip() { return QStringLiteral("打开"); }
	static QString EteBtnTip() { return QStringLiteral("首尾相接"); }
	static QString FixedAreaBtnText() { return QStringLiteral("固定区域"); }
	static QString SizeGroupName() { return QStringLiteral("大小"); }
	static QString PosLabelText() { return QStringLiteral("位置"); }
	static QString SizeLabelText() { return QStringLiteral("大小"); }
	static QString TextPropertyGroupName() { return QStringLiteral("文本"); }
	static QString SelectFontText() { return QStringLiteral("字体"); }
	static QString FontSizeText() { return QStringLiteral("字体大小"); }
	static QString WordsSpacingName() { return QStringLiteral("字间距"); }
	static QString LineSpacingName() { return QStringLiteral("行间距"); }
	static QString TextEffectText() { return QStringLiteral("风格"); }
	static QString AlignMentText() { return QStringLiteral("对齐方式"); }
	static QString ColorPropertyGroupName() { return QStringLiteral("颜色"); }
	static QString BackGroundLabelText() { return QStringLiteral("背景"); }
	static QString ScrollGroupName() { return QStringLiteral("滚动"); }
	static QString InterValText() { return QStringLiteral("间隔"); }
	static QString ScrollSpeedText() { return QStringLiteral("速度"); }
	static QString DefaultText() { return QStringLiteral("请输入文本内容"); }
	static QString Value() { return QStringLiteral("数值"); }
	static QString Style() { return QStringLiteral("风格"); }
};

class CTeleprompterTranslate : public QObject
{
	Q_OBJECT
public:
	static QString Teleprompter() { return QStringLiteral("提词器"); }
	static QString TeleprompterDuration() { return QStringLiteral("持续时长"); }
	static QString TeleprompterContent() { return QStringLiteral("内容"); }
	static QString AddTeleprompterLine() { return QStringLiteral("插入行"); }
	static QString DelTeleprompterLine() { return QStringLiteral("删除行"); }
	static QString TeleprompterDefaultTip() { return QStringLiteral("请添加提词器内容"); }

};

class CExcelPropertyFrameTranslate : public QObject
{
	Q_OBJECT
public:
	static QString BaseSetting() { return QStringLiteral("基础设置"); }
	static QString PageSize() { return QStringLiteral("页面尺寸"); }
	static QString PlaySetting() { return QStringLiteral("播放设置"); }
	static QString PlayType() { return QStringLiteral("播放类型"); }
	static QString PageTurning() { return QStringLiteral("翻页"); }
	static QString Scroll() { return QStringLiteral("滚动"); }
	static QString PlayMode() { return QStringLiteral("播放模式"); }
	static QString ManualMode() { return QStringLiteral("手动模式"); }
	static QString AutoMode() { return QStringLiteral("自动模式"); }
	static QString PageTurningInterval() { return QStringLiteral("翻页间隔"); }
	static QString ScrollSpeed() { return QStringLiteral("滚动速度"); }
	static QString Second() { return QStringLiteral("秒"); }
};

class CAPPUserManagerTranslate : public QObject
{
	Q_OBJECT
public:
	static QString Title() { return QStringLiteral("账号管理"); }
	static QString Account() { return QStringLiteral("账号"); }
	static QString Password() { return QStringLiteral("密码"); }
	static QString Access() { return QStringLiteral("权限"); }		//权限
	static QString Admin() { return QStringLiteral("管理员"); }			//管理员
	static QString Guide() { return QStringLiteral("讲解员"); }			//讲解员
	static QString Operation() { return QStringLiteral("操作"); }
	static QString AddAccount() { return QStringLiteral("账号注册"); }
	static QString EditAccount() { return QStringLiteral("账号编辑"); }
	static QString AddCountTip() { return QStringLiteral("添加账号"); }
	static QString DeleteCountTip() { return QStringLiteral("删除账号"); }
	static QString AddCountNameError() { return QStringLiteral("请输入4-18位英文字母或数字"); }
	static QString AddCountPasswordError() { return QStringLiteral("请输入6-18位英文字母或数字"); }
	static QString UserisAlreadyExisted() { return QStringLiteral(" is Already Existed"); }
	static QString OverMaxUserCount() { return QStringLiteral("用户数量已达上限"); }
	static QString DeleteTip() { return QStringLiteral("Sure to delete: "); }
	static QString DeleteUserTip() { return QStringLiteral("Already login user can not Delete "); }
	static QString EditUserTip() { return QStringLiteral("Already login user can not Edit "); }
	static QString ResetTip() { return QStringLiteral("将重置所有设置项,是否继续?"); }
	static QString AccountErrorTip() { return QStringLiteral("账号名错误,"); }
	static QString PasswordErrorTip() { return QStringLiteral("密码错误,"); }


};

//////////////////////////////静止-从左到右-从上到下-从右到左-从下到上
static QString GetTextScrollModeTranslate(int nType)
{
	QString strScrollMode = "";
	switch (nType)
	{
	case 0:	//静止
		strScrollMode = QStringLiteral("静止");
		break;
	case 1: //从左到右
		strScrollMode = QStringLiteral("从左到右");
		break;
	case 2: //从上到下
		strScrollMode = QStringLiteral("从上到下");
		break;
	case 3: //从右到左
		strScrollMode = QStringLiteral("从右到左");
		break;
	case 4: //从下到上
		strScrollMode = QStringLiteral("从下到上");
		break;
	default:
		strScrollMode = QStringLiteral("静止");
		break;
	}

	return strScrollMode;
}

class CProjectSettingTranslate : public QObject
{
	Q_OBJECT
public:
	static QStringList LeftBtnsList()
	{
		QStringList list;
		/*return list << QStringLiteral("播放设置") << QStringLiteral("素材设置") << QStringLiteral("HDR设置")
			<< QStringLiteral("音频设置") << QStringLiteral("3D设置") << QStringLiteral("场景设置") << QStringLiteral("控制模式设置");*/
		return list << QStringLiteral("播放设置") << QStringLiteral("素材设置") << QStringLiteral("时间线设置");
	}

	static QStringList PlayPageComBoxList()
	{
		QStringList list;
		return list << QStringLiteral("素材信息")
			<< QStringLiteral("播放帧率")
			<< QStringLiteral("解码显卡")
			<< QStringLiteral("默认解码")
			<< QStringLiteral("播放结束动作")
			<< QStringLiteral("空格按键功能");
	}

	static QStringList ResourceInfoOptions()
	{
		QStringList list;
		return list << QStringLiteral("不显示") << QStringLiteral("显示");
	}

	static QStringList FPSOptions()
	{
		QStringList list;
		return list << "30FPS" << "50FPS" << "60FPS" << "120FPS" << "240FPS";
	}

	static QStringList DefaultDecodingOptions()
	{
		QStringList list;
		return list << QStringLiteral("软解码") << QStringLiteral("硬解码");
	}

	//static QString AutoFixPlayWnd() { return QStringLiteral("Auto Fixed Playing Window"); }
	static QString AutoSelectOption() { return QStringLiteral("自动选择(默认)"); }

	static QString DefaultResVolume() { return QStringLiteral("默认素材音量"); }
	static QString DefaultPicDuration() { return QStringLiteral("默认图片时长"); }
	static QString DefaultToolDuration() { return QStringLiteral("默认小工具时长"); }
	static QString DefaultTurnPageInterval() { return QStringLiteral("默认翻页间隔"); }
	static QString DefaulutStreamDruation() { return QStringLiteral("默认流媒体时长"); }
	static QString Reminder() { return QStringLiteral("温馨提示"); }
	static QString ReminderTip() { return QStringLiteral("修改的参数只影响后续添加对象!"); }


	static QString StartEnable() { return QStringLiteral("开启"); }
	static QString HDRDevice() { return QStringLiteral("HDR 设备"); }
	static QString SDRDevice() { return QStringLiteral("SDR 设备"); }

	static QString AudioDelay() { return QStringLiteral("音频输出延时"); }

	static QString DefaultFadeInTime() { return QStringLiteral("默认淡入时间"); }
	static QString DefaultFadeOutTime() { return QStringLiteral("默认淡出时间"); }

	static QString EyeSeparation() { return QStringLiteral("视差"); }
	static QString TimeInvalidWarnning() { return QStringLiteral(" is Illegal"); }

	static QString AppExitPrjNotSaveTips() { return QStringLiteral("当前工程未保存,是否需要保存?"); }

	static QString AppExitScreenOpenTips() { return QStringLiteral("关闭软件前请先关闭屏幕!"); }

	static QString LoadPrjing() { return QStringLiteral("工程加载中..."); }

	static QString OfficeAuto() { return QStringLiteral("默认PDF、Word自适应宽高"); }

	static QString CloudSync() { return QStringLiteral("云端同步中..."); }		//云端同步中......


	static QString PPTRenderEngine() { return QStringLiteral("PPT渲染引擎"); }
	static QString PPTViewMode() { return QStringLiteral("PPT动画技术"); }

	static QStringList PPTRenderEngineOp()
	{
		QStringList list;
		return list << QStringLiteral("自动") << "Office" << QStringLiteral("WPS(同一场景仅支持播放一个动画PPT)");
	}

	static QStringList PPTViewModeOp()
	{
		QStringList list;
		return list << QStringLiteral("注入模式") << QStringLiteral("截图模式") << QStringLiteral("窗口模式");
	}

	static QString AdapterNeedRestart() { return QStringLiteral("修改后需重启软件"); }
	static QString RestartTip() { return QStringLiteral("切换显卡或帧率需要重启软件才能生效,是否立即重启？"); }

	static QString SingleGpuMode() { return QStringLiteral("单显卡模式"); }
    static QString MultiGpuMode() { return QStringLiteral("多显卡模式"); }
	static QString PartialDecode() { return QStringLiteral("局部解码"); }
	static QString VirtualScreenDecode() { return QStringLiteral("虚拟屏解码"); }
	static QString EnableOption() { return QStringLiteral("开启"); }
	static QString DisableOption() { return QStringLiteral("禁用"); }
	static QStringList SpaceKeyFunctionOptions()
	{
		QStringList list;
		return list << QStringLiteral("时间线播放/暂停");
	}

	static QString TimeLineMode() { return QStringLiteral("时间线模式"); }
	static QString TimeCodeMode() { return QStringLiteral("时码模式"); }
	static QString FrameNumberMode() { return QStringLiteral("帧号模式"); }
	static QString FrameRate() { return QStringLiteral("帧率"); }
	static QString CameraAnimDuration() { return QStringLiteral("相机动画时长"); }
	static QString FpsInconsistentTip() { return QStringLiteral("播放帧率与时间线帧率不一致"); }
	static QStringList TimeLineFpsOptions()
	{
		QStringList list;
		return list << "24FPS" << "25FPS" << "29.97 DF" << "29.97 FPS" << "30FPS" << "60FPS";
	}
};

class CSoftwareSettingTranslate : public QObject
{
	Q_OBJECT
public:
	static QStringList LeftBtnsList()
	{
		QStringList list;
		return list << QStringLiteral("启动设置");
	}

	static QString StartupWithWindows() { return QStringLiteral("随 Windows 启动"); }
	static QString StartupDelay() { return QStringLiteral("延迟"); }
	static QString AutoOpenRecentProject() { return QStringLiteral("自动打开最近工程"); }
	static QString OpenAllScreensWhenOpenProject() { return QStringLiteral("打开工程时打开所有屏幕"); }
	static QString HideStartupPage() { return QStringLiteral("隐藏启动页"); }
	static QString AutoMinimizeMainWindow() { return QStringLiteral("自动最小化主窗口"); }
	static QString Seconds() { return QStringLiteral("秒"); }

	static QString TabName() { return QStringLiteral("锁屏设置"); }
	static QString InputPassword() { return QStringLiteral("输入密码"); }
	static QString InputPasswordPlaceholder() { return QStringLiteral("请输入密码"); }
	static QString LockScreenText() { return QStringLiteral("锁屏文字"); }
	static QString LockScreenTextPlaceholder() { return QStringLiteral("请输入文字"); }
	static QString LockScreenPicture() { return QStringLiteral("图片"); }
	static QString LockScreenPicOpacity() { return QStringLiteral("图片不透明度"); }
	static QString MaxLockPicTip() { return QStringLiteral("最多可添加16张图片"); }
	static QString PicExistTip() { return QStringLiteral("图片已存在"); }
	static QString PassWordTipInfo() { return QStringLiteral("LockScreen_PasswordError"); }
	static QString PasswordEmptyTip() { return QStringLiteral("请输入锁屏密码"); }
};

class CDisplayerTranslate : public QObject
{
	Q_OBJECT
public:
	static QString WindowTitlePrefix() { return QStringLiteral("GrandMapping Player"); }
	static QString AutoSelectAdapter() { return QStringLiteral("自动选择适配器"); }
	static QString On() { return QStringLiteral("开启"); }
	static QString Off() { return QStringLiteral("关闭"); }
	static QString NetworkAdapter() { return QStringLiteral("网络适配器"); }
	static QString ControllerName() { return QStringLiteral("控制端名称"); }
	static QString Remote() { return QStringLiteral("远端"); }
	static QString Enable() { return QStringLiteral("启用"); }
	static QString Disable() { return QStringLiteral("禁用"); }
	static QString LocalIp() { return QStringLiteral("本机IP"); }
	static QString ControllerIp() { return QStringLiteral("控制端IP"); }
	static QString DongleNotFound() { return QStringLiteral("没有找到加密锁"); }
	static QString CachePath() { return QStringLiteral("数据缓存地址"); }
	static QString DiskFreeSpace() { return QStringLiteral("磁盘剩余容量"); }
	static QString ClearCache() { return QStringLiteral("清空缓存"); }
	static QString CachePathMigrateFailed() { return QStringLiteral("缓存文件迁移失败，已回滚已拷贝的内容。"); }
	static QString CachePathMigrateDeleteFailed() { return QStringLiteral("缓存文件已拷贝，但原文件夹删除失败，已回滚变更。"); }
	static QString CachePathMigrateInvalidTarget() { return QStringLiteral("无效的缓存路径，新路径不能与当前缓存目录相同或互为子目录。"); }
	static QString StartMarking() { return QStringLiteral("开启标识"); }
};

static QString ResourceTypeString(int iType)
{
	QString strResTpe = "";
	switch (iType)
	{
	case 0:	// ERT_Video
		strResTpe = CCommonTranslate::Video();
		break;
	case 1: // ERT_Audio
		strResTpe = CCommonTranslate::Picture();
		break;
	case 2:
		strResTpe = CCommonTranslate::TestChart();
		break;
	case 3:
		strResTpe = CCommonTranslate::Audio();
		break;
	case 4:
		strResTpe = CCommonTranslate::Folder();
		break;
	case 5:
		strResTpe = CCommonTranslate::CaptureCard();
		break;
	case 6:
		strResTpe = CCommonTranslate::CaptureCard();
		break;
	case 7:
		strResTpe = CStreamMediaTranslate::Title();
		break;
	case 8:
		strResTpe = CCommonTranslate::Text();
		break;
	case 9:
		strResTpe = "NDI";
		break;
	case 10:
		strResTpe = CCommonTranslate::Subtitle();
		break;
	case 11:
		strResTpe = CCommonTranslate::Teleprompter();
		break;
	default:
		break;
	}

	return strResTpe;
}

//////////////////////////////填充模式-全屏-保持比例-无缩放
static QString GetFillTypeTranslate(int nType)
{
	QString strFillType = "";
	switch (nType)
	{
	case 0:	//EMFT_FullWindow
		strFillType = CPropertyTranslate::FullWindow();
		break;
	case 1: //EMFT_KeepRatio
		strFillType = CPropertyTranslate::KeepRatio();
		break;
	case 2: //EMFT_NoScale
		strFillType = CPropertyTranslate::NoScale();
		break;
	default:
		strFillType = CPropertyTranslate::FullWindow();
		break;
	}

	return strFillType;
}
