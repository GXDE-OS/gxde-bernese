// Copyright 2026 CharOfString
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef TAG_OBJECT_TAG_OBJECT_H_
#define TAG_OBJECT_TAG_OBJECT_H_

#include "QObject"
#include "QPainter"
#include "QRectF"
#include "QSizeF"
#include "QTextDocument"
#include "QTextFormat"
#include "QTextObjectInterface"
#include "QtGlobal"

namespace bernese {

class TagObject : public QObject, public QTextObjectInterface {
  Q_OBJECT
  Q_INTERFACES(QTextObjectInterface)
 public:
  /** @brief Return the number of inline-object paint calls. */
  int PaintCount() const {
    return paints_;
  }

  /** @brief Provide a fixed 24 by 12 logical-pixel inline tag size. */
  QSizeF intrinsicSize(QTextDocument*, int, const QTextFormat&) override {
    return {24, 12};
  }

  /** @brief Paint a red inline tag and record that the handler was called. */
  void drawObject(QPainter* p, const QRectF& r, QTextDocument*, int,
      const QTextFormat&) override {
    ++paints_;
    p->fillRect(r, Qt::red);
  }

 private:
  int paints_ = 0;
};

}  // namespace bernese

#endif  // TAG_OBJECT_TAG_OBJECT_H_
