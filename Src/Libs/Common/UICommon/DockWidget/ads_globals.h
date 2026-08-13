#pragma once

#include "DockManager.h"

extern CDockManager* g_pDockManager;

class CDockSplitter;

namespace internal
{
	template <class T>
	T findParent(const QWidget* w)
	{
		QWidget* parentWidget = w->parentWidget();
		while (parentWidget)
		{
			T ParentImpl = qobject_cast<T>(parentWidget);
			if (ParentImpl)
			{
				return ParentImpl;
			}
			parentWidget = parentWidget->parentWidget();
		}
		return 0;
	}

	void hideEmptyParentSplitters(CDockSplitter* FirstParentSplitter);
}

#define DOCKWIDGET_DRAGTYPE "DockWidgetMove"