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

#ifndef IMAGE_UTILS_IMAGE_UTILS_H_
#define IMAGE_UTILS_IMAGE_UTILS_H_

#include "QImage"
#include "QString"
#include "QtGlobal"

#include "bernese/export.h"

namespace bernese {
namespace image_utils {

/**
 * @brief Resolve a Qt-style high-DPI asset path.
 *
 * Prefers the smallest adequate scale, otherwise the largest available.
 * Honors QT_HIGHDPI_DISABLE_2X_IMAGE_LOADING.
 *
 * @param path Original asset path, possibly already carrying a scale suffix.
 * @param scale Requested device scale.
 * @param source_scale Non-null output receiving the selected scale, or 1.
 * @return Selected asset path, or the original path if no alternative is found.
 */
BERNESE_EXPORT QString ScaledAssetPath(
    const QString& path, qreal scale, qreal* source_scale);

/**
 * @brief Apply an in-place approximation of Gaussian blur.
 *
 * Uses three separable box passes on premultiplied channels. Null images
 * and non-positive radii are unchanged; other images become ARGB32
 * premultiplied.
 *
 * @param image Image to modify; its device pixel ratio is preserved.
 * @param radius Blur radius in physical image pixels.
 */
BERNESE_EXPORT void Blur(QImage& image, qreal radius);

}  // namespace image_utils
}  // namespace bernese

#endif  // IMAGE_UTILS_IMAGE_UTILS_H_
