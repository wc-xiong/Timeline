#ifndef CMENUEX_H
#define CMENUEX_H
#include "UICommonGlobal.h"
#include <QMenu>
#include <QProxyStyle>
#include <QMouseEvent>
#include <QHideEvent>
#include <QPointer>

class QEvent;
class QPainter;
class QAction;
class QActionEvent;
class UICOMMON_EXPORT CustomIconStyle : public QProxyStyle
{
    Q_OBJECT

public:
    CustomIconStyle();
    virtual ~CustomIconStyle();

    void SetIconSize(int);

protected:
    int pixelMetric(PixelMetric m, const QStyleOption *opt = nullptr, const QWidget *widget = nullptr) const override;

private:
    int m_nSize;
};

class UICOMMON_EXPORT CMenuEx : public QMenu
{
	Q_OBJECT
public:
	// 是否存在Checkable选项（存在与否影响菜单左边距）
    CMenuEx(QWidget *parent = nullptr, bool bExistCheckableAction = false);
    virtual ~CMenuEx();
    using QMenu::addAction;
    void addAction(QAction* action);
protected:
	bool eventFilter(QObject *watched, QEvent *event) override;
    void actionEvent(QActionEvent* event) override;

	void paintEvent(QPaintEvent *event) override;

	void mouseMoveEvent(QMouseEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void hideEvent(QHideEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event)override;

signals:
	void signalClicked();

private:
    void InitActionShortcutStyle(QAction* action);
    void DrawShortcutText(QPainter& painter);
    void HideMenuTooltip();

    CustomIconStyle* m_pStyle;
    QPointer<QAction> m_pTooltipAction;	// 当前已显示 tooltip 的菜单项
};

#endif // CMENUEX_H
