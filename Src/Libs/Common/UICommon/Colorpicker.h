#pragma once
#include <QFrame>

class CColorPicker : public QWidget
{
    Q_OBJECT
public:
    explicit CColorPicker(QWidget* parent);
    ~CColorPicker() override;
	
	void setCurColor(int nr, int ng, int nb, bool bSignal=true);//修改RGB数值，计算右上角颜色

public slots:	
    void setColor(int nr, int ng, int nb);	//直接修改右上角颜色

signals:
    void newColor(int r, int g, int b);
	void newHueColor(qreal hue);

protected:
	void updateBackground();
	void updatePositon(qreal posX, qreal posY, bool bSignal = true);
    void paintEvent(QPaintEvent*) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;

private:
	QPointF  m_Position;			//选中点的位置
	bool m_bPressed = false;		//是否选中选中点
	int r=255,g=0,b=0;				//当前右上角的颜色
	QImage m_image;					//绘制背景图
};

class CColorHueSlider : public QWidget
{
    Q_OBJECT
public:
   explicit CColorHueSlider(QWidget* parent=nullptr);
    ~CColorHueSlider() override;

public slots:
    void setColor(qreal h);
signals:
    void newColor(int r, int g, int b);

protected:
	void updatePositon(qreal pos, bool bSignal = true);
    void paintEvent(QPaintEvent*) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;

private:
	QPointF  m_Position;			//滑块的位置
	bool m_bPressed=false;			//是否选中滑块
};

