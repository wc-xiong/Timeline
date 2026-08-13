#pragma once

#include "UICommonGlobal.h"
#include "TitleBar.h"
#include <QDialog>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QAbstractButton;
QT_END_NAMESPACE

namespace UICommon {
    class CFramelessHelper;
    class UICOMMON_EXPORT CFramelessDialog : public QDialog
    {
        Q_OBJECT
    public:
        explicit CFramelessDialog(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
        virtual ~CFramelessDialog();

		void SetTitleName(const QString &strName);

        // 设置按钮图标
        void setTitleButtonIcon(const int &buttonType, const QIcon &icon);
        /**
        * @brief 设置窗口是否可以移动
        */
        void setDialogMovable(bool b);
        /**
        * @brief 设置是否支持橡皮筋风格的移动效果
        */
        void setRubberBandOnMove(bool b);
        /**
        * @brief 设置窗口是否可以缩放
        */
        void setDialogResizable(bool b);
        /**
        * @brief 设置是否支持橡皮筋风格的缩放效果
        */
        void setRubberBandOnResize(bool b);
        // 设置标题文本的对齐方式
        void setWindowTitleAlignment(Qt::Alignment align = Qt::AlignCenter);

		// 设置标题栏是否显示，默认显示
		void SetTitleBarVisible(bool bShow);

        // 显示最小化
        void setMinButtonVisible(bool b);
		void setMinButtonEnable(bool b);
        // 显示最大化按钮
        void setMaxButtonVisible(bool b);
		void setMaxButtonEnabled(bool b);
        // 显示关闭按钮
        void setCloseButtonVisible(bool b);
        bool getCloseButtonVisible()const;
        // 确定按钮是否显示
        void setOKButtonVisible(bool b);
        // 取消按钮是否显示
        void setCancelButtonVisible(bool b);
        // 应用按钮是否显示
        void setApplyButtonVisible(bool b);
            // 设置Ok按钮是否为默认选中项
        void setOKButtonDefault(bool b);
        // 添加一个扩展的按钮
        void appendExtendTitleButton(QAction *pAction,
			QSize size,
			bool bImage,
			const QString &strImagePath = "");
        // 插入一个扩展按钮
        void insertExtendTitleButton(int idx,
            QAction *pAction,
			QSize size,
			bool bImage,
			const QString &strImagePath = "");
        // 标题栏右边添加选项
        QAbstractButton* appendRightTitleButton(QAction *pAction,
            QSize size,
            bool bImage,
            const QString &strImagePath = "");

        // 底部的布局区
        QHBoxLayout *bottomLayout() { return m_pBottomLayout; }
        // 底部Widget
        QWidget* bottomWidget(){ return m_pBottomWidget;}
        /**
            * @brief 设置下部工具条的隐藏与显示
            * @param b
            */
        void setBottomBarVisible(bool b=true);
        QVBoxLayout* getMainLayout();

		// 设置标题栏高度
		void SetTitleBarHeight(int height);

		// 设置背景颜色
		void SetBackGroundColor(QColor color);
    protected:
        // 获取内容区容器控件
        QWidget *getContentHolder();

		// 翻译文本
		virtual void UpdateTranslateText();
    protected:
        virtual void changeEvent(QEvent *event) Q_DECL_OVERRIDE;

		virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;

		virtual void keyPressEvent(QKeyEvent *event)override; // 按键事件

		virtual void paintEvent(QPaintEvent *event) override;

		virtual void resizeEvent(QResizeEvent *event) override;

    public:
        // 覆盖基类
        void setLayout(QLayout *layout);
        QLayout *layout() const;
        void setWindowTitle(const QString &title);
        QString windowTitle() const;
        void setWindowIcon(const QIcon &icon);
        QIcon windowIcon() const;

		// 因为添加了阴影，实际大小会比设置大小大出阴影区域，因此要重写设置大小接口（请使用以下接口设置大小）
		void resize(const QSize &);
		void resize(int w, int h);

		void setFixedHeight(int h);
		void setFixedSize(const QSize &s);
		void setFixedSize(int w, int h);
		void setFixedWidth(int w);

		int height() const;
		int width() const;
		QSize size() const;

		virtual int exec(const QPoint &point);

    public Q_SLOTS :
        void show();
        void showNormal();
        virtual int exec() override ;

	protected:
		virtual void Close();
    private:
        void createWidgets();
        void createConnects();
        void swithMaxNormal();

    protected:
        CTitleBar       *m_pTitleBar = nullptr;
        QVBoxLayout     *m_pMainLayout = nullptr;
        QHBoxLayout     *m_pBottomLayout = nullptr;
        QPushButton     *m_pOKButton = nullptr;
        QPushButton     *m_pCancelButton = nullptr;
        QPushButton     *m_pApplyButton = nullptr;
        QHBoxLayout     *m_pTitleBarLayout = nullptr;

		bool m_bDialogFocus = false;
		int m_nDlgShadowWidth = 10;
		int m_nDlgRadius = 5;
		bool m_bDlgMax = false;	// 窗口是否最大化，原接口在扩展屏上可能会失效

		QColor m_clrBackground;	// 弹窗背景色（不能用样式表设置，因为此背景色是在Painter事件中绘制）
    private:
		QWidget         *m_pContentHolder = nullptr;
        QWidget         *m_pBottomWidget = nullptr;
        QLayout         *m_pCentralLayout = nullptr;
		CFramelessHelper *m_pFramelessHelper = nullptr;
	};
}// namespace UICommon
