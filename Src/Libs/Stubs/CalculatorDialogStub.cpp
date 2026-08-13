#include "CalculatorDialog.h"

bool CTip::m_bHasUsed = false;

CalculatorDialog* CalculatorDialog::st_this = nullptr;
HHOOK CalculatorDialog::mouseHook = NULL;
QTextDocument* CalculatorDialog::m_pResultTEditDocument = nullptr;
QTextDocument* CalculatorDialog::m_pInputTEditDocument = nullptr;
QTextDocument* CalculatorDialog::m_pHistoryTEditDocument = nullptr;

CalculatorDialog::CalculatorDialog(QWidget* parent)
	: CFramelessDialog(parent)
{
}

CalculatorDialog::~CalculatorDialog() = default;

void CalculatorDialog::SetSpinUser(CSpinBox* user) { m_pSpinUser = user; }
void CalculatorDialog::SetDoubleSpinUser(CDoubleSpinBox* user) { m_pDoubleSpinUser = user; }
QSize CalculatorDialog::GetSize() { return QSize(200, 200); }

void CalculatorDialog::slotTitleBtnClick() {}
void CalculatorDialog::slotNumberBtnClick() {}
void CalculatorDialog::slotOperationBtnClick() {}
void CalculatorDialog::slotDeleteBtnClick() {}

void CalculatorDialog::InitWidget() {}
void CalculatorDialog::InitData() {}
void CalculatorDialog::CreateConnections() {}

int CalculatorDialog::MapToDrawFontAndRect(const QString&, int, int, int) { return 0; }
void CalculatorDialog::SetResultTEdit(const QString&) {}
void CalculatorDialog::SetHistory() {}
void CalculatorDialog::Calculate() {}
void CalculatorDialog::NumberKeyClick(int) {}
void CalculatorDialog::OperationKeyClick(int) {}

void CalculatorDialog::keyReleaseEvent(QKeyEvent* event) { CFramelessDialog::keyReleaseEvent(event); }
void CalculatorDialog::moveEvent(QMoveEvent* event) { CFramelessDialog::moveEvent(event); }
void CalculatorDialog::closeEvent(QCloseEvent* event) { CFramelessDialog::closeEvent(event); }

LRESULT CALLBACK CalculatorDialog::mouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

void CalculatorDialog::InitTextDocument() {}
void CalculatorDialog::UninitTextDocument() {}
void CalculatorDialog::UpdateTranslateText() {}
