#pragma once
#include "Framelessdialog.h"

class QProgressBar;
class QLabel;

enum EProgressType {
	EPT_Timing = 0,	// 设计时间时长，与更新步长，倒计时结束后自动关闭
	EPT_RangePrecent,	// 设置范围，需要手动更新当前，当更新值大于等于最大值时，窗口关闭，显示百分号
	EPT_RangeInt, // 设置范围，需要手动更新当前，当更新值大于等于最大值时，窗口关闭，显示整数值
};

namespace UICommon {
	class UICOMMON_EXPORT CProgressDialogEx : public CFramelessDialog
	{
		Q_OBJECT
	public:
		// nType = EPT_Timing：nValue1为时长，nValue2为步长，单位Ms
		// nType = EPT_MinMaxRange：nValue1为最小值，nValue2为最大值
		CProgressDialogEx(int nType, int nValue1, int nValue2, QWidget *parent = nullptr);

		CProgressDialogEx(QWidget *parent = nullptr);
		~CProgressDialogEx();

		void SetTimeConfig(int nTimeLenght, int nTimeInterval);

		void SetMinMaxRange(int nMinValue, int nMaxValue, int nProgressType);

		void SetProgressValue(int nValue);

		void SetTitleName(const QString &strText);

		void SetCloseBtnShowState(bool bShow);

		void SetMaxOffFlag(bool bFlag);		//设置到达最大值关闭标记

		int GetProgressCurValue();
	public slots:
		void slotAddValue(int nValue);

		void slotSetValue(int nValue);

		void slotAddValueAndSetTitle(int nValue, const QString &title);

	public:
		virtual void timerEvent(QTimerEvent *event) override;
		virtual int exec() override;
		virtual void keyPressEvent(QKeyEvent *event) override;

		void resizeEvent(QResizeEvent *event) override;

	protected:
		void InitWidget();	// 初始化，创建控件

		void CloseProgressDialog();


	signals:
		void signalUpdateProgress(int nValue);

	protected:
		void slotUpdateProgress(int nValue);

	private:
		QProgressBar* m_pProgress = nullptr;
		QLabel* m_pLabelValue = nullptr;

		int m_nProgressType = EPT_RangePrecent;

		int m_nMinValue = 0;
		int m_nMaxValue = 100;

		int m_nTimeInterval = 10;
		int m_nCurValue = 0;
		int m_nTimerID = -1;

		bool m_bMaxOff = true;  //最大值关闭

		QFrame* m_pLabelContain = nullptr;
	};
}
