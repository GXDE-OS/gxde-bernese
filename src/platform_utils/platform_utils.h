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

#ifndef PLATFORM_UTILS_PLATFORM_UTILS_H_
#define PLATFORM_UTILS_PLATFORM_UTILS_H_

#include "QString"
#include "QStringList"

#include "bernese/export.h"

namespace bernese {
namespace platform_utils {

/**
 * @brief List the keys of all installed Qt platform plugins.
 *
 * Reads the plugin metadata without loading the plugins. Searches the
 * folder in QT_QPA_PLATFORM_PLUGIN_PATH and the "platforms" folder of every
 * Qt library path, like Qt does. It replaces
 * QPlatformIntegrationFactory::keys().
 *
 * @note It also works before the application object exists, so it can be
 *       used to choose QT_QPA_PLATFORM.
 * @return Plugin keys such as "xcb" or "wayland", without duplicates.
 */
BERNESE_EXPORT QStringList PlatformPluginKeys();

/**
 * @brief Check whether a Qt platform plugin is installed.
 *
 * @param key Plugin key such as "dxcb"; compared without case, like Qt.
 * @return True when PlatformPluginKeys() contains key.
 */
BERNESE_EXPORT bool HasPlatformPlugin(const QString& key);

}  // namespace platform_utils
}  // namespace bernese

#endif  // PLATFORM_UTILS_PLATFORM_UTILS_H_
