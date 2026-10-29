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

#ifndef WINDOW_UTILS_WINDOW_STACK_H_
#define WINDOW_UTILS_WINDOW_STACK_H_

#include "QPoint"
#include "QStringList"

#include "bernese/export.h"

namespace bernese {
namespace window_utils {

/**
 * @brief Get the WM_CLASS of the top visible window at a point.
 *
 * Replaces DWindowManagerHelper::currentWorkspaceWindows(), which needs
 * D-XCB. It goes through _NET_CLIENT_LIST_STACKING from the top and skips
 * windows that are not mapped, so other workspaces do not count.
 *
 * Call on the GUI thread.
 * @param native_pos Point in native X11 root coordinates.
 * @return Instance name and class name from WM_CLASS. Empty when no window
 *         is there, it has no WM_CLASS, or off X11.
 */
BERNESE_EXPORT QStringList TopLevelWindowClassAt(const QPoint& native_pos);

}  // namespace window_utils
}  // namespace bernese

#endif  // WINDOW_UTILS_WINDOW_STACK_H_
