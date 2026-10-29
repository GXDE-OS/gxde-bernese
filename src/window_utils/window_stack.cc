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

#include "window_utils/window_stack.h"

#include <cstdlib>
#include <memory>

#include "QByteArray"
#include "QRect"
#include "xcb/xcb.h"

#include "x11_info/x11_info.h"

namespace bernese {
namespace window_utils {
namespace {

/**
 * @brief Frees XCB replies.
 */
struct FreeDeleter {
  /**
   * @brief Free one reply.
   *
   * @param reply Reply allocated by XCB.
   */
  void operator()(void* reply) const { std::free(reply); }
};

template <typename T>
using Reply = std::unique_ptr<T, FreeDeleter>;

/**
 * @brief Look up an X11 atom by name.
 *
 * @param connection A valid XCB connection.
 * @param name Atom name.
 * @return The atom, or XCB_ATOM_NONE when the lookup fails.
 */
xcb_atom_t InternAtom(xcb_connection_t* connection, const char* name) {
  Reply<xcb_intern_atom_reply_t> reply(xcb_intern_atom_reply(connection,
      xcb_intern_atom(connection, true, qstrlen(name), name), nullptr));
  return reply ? reply->atom : XCB_ATOM_NONE;
}

/**
 * @brief Get the root window of the default screen.
 *
 * @param connection A valid XCB connection.
 * @return The root window, or XCB_WINDOW_NONE when it is unknown.
 */
xcb_window_t DefaultRoot(xcb_connection_t* connection) {
  Display* display = X11Info::GetDisplay();
  if (!display) {
    return XCB_WINDOW_NONE;
  }

  const int number = DefaultScreen(display);
  xcb_screen_iterator_t it =
      xcb_setup_roots_iterator(xcb_get_setup(connection));
  for (int i = 0; it.rem > 0; ++i, xcb_screen_next(&it)) {
    if (i == number) {
      return it.data->root;
    }
  }

  return XCB_WINDOW_NONE;
}

/**
 * @brief Get the area a mapped window covers on the root window.
 *
 * @param connection A valid XCB connection.
 * @param root Root window.
 * @param window Client window.
 * @return The area in root coordinates. Empty when the window is not
 *         viewable or is gone.
 */
QRect ViewableArea(xcb_connection_t* connection, xcb_window_t root,
    xcb_window_t window) {
  Reply<xcb_get_window_attributes_reply_t> attributes(
      xcb_get_window_attributes_reply(connection,
          xcb_get_window_attributes(connection, window), nullptr));
  if (!attributes || attributes->map_state != XCB_MAP_STATE_VIEWABLE) {
    return QRect();
  }

  Reply<xcb_get_geometry_reply_t> geometry(xcb_get_geometry_reply(
      connection, xcb_get_geometry(connection, window), nullptr));
  Reply<xcb_translate_coordinates_reply_t> origin(
      xcb_translate_coordinates_reply(connection,
          xcb_translate_coordinates(connection, window, root, 0, 0),
          nullptr));
  if (!geometry || !origin) {
    return QRect();
  }

  return QRect(origin->dst_x, origin->dst_y, geometry->width,
      geometry->height);
}

/**
 * @brief Read WM_CLASS of a window.
 *
 * @param connection A valid XCB connection.
 * @param window Client window.
 * @return Instance name and class name, or an empty list.
 */
QStringList WindowClass(xcb_connection_t* connection, xcb_window_t window) {
  Reply<xcb_get_property_reply_t> reply(xcb_get_property_reply(connection,
      xcb_get_property(connection, false, window, XCB_ATOM_WM_CLASS,
          XCB_ATOM_STRING, 0, 512),
      nullptr));
  if (!reply) {
    return {};
  }

  // WM_CLASS is two strings, each ends with a null byte.
  const QByteArray value(
      static_cast<const char*>(xcb_get_property_value(reply.get())),
      xcb_get_property_value_length(reply.get()));
  QStringList names;
  for (const QByteArray& part : value.split('\0')) {
    if (!part.isEmpty()) {
      names.append(QString::fromLocal8Bit(part));
    }
  }

  return names;
}

}  // namespace

QStringList TopLevelWindowClassAt(const QPoint& native_pos) {
  xcb_connection_t* connection = X11Info::GetConnection();
  if (!connection) {
    return {};
  }

  const xcb_window_t root = DefaultRoot(connection);
  const xcb_atom_t stacking =
      InternAtom(connection, "_NET_CLIENT_LIST_STACKING");
  if (root == XCB_WINDOW_NONE || stacking == XCB_ATOM_NONE) {
    return {};
  }

  // 16384 windows is more than enough.
  Reply<xcb_get_property_reply_t> reply(xcb_get_property_reply(connection,
      xcb_get_property(connection, false, root, stacking, XCB_ATOM_WINDOW, 0,
          16384),
      nullptr));
  if (!reply) {
    return {};
  }

  const auto* windows =
      static_cast<const xcb_window_t*>(xcb_get_property_value(reply.get()));
  const int count = xcb_get_property_value_length(reply.get()) /
      static_cast<int>(sizeof(xcb_window_t));

  // The list goes from bottom to top.
  for (int i = count - 1; i >= 0; --i) {
    if (ViewableArea(connection, root, windows[i]).contains(native_pos)) {
      return WindowClass(connection, windows[i]);
    }
  }

  return {};
}

}  // namespace window_utils
}  // namespace bernese
