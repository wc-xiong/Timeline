#include "CrashDumpHandler.h"

#include "CommonFunctionFileOp.h"

#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QSystemSemaphore>

namespace
{
	CrashDumpHandlerOptions g_options;
	int g_nCrashRestartCnt = 0;

	LONG CALLBACK CrashDumpExceptionHandler(EXCEPTION_POINTERS* pException)
	{
		QSystemSemaphore lockAppSemaphore(g_options.semaphoreKey, 1, QSystemSemaphore::Open);
		lockAppSemaphore.acquire();

		QString logFilePath = GetDumpDirDefault(g_options.appName, g_options.companyShortName);
		QDir dstDir(logFilePath);
		if (!dstDir.exists())
		{
			if (!dstDir.mkpath(logFilePath))
			{
				qDebug() << __FILE__ << __LINE__ << "创建DumpCrashes文件夹失败！";
			}
		}

		QString dumpFileName = logFilePath + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".dmp";
		HANDLE hDumpFile = CreateFile((LPCWSTR)(dumpFileName.toStdWString().c_str()), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hDumpFile != INVALID_HANDLE_VALUE)
		{
			MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
			dumpInfo.ExceptionPointers = pException;
			dumpInfo.ThreadId = GetCurrentThreadId();
			dumpInfo.ClientPointers = TRUE;
			MiniDumpWriteDump(
				GetCurrentProcess(),
				GetCurrentProcessId(),
				hDumpFile,
				(MINIDUMP_TYPE)(MiniDumpWithDataSegs | MiniDumpWithProcessThreadData | MiniDumpWithUnloadedModules),
				&dumpInfo,
				NULL,
				NULL);
			CloseHandle(hDumpFile);
		}

		return EXCEPTION_EXECUTE_HANDLER;
	}
}

void InstallCrashDumpHandler(const CrashDumpHandlerOptions& options)
{
	g_options = options;
	SetUnhandledExceptionFilter(CrashDumpExceptionHandler);
}

void SetCrashRestartCount(int count)
{
	g_nCrashRestartCnt = count;
}

int GetCrashRestartCount()
{
	return g_nCrashRestartCnt;
}
