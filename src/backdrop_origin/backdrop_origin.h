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

#ifndef BACKDROP_ORIGIN_BACKDROP_ORIGIN_H_
#define BACKDROP_ORIGIN_BACKDROP_ORIGIN_H_

#include "QPoint"
#include "QRect"
#include "QSize"
#include "QString"

#include "bernese/export.h"

namespace bernese {
namespace wayland {

/**
 * @brief Convert a native work area into logical output coordinates.
 *
 * Uses the ratio of output dimensions, not the Wayland buffer DPR.
 *
 * @param native_output Output rectangle in the native coordinate space.
 * @param native_area Work area in the same native coordinate space.
 * @param logical_output Corresponding output rectangle in logical coordinates.
 * @return Clipped logical work area, or an empty rectangle for invalid
 * geometry.
 */
BERNESE_EXPORT QRect LogicalWorkArea(const QRect& native_output,
    const QRect& native_area, const QRect& logical_output);

/**
 * @brief Read the logical work area for a named XWayland output.
 *
 * Uses a separate, read-only, process-lifetime XCB connection through DISPLAY.
 * Call on the GUI thread. Missing DISPLAY, outputs or EWMH data are tolerated.
 *
 * @param output_name Exact RandR output name to match.
 * @param logical_output Output rectangle in logical coordinates.
 * @return Converted work area, or an empty rectangle when lookup fails.
 */
BERNESE_EXPORT QRect XwaylandWorkArea(
    const QString& output_name, const QRect& logical_output);

/**
 * @brief Locate a desktop surface within its wallpaper.
 *
 * @param screen Output rectangle in logical desktop coordinates.
 * @param surface Desktop surface size in logical pixels.
 * @param available Available work area in the same coordinates as screen.
 * @param fallback Origin to use when the work area does not match.
 * @return Output-local work-area origin, zero for a full-size surface, or
 * fallback.
 */
BERNESE_EXPORT QPoint BackdropOrigin(const QRect& screen, const QSize& surface,
    const QRect& available, const QPoint& fallback);

}  // namespace wayland
}  // namespace bernese

#endif  // BACKDROP_ORIGIN_BACKDROP_ORIGIN_H_
