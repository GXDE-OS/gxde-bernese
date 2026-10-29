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

#ifndef WINDOW_EFFECTS_WAYLAND_EFFECTS_H_
#define WINDOW_EFFECTS_WAYLAND_EFFECTS_H_

#include "QList"
#include "QRect"

QT_BEGIN_NAMESPACE
class QWindow;
QT_END_NAMESPACE

namespace bernese {
namespace window_effects {
namespace internal {

/**
 * @brief Check whether Qt runs on Wayland.
 *
 * @return True when Qt uses its Wayland platform plugin.
 */
bool IsWayland();

/**
 * @brief Blur what is behind a Wayland window.
 *
 * Uses org_kde_kwin_blur on the wl_surface of the window, like
 * DBlurEffectWidget.
 * @param window Shown window, may be null. Not owned.
 * @param areas Areas in logical pixels. Empty means the whole window, also
 *        after a resize.
 * @return True when the blur was sent.
 */
bool SetWaylandBlur(QWindow* window, const QList<QRect>& areas);

/**
 * @brief Remove the blur set by SetWaylandBlur().
 *
 * @param window Shown window, may be null. Not owned.
 * @return True when the request was sent.
 */
bool ClearWaylandBlur(QWindow* window);

/**
 * @brief Check whether the compositor has the blur protocol.
 *
 * @return True when org_kde_kwin_blur_manager exists.
 */
bool HasWaylandBlur();

/**
 * @brief Set the corner radius the compositor draws for a Wayland window.
 *
 * Uses the WindowRadius property of dde_shell, like DBlurEffectWidget.
 * @param window Shown window, may be null. Not owned.
 * @param radius Corner radius in logical pixels.
 * @return True when the radius was sent.
 */
bool SetWaylandWindowRadius(QWindow* window, int radius);

}  // namespace internal
}  // namespace window_effects
}  // namespace bernese

#endif  // WINDOW_EFFECTS_WAYLAND_EFFECTS_H_
