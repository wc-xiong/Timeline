#pragma once
#include "UICommonGlobal.h"
#include "BaseFrame.h"
#include <vector>

//组合布局窗口
//内部窗口设置隐藏、显示直接设置原窗口显示、隐藏即可。

QT_BEGIN_NAMESPACE
class QLineEdit;
class QLabel;
class QVBoxLayout;
class QPushButton;
class QLayoutItem;
class QRadioButton;
class QHBoxLayout;
class CCheckBoxEx;
class CSvgImageButton;
class CTipButton;
QT_END_NAMESPACE

//竖直组合布局窗口，可以折叠
class UICOMMON_EXPORT CGroupWidgetBox : public CBaseFrame
{
	Q_OBJECT

public:
	CGroupWidgetBox(QWidget* pParent = 0,bool flag = false, bool showSplitLine = false);
	~CGroupWidgetBox();
	void SetNameLabelProperty(const char* strProperty, QVariant flag);
public:
	//设置名称
	void SetName(const QString &strName);

	//添加内部窗口
	void AddContainWidget(QWidget* pWidget);

	//添加内部窗口
	void InsertContainWidget(QWidget* pWidget, int nIndex = -1);

	//添加内部布局
	void AddContainLayout(QLayout* pItem);

	//添加弹簧
	void AddContainStretch();

	//删除内部窗口
	void DelContainWidget(QWidget* pWidget);

	//删除内部布局
	void DelContainLayout(QLayout* pItem);

	//设置上下窗口之间间距
	void SetSpacing(int nSpacing);

	//设置表头展开行是否显示
	void SetHeadVisible(bool bVisible);

	//设置四周间距
	void SetContentsMargins(int nLeft, int nTop, int nRight, int nBottom);

	void SetContainContentsMargins(int nLeft, int nTop, int nRight, int nBottom);

	//删除全部窗口，布局
	void DelAllChildItems();

	void SetEnabled(bool bEnabled);

	void SetExpandBtnVisible(bool b);

	void SetExpanded(bool bExpand);

	//添加自定义按钮
	void SetUndefineBtn(const QString& path);

	//自定义按钮提示
	void SetUndefineBtnTip(const QString& tip);

	// 自定义按钮大小
	void SetUndefineBtnSize(QSize btnSize);

	void SetContainFrameStyle(const QString& strStyle, bool bTOrF = true);

	void AddSpacing(int nSpacingValue);
	void InsertSpacing(int nSpacingValue, int nIndex);

	void SetTipVisible(bool bVisible);
	void SetTipTextContent(const QString& text);
	void SetTipBubbleSize(int width, int height);

	// 在标题名(QLabel)后插入自定义控件（插入到tip按钮之前）
	void InsertHeadWidgetAfterName(QWidget* pWidget);

	// 在标题行最右侧插入控件（与属性行关键帧列对齐）
	void InsertHeadWidgetAtRight(QWidget* pWidget);

	inline const CCheckBoxEx * getCheckBox() 
	{
		return m_pChecCBtn;
	}

    void setSelected(bool flag);

	std::vector<QWidget*> GetAllChildWidgets();

	void SetHeaderStyleForExCtrCmd();
signals:
	void sigEnableChange(bool bEnable);
	void stateChanged(int value);
	void signalCheckBoxClicked(bool checked = false);
	void signalUndefineBtnClicked();
protected slots:
	void slotGroupStateChange();
	void slotGroupCheckBoxStateChange(bool checked);
protected:
	void InitWidget();

	void CreateConnections();

	void UpdateTranslateText() override {}

private:
	bool m_bGroupExpand;	//布局展开、收起状态

private:
	CSvgImageButton* m_pExpandBtn = nullptr;	//收起、展开按钮
	CCheckBoxEx * m_pChecCBtn = nullptr;
	QLabel* m_pNameLabel = nullptr;			//名称Label
	bool    m_bUseCheckBox = false;
	int     m_iCheckedState = 0;         //记录选中状态

	CSvgImageButton* m_pUndefineBtn = nullptr;

	QVBoxLayout* m_pVerLayout = nullptr;	//主要竖直布局

	QVBoxLayout* m_pContainVerLayout = nullptr;	//负责添加窗口布局
	QHBoxLayout* m_pHeadHorLayout = nullptr;//表头布局
	QFrame* m_pHeadFrame = nullptr;	//表头内部窗口
	QFrame* m_pContainFrame = nullptr;	//容纳添加窗口布局

	CTipButton* m_pTipBtn = nullptr;

	bool m_bShowSplitLine = false;
};

//水平组合布局窗口，不可以折叠
class UICOMMON_EXPORT CGroupWidgetHBox : public CBaseFrame
{
	Q_OBJECT

public:
	CGroupWidgetHBox(QWidget* pParent = 0, bool flag = false, bool showSplitLine = false);
	~CGroupWidgetHBox();

public:
	//设置名称
	void SetName(const QString &strName);

	//添加内部窗口
	void AddContainWidget(QWidget* pWidget);

	//添加内部窗口
	void InsertContainWidget(QWidget* pWidget, int nIndex = -1);

	//添加内部布局
	void AddContainLayout(QLayout* pItem);

	//添加间距
	void AddContainSpacing(int nSpacing);
	void AddHeadSpacing(int nSpacing);

	//添加弹簧
	void AddContainStretch();

	//删除内部窗口
	void DelContainWidget(QWidget* pWidget);

	//删除内部布局
	void DelContainLayout(QLayout* pItem);

	//设置上下窗口之间间距
	void SetSpacing(int nSpacing);

	//设置四周间距
	void SetContentsMargins(int nLeft, int nTop, int nRight, int nBottom);

	void SetContainContentsMargins(int nLeft, int nTop, int nRight, int nBottom);

	//删除全部窗口，布局
	void DelAllChildItems();

	void SetEnabled(bool bEnabled);

	inline const CCheckBoxEx * getCheckBox()
	{
		return m_pChecCBtn;
	}

	void setSelected(bool flag);

	std::list<QWidget*> GetAllChildWidgetList();

	void SetHeadNameWidth(int n);
signals:
	void sigEnableChange(bool bEnable);
	void signalCheckBoxClicked(bool checked = false);

protected slots:
	void slotGroupCheckBoxStateChange(bool checked);

protected:
	void InitWidget();

	void CreateConnections();

private:
	CCheckBoxEx * m_pChecCBtn = nullptr;
	QLabel* m_pNameLabel = nullptr;					//名称Label
	bool    m_bUseCheckBox = false;					//记录选中状态
	int     m_iCheckedState = 0;					

	QHBoxLayout* m_pLayout = nullptr;				//主要水平布局
	QVBoxLayout* m_pContainLayout = nullptr;		//负责添加窗口布局
	QHBoxLayout* m_pHeadLayout = nullptr;			//表头布局
	QFrame* m_pHeadFrame = nullptr;					//表头内部窗口
	QFrame* m_pContainFrame = nullptr;				//容纳添加窗口布局

	bool m_bShowSplitLine = false;
};