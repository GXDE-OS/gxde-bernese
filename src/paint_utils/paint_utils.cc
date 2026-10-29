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

#include "paint_utils/paint_utils.h"

#include "QImage"
#include "QPaintDevice"
#include "QPaintEngine"
#include "QPainter"
#include "QPixmap"
#include "QRect"

namespace bernese {
namespace paint_utils {

QImage CopyPaintedArea(QPainter* painter, const QRect& rect) {
  if (painter == nullptr || !painter->isActive()) {
    return QImage();
  }

  // The engine device is the real target. For a widget this is the backing
  // store image, not the widget itself.
  QPaintDevice* device = painter->paintEngine()->paintDevice();
  const QRect device_rect = painter->deviceTransform().mapRect(rect);
  QImage area;
  if (const auto* image = dynamic_cast<const QImage*>(device)) {
    // Copy through a const reference. Keeping a shallow copy alive would make
    // the next painter on this image detach it from the backing store.
    area = image->copy(device_rect);
  } else if (const auto* pixmap = dynamic_cast<const QPixmap*>(device)) {
    area = pixmap->toImage().copy(device_rect);
  } else {
    return QImage();
  }

  area.setDevicePixelRatio(painter->device()->devicePixelRatioF());
  return area;
}

}  // namespace paint_utils
}  // namespace bernese
