#pragma once
#include <QWidget>
#include <QScrollBar>
// 前置声明
class CLayerLabelsWidget;
class CLayersWidget;
class CLayersControlWidget;

class CLayersViewWidget : public QWidget
{
	Q_OBJECT
public:
	CLayersViewWidget(QWidget* pParent = 0);
	~CLayersViewWidget();

	void	UpdateTranslateText();
	void	UpdateView(uint32_t type);
	void	SetHorScrollbarVal(int val);
	void	SetVerScrollbarVal(int val);
	void	SetScrollCur(int val);
	CLayersWidget* GetLayersWidget() const { return m_pLayersView; }

protected:
	void    InitWidget();
	void    CreateConnects();

signals:
	void	signalUpdateView(uint32_t);
	void	signalLayerLabelWheel(int);
	void	signalLayerViewWheel(int);
	void	signalUpdateDragLine(int index);
	void	signalHorScrollValueChanged(int value);
	void	signalVerScrollValueChanged(int value);

private:
	CLayerLabelsWidget*		m_pLayersLabel;			// 左侧图层列表
	CLayersControlWidget*	m_pLayersControl;		// 控制按钮
	CLayersWidget*			m_pLayersView;			// 右侧图层视图

	//滚动条
	QScrollBar* m_pHorScrollbar = nullptr;
	QScrollBar* m_pVerScrollbar = nullptr;
};
