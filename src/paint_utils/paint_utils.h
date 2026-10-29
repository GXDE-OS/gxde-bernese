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

#ifndef PAINT_UTILS_PAINT_UTILS_H_
#define PAINT_UTILS_PAINT_UTILS_H_

#include "QImage"
#include "QPainter"
#include "QRect"

#include "bernese/export.h"

namespace bernese {
namespace paint_utils {

/**
 * @brief Copy pixels that are already painted on the device of a painter.
 *
 * In a widget paint event, the painter draws into the window backing store.
 * This function reads what the window has painted there so far, for example
 * the content under a widget that wants to blur it. It replaces
 * QPlatformBackingStore::toImage().
 *
 * @note Call it while the painter is active, before you paint over the area.
 * @param painter Active borrowed painter, for example one opened on a widget
 *                in its paint event.
 * @param rect Area in the logical coordinates of the painter.
 * @return Copy in device pixels. Like QImage::copy(), parts outside the
 *         device are filled with zero bytes, which is transparent for formats
 *         with alpha. Its device pixel ratio is the one of the painted widget
 *         or image. Returns a null image when the painter is inactive or its
 *         device is neither a QImage nor a QPixmap.
 */
BERNESE_EXPORT QImage CopyPaintedArea(QPainter* painter, const QRect& rect);

}  // namespace paint_utils
}  // namespace bernese

#endif  // PAINT_UTILS_PAINT_UTILS_H_
