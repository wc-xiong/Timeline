#pragma once

#include <qglobal.h>

#if defined(HISTORYSTACK_LIBRARY)
#  define HISTORYSTACK_EXPORT Q_DECL_EXPORT
#elif  defined(HISTORYSTACK_STATIC_LIB) // Abuse single files for manual tests
#  define HISTORYSTACK_EXPORT
#else
#  define HISTORYSTACK_EXPORT Q_DECL_IMPORT
#endif