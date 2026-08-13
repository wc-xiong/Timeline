#pragma once
#include "Framelessdialog.h"
#include "TextEdit.h"
#include "PushButtonEx.h"
#include <QPainter>
#include <QEvent>
#include "SpinBoxEx.h"
#include "DoubleSpinBoxEx.h"
#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <QPainterPath>
#include <QFont>
#include <QApplication>
#include <windows.h>

using namespace UICommon;

enum CalculatorState
{
	None,
	NumberInput1,
	OperationInput,
	NumberInput2,
	Result,
};

class CDoubleSpinBox;
class CSpinBox;

//气泡提示框共享提示次数
class CTip
{
public:
	static bool m_bHasUsed;
};

/////////////////////////////////////////////////////////////

class CalculatorDialog : public CFramelessDialog
{
	Q_OBJECT
public:
	CalculatorDialog(QWidget* parent = nullptr);
	~CalculatorDialog();

	void SetSpinUser(CSpinBox *User);
	void SetDoubleSpinUser(CDoubleSpinBox *User);

	QSize GetSize();
signals:
	void signalActive(bool);
public slots:
	void slotTitleBtnClick();
	void slotNumberBtnClick();
	void slotOperationBtnClick();
	void slotDeleteBtnClick();

protected:
	void InitWidget();						// 初始化界面，控件
	void InitData();
	void CreateConnections();

	int MapToDrawFontAndRect(const QString &strDrawTex, int width, int height, int max);
	void SetResultTEdit(const QString &strDrawText);
	void SetHistory();
	void Calculate();

	void NumberKeyClick(int ikey);
	void OperationKeyClick(int ikey);

	virtual void keyReleaseEvent(QKeyEvent *event) override;
	void moveEvent(QMoveEvent *event) override;
	void closeEvent(QCloseEvent *event) override;

	static LRESULT CALLBACK mouseProc(int nCode, WPARAM wParam, LPARAM lParam);

	static void InitTextDocument();
	static void UninitTextDocument();

	virtual void UpdateTranslateText()override;
private:

	//标题
	QAbstractButton* m_pHistoryBtn = nullptr;
	QAbstractButton* m_pApplyBtn = nullptr;

	//左边输入区域
	CTextEdit	*m_pResultTEdit = nullptr;
	CTextEdit	*m_pInputTEdit = nullptr;

	//按键
	CPushButton *m_pSignBtn = nullptr;
	CPushButton *m_pClearBtn = nullptr;
	CPushButton *m_pDeleteBtn = nullptr;

	CPushButton *m_pDivideBtn = nullptr;
	CPushButton *m_pMultiplyBtn = nullptr;
	CPushButton *m_pSubtractBtn = nullptr;
	CPushButton *m_pAddBtn = nullptr;

	CPushButton *m_pEqualBtn = nullptr;

	CPushButton *m_pPointBtn = nullptr;

	QVector<CPushButton *> m_vecNumberBtns;
	//右边历史结果区域
	CTextEdit	*m_pHistoryTEdit = nullptr;
	QLabel		*m_pBlackLabel = nullptr;
	CPushButton *m_pClearAllBtn = nullptr;

	QVBoxLayout* RightMainLayout = nullptr;
	QHBoxLayout* MainLayout = nullptr;

	int m_nState = None;
	bool m_bHasMove = false;
	CDoubleSpinBox	*m_pDoubleSpinUser = nullptr;
	CSpinBox		*m_pSpinUser = nullptr;


	static CalculatorDialog* st_this;			//静态对象指针用来在回调函数里传递信息
	static HHOOK mouseHook;						//鼠标钩子-通过鼠标钩子阻止点击按钮后的抬起事件实现相关功能

	static QTextDocument* m_pResultTEditDocument;
	static QTextDocument* m_pInputTEditDocument;
	static QTextDocument* m_pHistoryTEditDocument;
};
