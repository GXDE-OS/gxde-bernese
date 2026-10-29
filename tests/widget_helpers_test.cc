// Copyright 2026 CharOfString
// SPDX-License-Identifier: GPL-3.0-or-later
#include "QColor"
#include "QImage"
#include "QObject"
#include "QPaintEvent"
#include "QPainter"
#include "QPixmap"
#include "QRect"
#include "QStyleOptionTabBarBase"
#include "QTabBar"
#include "QWidget"
#include "QtTest"

#include "paint_utils/paint_utils.h"
#include "platform_utils/platform_utils.h"
#include "tab_bar_utils/tab_bar_utils.h"

namespace bernese {

// Child widget that saves what lies under it when it is painted.
class UnderlayProbe : public QWidget {
 public:
  using QWidget::QWidget;

  QImage underlay;

 protected:
  /**
   * @brief Copy the pixels under this widget before painting nothing.
   *
   * @param event Unused paint event.
   */
  void paintEvent(QPaintEvent* event) override {
    Q_UNUSED(event);
    QPainter painter(this);
    underlay = paint_utils::CopyPaintedArea(&painter, rect());
  }
};

class WidgetHelpersTest : public QObject {
  Q_OBJECT
 private slots:

  /** @brief Verify copies follow the painter transform and fill outside
   * the device. */
  void CopyPaintedImage() {
    QImage target(40, 30, QImage::Format_ARGB32_Premultiplied);
    target.fill(Qt::red);
    QPainter painter(&target);
    painter.fillRect(10, 5, 4, 4, Qt::blue);
    painter.translate(10, 5);
    const QImage area =
        paint_utils::CopyPaintedArea(&painter, QRect(0, 0, 8, 6));
    QCOMPARE(area.size(), QSize(8, 6));
    QCOMPARE(area.pixelColor(0, 0), QColor(Qt::blue));
    QCOMPARE(area.pixelColor(7, 5), QColor(Qt::red));
    const QImage clipped =
        paint_utils::CopyPaintedArea(&painter, QRect(25, 20, 10, 10));
    QCOMPARE(clipped.size(), QSize(10, 10));
    QCOMPARE(clipped.pixelColor(2, 2), QColor(Qt::red));
    QCOMPARE(qAlpha(clipped.pixel(9, 9)), 0);
    painter.end();
    QVERIFY(paint_utils::CopyPaintedArea(&painter, QRect(0, 0, 1, 1)).isNull());
    QVERIFY(paint_utils::CopyPaintedArea(nullptr, QRect(0, 0, 1, 1)).isNull());
  }

  /** @brief Verify a child paint event sees what its parent painted. */
  void CopyPaintedWidget() {
    QWidget parent;
    parent.resize(60, 40);
    parent.setAutoFillBackground(true);
    QPalette palette = parent.palette();
    palette.setColor(QPalette::Window, Qt::green);
    parent.setPalette(palette);
    auto* probe = new UnderlayProbe(&parent);
    probe->setGeometry(10, 10, 20, 10);
    parent.grab();
    QCOMPARE(probe->underlay.size(), QSize(20, 10));
    QCOMPARE(probe->underlay.pixelColor(5, 5), QColor(Qt::green));
  }

  /** @brief Verify installed platform plugins are found by key. */
  void PlatformPlugins() {
    QVERIFY(platform_utils::PlatformPluginKeys().contains(
        QStringLiteral("offscreen")));
    QVERIFY(platform_utils::HasPlatformPlugin(QStringLiteral("OffScreen")));
    QVERIFY(!platform_utils::HasPlatformPlugin(QStringLiteral("no-such-qpa")));
  }

  /** @brief Verify the tab bar base option copies the tab bar state. */
  void TabBarBaseOption() {
    QWidget parent;
    auto* tab_bar = new QTabBar(&parent);
    tab_bar->setShape(QTabBar::RoundedSouth);
    tab_bar->setDocumentMode(true);
    tab_bar->resize(120, 30);
    QStyleOptionTabBarBase option;
    tab_bar_utils::InitTabBarBaseOption(&option, tab_bar);
    QCOMPARE(option.shape, QTabBar::RoundedSouth);
    QVERIFY(option.documentMode);
    QVERIFY(tab_bar->rect().contains(option.rect));
  }

  /** @brief Verify scrolling shows a tab without changing the current one. */
  void ScrollToTab() {
    QTabBar tab_bar;
    tab_bar.setUsesScrollButtons(true);
    for (int i = 0; i < 20; ++i) {
      tab_bar.addTab(QStringLiteral("Tab number %1").arg(i));
    }
    tab_bar.resize(200, 30);
    tab_bar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&tab_bar));
    QVERIFY(tab_bar_utils::ScrollButton(
        &tab_bar, tab_bar_utils::ScrollDirection::kTowardLast));
    QVERIFY(!tab_bar.rect().contains(tab_bar.tabRect(19)));
    tab_bar_utils::ScrollToTab(&tab_bar, 19);
    QVERIFY(tab_bar.rect().contains(tab_bar.tabRect(19)));
    QCOMPARE(tab_bar.currentIndex(), 0);
    tab_bar_utils::ScrollToTab(&tab_bar, 0);
    QVERIFY(tab_bar.rect().contains(tab_bar.tabRect(0)));
    QVERIFY(!tab_bar_utils::ScrollTabs(
        &tab_bar, tab_bar_utils::ScrollDirection::kTowardFirst));
  }
};

}  // namespace bernese

QTEST_MAIN(bernese::WidgetHelpersTest)
#include "widget_helpers_test.moc"
