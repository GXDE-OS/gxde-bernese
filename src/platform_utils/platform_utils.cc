/*
 * Copyright (C) 2026 CharOfString <root@charofstring.cc>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "platform_utils/platform_utils.h"

#include "QCoreApplication"
#include "QDir"
#include "QFileInfo"
#include "QJsonArray"
#include "QJsonObject"
#include "QJsonValue"
#include "QPluginLoader"
#include "QString"
#include "QStringList"

namespace bernese {
namespace platform_utils {

namespace {

// Interface ID that every Qt platform plugin declares in its metadata.
const char kPlatformInterface[] =
    "org.qt-project.Qt.QPA.QPlatformIntegrationFactoryInterface";

/**
 * @brief Collect the folders that Qt searches for platform plugins.
 *
 * @return Folder paths in search order. Some of them may not exist.
 */
QStringList PluginFolders() {
  QStringList folders;
  const QString extra_path =
      qEnvironmentVariable("QT_QPA_PLATFORM_PLUGIN_PATH");
  if (!extra_path.isEmpty()) {
    folders << extra_path;
  }

  for (const QString& path : QCoreApplication::libraryPaths()) {
    folders << path + QStringLiteral("/platforms");
  }

  return folders;
}

/**
 * @brief Read the platform keys of one plugin file.
 *
 * @param file_path Path to a shared library file.
 * @return Keys from its metadata, or an empty list when it is not a Qt
 *         platform plugin.
 */
QStringList KeysOfPlugin(const QString& file_path) {
  // metaData() only reads the file. It does not run any plugin code.
  const QJsonObject meta = QPluginLoader(file_path).metaData();
  const QString interface = meta.value(QStringLiteral("IID")).toString();
  if (!interface.startsWith(QLatin1String(kPlatformInterface))) {
    return QStringList();
  }

  QStringList keys;
  const QJsonObject data = meta.value(QStringLiteral("MetaData")).toObject();
  for (const QJsonValue& key : data.value(QStringLiteral("Keys")).toArray()) {
    keys << key.toString();
  }

  return keys;
}

}  // namespace

QStringList PlatformPluginKeys() {
  QStringList keys;
  for (const QString& folder : PluginFolders()) {
    const QFileInfoList files = QDir(folder).entryInfoList(QDir::Files);
    for (const QFileInfo& file : files) {
      keys << KeysOfPlugin(file.absoluteFilePath());
    }
  }

  keys.removeDuplicates();
  return keys;
}

bool HasPlatformPlugin(const QString& key) {
  return PlatformPluginKeys().contains(key, Qt::CaseInsensitive);
}

}  // namespace platform_utils
}  // namespace bernese
