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

#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "QPoint"
#include "QRect"
#include "QSize"
#include "QString"
#include "QtGlobal"
#include "QtMath"
#include "xcb/randr.h"
#include "xcb/xcb.h"

#include "backdrop_origin/backdrop_origin.h"
#include "backdrop_origin/connection/connection.h"

namespace bernese {
namespace wayland {
namespace {

/**
 * @brief Find the X11 output position and size for an exact RandR name.
 *
 * @param connection Borrowed valid XCB connection.
 * @param root Root window of the selected X screen.
 * @param output_name RandR output name to match.
 * @return X11 output geometry, or an empty rectangle if lookup fails.
 */
QRect FindX11OutputGeometry(xcb_connection_t* connection, xcb_window_t root,
    const QString& output_name) {
  QRect native_output;
  auto* resources = xcb_randr_get_screen_resources_current_reply(connection,
      xcb_randr_get_screen_resources_current(connection, root), nullptr);

  if (!resources) {
    return {};
  }

  const auto* outputs =
      xcb_randr_get_screen_resources_current_outputs(resources);
  for (int i = 0;
      i < xcb_randr_get_screen_resources_current_outputs_length(resources);
      ++i) {
    auto* info = xcb_randr_get_output_info_reply(connection,
        xcb_randr_get_output_info(
            connection, outputs[i], resources->config_timestamp),
        nullptr);
    if (info && info->connection == XCB_RANDR_CONNECTION_CONNECTED &&
        info->crtc != XCB_NONE &&
        QString::fromUtf8(
            reinterpret_cast<const char*>(xcb_randr_get_output_info_name(info)),
            xcb_randr_get_output_info_name_length(info)) == output_name) {
      auto* crtc = xcb_randr_get_crtc_info_reply(connection,
          xcb_randr_get_crtc_info(
              connection, info->crtc, resources->config_timestamp),
          nullptr);
      if (crtc) {
        native_output = QRect(crtc->x, crtc->y, crtc->width, crtc->height);
      }
      std::free(crtc);
    }
    std::free(info);
    if (native_output.isValid()) {
      break;
    }
  }

  std::free(resources);
  return native_output;
}

/**
 * @brief Read a slice of a CARDINAL property from a root window.
 *
 * @param connection Borrowed valid XCB connection.
 * @param root Window holding the property.
 * @param name Atom name; missing atoms are not created.
 * @param offset Offset in 32-bit units.
 * @param length Maximum number of 32-bit units to read.
 * @return Allocated reply, or null; the caller must release it with std::free.
 */
xcb_get_property_reply_t* ReadCardinalProperty(xcb_connection_t* connection,
    xcb_window_t root, const char* name, uint32_t offset, uint32_t length) {
  auto* atom = xcb_intern_atom_reply(connection,
      xcb_intern_atom(connection, true, std::strlen(name), name), nullptr);
  xcb_get_property_reply_t* reply = nullptr;
  if (atom && atom->atom != XCB_ATOM_NONE) {
    reply = xcb_get_property_reply(connection,
        xcb_get_property(connection, false, root, atom->atom, XCB_ATOM_CARDINAL,
            offset, length),
        nullptr);
  }
  std::free(atom);
  return reply;
}

/**
 * @brief Read the current desktop work area from EWMH properties.
 *
 * Defaults to desktop zero if the current-desktop property is missing.
 *
 * @param connection Borrowed valid XCB connection.
 * @param root Root window holding the EWMH properties.
 * @return Native work-area rectangle, or empty on invalid data.
 */
QRect ReadDesktopWorkArea(xcb_connection_t* connection, xcb_window_t root) {
  uint32_t desktop = 0;
  auto* reply =
      ReadCardinalProperty(connection, root, "_NET_CURRENT_DESKTOP", 0, 1);
  if (reply && reply->format == 32 && reply->value_len == 1) {
    desktop = *static_cast<const uint32_t*>(xcb_get_property_value(reply));
  }
  std::free(reply);
  // Bound an untrusted desktop index before calculating the property offset.
  if (desktop > 1024) {
    return {};
  }

  // EWMH stores one (x, y, width, height) tuple per virtual desktop.
  reply =
      ReadCardinalProperty(connection, root, "_NET_WORKAREA", desktop * 4, 4);
  QRect result;
  if (reply && reply->format == 32 && reply->value_len == 4) {
    const auto* values =
        static_cast<const uint32_t*>(xcb_get_property_value(reply));
    result =
        QRect(static_cast<int32_t>(values[0]), static_cast<int32_t>(values[1]),
            static_cast<int>(values[2]), static_cast<int>(values[3]));
  }
  std::free(reply);
  return result;
}

}  // namespace

QRect LogicalWorkArea(const QRect& native_output, const QRect& native_area,
    const QRect& logical_output) {
  if (!native_output.isValid() || !logical_output.isValid()) {
    return {};
  }

  const QRect area = native_area.intersected(native_output);
  if (!area.isValid()) {
    return {};
  }

  // Use the output geometry, not QScreen::devicePixelRatio(): Wayland may
  // advertise an integer buffer scale (2) for a fractional output (1.25).
  const qreal horizontal_scale =
      static_cast<qreal>(logical_output.width()) / native_output.width();
  const qreal vertical_scale =
      static_cast<qreal>(logical_output.height()) / native_output.height();
  const int left = qRound((area.x() - native_output.x()) * horizontal_scale);
  const int top = qRound((area.y() - native_output.y()) * vertical_scale);
  const int right =
      qRound((area.x() + area.width() - native_output.x()) * horizontal_scale);
  const int bottom =
      qRound((area.y() + area.height() - native_output.y()) * vertical_scale);
  return QRect(logical_output.topLeft() + QPoint(left, top),
      QSize(right - left, bottom - top));
}

QRect XwaylandWorkArea(
    const QString& output_name, const QRect& logical_output) {
  if (qEnvironmentVariableIsEmpty("DISPLAY")) {
    return {};
  }

  static const internal::Connection* const kConnection =
      new internal::Connection;
  auto* connection = kConnection->value;
  if (xcb_connection_has_error(connection)) {
    return {};
  }

  auto screen = xcb_setup_roots_iterator(xcb_get_setup(connection));
  for (int i = 0; i < kConnection->screen && screen.rem; ++i) {
    xcb_screen_next(&screen);
  }
  if (!screen.rem) {
    return {};
  }

  const xcb_window_t root = screen.data->root;
  const QRect native_output =
      FindX11OutputGeometry(connection, root, output_name);
  if (!native_output.isValid()) {
    return {};
  }

  const QRect native_work_area = ReadDesktopWorkArea(connection, root);
  return LogicalWorkArea(native_output, native_work_area, logical_output);
}

QPoint BackdropOrigin(const QRect& screen, const QSize& surface,
    const QRect& available, const QPoint& fallback) {
  // Reject stale work areas, another output, and incompatible coordinate
  // scales. Full-size surfaces need no exclusive-zone translation.
  if (surface == screen.size()) {
    return {};
  }
  if (available.isValid() && available.size() == surface &&
      screen.contains(available)) {
    return available.topLeft() - screen.topLeft();
  }
  return fallback;
}

}  // namespace wayland
}  // namespace bernese
