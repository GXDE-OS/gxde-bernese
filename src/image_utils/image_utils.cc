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

#include "image_utils/image_utils.h"

#include "QFileInfo"
#include "QImage"
#include "QRegularExpression"
#include "QString"
#include "QtGlobal"
#include "QtMath"

#include "image_utils/channel_sum/channel_sum.h"

namespace bernese {
namespace image_utils {

using internal::ChannelSum;

namespace {

enum class BlurAxis {
  kHorizontal,
  kVertical
};

/**
 * @brief Sample a row or column, repeating edge pixels beyond its bounds.
 *
 * @param image Nonempty ARGB32 premultiplied source.
 * @param line_index Valid row or column index for the selected axis.
 * @param position Sample position along that row or column.
 * @param axis Direction in which position advances.
 * @return Source pixel at the clamped position.
 */
QRgb SampleClamped(
    const QImage& image, int line_index, int position, BlurAxis axis) {
  const bool vertical = axis == BlurAxis::kVertical;
  const int length = vertical ? image.height() : image.width();
  const int clamped_position = qBound(0, position, length - 1);
  const int x = vertical ? line_index : clamped_position;
  const int y = vertical ? clamped_position : line_index;
  const auto* pixels = reinterpret_cast<const QRgb*>(image.constScanLine(y));
  return pixels[x];
}

/**
 * @brief Run one sliding box-filter pass along an image axis.
 *
 * Advances the window by removing one sample and adding the next.
 * Edge pixels repeat beyond the image boundary.
 *
 * @param source Nonempty ARGB32 premultiplied source.
 * @param target Distinct output image with matching size and format.
 * @param radius Positive box radius in physical pixels.
 * @param axis Direction of the one-dimensional filter.
 */
void BlurAlongAxis(
    const QImage& source, QImage& target, int radius, BlurAxis axis) {
  const bool vertical = axis == BlurAxis::kVertical;
  const int line_length = vertical ? source.height() : source.width();
  const int line_count = vertical ? source.width() : source.height();
  const int sample_count = 2 * radius + 1;

  for (int line_index = 0; line_index < line_count; ++line_index) {
    ChannelSum sum;
    for (int position = -radius; position <= radius; ++position) {
      sum.Accumulate(SampleClamped(source, line_index, position, axis), 1);
    }

    for (int position = 0; position < line_length; ++position) {
      const int x = vertical ? line_index : position;
      const int y = vertical ? position : line_index;
      auto* pixels = reinterpret_cast<QRgb*>(target.scanLine(y));
      pixels[x] = sum.Average(sample_count);

      sum.Accumulate(
          SampleClamped(source, line_index, position - radius, axis), -1);
      sum.Accumulate(
          SampleClamped(source, line_index, position + radius + 1, axis), 1);
    }
  }
}

}  // namespace

QString ScaledAssetPath(const QString& path, qreal scale, qreal* source_scale) {
  *source_scale = 1;
  const int dot = path.lastIndexOf(QLatin1Char('.'));
  const int suffix = dot > path.lastIndexOf(QLatin1Char('/'))
                         ? dot
                         : static_cast<int>(path.size());
  const QRegularExpression explicit_scale(QStringLiteral("@([2-9])x$"));
  const auto match = explicit_scale.match(path.left(suffix));
  if (match.hasMatch()) {
    *source_scale = match.captured(1).toInt();
    return path;
  }
  QString result = path;
  if (scale > 1 &&
      !qEnvironmentVariableIsSet("QT_HIGHDPI_DISABLE_2X_IMAGE_LOADING")) {
    // Prefer the smallest adequate asset, otherwise the largest available.
    for (int ratio = 9; ratio >= 2; --ratio) {
      QString candidate = path;
      candidate.insert(suffix, QStringLiteral("@%1x").arg(ratio));
      if (QFileInfo::exists(candidate) &&
          (*source_scale == 1 || ratio >= scale)) {
        result = candidate;
        *source_scale = ratio;
      }
    }
  }
  return result;
}

void Blur(QImage& image, qreal radius) {
  if (image.isNull() || radius <= 0) {
    return;
  }

  const qreal device_pixel_ratio = image.devicePixelRatio();
  image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
  const int box_radius = qMax(1, qRound(radius / 3));
  QImage buffer(image.size(), image.format());

  // Three horizontal/vertical box pairs approximate a Gaussian. Alternate the
  // buffers so no pass overwrites pixels that it still needs to read.
  for (int pass = 0; pass < 3; ++pass) {
    BlurAlongAxis(image, buffer, box_radius, BlurAxis::kHorizontal);
    BlurAlongAxis(buffer, image, box_radius, BlurAxis::kVertical);
  }

  image.setDevicePixelRatio(device_pixel_ratio);
}

}  // namespace image_utils
}  // namespace bernese
