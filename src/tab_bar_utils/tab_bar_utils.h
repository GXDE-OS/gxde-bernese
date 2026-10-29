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

#ifndef TAB_BAR_UTILS_TAB_BAR_UTILS_H_
#define TAB_BAR_UTILS_TAB_BAR_UTILS_H_

#include "QStyleOptionTabBarBase"
#include "QTabBar"
#include "QToolButton"

#include "bernese/export.h"

namespace bernese {
namespace tab_bar_utils {

// Direction in which the built-in scroll buttons of a tab bar move.
enum class ScrollDirection {
  kTowardFirst,  // Shows tabs with lower indexes, like the left arrow.
  kTowardLast    // Shows tabs with higher indexes, like the right arrow.
};

/**
 * @brief Fill the style option that paints the base line of a tab bar.
 *
 * Use it in a QTabBar subclass that paints itself. It replaces
 * QTabBarPrivate::initStyleBaseOption().
 *
 * @param option Non-null option to fill. The tab rectangles are not set.
 * @param tab_bar Non-null borrowed tab bar to read the state from.
 */
BERNESE_EXPORT void InitTabBarBaseOption(
    QStyleOptionTabBarBase* option, const QTabBar* tab_bar);

/**
 * @brief Find a built-in scroll button of a tab bar.
 *
 * Qt creates the buttons when the tab bar is built and names them
 * "ScrollLeftButton" and "ScrollRightButton".
 *
 * @param tab_bar Borrowed tab bar, which may be null.
 * @param direction Which of the two buttons to return.
 * @return Borrowed button owned by the tab bar, or null if not found.
 */
BERNESE_EXPORT QToolButton* ScrollButton(
    const QTabBar* tab_bar, ScrollDirection direction);

/**
 * @brief Scroll the tabs by one step, like a click on a scroll arrow.
 *
 * The current tab does not change. This is the public way to move the
 * private scroll offset of QTabBar.
 *
 * @param tab_bar Borrowed tab bar, which may be null.
 * @param direction Direction to scroll in.
 * @return True if the tabs moved.
 */
BERNESE_EXPORT bool ScrollTabs(QTabBar* tab_bar, ScrollDirection direction);

/**
 * @brief Scroll until a tab is fully inside the tab bar.
 *
 * The current tab does not change. It replaces
 * QTabBarPrivate::makeVisible().
 *
 * @param tab_bar Borrowed tab bar, which may be null.
 * @param index Tab to show; invalid indexes do nothing.
 */
BERNESE_EXPORT void ScrollToTab(QTabBar* tab_bar, int index);

}  // namespace tab_bar_utils
}  // namespace bernese

#endif  // TAB_BAR_UTILS_TAB_BAR_UTILS_H_
