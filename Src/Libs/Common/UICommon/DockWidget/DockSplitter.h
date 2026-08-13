#pragma once
#include <QSplitter>
#include <QSplitterHandle>
#include "../UICommonGlobal.h"

class QRubberBand;

class CSplitterHandleEx : public QSplitterHandle
{
	Q_OBJECT
public:
	CSplitterHandleEx(Qt::Orientation orientation, QSplitter *parent = Q_NULLPTR);

protected:
	void mouseMoveEvent(QMouseEvent *e) override;
	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;

private:
	QRubberBand* m_pRubberBand = nullptr;
	QPoint m_ptOrigin;
	QSize m_szRubberBand;
	QWidget* m_pCentralWnd = nullptr;
	int m_nCentralWndIndex = -1;

}; // class CSplitterHandleEx

//////////////////////////////////////////////////////////////////////////


class UICOMMON_EXPORT CDockSplitter : public QSplitter
{
	Q_OBJECT
public:
	CDockSplitter(QWidget *parent = Q_NULLPTR);
	CDockSplitter(Qt::Orientation orientation, QWidget *parent = Q_NULLPTR);
	virtual ~CDockSplitter();

	/**
	 * Returns true, if any of the internal widgets is visible
	 */
	bool hasVisibleContent() const;

protected:
	QSplitterHandle *createHandle() override;

}; // class CDockSplitter