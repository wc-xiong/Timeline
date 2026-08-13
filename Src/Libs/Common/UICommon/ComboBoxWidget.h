#pragma once
#include "UICommonGlobal.h"
#include <QWidget>
#include <QIcon>

//ComboBox窗口，包括QLabel，ComboBox

QT_BEGIN_NAMESPACE
class CComboBoxEx;
class QLabel;
class QHBoxLayout;
class QCompleter;
class CTipButton;
QT_END_NAMESPACE

#include <QApplication>
#include <QMouseEvent>
#include <QLineEdit>

class CComboBoxEditEx : public QLineEdit
{
	Q_OBJECT

public:
	CComboBoxEditEx(QWidget* pParent = 0) :
		QLineEdit(pParent) {

	}
	~CComboBoxEditEx() {

	}

	virtual void mousePressEvent(QMouseEvent* event) override
	{
		QApplication::sendEvent(parent(), event);
		QLineEdit::mousePressEvent(event);
	}
	virtual void mouseReleaseEvent(QMouseEvent* event) override
	{
		QApplication::sendEvent(parent(), event);

		QLineEdit::mousePressEvent(event);
	}
};

class UICOMMON_EXPORT CComboBoxWidget : public QWidget
{
	Q_OBJECT

public:
	CComboBoxWidget(QWidget* pParent = 0, bool bAddTip = false);
	~CComboBoxWidget();

public:

	//设置ComboBox名称
	void SetName(const QString &strName);
	
	//设置QLabel设置，宽度，文字对齐方式、QLabel与ComboBox中间间距
	void SetLabelConfig(int nWidth, Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter, int nInterval = 0, int nInterval1 = 0);

	//设置文字对齐方式、QLabel与ComboBox中间间距
	void SetTextConfig(int nInterval, Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter);

	//默认为设置QLabel与ComboBox中间间距，修改可以在任意地方插入间距
	void SetInterval(int nInterval = 0, int pos = 1);

	void SetEditable();

	//添加多个选项，可选择清除之前添加选项（默认清除）,bSelectBreIndex: 是否保持之前选中Index，bDelPreOptions：是否删除之前选项
	void AddItems(const QStringList &listItems, bool bSelectPreIndex = false, bool bDelPreOptions = true);

	//添加多个选项的提示
	void AddItemsTip(const QStringList &listItems);

	void AddItemTip(int index, const QString& strTip);
	
	//添加单个选项，可设置icon
	void AddItem(const QString &strItem, const QIcon &icon = QIcon());

	//添加单个选项，可额外绑定数据
	void AddItem(const QString &atext, const QVariant &auserData);

	//设置某个选项不可用
	void SetItemEnable(int index, bool b);

	//设置ComboBox是否只读
	void SetReadOnly(bool bReadOnly);

	//设置ComboBox下拉时最多出现几个选项（显示不完出滚动条)
	void SetMaxVisibleItems(int nCount);

	// 设置当前选中
	void SetSelectIndex(int nIndex);

	//当前ComboBox选中下标
	int GetSelectIndex();

	//当前选中项的Text
	QString GetSelectText();

	void SetComboxWidth(int nWidth);

	void SetComboBoxEnable(bool bEnable);

	// 获取item的index
	int GetIndexFromItem(QString strItem);

	// 清除items
	void Clear();

	// Item Count
	int Count();

	void SetComboBoxShowText(const QString &strText);

	void SetCompleter(QCompleter* completer);

	QCompleter* GetCompleter();

	void SetComboBoxEditable(bool bEnable);
	// 设置标签样式
	bool SetLabelProperty(const char*, QVariant);

	void SetComboProperty(const char* strProperty, QVariant flag);

	void SetLimitLabelWidth(bool b) { m_bLimitLabelWidth = b; }

	// 设置默认提示
	void SetPlaneHolder(const QString& text);

	void SetBottomLine(bool bFlag = true);

	void SetWidgetContentMargin(int nLeft, int nTop, int nRight, int nBottom);

	QVariant CurrentData();

	inline const CComboBoxEx * getComboBox() const
	{
		return m_pCombo;
	}

	CComboBoxEx* GetComboBox() { return m_pCombo; }
	/**
	 * @brief 获取内部的水平布局指针（供外部操作）
	 * @return 布局指针（QHBoxLayout*）
	 */
	QHBoxLayout* GetLayout() const { return m_pHorLayout; }

	void SetComboTextAlignment(Qt::Alignment nAlignment);

	// 动态设置widget样式
	void SetWidgetProperty(const char* strProperty, QVariant flag);

	void SetTipTextContent(const QString& text);
	void SetTipBubbleSize(int width, int height);

	// 下拉框当前项文本省略显示并显示 tooltip
	void SetElideComboDisplayText(bool bElide);
signals:
	
	//选中项信号(下标)
	void signalActivated(int index);

	//ComboBox选中项发生变化（下标）
	void signalSelectChange(int nIndex);
	
	//ComboBox选中项发生变化（选中项Text）
	void signalSelectTextChange(const QString &strText);

	//当前Combox显示文字变化
	void signalEditTextChange(const QString &strText);

protected slots:
	void slotActivated(int nIndex);

	void slotSelectChange(int nIndex);

	void slotSelectChange(const QString &strText);
	
	void slotEditTextChanged(const QString &text);

	void slotUpdateComboDisplayText(int nIndex);


protected:
	void InitWidget();

	void CreateConnections();

	void showEvent(QShowEvent *event) override;


private:
	QStringList m_listOptions;	//当前Combox中所有选项

	bool m_bLimitLabelWidth = false;	// 是否限制QLabel宽度

	QString m_strLabelText;		// QLabel Text

private:
	QLabel* m_pNameLabel = nullptr;
	CTipButton* m_pTipBtn = nullptr;
	CComboBoxEx* m_pCombo = nullptr;
	QHBoxLayout* m_pHorLayout = nullptr;

	bool m_bPlaceholder = false;
	QString m_strPlaceholder;

	bool m_bAddTip = false;

	bool m_bElideComboDisplayText = false; // 下拉框当前项是否省略显示
};
