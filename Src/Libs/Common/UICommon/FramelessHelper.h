#pragma once

#include <QObject>
#include "UICommonGlobal.h"

namespace UICommon {

	// 无边框窗口缩放移动支持
	class CFramelessHelperPrivate;
	class UICOMMON_EXPORT CFramelessHelper : public QObject
	{
	public:
		explicit CFramelessHelper(QObject* parent = nullptr);
		~CFramelessHelper();

		void ActiveOn(QWidget* toplevelWidget);		// 激活窗口
		void SetMovable(bool bValue);				// 支持窗口移动
		void SetResizable(bool bValue);				// 支持窗口缩放
		void SetRubberBandOnMove(bool bValue);		// 支持橡皮筋模式移动
		void SetRubberBandOnResize(bool bValue);	// 支持橡皮筋模式缩放
		void SetBorderWidth(int nValue);			// 设置边框宽度
		void SetTitleHeight(int nValue);			// 设置标题栏高度

	protected:
		virtual bool eventFilter(QObject *watched, QEvent *event);	// 重写事件过滤器

	private:
		CFramelessHelperPrivate  *d_ptr;
	};
}
