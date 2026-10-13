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

#include "file_dialog_utils/file_dialog_utils.h"

#include "QRegularExpression"
#include "QString"
#include "QStringList"

namespace bernese {
namespace file_dialog_utils {

QStringList CleanFilterList(QString filter) {
  // QFileDialog syntax: "Images (*.png *.jpg)".
  const QRegularExpression details(QStringLiteral("^.*\\(([^()]*)\\)$"));
  const auto match = details.match(filter);
  if (match.hasMatch()) {
    filter = match.captured(1);
  }

  return filter.split(QLatin1Char(' '), Qt::SkipEmptyParts);
}

}  // namespace file_dialog_utils
}  // namespace bernese
