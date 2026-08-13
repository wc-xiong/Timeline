#pragma once

#include "ResourceModelGlobal.h"

#include <QString>

/// 将控制端源路径规范为缓存相对路径（如 Resources/C/...）。
RESOURCEMODEL_EXPORT QString NormalizeMaterialCacheRelativePath(const QString& sourcePath);

/// 缓存根目录 + 相对路径，与素材传输落盘路径一致。
RESOURCEMODEL_EXPORT QString MakeMaterialCacheAbsolutePath(const QString& sourcePath);
