// Copyright 2026 CharOfString
// SPDX-License-Identifier: GPL-3.0-or-later

#include <limits>

#include "QScreen"
#include "QtTest"

#include "window_utils/window_geometry.h"

namespace bernese {

class WindowGeometryTest : public QObject {
  Q_OBJECT

 private slots:
  /** @brief Cover unscaled, mixed-DPI, negative and fractional coordinates. */
  void Coordinates_data() {
    QTest::addColumn<QRect>("logical");
    QTest::addColumn<QPoint>("origin");
    QTest::addColumn<qreal>("scale");
    QTest::addColumn<QRect>("native");
    QTest::newRow("unscaled") << QRect(20, 30, 408, 600) << QPoint()
        << qreal(1) << QRect(20, 30, 408, 600);
    QTest::newRow("right-screen") << QRect(1930, 20, 408, 540)
        << QPoint(1920, 0) << qreal(2)
        << QRect(1940, 40, 816, 1080);
    QTest::newRow("left-screen") << QRect(-1900, -180, 408, 600)
        << QPoint(-1920, -200) << qreal(1.5)
        << QRect(-1890, -170, 612, 900);
    QTest::newRow("fractional") << QRect(10, 20, 408, 600) << QPoint()
        << qreal(1.25) << QRect(13, 25, 510, 750);
    QTest::newRow("hidden-at-edge") << QRect(3840, 0, 408, 540)
        << QPoint(1920, 0) << qreal(2)
        << QRect(5760, 0, 816, 1080);
  }

  /** @brief Check expected coordinates and rounding in both directions. */
  void Coordinates() {
    QFETCH(QRect, logical);
    QFETCH(QPoint, origin);
    QFETCH(qreal, scale);
    QFETCH(QRect, native);
    QCOMPARE(window_utils::ToNativePixels(logical, origin, scale), native);
    QCOMPARE(window_utils::FromNativePixels(native, origin, scale), logical);
  }

  /** @brief Invalid ratios must not produce invalid integer conversions. */
  void InvalidScale() {
    const qreal invalid[] = {0, -1, std::numeric_limits<qreal>::infinity(),
        std::numeric_limits<qreal>::quiet_NaN()};
    for (qreal scale : invalid) {
      QVERIFY(window_utils::ToNativePixels(QRect(1, 2, 3, 4), {}, scale)
                  .isNull());
      QVERIFY(window_utils::FromNativePixels(QRect(1, 2, 3, 4), {}, scale)
                  .isNull());
    }
    QVERIFY(window_utils::NativeWindowGeometry(nullptr).isNull());
  }

  /** @brief Read geometry without forcing creation of a platform window. */
  void WindowGeometry() {
    QWindow window;
    const QScreen* screen = window.screen();
    QVERIFY(screen);
    QCOMPARE(window.devicePixelRatio(), qreal(2));
    const QPoint origin = screen->geometry().topLeft();
    window.setGeometry(QRect(origin + QPoint(20, 30), QSize(408, 600)));
    QCOMPARE(window_utils::NativeWindowGeometry(&window),
        QRect(origin + QPoint(40, 60), QSize(816, 1200)));
  }
  /** @brief Hidden widgets must update their existing native window too. */
  void HiddenGeometry() {
    QWidget widget;
    widget.setWindowFlags(Qt::FramelessWindowHint);
    widget.setGeometry(10, 20, 408, 600);
    widget.show();
    QCoreApplication::processEvents();
    widget.hide();
    const QRect requested(100, 120, 408, 700);
    widget.setGeometry(requested);
    // The widget cache is already correct, but its QWindow is still stale.
    QVERIFY(widget.windowHandle()->geometry() != requested);
    window_utils::SetTopLevelGeometry(&widget, widget.screen(), requested);
    QCOMPARE(widget.geometry(), requested);
    QCOMPARE(widget.windowHandle()->geometry(), requested);
    QVERIFY(!widget.isVisible());
  }

  /** @brief Respect size hints and avoid creating windows for cached moves. */
  void GeometryConstraints() {
    QWidget widget;
    widget.setMinimumSize(100, 200);
    widget.setMaximumSize(400, 600);
    window_utils::SetTopLevelGeometry(&widget, nullptr, QRect(10, 20, 800, 50));
    QCOMPARE(widget.geometry(), QRect(10, 20, 400, 200));
    QVERIFY(!widget.windowHandle());
    QWidget child(&widget);
    const QRect oldRect = child.geometry();
    window_utils::SetTopLevelGeometry(&child, nullptr, QRect(20, 30, 50, 50));
    QCOMPARE(child.geometry(), oldRect);
    window_utils::SetTopLevelGeometry(nullptr, nullptr, {});
  }

  /** @brief Plain windows get the same clamping as widgets. */
  void WindowOverload() {
    QWindow window;
    window.setMinimumSize(QSize(100, 200));
    window.setMaximumSize(QSize(400, 600));
    window_utils::SetTopLevelWindowGeometry(
        &window, window.screen(), QRect(10, 20, 800, 50));
    QCOMPARE(window.geometry(), QRect(10, 20, 400, 200));

    QWindow child(&window);
    const QRect old_rect = child.geometry();
    window_utils::SetTopLevelWindowGeometry(
        &child, nullptr, QRect(1, 2, 30, 40));
    QCOMPARE(child.geometry(), old_rect);
    window_utils::SetTopLevelWindowGeometry(nullptr, nullptr, {});
  }

  /** @brief Preserve width while moving across screens with different DPI. */
  void CrossScreenGeometry() {
    if (QGuiApplication::screens().size() != 3) {
      QSKIP("Requires the three-screen offscreen configuration");
    }
    QWidget widget;
    widget.setWindowFlags(Qt::FramelessWindowHint);
    widget.show();
    QCoreApplication::processEvents();
    for (QScreen* screen : QGuiApplication::screens()) {
      const QRect area = screen->geometry();
      const QRect requested(area.right() - 407, area.top(), 408, area.height());
      widget.setFixedHeight(area.height());
      window_utils::SetTopLevelGeometry(&widget, screen, requested);
      QCoreApplication::processEvents();
      QCOMPARE(widget.screen(), screen);
      QCOMPARE(widget.geometry(), requested);
      QCOMPARE(widget.windowHandle()->geometry(), requested);
    }
  }

};

}  // namespace bernese

QTEST_MAIN(bernese::WindowGeometryTest)
#include "window_geometry_test.moc"
