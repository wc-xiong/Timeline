#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QResizeEvent>
#include <QShowEvent>

// 主窗口：上方素材库 | 预览 | 属性，下方时间线
class CMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit CMainWindow(QWidget* parent = nullptr);
	~CMainWindow() override = default;

protected:
	void showEvent(QShowEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

private:
	void ApplyWindowChrome();
	void ForceUiRefresh();

	QSplitter* m_pRootSplitter = nullptr;
	QSplitter* m_pTopSplitter = nullptr;
};
