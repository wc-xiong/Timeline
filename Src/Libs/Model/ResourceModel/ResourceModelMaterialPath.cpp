#include "ResourceModelMaterialPath.h"

#include "Model/GlobalModel/DisplayerDataMgr.h"

#include <QDir>
#include <QFileInfo>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

QString NormalizeMaterialCacheRelativePath(const QString& path)
{
    if (path.isEmpty())
        return path;

    QString normalized = QDir::fromNativeSeparators(path);
    const QFileInfo info(normalized);
    if (info.isAbsolute())
        normalized = info.absoluteFilePath();
    normalized = QDir::fromNativeSeparators(normalized);

    if (normalized.size() >= 2 && normalized.at(1) == QLatin1Char(':'))
    {
        normalized.remove(QLatin1Char(':'));
        normalized[0] = normalized.at(0).toUpper();
    }

    return QStringLiteral("Resources/%1").arg(normalized);
}

QString MakeMaterialCacheAbsolutePath(const QString& sourcePath)
{
    const QString relativePath = NormalizeMaterialCacheRelativePath(sourcePath);
    if (relativePath.isEmpty())
        return QString();

    const QString cacheRoot = g_pDisplayDataMgr->GetFileCachePath();
    if (cacheRoot.isEmpty())
        return QString();

    return QStringLiteral("%1/%2").arg(cacheRoot, relativePath);
}
