#pragma once

// MSVC + Qt5: QStringLiteral("中文") 展开为 u"" "中文"（见 qstringliteral.h）。
// MSVC 拼接 UTF-16 与窄字符串时，即使加了 /utf-8，仍常按系统代码页解读窄串，
// 导致中文乱码且二进制中找不到正确 UTF-16。
// 将参数直接作为 u"..." 字面量，避免错误拼接。
#if defined(_MSC_VER)
#  include <QtCore/qstringliteral.h>
#  undef QT_UNICODE_LITERAL
#  define QT_UNICODE_LITERAL(str) u##str
#endif
