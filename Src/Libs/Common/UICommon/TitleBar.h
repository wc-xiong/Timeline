#pragma once
#include <QSize>
#include <QFrame>
#include <QToolButton>
#include <QHBoxLayout>
#include <QComboBox>

#include "BaseFrame.h"
#include "UICommonGlobal.h"

QT_BEGIN_NAMESPACE
class QIcon;
class QLabel;
class QMouseEvent;
QT_END_NAMESPACE

namespace UICommon {
	//通用标题栏组件
	class UICOMMON_EXPORT CTitleBar : public CBaseFrame
	{
		Q_OBJECT
	public:
		CTitleBar(QWidget* parent = nullptr);
		virtual ~CTitleBar();

	public:
		// 标题栏控件类型
		enum EnTitleButtonType
		{
			ETBTLogoButton = 0,     // logo图标
			ETBTLogoButton2,        // logo2
			ETBTMinButton,          // 最小化按钮
			ETBTMaxButton,          // 最大化按钮
			ETBTRestoreButton,      // 恢复按钮
			ETBTCloseButton,        // 关闭按钮
			ETBTCount
		};
		// 设置标题对齐方式
		void SetTitleAlignment(Qt::Alignment align = Qt::AlignCenter);
		// 设置按钮图标
		void SetTitleButtonIcon(const int &buttonType, const QIcon &icon);
		QIcon TitleButtonIcon(const int& buttonType);
		// 设置按钮是否显示
		void SetButtonVisible(const int& iButtonType, bool bVisible);
		bool IsButtonVisible(const int&buttonType);

		void SetButtonEnable(const int& iButtonType, bool bEnable);
		bool IsButtonEnable(const int&buttonType);

		void SetButtonFocusPolicy(const int& iButtonType, int policy);

		// 设置最大化
		void SetMaximized(bool bValue);

		// 设置名称
		virtual void SetTitleName(const QString &title);
		virtual QString GetTitleName() const;

		void SetIconVisible(bool bValue);
		void SetSupportDoubleClicked(bool bSupport);
		void SetTitleNameProperty(const char* strProperty, bool bValue);
		void SetTitleFixedWidth(int nWidth);

		// 标题栏左边添加分割线
		void AppendLeftTitleSplitLine();
		QAbstractButton*  TitleButton(const int& buttonType);
		// 标题栏左边添加选项
		QAbstractButton* AppendLeftTitleButton(QAction *pAction, QSize size, bool bImage, const QString &strImagePath = "");
		// 标题栏左边插入选项
		QAbstractButton* InsertLeftTitleButton(int idx, QAction *pAction, QSize size, bool bImage, const QString &strImagePath = "");
		// 标题栏右边添加选项
		QAbstractButton* AppendRightTitleButton(QAction *pAction, QSize size, bool bImage, const QString &strImagePath = "");
		// 标题栏右边插入选项
		QAbstractButton* InsertRightTitleButton(int idx, QAction *pAction, QSize size, bool bImage, const QString &strImagePath = "");
		//标题栏右边插入下拉框
		void InsertRightTitleWidget(QWidget* pWnd);

		// 对于标题解释Label控制
		void SetTitleTipsVisible(bool bVisible);
		void SetTitleTipsText(const QString& strText);
		void SetTitleTipsPty(const char* strProperty, const QVariant& value);

		void SetTitleNameElided(bool bElided);
	signals:
		void signalMax();
		void signalMin();
		void signalNormal();
		void signalClose();

	public slots:
		void    slotSwitchMaxMin();    // 切换最大最小
		void    slotClearChecked();    // 清除选中

	protected:
		// 鼠标事件
		void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

		void mousePressEvent(QMouseEvent *event) override;

		void mouseReleaseEvent(QMouseEvent *event) override;

		void mouseMoveEvent(QMouseEvent *event) override;

		void UpdateTranslateText() override;

	private:
		void    InitWidget();           // 初始化，创建控件
		void    CreateConnects();       // 连接信号槽

	protected:
		bool	m_bIsMaximized = false;				// 是否最大化
		bool	m_bSupportDoubleClicked = false;	// 是否支持双击改变界面最大化状态

		bool	m_bMousePressed = false;
		QPoint	m_ptDragPos;
		bool	m_bChangeFlag = true;
		int		m_nOldwidth;
		QPoint	m_ptLeftMouseDown;

		bool	m_bElideTitleName = false;
		QString m_realTitleName;

		QLabel*						m_pLabelTitle = nullptr;			// 标题栏名称
		QLabel* m_pTitleTips = nullptr;
		QHBoxLayout*				m_pLeftTitleBtnLayout = nullptr;
		QHBoxLayout*				m_pRightTitleBtnLayout = nullptr;
		QVector<bool>				m_vecButtonVisible;					// 对应按钮显示标识
		QVector<bool>				m_vecButtonEnabled;					// 对应按钮使能标识
		QVector<QAbstractButton*>   m_vecButtons;						// 标题栏按钮组列表	
	};
}// namespace UICommon

