#pragma once

#include <QString>

// 加载 QSS 并处理 @N$ DPI 占位
void InitMainScreenDpi();
float GetSystemDpi();
int SysDpiZoom(int nSize);
QString ProcessDpiStyleSheet(const QString& rawQss);
void InitSystemDpi(const QString& qssPath = QStringLiteral(":/style/dark/stylesheet/style.css"));
