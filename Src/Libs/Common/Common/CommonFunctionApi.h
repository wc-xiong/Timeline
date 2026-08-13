#pragma once
#include <QString>
#include <QLatin1Char>
#include <QTextCodec>
#include <QTime>
#include <QRgb>
#include <QColor>
#include <Shlwapi.h>
#include <windows.h>
#include <WinSvc.h>
#include <processthreadsapi.h>
#include <shobjidl.h>
#include <ShlObj_core.h>
#include <ShlObj.h>

#include <WtsApi32.h>
#include <Userenv.h>
#include <TlHelp32.h>

#include <vector>
#include <QDir>
#include <QProcess>

#include <codecvt>
#include <string>

#include <QPainterPath>

#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Shlwapi.lib")

#pragma warning( disable : 4996 )

#define SafeDelete(p)		do{ if(p){delete p; p=nullptr;}}while(0)
#define SafeDeleteArray(p)	do{ if(p){delete []p; p=nullptr;}}while(0)

#define QI(if, src, dst) (src)->QueryInterface(IID_##if, (void**)(dst.ReleaseAndGetAddressOf()))

//将int转化成时间形式（00:00:00.000）
QString FormatMsTime2String(__int64 nLength);

// int转化成QTime对象（int传入毫秒值）
QTime IntConvertToTime(__int64 nMS);

// QTime转化成Int（毫秒为单位）
__int64 TimeConvertToIntMs(QTime time);

//////////////////////////////////////////////////////////////////////////
inline QString FormatMsTime2String(__int64 nLength)
{
	QString strTime;
	int hh = (int)(nLength / 3600000);
	int mm = (int)((nLength - hh * 3600000) / 60000);
	int ss = (int)(nLength - hh * 3600000 - mm * 60000) / 1000;
	int ms = (int)(nLength - hh * 3600000 - mm * 60000 - ss * 1000);

	strTime = QString("%1:%2:%3.%4")
		.arg(hh, 2, 10, QLatin1Char('0'))
		.arg(mm, 2, 10, QLatin1Char('0'))
		.arg(ss, 2, 10, QLatin1Char('0'))
		.arg(ms, 3, 10, QLatin1Char('0'));	//(int，位数，进制，不够位数所补充字符)
	return strTime;
}

inline QTime IntConvertToTime(__int64 nMS)
{
	int _ms = nMS % 1000;
	int _s = nMS / 1000 % 60;
	int _min = nMS / 1000 / 60 % 60;
	int _h = nMS / 1000 / 60 / 60;
	return QTime(_h, _min, _s, _ms);
}

inline __int64 TimeConvertToIntMs(QTime time)
{
	return (time.hour() * 3600 + time.minute() * 60 + time.second()) * 1000 + time.msec();
}

// 高精度 CPU 时间
static inline __int64 QueryHighTickCountUS()
{
	LARGE_INTEGER f, t;
	if (!QueryPerformanceFrequency(&f) || !QueryPerformanceCounter(&t))
		return 0;

	return t.QuadPart / (f.QuadPart / 1000000);
}

static inline __int64 QueryHighTickCountMS()
{
	return QueryHighTickCountUS() / 1000;
}

static inline __int64 QueryHighTickCountS()
{
	return QueryHighTickCountMS() / 1000;
}

inline QString BSTRToQString(BSTR bstr)
{
	//char buf[260] = { 0 };
	//int len = WideCharToMultiByte(CP_ACP, 0, bstr, wcslen(bstr), NULL, 0, NULL, NULL);
	//WideCharToMultiByte(CP_ACP, 0, bstr, wcslen(bstr), buf, len, NULL, NULL);
	BSTR dstCvt = SysAllocString(bstr);

	return QString::fromUtf16(reinterpret_cast<ushort*>(dstCvt));
}

inline bool IsValidFileName(QString& strName)
{
	for (auto i : strName)
	{
		if (i == '\\')
			return false;
		else if (i == '/')
			return false;
		else if (i == ':')
			return false;
		else if (i == '*')
			return false;
		else if (i == '?')
			return false;
		else if (i == '\"')
			return false;
		else if (i == '<')
			return false;
		else if (i == '>')
			return false;
		else if (i == '|')
			return false;
	}
	return strName.length() > 0; // 长度不为 0，至少包含一个非空白字符
}


static bool IsProcessExist(QString strExeName)
{
	bool bIsExist = false; // 进程是否存在
	QProcess process;
	process.start("tasklist.exe");
	if (process.waitForFinished())
	{
		QByteArray result = process.readAll();
		QString exe = QString(result);
		if (exe.contains(strExeName))
		{
			bIsExist = true; // 找到了
		}
	}
	process.kill();

	return bIsExist;
}

// 判断是否在SYSTEM权限下
static bool IsSystemPrivilegeImp()
{
	static bool isSystemPrivilege = false;
	if (isSystemPrivilege)
	{
		return isSystemPrivilege;
	}

	char szPath[MAX_PATH] = { 0 };
	if (::SHGetSpecialFolderPathA(NULL, szPath, CSIDL_APPDATA, TRUE))
	{
		std::string flag("config\\systemprofile");
		std::string path(szPath);
		if (path.find(flag) != std::string::npos)
		{
			isSystemPrivilege = true;
		}
	}

	return isSystemPrivilege;
}

// 模拟当前登陆用户
static bool ImpersonateLoddedOnUserWrapper(HANDLE& hToken)
{
	DWORD dwConsoleSessionId = WTSGetActiveConsoleSessionId();
	if (WTSQueryUserToken(dwConsoleSessionId, &hToken))
	{
		if (ImpersonateLoggedOnUser(hToken))
			return true;
	}

	return false;
}

// 获取正确的目录
static BOOL WINAPI SHGetSpecialFolderPathWrapper(HWND hwnd, LPWSTR lpszPath, int csidl, BOOL fCreate)
{
	BOOL ret = FALSE;
	do {
		if (!IsSystemPrivilegeImp())
		{
			if (SHGetSpecialFolderPath(hwnd, lpszPath, csidl, fCreate))
			{
				ret = TRUE;
				break;
			}
		}

		HANDLE hToken = NULL;
		if (!ImpersonateLoddedOnUserWrapper(hToken))
			break;

		if (S_OK == SHGetFolderPath(NULL, csidl, hToken, SHGFP_TYPE_DEFAULT, lpszPath))
		{
			ret = TRUE;
			RevertToSelf();

			break;
		}

	} while (0);

	return ret;
}

static BOOL	IsPathExist(LPCTSTR lpPath)// 判断路径是否存在
{
	DWORD dwAttrib = GetFileAttributes(lpPath);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

static BOOL MakeSureDirectoryPathExistsEx(LPCTSTR lpPath)// 确保指定的路径一定存在(如果不存在，则新建)
{
	if (lpPath == NULL)
		return FALSE;

	if (IsPathExist(lpPath))
		return TRUE;

	int err = SHCreateDirectoryEx(NULL, lpPath, NULL);
	if (err != ERROR_SUCCESS && err != ERROR_FILE_EXISTS && err != ERROR_ALREADY_EXISTS)
		return FALSE;

	return TRUE;
}

static QString GetMyDocumentPath()	//获取我的文档
{
	QString targetPath;

	TCHAR szPath[MAX_PATH + 1];

	if (SHGetSpecialFolderPathWrapper(NULL, szPath, CSIDL_PERSONAL, FALSE))
	{
#ifndef GRANDSHOW_FE
		targetPath = QString::fromStdWString(szPath) + "\\ColorLight\\GrandShow CE";
#else
		targetPath = QString::fromStdWString(szPath) + "\\ColorLight\\GrandShow CE Basic";
#endif
	}

	return targetPath;
}

static QString GetAppPath()// 获得APP执行路径的绝对路径
{
	QString strAppPath;
	if (strAppPath.isEmpty())
	{
		HMODULE hModule = GetModuleHandle(NULL);
		assert(hModule);

		TCHAR tcPath[_MAX_PATH];
		ZeroMemory(tcPath, sizeof(TCHAR) * _MAX_PATH);
		DWORD dwRet = GetModuleFileName(hModule, tcPath, _MAX_PATH);

		if (dwRet == 0)
			return "";

		strAppPath = QString::fromStdWString(tcPath);
		int pos = strAppPath.lastIndexOf('\\');
		assert(pos != -1);
		strAppPath = strAppPath.left(pos + 1);
	}

	return strAppPath;
}

static QString GetFileNameFromPath(const QString& filePath, bool bWithEx)	//根据路径获取文件名，可携带后缀
{
	QDir dir(filePath);

	return dir.dirName();
}

inline QColor StringToColor(const QString & mRgbStr)
{
    QColor color;
    quint64 colorInt = mRgbStr.toUInt(NULL,16);
    color = QColor(colorInt);
    color.setAlpha(colorInt >> 24);
    return color;
}

inline QString ColorToString(const QColor & color)
{
    QRgb mRgb = qRgba(color.red(), color.green(), color.blue(), color.alpha());
    QString mRgbStr = QString::number(mRgb, 16);
    return mRgbStr;
}

static std::string ToStdString(std::wstring& wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return std::string(converter.to_bytes(wstr));

	// 另一版本转化
// 	std::string strLocale = setlocale(LC_ALL, "");
// 	const wchar_t *wchSrc = wstr.c_str();
// 	size_t nDestSize = wcstombs(NULL, wchSrc, 0) + 1;
// 	char *chDest = new char[nDestSize];
// 	memset(chDest, 0, nDestSize);
// 	wcstombs(chDest, wchSrc, nDestSize);
// 	std::string strResult = chDest;
// 	delete[] chDest;
// 	setlocale(LC_ALL, strLocale.c_str());
// 	return strResult;
}

static std::wstring ToStdWString(std::string str)
{
// 	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
// 	return std::wstring(converter.from_bytes(str));

	std::string strLocale = setlocale(LC_ALL, "");
	const char *chSrc = str.c_str();
	size_t nDestSize = mbstowcs(NULL, chSrc, 0) + 1;
	wchar_t *wchDest = new wchar_t[nDestSize];
	wmemset(wchDest, 0, nDestSize);
	mbstowcs(wchDest, chSrc, nDestSize);
	std::wstring wstrResult = wchDest;
	delete[] wchDest;
	setlocale(LC_ALL, strLocale.c_str());
	return wstrResult;

	//return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().from_bytes(str);
}

static QPainterPath GetPainterRoundPath(const QRect &rcDraw, float fRadius)
{
	QPainterPath path;

	const QSize szRectRound = QSize(fRadius * 2, fRadius * 2);

	//设置起点为矩形左上圆角圆心
	path.moveTo(rcDraw.topLeft().x() + fRadius, rcDraw.topLeft().y());
	// 左上角圆角
	path.arcTo(QRect(rcDraw.topLeft(), szRectRound), 90, 90);
	path.lineTo(rcDraw.bottomLeft().x(), rcDraw.bottomLeft().y() - fRadius);

	// 左下角圆角
	path.arcTo(QRect(QPoint(rcDraw.bottomLeft().x(), rcDraw.bottomLeft().y() - fRadius * 2), szRectRound), 180, 90);
	path.lineTo(QPoint(rcDraw.bottomRight().x() - fRadius, rcDraw.bottomRight().y()));

	// 右下角圆角
	path.arcTo(QRect(QPoint(rcDraw.bottomRight().x() - fRadius * 2, rcDraw.bottomRight().y() - fRadius * 2), szRectRound), 270, 90);
	path.lineTo(rcDraw.topRight().x(), rcDraw.topRight().y() + fRadius);

	// 左上角圆角
	path.arcTo(QRect(QPoint(rcDraw.topRight().x() - (fRadius * 2), rcDraw.topRight().y()), szRectRound), 0, 90);
	path.lineTo(rcDraw.topLeft().x() + fRadius, rcDraw.topLeft().y());

	return path;
}

// 将字符串转换为小写
inline std::string ToLowerCase(const std::string& str)
{
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}


// 
static QString HexToQString(const QByteArray& data, bool bChinese)
{
	QString str;
	if (bChinese)
	{
		QTextCodec *codec = QTextCodec::codecForName("GBK");
		str = codec->toUnicode(QByteArray::fromHex(data));
	}
	else
	{
		str = QString(QByteArray::fromHex(data));
	}

	return str;
}

static bool FloatIsEqual(float num1, float num2, float epsilon = 1e-4)
{
	return std::fabs(num1 - num2) < epsilon;
}