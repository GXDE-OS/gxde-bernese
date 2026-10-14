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

#include "window_utils/window_geometry.h"

#include <cmath>

#include <QScreen>

namespace bernese {
namespace window_utils {

QRect ToNativePixels(
    const QRect& rect, const QPoint& screen_origin, qreal scale) {
  if (!std::isfinite(scale) || scale <= 0) {
    return {};
  }

  const QPoint offset = rect.topLeft() - screen_origin;
  const QPoint position = screen_origin +
      QPoint(qRound(offset.x() * scale), qRound(offset.y() * scale));

  const QSize size(qRound(rect.width() * scale), qRound(rect.height() * scale));
  return QRect(position, size);
}

QRect FromNativePixels(
    const QRect& rect, const QPoint& screen_origin, qreal scale) {
  if (!std::isfinite(scale) || scale <= 0) {
    return {};
  }

  return ToNativePixels(rect, screen_origin, 1 / scale);
}

QRect NativeWindowGeometry(const QWindow* window) {
  if (!window || !window->screen()) {
    return {};
  }

  return ToNativePixels(window->geometry(),
      window->screen()->geometry().topLeft(), window->devicePixelRatio());
}

void SetTopLevelGeometry(
    QWidget* widget, QScreen* screen, const QRect& rect) {
  if (!widget || !widget->isWindow()) {
    return;
  }

  if (screen) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    widget->setScreen(screen);
#else
    if (QWindow* window = widget->windowHandle()) {
      window->setScreen(screen);
    }
#endif
  }

  const QRect requested(rect.topLeft(),
      rect.size().boundedTo(widget->maximumSize())
          .expandedTo(widget->minimumSize()));
  widget->setGeometry(requested);

  // QWidget caches moves while hidden and skips unchanged logical geometry.
  // QWindow reads the platform geometry, so it can detect a stale native size.
  if (QWindow* window = widget->windowHandle()) {
    window->setGeometry(requested);
  }
}

}  // namespace window_utils
}  // namespace bernese
