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
 * ----------------------------------------------------------------------------
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *               2026 CharOfString
 *
 * Author: dragondjf <dingjiangfeng@deepin.com>
 *         CharOfString <root@charofstring.cc>
 *
 * Maintainer: CharOfString <root@charofstring.cc>
 *             dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 * ----------------------------------------------------------------------------
 * This class was adapted & modified from Deepin DDE.
 */

#include "QGuiApplication"
#include "QMouseEvent"
#include "QRect"
#include "QWidget"
#include "QWindow"
#include "QtGlobal"
#include "X11/Xlib.h"

#include "window_utils/window_utils.h"
#include "x11_info/x11_info.h"

namespace bernese {
namespace window_utils {

Qt::Edges EdgesAt(QWidget* widget, int x, int y, int border) {
  Qt::Edges edges;
  if (widget == nullptr) {
    return edges;
  }

  if (y >= 0 && y < border) {
    edges |= Qt::TopEdge;
  } else if (y >= widget->height() - border && y < widget->height()) {
    edges |= Qt::BottomEdge;
  }

  if (x >= 0 && x < border) {
    edges |= Qt::LeftEdge;
  } else if (x >= widget->width() - border && x < widget->width()) {
    edges |= Qt::RightEdge;
  }

  return edges;
}

void MoveWindow(
    QWidget* widget, QMouseEvent* event, const QRect& draggable_rect) {
  if (widget == nullptr) {
    return;
  }

  if (draggable_rect.contains(event->pos())) {
    widget->winId();
    if (QWindow* window = widget->window()->windowHandle()) {
      window->startSystemMove();
    }
  }
}

void ResizeWindow(QWidget* widget, QMouseEvent* event, int border) {
  if (widget == nullptr) {
    return;
  }

  const Qt::Edges edges =
      EdgesAt(widget, event->pos().x(), event->pos().y(), border);
  if (edges) {
    widget->winId();
    if (QWindow* window = widget->window()->windowHandle()) {
      window->startSystemResize(edges);
    }
  }
}

void ShowMaximizedWindow(QWidget* widget) {
  if (widget == nullptr) {
    return;
  }

  widget->showMaximized();
}

void ShowMinimizedWindow(QWidget* widget) {
  if (widget == nullptr) {
    return;
  }

  widget->showMinimized();
}

void ShowNormalWindow(QWidget* widget) {
  if (widget == nullptr) {
    return;
  }

  widget->showNormal();
}

void ToggleMaximizedWindow(QWidget* widget) {
  if (widget == nullptr) {
    return;
  }

  if (widget->isMaximized()) {
    widget->showNormal();
  } else {
    widget->showMaximized();
  }
}

bool UpdateCursorShape(QWidget* widget, int x, int y, int border) {
  if (widget == nullptr) {
    return false;
  }

  const Qt::Edges edges = EdgesAt(widget, x, y, border);
  if (!edges) {
    widget->setCursor(Qt::ArrowCursor);
    return false;
  }

  if (edges == (Qt::TopEdge | Qt::LeftEdge) ||
      edges == (Qt::BottomEdge | Qt::RightEdge)) {
    widget->setCursor(Qt::SizeFDiagCursor);
  } else if (edges == (Qt::TopEdge | Qt::RightEdge) ||
      edges == (Qt::BottomEdge | Qt::LeftEdge)) {
    widget->setCursor(Qt::SizeBDiagCursor);
  } else if (edges & (Qt::TopEdge | Qt::BottomEdge)) {
    widget->setCursor(Qt::SizeVerCursor);
  } else {
    widget->setCursor(Qt::SizeHorCursor);
  }

  return true;
}

bool SetNormalCursor(QWidget* widget) {
  if (widget == nullptr) {
    return false;
  }

  widget->setCursor(Qt::ArrowCursor);
  return true;
}

bool SetHandleCursor(QWidget* widget) {
  if (widget == nullptr) {
    return false;
  }

  widget->setCursor(Qt::PointingHandCursor);
  return true;
}

bool SetCursorByCursorType(QWidget* widget, unsigned int cursor_type) {
  if (widget == nullptr) {
    return false;
  }

  Display* display = bernese::X11Info::GetDisplay();
  if (!display) {
    return false;
  }

  const WId window = widget->winId();
  QGuiApplication::sync();
  const Cursor cursor = XCreateFontCursor(display, cursor_type);

  if (!cursor) {
    return false;
  }

  const int result = XDefineCursor(display, window, cursor);
  XFreeCursor(display, cursor);
  XFlush(display);
  return result != 0;
}

}  // namespace window_utils
}  // namespace bernese
