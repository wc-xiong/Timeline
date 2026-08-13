#pragma once

#include <QObject>
#include <QString>
#include "UITranslate.h"

enum ELogOperateType
{
	ELOT_Button,
	ELOT_Menu,
	ELOT_Key,
	ELOT_Drag
};


class CLogRecordTranslate : public QObject
{
	Q_OBJECT
public:
	static QString TopbarButtonAdd() { return QStringLiteral("切片优化"); }
	static QString RButtonAdd() { return QStringLiteral("优化组%1"); }
	static QString AddSuccess()		{ return QStringLiteral("添加成功"); }
	static QString AddFailed()		{ return QStringLiteral("添加失败"); }
	static QString DeleteFailed() { return QStringLiteral("删除失败"); }
	static QString SingleChoice() { return QStringLiteral("模型"); }
	static QString MultipleChoice() { return QStringLiteral("多选"); }
	static QString RButtonDelete()	{ return QStringLiteral("右键"); }
	static QString ButtonDelete()	{ return QStringLiteral("按钮"); }
	static QString Cancel()			{ return QStringLiteral("取消"); }
	static QString OK()				{ return QStringLiteral("确定"); }
	static QString Delete()			{ return QStringLiteral("删除"); }

	static QString Save()			{ return QStringLiteral("保存"); }
	static QString NoSave()			{ return QStringLiteral("不保存"); }

	static QString None()			{ return QStringLiteral("无"); }				// 无

	static QString Close()			{ return QStringLiteral("关闭"); }			// 关闭
	static QString Open()			{ return QStringLiteral("开启"); }				// 开启
	static QString CloseSoftWare() { return QStringLiteral("关闭软件"); }

	static QString AddMedia()		{ return QStringLiteral("添加媒体"); }			// 添加媒体
	static QString DelMedia()		{ return QStringLiteral("删除媒体"); }			// 删除媒体
	static QString ShortCut()		{ return QStringLiteral("快捷键"); }			// 快捷键

	static QString TitleWnd() { return QStringLiteral("标题栏"); }	// 标题栏
	static QString Project() { return QStringLiteral("工程文件"); }
	static QString OpenProject() { return QStringLiteral("打开工程"); }
	static QString SaveProject() { return QStringLiteral("保存工程"); }
	static QString Redo() { return QStringLiteral("恢复"); }
	static QString Undo() { return QStringLiteral("撤销"); }
	static QString UnLock() { return QStringLiteral("解锁"); }

	static QString ReplaceMedia() { return QStringLiteral("媒体替换"); }			// 媒体替换
	static QString ReplaceSuccess() { return QStringLiteral("替换成功"); }			// 媒体替换
	static QString ReplaceFailed() { return QStringLiteral("替换失败"); }			// 媒体替换

	static QString ClickButton() { return QStringLiteral("点击按钮"); }
	static QString RButtonMenu() { return QStringLiteral("右键菜单"); }
	static QString MenuOpen() { return QStringLiteral("菜单打开"); }
	static QString MenuSave() { return QStringLiteral("菜单保存"); }

	static QString EditOp() { return QStringLiteral("自定义"); }			// 编辑设置

	static QString Yes() { return QStringLiteral("是"); }
	static QString No() { return QStringLiteral("否"); }

	static QString LogProperty() { return QStringLiteral("切片优化"); }	//属性

	static QString GetLogOperate(int type)
	{
		switch (type)
		{
		case ELOT_Button:
			return ClickButton();
		case ELOT_Menu:
			return RButtonMenu();
		case ELOT_Key:
			return ShortCut();
		case ELOT_Drag:
			return CCommonTranslate::Drag();
		default:
			return QString();
		}
	}
};

//幻彩字属性界面日志翻译  CLogColorTextPtyTranslate
class CLogCTPTranslate : public QObject
{
public:
	CLogCTPTranslate(QObject *parent = nullptr) : QObject(parent) {}
	//共用：颜色、滑动条、SpinBox
	static QString PropertyGroupEnableChange() { return QStringLiteral("优化选中项"); }
	static QString ColorChange() { return QStringLiteral("优化未选项"); }
	static QString SliderChange() { return QStringLiteral("优化后切片隐藏，不可编辑"); }
	static QString SpinboxChange() { return QStringLiteral("数值修改"); }
	//文本
	static QString TextChange() { return QStringLiteral("文本修改"); }
	static QString FontChange() { return QStringLiteral("字体修改"); }
	static QString FontStyleChange() { return QStringLiteral("字体风格修改"); }
	static QString AlignMentChange() { return QStringLiteral("对齐方式修改"); }
	//幻彩效果
	static QString ColorTextTypeChange() { return QStringLiteral("幻彩效果修改"); }
	//填充
	static QString FillModeChange() { return QStringLiteral("填充模式修改"); }
	static QString FillGradientPositionChange() { return QStringLiteral("渐变颜色点位置修改"); }
	static QString AddGradientColor() { return QStringLiteral("添加渐变颜色点"); }
	static QString DelGradientColor() { return QStringLiteral("删除渐变颜色点"); }
	static QString Position() { return QStringLiteral("位置 %1"); }
	static QString FillTextureChange() { return QStringLiteral("填充纹理修改"); }
	static QString ImportTexture() { return QStringLiteral("导入纹理"); }
	static QString DirectionComboChange() { return QStringLiteral("渐变颜色方向修改"); }
	static QString RollComboChange() { return QStringLiteral("填充滚动方式修改"); }
	static QString TextureComboChange() { return QStringLiteral("纹理填充方式修改"); }
	static QString SpeedChange() { return QStringLiteral("填充滚动速度修改"); }
	//阴影
	static QString AmbiguityChange() { return QStringLiteral("模糊度修改"); }
	//背景
	static QString BackgroundModeChange() { return QStringLiteral("背景模式修改"); }
};
