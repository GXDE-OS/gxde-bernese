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

#ifndef WINDOW_UTILS_WINDOW_UTILS_H_
#define WINDOW_UTILS_WINDOW_UTILS_H_

#include "QMouseEvent"
#include "QRect"
#include "QWidget"

#include "bernese/export.h"

namespace bernese {
namespace window_utils {

/**
 * @brief Detect if a point is on an edge.
 *
 * Actually handy if you need to tell your mouse is on which side of the
 * borderless window so you know if the action is resize width/height/whole
 * window.
 * @param widget Your QWidget.
 * @param x The coordinate of the QWidget so we can calculate the mouse offset
 *          related to the window.
 * @param y The coordinate of the QWidget so we can calculate the mouse offset
 *          related to the window.
 * @param border Hit-test width in logical pixels.
 * @return Matching resize edges, or an empty set.
 */
BERNESE_EXPORT Qt::Edges EdgesAt(QWidget* widget, int x, int y, int border);

/**
 * @brief Request an window move from a draggable region.
 *
 * @param widget Your QWidget.
 * @param event QMouse event of your widget.
 * @param draggable_rect Draggable area in widget.
 */
BERNESE_EXPORT void MoveWindow(
    QWidget* widget, QMouseEvent* event, const QRect& draggable_rect);

/**
 * @brief Request interactive resizing at a widget border.
 *
 * The platform/compositor decides whether to honor the request.
 *
 * @param widget Non-null borrowed widget whose top-level window is resized.
 * @param event Non-null borrowed mouse event in widget coordinates.
 * @param border_inner_size Border hit-test width in logical pixels; defaults
 * to 6.
 */
BERNESE_EXPORT void ResizeWindow(
    QWidget* widget, QMouseEvent* event, int border_inner_size = 6);

/**
 * @brief Show a maximized top-level widget.
 *
 * @param widget Your QWidget.
 */
BERNESE_EXPORT void ShowMaximizedWindow(QWidget* widget);

/**
 * @brief Show a minimized top-level widget.
 *
 * @param widget Your QWidget.
 */
BERNESE_EXPORT void ShowMinimizedWindow(QWidget* widget);

/**
 * @brief Show a restored top-level widget.
 *
 * @param widget Your QWidget.
 */
BERNESE_EXPORT void ShowNormalWindow(QWidget* widget);

/**
 * @brief Toggle a maximized top-level widget.
 *
 * @param widget Your QWidget.
 */
BERNESE_EXPORT void ToggleMaximizedWindow(QWidget* widget);

/**
 * @brief Set the widget cursor to the standard arrow.
 *
 * @param widget Your QWidget.
 * @return True after setting the cursor.
 */
BERNESE_EXPORT bool SetNormalCursor(QWidget* widget);

/**
 * @brief Set the widget cursor to the pointing hand.
 *
 * @param widget Your QWidget.
 * @return True after setting the cursor.
 */
BERNESE_EXPORT bool SetHandleCursor(QWidget* widget);

/**
 * @brief Apply an X cursor-font shape to a widget.
 *
 *
 * @note Must be called from a GUI thread.
 * @param widget Your QWidget.
 * @param cursor_type X cursor-font shape identifier.
 * @return True if the XDefineCursor request reports success.
 */
BERNESE_EXPORT bool SetCursorByCursorType(QWidget* widget,
    unsigned int cursor_type);

/**
 * @brief Select a resize cursor from the widget border under a point.
 *
 * @param widget Your QWidget.
 * @param x Widget-local horizontal coordinate in logical pixels.
 * @param y Widget-local vertical coordinate in logical pixels.
 * @param border_inner_size Border hit-test width in logical pixels; defaults
 * to 6.
 * @return True at a resize edge; false in the interior, which uses an arrow.
 */
BERNESE_EXPORT bool UpdateCursorShape(QWidget* widget, int x, int y,
    int border_inner_size = 6);

}  // namespace window_utils
}  // namespace bernese

#endif  // WINDOW_UTILS_WINDOW_UTILS_H_
