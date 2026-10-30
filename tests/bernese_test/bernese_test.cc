// Copyright 2026 CharOfString
// SPDX-License-Identifier: GPL-3.0-or-later
#include "QAbstractTextDocumentLayout"
#include "QBrush"
#include "QChar"
#include "QColor"
#include "QFont"
#include "QFontInfo"
#include "QFontMetricsF"
#include "QGuiApplication"
#include "QImage"
#include "QLabel"
#include "QList"
#include "QMouseEvent"
#include "QObject"
#include "QPainter"
#include "QPixmap"
#include "QPoint"
#include "QRect"
#include "QRectF"
#include "QSize"
#include "QSizeF"
#include "QString"
#include "QStringList"
#include "QTemporaryDir"
#include "QTextBlock"
#include "QTextCharFormat"
#include "QTextCursor"
#include "QTextDocument"
#include "QTextFormat"
#include "QTextLayout"
#include "QTextObjectInterface"
#include "QTextOption"
#include "QWidget"
#include "QtGlobal"
#include "QtMath"
#include "QtTest"

#include "backdrop_origin/backdrop_origin.h"
#include "file_dialog_utils/file_dialog_utils.h"
#include "image_utils/image_utils.h"
#include "tag_object/tag_object.h"
#include "text_layout_utils/text_layout_utils.h"
#include "window_effects/window_effects.h"
#include "window_utils/window_stack.h"
#include "window_utils/window_utils.h"
#include "x11_info/x11_info.h"

namespace bernese {

class BerneseTest : public QObject {
  Q_OBJECT
 private slots:

  /** @brief Verify connection availability and timestamps match the Qt
   * platform. */
  void OwnedX11Connections() {
    if (QGuiApplication::platformName() == QLatin1String("xcb")) {
      QVERIFY(bernese::X11Info::IsPlatformX11());
      QVERIFY(bernese::X11Info::GetDisplay());
      QVERIFY(bernese::X11Info::GetConnection());
      QVERIFY(bernese::X11Info::GetTimestamp() > 0);
    } else {
      QVERIFY(!bernese::X11Info::IsPlatformX11());
      QVERIFY(!bernese::X11Info::GetDisplay());
      QVERIFY(!bernese::X11Info::GetConnection());
      QCOMPARE(bernese::X11Info::GetTimestamp(), 0UL);
    }
  }

  /** @brief Verify nested-widget wallpaper capture keeps its sampling origin.
   */
  void SameWindowBackdrop() {
    // X11 embeds the icon canvas in the wallpaper window. Capture through
    // that widget tree, including viewport offsets, without a work-area
    // conversion or a second top-level render.
    QLabel background;
    background.resize(640, 480);
    background.winId();
    const qreal scale = background.devicePixelRatioF();
    qInfo() << "Same-window backdrop platform"
            << QGuiApplication::platformName() << "DPR" << scale;
    QImage wallpaper(qCeil(640 * scale), qCeil(480 * scale),
        QImage::Format_ARGB32_Premultiplied);
    wallpaper.setDevicePixelRatio(scale);
    wallpaper.fill(Qt::blue);
    QPainter pattern(&wallpaper);
    pattern.fillRect(QRect(0, 145, 640, 10), Qt::red);
    pattern.end();
    background.setPixmap(QPixmap::fromImage(wallpaper));
    QWidget canvas(&background);
    canvas.setGeometry(0, 0, 640, 480);
    QWidget viewport(&canvas);
    viewport.setGeometry(10, 32, 600, 394);
    QWidget item(&viewport);
    item.setGeometry(90, 100, 80, 80);
    QCOMPARE(item.window(), &background);
    const QPoint source = item.mapTo(item.window(), QPoint());
    QCOMPARE(source, QPoint(100, 132));
    item.hide();
    QImage sample(qCeil(80 * scale), qCeil(80 * scale), wallpaper.format());
    sample.setDevicePixelRatio(scale);
    sample.fill(Qt::transparent);
    QPainter painter(&sample);
    item.window()->render(&painter, {}, QRegion(QRect(source, item.size())),
        QWidget::DrawWindowBackground | QWidget::DrawChildren);
    painter.end();
    QCOMPARE(sample.pixelColor(qRound(20 * scale), qRound(18 * scale)),
        QColor(Qt::red));
    QCOMPARE(sample.pixelColor(qRound(20 * scale), qRound(50 * scale)),
        QColor(Qt::blue));
  }

  /** @brief Verify native-to-logical conversion at 1.25 scale and mixed
   * origins. */
  void FractionalWorkArea() {
    // Live 1.25x setup: XWayland publishes physical pixels, while the
    // layer-shell configure and our widgets use logical pixels.
    const QRect logical_output(0, 0, 1536, 960);
    const QRect available = bernese::wayland::LogicalWorkArea(
        {0, 0, 1920, 1200}, {0, 40, 1920, 1092}, logical_output);
    QCOMPARE(available, QRect(0, 32, 1536, 874));
    QCOMPARE(bernese::wayland::BackdropOrigin(
                 logical_output, {1536, 874}, available, {}),
        QPoint(0, 32));

    // Origins on mixed-scale outputs cannot be converted by dividing
    // every global coordinate by a single DPR.
    QCOMPARE(bernese::wayland::LogicalWorkArea({1920, 0, 1920, 1200},
                 {1920, 40, 1920, 1092}, {1536, 0, 1536, 960}),
        QRect(1536, 32, 1536, 874));
    QCOMPARE(bernese::wayland::LogicalWorkArea({-1920, 0, 1920, 1200},
                 {-1920, 40, 1920, 1092}, {-1536, 0, 1536, 960}),
        QRect(-1536, 32, 1536, 874));
    QVERIFY(bernese::wayland::LogicalWorkArea(
        {0, 0, 1920, 1200}, {1920, 40, 1920, 1092}, logical_output)
            .isEmpty());
  }

  /** @brief Verify fallback origins and wallpaper sampling across output
   * scales. */
  void BackdropCoordinates() {
    const QRect output(1920, 0, 640, 480);
    const QRect work_area(1920, 32, 640, 394);
    const QPoint origin = bernese::wayland::BackdropOrigin(
        output, work_area.size(), work_area, {});
    QCOMPARE(origin, QPoint(0, 32));
    QCOMPARE(bernese::wayland::BackdropOrigin(
                 output, output.size(), work_area, {9, 9}),
        QPoint());
    QCOMPARE(
        bernese::wayland::BackdropOrigin(output, {640, 395}, work_area, {7, 8}),
        QPoint(7, 8));
    QCOMPARE(bernese::wayland::BackdropOrigin(
                 output, work_area.size(), QRect(0, 32, 640, 394), {7, 8}),
        QPoint(7, 8));
    QCOMPARE(bernese::wayland::BackdropOrigin(
                 output, {600, 480}, QRect(1960, 0, 600, 480), {}),
        QPoint(40, 0));

    // A distinctive wallpaper stripe must stay at the same physical row
    // when sampled through a nested viewport below a reserved top panel.
    for (qreal scale : {1.0, 1.25, 1.5, 1.75, 2.0}) {
      // The live compositor advertises screen DPR 2 even when the
      // target window uses fractional DPR 1.25.
      QImage wallpaper(640 * 2, 480 * 2, QImage::Format_ARGB32_Premultiplied);
      wallpaper.setDevicePixelRatio(2);
      wallpaper.fill(Qt::blue);
      QPainter pattern(&wallpaper);
      pattern.fillRect(QRect(0, 145, 640, 10), Qt::red);
      pattern.end();
      QLabel background;
      background.resize(output.size());
      background.setPixmap(QPixmap::fromImage(wallpaper));
      QWidget icons;
      icons.resize(work_area.size());
      QWidget viewport(&icons);
      viewport.setGeometry(10, 7, 600, 350);
      QWidget item(&viewport);
      item.setGeometry(90, 93, 80, 80);
      const QPoint source = item.mapTo(&icons, QPoint()) + origin;
      QImage sample(80 * scale, 80 * scale, wallpaper.format());
      sample.setDevicePixelRatio(scale);
      sample.fill(Qt::transparent);
      QPainter painter(&sample);
      background.render(&painter, {}, QRegion(QRect(source, item.size())),
          QWidget::DrawWindowBackground);
      painter.end();
      QCOMPARE(sample.pixelColor(20 * scale, 18 * scale), QColor(Qt::red));
      QCOMPARE(sample.pixelColor(20 * scale, 50 * scale), QColor(Qt::blue));
    }
  }

  /** @brief Verify labelled and bare file-filter pattern extraction. */
  void Filters() {
    QCOMPARE(
        bernese::file_dialog_utils::CleanFilterList("Images (*.png *.jpg)"),
        QStringList({"*.png", "*.jpg"}));
    QCOMPARE(bernese::file_dialog_utils::CleanFilterList("All files (*)"),
        QStringList({"*"}));
    QCOMPARE(bernese::file_dialog_utils::CleanFilterList("*.tar.gz  *.zip"),
        QStringList({"*.tar.gz", "*.zip"}));
    QCOMPARE(
        bernese::file_dialog_utils::CleanFilterList("Images (old) (*.png)"),
        QStringList({"*.png"}));
  }

  /** @brief Supply Latin, Chinese, bidi and emoji filename test cases. */
  void Wrapping_data() {
    QTest::addColumn<QString>("text");
    QTest::newRow("latin") << QStringLiteral(
        "A long filename with several words and a suffix.txt");
    QTest::newRow("chinese")
        << QStringLiteral("这是一份名字很长的中文文件用于测试换行和省略.txt");
    QTest::newRow("bidi") << QString::fromUtf8(
        "تقرير طويل جدا باللغة العربية test 123.txt");
    QTest::newRow("emoji") << QString::fromUtf8(
        "Family 👨‍👩‍👧‍👦 and café with a very long "
        "filename.txt");
  }

  /** @brief Verify elision stays within bounds and unelided wrapping preserves
   * text. */
  void Wrapping() {
    QFETCH(QString, text);
    const QFont font("Sans Serif", 12);
    const qreal height = QFontMetricsF(font).height();
    for (auto mode : {Qt::ElideRight, Qt::ElideLeft, Qt::ElideMiddle}) {
      QTextLayout layout(text, font);
      QStringList lines;
      QList<QRectF> regions;
      bernese::text_layout_utils::DrawElidedText(&layout, {100, height * 2},
          QTextOption::WrapAtWordBoundaryOrAnywhere, mode, height,
          Qt::AlignHCenter, &lines, nullptr, {}, {}, {}, Qt::NoBrush, 4,
          &regions);
      QCOMPARE(lines.size(), 2);
      QCOMPARE(regions.size(), 2);
      QVERIFY(lines.last().contains(QChar(0x2026)));
      for (const auto& r : regions) {
        QVERIFY2(r.width() <= 100.1,
            qPrintable(QStringLiteral("width=%1 font=%2 lines=%3")
                    .arg(r.width())
                    .arg(QFontInfo(font).family(), lines.join("|"))));
        QVERIFY(r.top() >= 0);
        QVERIFY(r.bottom() <= height * 2 + .1);
      }
    }
    QTextLayout full(text, font);
    QStringList lines;
    bernese::text_layout_utils::DrawElidedText(&full, {100, 10000},
        QTextOption::WrapAtWordBoundaryOrAnywhere, Qt::ElideNone, height, 0,
        &lines, nullptr, {}, {}, {}, Qt::NoBrush, 4, nullptr);
    QCOMPARE(lines.join(QString()), text);
  }

  /** @brief Verify default line height and empty-area handling. */
  void ZeroLineHeightAndEmptyArea() {
    QTextLayout layout(
        "a long filename with multiple lines", QFont("Sans Serif", 12));
    QStringList lines;
    bernese::text_layout_utils::DrawElidedText(&layout, {90, 20},
        QTextOption::WrapAnywhere, Qt::ElideRight, 0, 0, &lines, nullptr, {},
        {}, {}, Qt::NoBrush, 4, nullptr);
    QCOMPARE(lines.size(), 1);
    QVERIFY(lines.first().contains(QChar(0x2026)));
    lines.clear();
    bernese::text_layout_utils::DrawElidedText(&layout, {0, 20},
        QTextOption::WrapAnywhere, Qt::ElideRight, 0, 0, &lines, nullptr, {},
        {}, {}, Qt::NoBrush, 4, nullptr);
    QVERIFY(lines.isEmpty());
  }

  void LayoutBoundsOnly() {
    const QFont font("Sans Serif", 12);
    QTextLayout layout("short", font);
    QList<QRectF> regions;
    bernese::text_layout_utils::DrawElidedText(&layout, {1000, 1000},
        QTextOption::NoWrap, Qt::ElideNone, 0, 0, nullptr, nullptr, {}, {},
        {}, Qt::NoBrush, 0, &regions);
    QCOMPARE(regions.size(), 1);
    QVERIFY(regions.first().width() > 0);
    QCOMPARE(regions.first().height(), QFontMetricsF(font).height());
  }

  /** @brief Verify inline objects survive text elision and painting. */
  void InlineTags() {
    for (int line_count : {1, 2, 5}) {
      TagObject tag;
      QTextDocument document;
      document.setDefaultFont(QFont("Sans Serif", 12));
      document.setPlainText(
          "Long tagged filename for checking Wrapping and truncation.txt");
      document.documentLayout()->registerHandler(
          QTextFormat::UserObject + 1, &tag);
      QTextCharFormat format;
      format.setObjectType(QTextFormat::UserObject + 1);
      QTextCursor cursor(&document);
      cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
      QImage image(140, 150, QImage::Format_ARGB32_Premultiplied);
      image.fill(Qt::transparent);
      QPainter painter(&image);
      painter.setFont(document.defaultFont());
      const qreal h = QFontMetricsF(painter.font()).height();
      QStringList lines;
      QList<QRectF> regions;
      bernese::text_layout_utils::DrawElidedText(document.firstBlock().layout(),
          {130, line_count * h}, QTextOption::WrapAtWordBoundaryOrAnywhere,
          Qt::ElideRight, h, Qt::AlignHCenter, &lines, &painter, {},
          QColor(Qt::gray), {0, 1}, QBrush(Qt::blue), 4, &regions, &document);
      QVERIFY(tag.PaintCount() > 0);
      QVERIFY(lines.first().startsWith(QChar::ObjectReplacementCharacter));
      QVERIFY(lines.size() <= line_count);
      for (const auto& r : regions) {
        QVERIFY(r.width() <= 130.1);
      }
      painter.end();
      if (line_count == 2) {
        image.save("/tmp/gxde-public-qt-tags.png");
      }
    }
  }

  /** @brief Verify preferred high-DPI assets and scale fallback. */
  void ScaledAssets() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath("icon.png");
    QImage icon(16, 16, QImage::Format_ARGB32_Premultiplied);
    icon.fill(Qt::red);
    QVERIFY(icon.save(path));
    QVERIFY(icon.save(directory.filePath("icon@2x.png")));
    QVERIFY(icon.save(directory.filePath("icon@4x.png")));
    qreal scale;
    QCOMPARE(bernese::image_utils::ScaledAssetPath(path, 1, &scale), path);
    QCOMPARE(scale, 1.0);
    QCOMPARE(bernese::image_utils::ScaledAssetPath(path, 1.5, &scale),
        directory.filePath("icon@2x.png"));
    QCOMPARE(scale, 2.0);
    QCOMPARE(bernese::image_utils::ScaledAssetPath(path, 3, &scale),
        directory.filePath("icon@4x.png"));
    QCOMPARE(scale, 4.0);
    QCOMPARE(bernese::image_utils::ScaledAssetPath(path, 6, &scale),
        directory.filePath("icon@4x.png"));
    QCOMPARE(bernese::image_utils::ScaledAssetPath(
                 directory.filePath("icon@2x.png"), 2, &scale),
        directory.filePath("icon@2x.png"));
    QCOMPARE(scale, 2.0);
  }

  /** @brief Blur works on X11 only for hidden windows, never for null. */
  void BlurBehindHints() {
    QWindow window;
    window.resize(100, 100);
    const bool x11 = bernese::X11Info::IsPlatformX11();
    QCOMPARE(bernese::window_effects::SetBlurBehind(&window), x11);
    QCOMPARE(bernese::window_effects::SetBlurBehind(
                 &window, {QRect(0, 0, 50, 50)}, 8),
        x11);
    QCOMPARE(bernese::window_effects::ClearBlurBehind(&window), x11);
    QVERIFY(!bernese::window_effects::SetBlurBehind(nullptr));
    QVERIFY(!bernese::window_effects::ClearBlurBehind(nullptr));
    QVERIFY(!bernese::window_effects::SetWindowRadius(nullptr, 1));
    // A window that was never shown has no Wayland surface.
    QVERIFY(!bernese::window_effects::SetWindowRadius(&window, 1));
    QVERIFY(!bernese::window_effects::SetNoTitleBar(nullptr, true));
    QVERIFY(!bernese::window_effects::SetNoTitleBar(&window, true));

    // No window manager to ask off X11 and Wayland.
    if (!x11 && QGuiApplication::platformName() != "wayland") {
      QVERIFY(!bernese::window_effects::HasCompositingManager());
      QVERIFY(!bernese::window_effects::IsBlurBehindSupported());
    }
  }

  /** @brief Wayland blur can be set again after hide and show. */
  void WaylandBlur() {
    if (QGuiApplication::platformName() != "wayland") {
      QSKIP("Needs the Wayland platform plugin.");
    }

    const bool supported = bernese::window_effects::IsBlurBehindSupported();
    QWindow window;
    window.resize(100, 100);
    for (int round = 0; round < 2; ++round) {
      window.show();
      QVERIFY(QTest::qWaitForWindowExposed(&window));
      QCOMPARE(bernese::window_effects::SetBlurBehind(&window), supported);
      QCOMPARE(bernese::window_effects::SetBlurBehind(
                   &window, {QRect(0, 0, 50, 50)}),
          supported);
      bernese::window_effects::SetWindowRadius(&window, 1);
      bernese::window_effects::SetNoTitleBar(&window, true);
      QCOMPARE(bernese::window_effects::ClearBlurBehind(&window), supported);
      window.hide();
    }
  }

  /** @brief Window lookup off X11 and outside every screen. */
  void WindowClassAtPoint() {
    if (!bernese::X11Info::IsPlatformX11()) {
      QVERIFY(bernese::window_utils::TopLevelWindowClassAt(QPoint(0, 0))
                  .isEmpty());
    }

    // No window is this far from every screen.
    QVERIFY(bernese::window_utils::TopLevelWindowClassAt(
        QPoint(-1000000, -1000000))
                .isEmpty());
  }

  /** @brief Verify border cursor selection and window-state transitions. */
  void PublicWindowHelpers() {
    QWidget widget;
    widget.resize(100, 100);
    QVERIFY(bernese::window_utils::UpdateCursorShape(&widget, 0, 0));
    QCOMPARE(widget.cursor().shape(), Qt::SizeFDiagCursor);
    QVERIFY(bernese::window_utils::UpdateCursorShape(&widget, 99, 0));
    QCOMPARE(widget.cursor().shape(), Qt::SizeBDiagCursor);
    QVERIFY(bernese::window_utils::UpdateCursorShape(&widget, 50, 0));
    QCOMPARE(widget.cursor().shape(), Qt::SizeVerCursor);
    QVERIFY(bernese::window_utils::UpdateCursorShape(&widget, 0, 50));
    QCOMPARE(widget.cursor().shape(), Qt::SizeHorCursor);
    QVERIFY(!bernese::window_utils::UpdateCursorShape(&widget, 50, 50));
    QCOMPARE(widget.cursor().shape(), Qt::ArrowCursor);
    bernese::window_utils::ShowMaximizedWindow(&widget);
    QVERIFY(widget.isMaximized());
    bernese::window_utils::ToggleMaximizedWindow(&widget);
    QVERIFY(!widget.isMaximized());
    bernese::window_utils::ShowMinimizedWindow(&widget);
    QVERIFY(widget.isMinimized());
    bernese::window_utils::ShowNormalWindow(&widget);
    QVERIFY(!widget.isMinimized());
  }

  /** @brief Verify blur preserves flat images, alpha constraints and symmetry.
   */
  void Blur() {
    QImage flat(20, 20, QImage::Format_ARGB32_Premultiplied);
    flat.fill(qRgba(80, 40, 20, 128));
    flat.setDevicePixelRatio(2);
    const auto original = flat;
    bernese::image_utils::Blur(flat, 6);
    QCOMPARE(flat, original);
    QCOMPARE(flat.devicePixelRatio(), 2.0);
    QImage impulse(31, 31, QImage::Format_ARGB32_Premultiplied);
    impulse.fill(Qt::transparent);
    impulse.setPixel(15, 15, qRgba(255, 0, 0, 255));
    bernese::image_utils::Blur(impulse, 6);
    QVERIFY(qAlpha(impulse.pixel(15, 15)) < 255);
    QVERIFY(qAlpha(impulse.pixel(14, 15)) > 0);
    for (int y = 0; y < impulse.height(); ++y) {
      for (int x = 0; x < impulse.width(); ++x) {
        QRgb pixel = impulse.pixel(x, y);
        QVERIFY(qRed(pixel) <= qAlpha(pixel));
        QCOMPARE(pixel, impulse.pixel(30 - x, y));
      }
    }
  }
};

}  // namespace bernese

QTEST_MAIN(bernese::BerneseTest)
#include "bernese_test.moc"
