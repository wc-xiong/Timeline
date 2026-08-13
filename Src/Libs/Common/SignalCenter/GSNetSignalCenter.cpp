#include "GSNetSignalCenter.h"

namespace
{
	void RegisterPlayCommandMetaTypes()
	{
		static bool registered = false;
		if (registered)
			return;
		registered = true;

		qRegisterMetaType<SCellPlayState>("SCellPlayState");
		qRegisterMetaType<SSeekParam>("SSeekParam");
		qRegisterMetaType<SIpPlayState>("SIpPlayState");
		qRegisterMetaType<SAllPlayState>("SAllPlayState");
		qRegisterMetaType<std::vector<SCellPlayState>>("std::vector<SCellPlayState>");
		qRegisterMetaType<std::chrono::microseconds>("std::chrono::microseconds");
	}
}

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CGSNetSignalCenter* CGSNetSignalCenter::GetInstance()
{
	RegisterPlayCommandMetaTypes();
	static CGSNetSignalCenter instace;
	return &instace;
}