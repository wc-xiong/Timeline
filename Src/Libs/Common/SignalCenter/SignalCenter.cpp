#include "SignalCenter.h"

#include <QDebug>
#include <QKeyEvent>
#include <windows.h>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

bool CSignalCenter::eventFilter(QObject *watched, QEvent *event)
{
	switch (event->type())
	{
	//case QEvent::KeyPress:
	//{
	//	QKeyEvent* e = dynamic_cast<QKeyEvent*>(event);
	//	if (e->modifiers() == (Qt::AltModifier | Qt::ControlModifier) && e->key() == Qt::Key_O)
	//	{
	//		emit signalOpenAllScreenByShotrcut();
	//	}
	//	else if (e->modifiers() == (Qt::AltModifier | Qt::ControlModifier) && e->key() == Qt::Key_C)
	//	{
	//		emit signalCloseAllScreenByShotrcut();
	//	}

	//}
	//break;
	case QEvent::KeyRelease:
	{
		QKeyEvent* e = dynamic_cast<QKeyEvent*>(event);
		if (e->modifiers() == Qt::NoModifier && e->key() == Qt::Key_T)
		{
			emit signalShotrcut();
		}
		else if (e->modifiers() == Qt::NoModifier && e->key() == Qt::Key_Space)
		{
			emit signalPlayPause();
		}
	}
	break;
	default:
		break;
	}

	return false;
}

bool CSignalCenter::IsCtrl()
{
	return (GetKeyState(VK_CONTROL) & 0x8000);
}
bool CSignalCenter::IsShift()
{
	return (GetKeyState(VK_SHIFT) & 0x8000);
}
bool CSignalCenter::IsAlt()
{
	return (GetKeyState(VK_MENU) & 0x8000);
}
bool CSignalCenter::IsWin()
{
	return (GetKeyState(VK_LWIN) & 0x8000);;
}
bool CSignalCenter::IsW()
{
	return (GetKeyState('W') & 0x8000);
}
bool CSignalCenter::IsE()
{
	return (GetKeyState('E') & 0x8000);
}

bool CSignalCenter::IsR()
{
	return (GetKeyState('R') & 0x8000);
}

bool CSignalCenter::IsS()
{
	return (GetKeyState('S') & 0x8000);
}
