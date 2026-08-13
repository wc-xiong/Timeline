#pragma once
#include "UICommonGlobal.h"
#include <QWidget>
#include <QRegExpValidator>
#include <QLineEdit>
//编辑框窗口，包括QLabel，LineEdit

QT_BEGIN_NAMESPACE
class QLineEdit;
class QLabel;
class QHBoxLayout;
class CLineEditEx;
class CCheckBoxEx;
class QCompleter;
QT_END_NAMESPACE

class UICOMMON_EXPORT CLineEditWidget : public QWidget
{
	Q_OBJECT

public:
	CLineEditWidget(QWidget* pParent = 0,bool flag = false);
	~CLineEditWidget();

public:
	bool IsChecked();
	void SetChecked(bool bVlaue, bool signalEmitted = true);
	//设置ComboBox名称
	void SetName(const QString &strName,bool b = false, bool bElideMarginChangedByDpi = false);
	
	//设置QLabel设置，宽度，文字对齐方式、QLabel与TimeEdit中间间距
	void SetLabelConfig(int nWidth, Qt::Alignment alignment = Qt::AlignLeft | Qt::AlignVCenter, int nInterval = 0);

	void SetEditConfig(Qt::Alignment alignment);

	//QLabel与TimeEdit中间间距
	void SetLabelEditInterval(int nInterval = 0);

	void SetLineEditWidth(int nWidth);

	void SetBottomLine(bool bFlag = true);

	//设置是否只读
	void SetReadOnly(bool bReadOnly);

	void SetText(const QString &text);

	QString GetText();

	QString GetName();

	//设置验证器
	void SetValidator(QValidator *validator);

	// 设置LineEdit输入模式
	void SetEchoMode(QLineEdit::EchoMode mode);

	// 设置默认文本
	void SetPlaceholderText(const QString &holderText);

	void SetMaxInputLen(int length);

	void SetCompleter(QCompleter* completer);

	QCompleter* GetCompleter();

	void SetFocus();

	int GetEditCursorPos();
	void SetEditCursorPos(int nPos);

	void SetLabelProperty(const char* strProperty, bool bValue);

	void SetLineEditProperty(const char* strProperty, bool bValue);

	void SetLineEditTextColor(const char* strProperty);

	QWidget* GetLineEdit();

	void SetWidgetProperty(const char* strProperty, QVariant flag);

signals:
	void signalTextChange(const QString &text);
	void signalClicked();
	void signalEditingFinished();

	void signalEditFinished(const QString& text);

protected slots:
	void slotTextChange(const QString &text);
	void slotCheckBoxClicked();

	void slotEditFinish();

protected:
	void InitWidget();
	void CreateConnections();

private:
	QString m_tCurtext;

private:
	bool    m_bUseCheckBox = false;
	CCheckBoxEx *m_pCheckBox = nullptr;

	QLabel* m_pNameLabel = nullptr;
	CLineEditEx* m_pEdit = nullptr;
	QHBoxLayout* m_pHorLayout = nullptr;

	bool m_bFistSetEchoMode = true;
};