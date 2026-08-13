#pragma once

#include "UICommonGlobal.h"
#include "Framelessdialog.h"

class QLabel;
class CSpinBox;
class CColorPicker;
class CColorHueSlider;

class UICOMMON_EXPORT CColorDialog :  public UICommon::CFramelessDialog
{
    Q_OBJECT
public:
    explicit CColorDialog(QWidget *parent = nullptr);  
    ~CColorDialog();

    void InitColor(const QColor& color);
public slots:
	void slotColorSpinBoxchange(int);

signals:
	void signalNewColor(QColor);
private:
    void initUI();
    void initConnect();


private:
    CColorPicker* m_pColorPicker = nullptr;
    CColorHueSlider* m_pColorHueSlider =nullptr;

	QLabel *m_pRGBLabel = nullptr;
	CSpinBox *m_RSpinBox = nullptr;
	CSpinBox *m_GSpinBox = nullptr;
	CSpinBox *m_BSpinBox = nullptr;
};


