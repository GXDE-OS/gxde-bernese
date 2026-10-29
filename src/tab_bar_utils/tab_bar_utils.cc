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

#include "tab_bar_utils/tab_bar_utils.h"

#include "QRect"
#include "QSize"
#include "QString"
#include "QStyle"
#include "QStyleOptionTab"
#include "QStyleOptionTabBarBase"
#include "QTabBar"
#include "QToolButton"

namespace bernese {
namespace tab_bar_utils {

namespace {

/**
 * @brief Check whether tabs of a shape are stacked from top to bottom.
 *
 * @param shape Shape of the tab bar.
 * @return True for the west and east shapes.
 */
bool IsVertical(QTabBar::Shape shape) {
  return shape == QTabBar::RoundedWest || shape == QTabBar::RoundedEast ||
         shape == QTabBar::TriangularWest || shape == QTabBar::TriangularEast;
}

}  // namespace

void InitTabBarBaseOption(
    QStyleOptionTabBarBase* option, const QTabBar* tab_bar) {
  QStyleOptionTab overlap_option;
  overlap_option.shape = tab_bar->shape();
  const int overlap = tab_bar->style()->pixelMetric(
      QStyle::PM_TabBarBaseOverlap, &overlap_option, tab_bar);

  option->initFrom(tab_bar);
  option->shape = tab_bar->shape();
  option->documentMode = tab_bar->documentMode();
  if (tab_bar->parentWidget() == nullptr || overlap <= 0) {
    return;
  }

  // The base line is the strip of the tab bar next to the page.
  const QSize size = tab_bar->size();
  switch (tab_bar->shape()) {
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
      option->rect = QRect(0, size.height() - overlap, size.width(), overlap);
      break;
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
      option->rect = QRect(0, 0, size.width(), overlap);
      break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
      option->rect = QRect(0, 0, overlap, size.height());
      break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
      option->rect = QRect(size.width() - overlap, 0, overlap, size.height());
      break;
  }
}

QToolButton* ScrollButton(const QTabBar* tab_bar, ScrollDirection direction) {
  if (tab_bar == nullptr) {
    return nullptr;
  }

  const QString name = direction == ScrollDirection::kTowardFirst
                           ? QStringLiteral("ScrollLeftButton")
                           : QStringLiteral("ScrollRightButton");
  return tab_bar->findChild<QToolButton*>(name, Qt::FindDirectChildrenOnly);
}

bool ScrollTabs(QTabBar* tab_bar, ScrollDirection direction) {
  QToolButton* button = ScrollButton(tab_bar, direction);
  if (button == nullptr || tab_bar->count() == 0) {
    return false;
  }

  // The button slot scrolls by one tab. A disabled button does nothing.
  const QRect before = tab_bar->tabRect(0);
  button->click();
  return tab_bar->tabRect(0) != before;
}

void ScrollToTab(QTabBar* tab_bar, int index) {
  if (tab_bar == nullptr || index < 0 || index >= tab_bar->count()) {
    return;
  }

  const bool vertical = IsVertical(tab_bar->shape());
  const QRect bar = tab_bar->rect();
  const int bar_start = vertical ? bar.top() : bar.left();
  const int bar_end = vertical ? bar.bottom() : bar.right();

  // Every step moves by at least one tab, so count() steps are enough.
  for (int step = 0; step < tab_bar->count(); ++step) {
    // Undo right-to-left mirroring so that start means the first tab side.
    const QRect tab = QStyle::visualRect(
        tab_bar->layoutDirection(), bar, tab_bar->tabRect(index));
    const int start = vertical ? tab.top() : tab.left();
    const int end = vertical ? tab.bottom() : tab.right();

    ScrollDirection direction;
    if (start < bar_start) {
      direction = ScrollDirection::kTowardFirst;
    } else if (end > bar_end) {
      direction = ScrollDirection::kTowardLast;
    } else {
      return;
    }

    if (!ScrollTabs(tab_bar, direction)) {
      return;
    }
  }
}

}  // namespace tab_bar_utils
}  // namespace bernese
