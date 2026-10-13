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

#include "text_layout_utils/text_layout_utils.h"

#include "QAbstractTextDocumentLayout"
#include "QBrush"
#include "QChar"
#include "QColor"
#include "QFont"
#include "QFontMetricsF"
#include "QList"
#include "QMarginsF"
#include "QPainter"
#include "QPainterPath"
#include "QPen"
#include "QPointF"
#include "QRectF"
#include "QSizeF"
#include "QString"
#include "QStringList"
#include "QTextBlock"
#include "QTextCharFormat"
#include "QTextCursor"
#include "QTextDocument"
#include "QTextLayout"
#include "QTextObjectInterface"
#include "QTextOption"
#include "QtGlobal"

namespace bernese {
namespace text_layout_utils {
namespace {

/**
 * @brief Paint a rounded line background joined to the previous line.
 *
 * @param painter Non-null borrowed painter; opacity and antialiasing are
 * restored.
 * @param rect Current logical line rectangle.
 * @param background Fill brush.
 * @param background_radius Logical corner radius and horizontal padding.
 * @param previous_rect Background rectangle returned for the preceding line.
 * @return Rectangle to pass when drawing the next line.
 */
QRectF DrawBackground(QPainter* painter, const QRectF& rect,
    const QBrush& background, qreal background_radius,
    const QRectF& previous_rect) {
  QRectF last_line_rect = previous_rect;

  const QMarginsF margins(background_radius, 0, background_radius, 0);
  QRectF background_bounds = rect;
  QPainterPath path;

  if (last_line_rect.isValid()) {
    if (qAbs(rect.width() - last_line_rect.width()) < background_radius * 2) {
      background_bounds.setWidth(last_line_rect.width());
      background_bounds.moveCenter(rect.center());
      path.moveTo(last_line_rect.x() - background_radius,
          last_line_rect.bottom() - background_radius);
      path.lineTo(last_line_rect.x(), last_line_rect.bottom() - 1);
      path.lineTo(last_line_rect.right(), last_line_rect.bottom() - 1);
      path.lineTo(last_line_rect.right() + background_radius,
          last_line_rect.bottom() - background_radius);
      path.lineTo(last_line_rect.right() + background_radius,
          background_bounds.bottom() - background_radius);
      path.arcTo(background_bounds.right() - background_radius,
          background_bounds.bottom() - background_radius * 2,
          background_radius * 2, background_radius * 2, 0, -90);
      path.lineTo(background_bounds.x(), background_bounds.bottom());
      path.arcTo(background_bounds.x() - background_radius,
          background_bounds.bottom() - background_radius * 2,
          background_radius * 2, background_radius * 2, 270, -90);
      last_line_rect = background_bounds;
    } else if (last_line_rect.width() > rect.width()) {
      background_bounds += margins;
      path.moveTo(
          background_bounds.x() - background_radius, background_bounds.y() - 1);
      path.arcTo(background_bounds.x() - background_radius * 2,
          background_bounds.y() - 1, background_radius * 2,
          background_radius * 2 + 1, 90, -90);
      path.lineTo(background_bounds.x(),
          background_bounds.bottom() - background_radius);
      path.arcTo(background_bounds.x(),
          background_bounds.bottom() - background_radius * 2,
          background_radius * 2, background_radius * 2, 180, 90);
      path.lineTo(background_bounds.right() - background_radius,
          background_bounds.bottom());
      path.arcTo(background_bounds.right() - background_radius * 2,
          background_bounds.bottom() - background_radius * 2,
          background_radius * 2, background_radius * 2, 270, 90);
      path.lineTo(background_bounds.right(),
          background_bounds.top() + background_radius);
      path.arcTo(background_bounds.right(), background_bounds.top() - 1,
          background_radius * 2, background_radius * 2 + 1, 180, -90);
      path.closeSubpath();
      last_line_rect = rect;
    } else {
      background_bounds += margins;
      path.moveTo(
          last_line_rect.x() - background_radius * 2, last_line_rect.bottom());
      path.arcTo(last_line_rect.x() - background_radius * 3,
          last_line_rect.bottom() - background_radius * 2,
          background_radius * 2, background_radius * 2, 270, 90);
      path.lineTo(last_line_rect.x(), last_line_rect.bottom() - 1);
      path.lineTo(last_line_rect.right(), last_line_rect.bottom() - 1);
      path.lineTo(last_line_rect.right() + background_radius,
          last_line_rect.bottom() - background_radius * 2);
      path.arcTo(last_line_rect.right() + background_radius,
          last_line_rect.bottom() - background_radius * 2,
          background_radius * 2, background_radius * 2, 180, 90);

      path.addRoundedRect(
          background_bounds, background_radius, background_radius);
      last_line_rect = rect;
    }
  } else {
    last_line_rect = background_bounds;
    path.addRoundedRect(
        background_bounds + margins, background_radius, background_radius);
  }

  const bool antialiasing = painter->testRenderHint(QPainter::Antialiasing);
  const qreal opacity = painter->opacity();

  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setOpacity(1);
  painter->fillPath(path, background);
  painter->setRenderHint(QPainter::Antialiasing, antialiasing);
  painter->setOpacity(opacity);

  return last_line_rect;
}

/**
 * @brief Elide text and verify the shaped line fits the available width.
 *
 * Rechecks shaped bounds because bidi and fallback fonts can exceed advances.
 *
 * @param remaining Text to shorten.
 * @param mode Qt elision mode.
 * @param available_width Maximum logical line width.
 * @param flags Flags passed to QFontMetricsF::elidedText.
 * @param font Font used for shaping and measuring.
 * @param text_option Layout direction and alignment options.
 * @return Elided text, possibly empty when nothing fits.
 */
QString ElideTextToWidth(const QString& remaining, Qt::TextElideMode mode,
    qreal available_width, int flags, const QFont& font,
    const QTextOption& text_option) {
  qreal elide_width = available_width;
  const QFontMetricsF metrics(font);
  QString elided_text = metrics.elidedText(remaining, mode, elide_width, flags);
  // Ellipsis insertion can change bidi shaping/fallback-font ink
  // bounds. Check the actual line, rather than trusting advances.
  while (mode != Qt::ElideNone && !elided_text.isEmpty()) {
    QTextLayout probe(elided_text, font);
    QTextOption option = text_option;
    option.setWrapMode(QTextOption::NoWrap);
    probe.setTextOption(option);
    probe.beginLayout();
    QTextLine probe_line = probe.createLine();
    probe_line.setLineWidth(available_width);
    const qreal actual_width = probe_line.naturalTextRect().width();
    probe.endLayout();
    if (actual_width <= available_width) {
      break;
    }
    elide_width = qMax(static_cast<qreal>(0),
        elide_width -
            qMax(static_cast<qreal>(1), actual_width - available_width));
    elided_text = metrics.elidedText(remaining, mode, elide_width, flags);
    if (elide_width == 0) {
      elided_text.clear();
      break;
    }
  }

  return elided_text;
}

}  // namespace

void DrawElidedText(QTextLayout* layout, const QSizeF& size,
    QTextOption::WrapMode word_wrap, Qt::TextElideMode mode, qreal line_height,
    int flags, QStringList* lines, QPainter* painter, QPointF offset,
    const QColor& shadow_color, const QPointF& shadow_offset,
    const QBrush& background, qreal background_radius,
    QList<QRectF>* bounding_region, QTextDocument* document) {
  if (size.width() <= 0 || size.height() <= 0 || layout == nullptr ||
      lines == nullptr || painter == nullptr || document == nullptr) {
    return;
  }

  if (painter) {
    layout->setFont(painter->font());
  }

  if (line_height <= 0) {
    line_height = QFontMetricsF(layout->font()).height();
  }

  qreal height = 0;
  bool draw_background = background.style() != Qt::NoBrush;
  bool draw_shadow = shadow_color.isValid();

  QString text = document ? document->firstBlock().text() : layout->text();
  QTextOption text_option = layout->textOption();

  text_option.setWrapMode(word_wrap);

  if (flags & Qt::AlignRight) {
    text_option.setAlignment(Qt::AlignRight);
  } else if (flags & Qt::AlignHCenter) {
    text_option.setAlignment(Qt::AlignHCenter);
  }

  if (painter) {
    text_option.setTextDirection(painter->layoutDirection());
    layout->setFont(painter->font());
  }
  layout->setTextOption(text_option);

  auto draw_line_shadow = [&](const QTextLine& line) {
    const QPen pen = painter->pen();

    painter->setPen(shadow_color);
    line.draw(painter, shadow_offset);

    // restore
    painter->setPen(pen);
  };

  layout->beginLayout();

  QTextLine line = layout->createLine();
  QRectF last_line_rect;

  while (line.isValid()) {
    height += line_height;

    // The last visible line must include the remaining text through elision.
    if (height + line_height > size.height()) {
      const int start = line.textStart();
      line.setLineWidth(size.width());
      QString remaining = text.mid(start);
      QTextCharFormat object_format;
      qreal object_width = 0;
      const bool has_object =
          document && start == 0 &&
          remaining.startsWith(QChar::ObjectReplacementCharacter);
      // Preserve the leading inline tag and reserve its width before elision.
      if (has_object) {
        QTextCursor cursor(document);
        cursor.setPosition(1, QTextCursor::KeepAnchor);
        object_format = cursor.charFormat();
        auto* handler = document->documentLayout()->handlerForObject(
            object_format.objectType());
        if (handler) {
          object_width =
              handler->intrinsicSize(document, 0, object_format).width();
        }
        remaining.remove(0, 1);
      }
      const qreal available_width =
          qMax(static_cast<qreal>(0), size.width() - object_width);
      QString elided_text = ElideTextToWidth(
          remaining, mode, available_width, flags, layout->font(), text_option);
      const QFont font = layout->font();
      layout->endLayout();
      if (document) {
        document->setPlainText(elided_text);
        if (has_object) {
          QTextCursor cursor(document);
          cursor.insertText(
              QString(QChar::ObjectReplacementCharacter), object_format);
          elided_text.prepend(QChar::ObjectReplacementCharacter);
        }
        layout = document->firstBlock().layout();
      } else {
        layout->setText(elided_text);
      }
      layout->setFont(font);
      text_option.setWrapMode(QTextOption::NoWrap);
      layout->setTextOption(text_option);
      layout->beginLayout();
      line = layout->createLine();
      if (!line.isValid()) {
        break;
      }
      line.setLineWidth(size.width());
      text = elided_text;
    } else {
      line.setLineWidth(size.width());
    }

    line.setPosition(offset);

    QRectF rect = line.naturalTextRect();
    rect.setHeight(line_height);

    if (painter) {
      if (draw_background) {
        last_line_rect = DrawBackground(
            painter, rect, background, background_radius, last_line_rect);
      }

      if (draw_shadow) {
        draw_line_shadow(line);
      }

      line.draw(painter, QPointF(0, 0));
    }

    if (bounding_region) {
      bounding_region->append(rect);
    }

    offset.setY(offset.y() + line_height);

    if (lines) {
      lines->append(text.mid(line.textStart(), line.textLength()));
    }

    if (height + line_height > size.height()) {
      break;
    }

    line = layout->createLine();
  }

  layout->endLayout();
}

}  // namespace text_layout_utils
}  // namespace bernese
