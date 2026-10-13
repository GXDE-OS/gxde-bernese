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

#ifndef IMAGE_UTILS_CHANNEL_SUM_CHANNEL_SUM_H_
#define IMAGE_UTILS_CHANNEL_SUM_CHANNEL_SUM_H_

#include "QColor"
#include "QtGlobal"

namespace bernese {
namespace image_utils {
namespace internal {

// Sum premultiplied channels together to preserve color at transparent edges.
struct ChannelSum {
  qint64 red = 0;
  qint64 green = 0;
  qint64 blue = 0;
  qint64 alpha = 0;

  /**
   * @brief Add or subtract one premultiplied pixel from the running sum.
   *
   * @param pixel Premultiplied pixel channels.
   * @param weight Use 1 to add or -1 to remove a sample.
   */
  void Accumulate(QRgb pixel, int weight) {
    red += weight * qRed(pixel);
    green += weight * qGreen(pixel);
    blue += weight * qBlue(pixel);
    alpha += weight * qAlpha(pixel);
  }

  /**
   * @brief Compute the averaged premultiplied pixel.
   *
   * @param sample_count Positive number of samples represented by the sum.
   * @return Packed averaged channels.
   */
  QRgb Average(int sample_count) const {
    return qRgba(red / sample_count, green / sample_count, blue / sample_count,
        alpha / sample_count);
  }
};

}  // namespace internal
}  // namespace image_utils
}  // namespace bernese

#endif  // IMAGE_UTILS_CHANNEL_SUM_CHANNEL_SUM_H_
