#pragma once

#include <QScrollArea>

class CMediaPropertyFrame;
class CTimeLinePropertyFrame;
class CTimeLineTransFxPropertyFrame;

/**
 * Timeline 属性宿主：
 * - 选中转场 → CTimeLineTransFxPropertyFrame
 * - 选中媒体片段 → CMediaPropertyFrame
 * - 否则 → CTimeLinePropertyFrame
 */
class CTimelinePropertyHost : public QScrollArea
{
	Q_OBJECT
public:
	explicit CTimelinePropertyHost(QWidget* parent = nullptr);

public slots:
	void RefreshBySelection();

private:
	enum class EFrameKind
	{
		TimeLine,
		Media,
		TransFx,
	};

	void ShowFrame(EFrameKind kind);

	CMediaPropertyFrame* m_pMediaFrame = nullptr;
	CTimeLinePropertyFrame* m_pTimeLineFrame = nullptr;
	CTimeLineTransFxPropertyFrame* m_pTransFxFrame = nullptr;
	EFrameKind m_curFrame = EFrameKind::TimeLine;
};
