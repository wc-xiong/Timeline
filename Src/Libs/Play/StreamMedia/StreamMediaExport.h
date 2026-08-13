#pragma once
#define STREAMMEDIA_STATIC_LIB
#include <QUuid>
#include <QString>
#include <list>
#include <memory>

class IResourceBase;

inline bool StreamMedia_Final() { return true; }
inline void StreamMedia_UnBindResDestroy(const std::shared_ptr<IResourceBase>&) {}
inline void StreamMedia_DealStreamAlwaysOpen(const std::shared_ptr<IResourceBase>&) {}
inline void StreamMedia_DestroyBeforeLoadPrj(std::list<std::shared_ptr<IResourceBase>>) {}
inline bool StreamMedia_DestroyStream(int, const QString&, bool, bool = false) { return true; }
inline void StreamMedia_SetStreamBindResPlayId(int, const QString&, const QUuid&, bool) {}
inline int StreamMedia_GetStreamBindIdCount(int, const QString&) { return 0; }
