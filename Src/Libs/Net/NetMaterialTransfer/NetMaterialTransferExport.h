#pragma once
#define NETMATERIALTRANSFER_STATIC_LIB
#include <QList>
#include <QString>
#include <QUuid>
#include <functional>
#include <string>
#include <utility>
#include <vector>
#include <qglobal.h>

#if defined(NETMATERIALTRANSFER_LIBRARY)
#  define NETMATERIALTRANSFER_EXPORT Q_DECL_EXPORT
#elif defined(NETMATERIALTRANSFER_STATIC_LIB)
#  define NETMATERIALTRANSFER_EXPORT
#else
#  define NETMATERIALTRANSFER_EXPORT Q_DECL_IMPORT
#endif

namespace MaterialTransfer
{
	inline void InitMaterialTransfer(bool) {}
	inline void UnitMaterialTransfer() {}
	inline bool RequestTransfer(const QUuid&, const QString&, const std::string& = std::string()) { return false; }
	inline void CancelTransfer(const QUuid&) {}
}
