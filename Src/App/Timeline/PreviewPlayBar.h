#pragma once

#include <QWidget>

class CSvgImageButton;

// 预览区底部：播放/暂停合一按钮（随播控状态切换图标）
class CPreviewPlayBar : public QWidget
{
	Q_OBJECT
public:
	explicit CPreviewPlayBar(QWidget* parent = nullptr);

	void SyncPlayState();

private:
	void InitWidget();
	void CreateConnects();
	void OnPlayPauseClicked();

	CSvgImageButton* m_pBtnPlayPause = nullptr;
	bool m_bShowingPauseIcon = false;
};
