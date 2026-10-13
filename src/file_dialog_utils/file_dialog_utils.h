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

#ifndef FILE_DIALOG_UTILS_FILE_DIALOG_UTILS_H_
#define FILE_DIALOG_UTILS_FILE_DIALOG_UTILS_H_

#include "QString"
#include "QStringList"

#include "bernese/export.h"

namespace bernese {
namespace file_dialog_utils {

/**
 * @brief Extract patterns from a file-dialog name filter.
 *
 * @param filter A labelled filter such as "Images (*.png *.jpg)" or bare
 * patterns.
 * @return Space-separated patterns with empty entries removed.
 */
BERNESE_EXPORT QStringList CleanFilterList(QString filter);

}  // namespace file_dialog_utils
}  // namespace bernese

#endif  // FILE_DIALOG_UTILS_FILE_DIALOG_UTILS_H_
