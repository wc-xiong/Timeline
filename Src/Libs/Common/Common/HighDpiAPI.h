#pragma once

#include <qlibrary.h>
#include <qsysinfo.h>
#include <qt_windows.h>
#include <Tlhelp32.h>

#include "Common/Common/CommonGlobal.h"

class User32DLL
{
public:
	User32DLL()
		: setProcessDPIAware(0)
	{
		library.setFileName(QStringLiteral("User32"));
		if (!library.load())
			return;
		setProcessDPIAware = (SetProcessDPIAware)library.resolve("SetProcessDPIAware");
	}

	bool isValid() const
	{
		return setProcessDPIAware;
	}

	typedef BOOL(WINAPI *SetProcessDPIAware)();

	// Windows Vista onwards
	SetProcessDPIAware setProcessDPIAware;

private:
	QLibrary library;
};

// This must match PROCESS_DPI_AWARENESS in ShellScalingApi.h
enum DpiAwareness {
	PROCESS_PER_UNAWARE = 0,
	PROCESS_PER_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
};

// Shell scaling library (Windows 8.1 onwards)
class ShcoreDLL {
public:
	ShcoreDLL()
		: getProcessDpiAwareness(0), setProcessDpiAwareness(0)
	{
		if (QSysInfo::windowsVersion() < QSysInfo::WV_WINDOWS8_1)
			return;
		library.setFileName(QStringLiteral("SHCore"));
		if (!library.load())
			return;
		getProcessDpiAwareness = (GetProcessDpiAwareness)library.resolve("GetProcessDpiAwareness");
		setProcessDpiAwareness = (SetProcessDpiAwareness)library.resolve("SetProcessDpiAwareness");
	}

	bool isValid() const
	{
		return getProcessDpiAwareness && setProcessDpiAwareness;
	}

	typedef HRESULT(WINAPI *GetProcessDpiAwareness)(HANDLE, DpiAwareness *);
	typedef HRESULT(WINAPI *SetProcessDpiAwareness)(DpiAwareness);

	GetProcessDpiAwareness getProcessDpiAwareness;
	SetProcessDpiAwareness setProcessDpiAwareness;

private:
	QLibrary library;
};


static DWORD GetParentProcessId()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		qErrnoWarning(GetLastError(), "CreateToolhelp32Snapshot failed.");
		CloseHandle(CloseHandle);
		return NULL;
	}

	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnapshot, &pe)) {
		qWarning("Cannot retrieve parent process handle.");
		CloseHandle(hSnapshot);
		return NULL;
	}

	DWORD parentPid = NULL;
	const DWORD pid = GetCurrentProcessId();
	do {
		if (pe.th32ProcessID == pid) {
			parentPid = pe.th32ParentProcessID;
			break;
		}
	} while (Process32Next(hSnapshot, &pe));
	CloseHandle(hSnapshot);
	return parentPid;
}

void InitDpiAwareness()
{
	ShcoreDLL shcore;
	if (shcore.isValid()) {
		DpiAwareness dpiAwareness = PROCESS_PER_MONITOR_DPI_AWARE;
		const DWORD pid = GetParentProcessId();
		if (pid) {
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
			DpiAwareness parentDpiAwareness;
			HRESULT hr = shcore.getProcessDpiAwareness(hProcess, &parentDpiAwareness);
			CloseHandle(hProcess);
			if (hr == S_OK)
				dpiAwareness = PROCESS_PER_MONITOR_DPI_AWARE/*parentDpiAwareness*/;
		}
		if (shcore.setProcessDpiAwareness(dpiAwareness) != S_OK)
			qErrnoWarning(GetLastError(), "SetProcessDPIAwareness failed.");
	}
	else {
		// Fallback. Use SetProcessDPIAware unconditionally.
		User32DLL user32;
		if (user32.isValid())
			user32.setProcessDPIAware();
	}
}

enum MONITOR_DPI_TYPE {
	MDT_EFFECTIVE_DPI,
	MDT_ANGULAR_DPI,
	MDT_RAW_DPI, MDT_DEFAULT
};

static HRESULT WINAPI GetDpiForMonitor(_In_ HMONITOR hmonitor, _In_ MONITOR_DPI_TYPE dpiType, _Out_ UINT *dpiX, _Out_ UINT *dpiY)
{
	HINSTANCE hInstWinSta = LoadLibraryW(L"SHCore.dll"); if (hInstWinSta == nullptr) return E_NOINTERFACE; typedef HRESULT(WINAPI * PFN_GDFM)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*); PFN_GDFM pGetDpiForMonitor = (PFN_GDFM)GetProcAddress(hInstWinSta, "GetDpiForMonitor"); if (pGetDpiForMonitor == nullptr) return E_NOINTERFACE; return pGetDpiForMonitor(hmonitor, dpiType, dpiX, dpiY);
}

UINT g_xDPI;
UINT g_yDPI;
UINT GetDPI()
{
	// 获取DPI比例
	HRESULT hr = E_FAIL;
	hr = GetDpiForMonitor(MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTONEAREST), MDT_EFFECTIVE_DPI, (UINT*)&g_xDPI, (UINT*)&g_yDPI);
	if (hr != S_OK)
	{
		g_xDPI = GetDeviceCaps(GetDC(GetDesktopWindow()), LOGPIXELSX);
		g_yDPI = GetDeviceCaps(GetDC(GetDesktopWindow()), LOGPIXELSY);
	}
	int nScale = g_xDPI / 96.0 * 100;
	return nScale;
}