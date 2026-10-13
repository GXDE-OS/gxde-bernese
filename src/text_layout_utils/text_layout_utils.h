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

#ifndef TEXT_LAYOUT_UTILS_TEXT_LAYOUT_UTILS_H_
#define TEXT_LAYOUT_UTILS_TEXT_LAYOUT_UTILS_H_

#include <QBrush>
#include <QColor>
#include <QList>
#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QStringList>
#include <QTextDocument>
#include <QTextLayout>
#include <QTextOption>
#include <QtGlobal>

#include "bernese/export.h"

namespace bernese {
namespace text_layout_utils {

/**
 * @brief Lay out text and optionally paint it within a bounded area.
 *
 * @note No ownership of input objects is transferred.
 * @note Your QTextLayout's text and layout state may change.
 * @param layout Non-null borrowed layout; its 
 * @param size Available area in logical pixels; non-positive dimensions do
 *             nothing.
 * @param word_wrap Wrapping mode before the final visible line.
 * @param mode Elision mode for text exceeding the available area.
 * @param line_height Logical line height; non-positive values use the font
 *                    height.
 * @param flags Qt alignment and font-metric elision flags.
 * @param lines Optional output list to which visible line strings are appended.
 * @param painter Optional borrowed painter; null performs layout only.
 * @param offset Initial line position in logical painter coordinates.
 * @param shadow_color Shadow color; an invalid color disables the shadow.
 * @param shadow_offset Offset supplied when painting the line shadow.
 * @param background Background brush; Qt::NoBrush disables the background.
 * @param background_radius Background corner radius in logical pixels.
 * @param bounding_region Optional output list to which line rectangles are
 *                        appended.
 * @param document Optional borrowed document owning layout; preserves a leading
 *                 inline object.
 */
BERNESE_EXPORT void DrawElidedText(QTextLayout* layout, const QSizeF& size,
    QTextOption::WrapMode word_wrap, Qt::TextElideMode mode, qreal line_height,
    int flags, QStringList* lines, QPainter* painter, QPointF offset,
    const QColor& shadow_color, const QPointF& shadow_offset,
    const QBrush& background, qreal background_radius,
    QList<QRectF>* bounding_region, QTextDocument* document = nullptr);

}  // namespace text_layout_utils
}  // namespace bernese

#endif  // TEXT_LAYOUT_UTILS_TEXT_LAYOUT_UTILS_H_
