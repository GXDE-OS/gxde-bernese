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

#ifndef WINDOW_UTILS_WINDOW_GEOMETRY_H_
#define WINDOW_UTILS_WINDOW_GEOMETRY_H_

#include "QPoint"
#include "QRect"
#include "QScreen"
#include "QWidget"
#include "QWindow"

#include "bernese/export.h"

namespace bernese {
namespace window_utils {

/**
 * @brief Converts a rectangle from logical pixels to native pixels.
 *
 * Scales the offset of @p rect from @p screen_origin by @p scale.
 * Needed for high-DPI and multi-monitor setups.
 *
 * @note This is for X11.
 * @param rect Rectangle in logical pixels.
 * @param screen_origin Screen origin (same in logical and native coords).
 * @param scale Device pixel ratio, must be finite and > 0.
 * @return Rectangle in native pixels, or an empty QRect if @p scale is
 *         invalid.
 */
BERNESE_EXPORT QRect ToNativePixels(
    const QRect& rect, const QPoint& screen_origin, qreal scale);

/**
 * @brief Converts a rectangle from native pixels to logical pixels.
 *
 * Inverse of ToNativePixels(). Implemented as
 * ToNativePixels(rect, screen_origin, 1 / scale).
 *
 * @note This is for X11.
 * @param rect Rectangle in native pixels.
 * @param screen_origin Screen origin (same in native and logical coords).
 * @param scale Device pixel ratio, must be finite and > 0.
 * @return Rectangle in logical pixels, or an empty QRect if @p scale is
 *         invalid.
 */
BERNESE_EXPORT QRect FromNativePixels(
    const QRect& rect, const QPoint& screen_origin, qreal scale);

/**
 * @brief Read a top-level window's client rectangle in X11 native pixels.
 *
 * Uses public Qt geometry and the window's own screen and pixel ratio. Call
 * on the GUI thread. This does not create a platform window. Intended for X11
 * event coordinates and foreign windows; Wayland has no global window origin.
 * Qt exposes integer logical geometry, so reconstructing native pixels can
 * round boundaries that are not representable at the window's current scale.
 * @param window Borrowed top-level window, which may be null.
 * @return Native rectangle, or an empty rectangle if window or screen is null.
 */
BERNESE_EXPORT QRect NativeWindowGeometry(const QWindow* window);

/**
 * @brief Moves a top-level widget to a screen and sets its geometry.
 *
 * Clamps the size to the widget's min/max size. Sets the geometry on both
 * the QWidget and its QWindow; the QWindow call fixes stale native sizes
 * after a screen or DPI change.
 *
 * @param widget Top-level widget. No-op if null or not a window.
 * @param screen Target screen, or null to keep the current screen.
 * @param rect   New geometry in logical pixels.
 */
BERNESE_EXPORT void SetTopLevelGeometry(
    QWidget* widget, QScreen* screen, const QRect& rect);

/**
 * @brief Moves a top-level window to a screen and sets its geometry.
 *
 * QWindow version of SetTopLevelGeometry(), for windows with no widget,
 * like QQuickWindow. The size stays within the min and max size.
 *
 * @param window Top-level window. No-op if null or if it has a parent.
 * @param screen Target screen, or null to keep the current screen.
 * @param rect   New geometry in logical pixels.
 */
BERNESE_EXPORT void SetTopLevelWindowGeometry(
    QWindow* window, QScreen* screen, const QRect& rect);

}  // namespace window_utils
}  // namespace bernese

#endif  // WINDOW_UTILS_WINDOW_GEOMETRY_H_
