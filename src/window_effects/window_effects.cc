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

#include "window_effects/window_effects.h"

#include <cstdint>
#include <cstdlib>
#include <vector>

#include "QByteArray"
#include "QRegion"
#include "QtGlobal"
#include "xcb/xcb.h"

#include "window_effects/wayland_effects.h"
#include "x11_info/x11_info.h"

namespace bernese {
namespace window_effects {
namespace {

// Rounded areas read by Deepin WM: x, y, width, height, x radius, y radius.
constexpr char kDeepinBlurAtom[] = "_NET_WM_DEEPIN_BLUR_REGION_ROUNDED";
// Plain areas read by KWin: x, y, width, height.
constexpr char kKdeBlurAtom[] = "_KDE_NET_WM_BLUR_BEHIND_REGION";

/**
 * @brief Look up an X11 atom by name.
 *
 * @param connection A valid XCB connection.
 * @param name Atom name.
 * @return The atom, or XCB_ATOM_NONE when the lookup fails.
 */
xcb_atom_t InternAtom(xcb_connection_t* connection, const char* name) {
  auto* reply = xcb_intern_atom_reply(connection,
      xcb_intern_atom(connection, false, qstrlen(name), name), nullptr);
  if (!reply) {
    return XCB_ATOM_NONE;
  }

  const xcb_atom_t atom = reply->atom;
  std::free(reply);
  return atom;
}

/**
 * @brief Get the root window of the default screen.
 *
 * @param connection A valid XCB connection.
 * @param screen_number Receives the default screen number. May be null.
 * @return The root window, or XCB_WINDOW_NONE when it is unknown.
 */
xcb_window_t DefaultRoot(xcb_connection_t* connection, int* screen_number) {
  Display* display = X11Info::GetDisplay();
  if (!display) {
    return XCB_WINDOW_NONE;
  }

  const int number = DefaultScreen(display);
  if (screen_number) {
    *screen_number = number;
  }

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
 * @brief Check whether a window has a property.
 *
 * @param connection A valid XCB connection.
 * @param window Window to look at.
 * @param property Property atom.
 * @return True when the property exists.
 */
bool HasProperty(xcb_connection_t* connection, xcb_window_t window,
    xcb_atom_t property) {
  auto* reply = xcb_get_property_reply(connection,
      xcb_get_property(connection, false, window, property,
          XCB_GET_PROPERTY_TYPE_ANY, 0, 0),
      nullptr);
  if (!reply) {
    return false;
  }

  const bool exists = reply->type != XCB_ATOM_NONE;
  std::free(reply);
  return exists;
}

/**
 * @brief Check whether the window manager lists an atom in _NET_SUPPORTED.
 *
 * @param connection A valid XCB connection.
 * @param root Root window.
 * @param atom Atom to look for.
 * @return True when the atom is listed.
 */
bool IsNetSupported(xcb_connection_t* connection, xcb_window_t root,
    xcb_atom_t atom) {
  const xcb_atom_t supported = InternAtom(connection, "_NET_SUPPORTED");
  if (supported == XCB_ATOM_NONE) {
    return false;
  }

  // 4096 atoms is more than any WM lists.
  auto* reply = xcb_get_property_reply(connection,
      xcb_get_property(connection, false, root, supported, XCB_ATOM_ATOM, 0,
          4096),
      nullptr);
  if (!reply) {
    return false;
  }

  const auto* atoms =
      static_cast<const xcb_atom_t*>(xcb_get_property_value(reply));
  const int count = xcb_get_property_value_length(reply) /
      static_cast<int>(sizeof(xcb_atom_t));
  bool found = false;
  for (int i = 0; i < count && !found; ++i) {
    found = atoms[i] == atom;
  }

  std::free(reply);
  return found;
}

/**
 * @brief Replace a CARDINAL list property on a window.
 *
 * @param connection A valid XCB connection.
 * @param window Target X11 window.
 * @param name Property name.
 * @param values New property values.
 */
void SetCardinals(xcb_connection_t* connection, xcb_window_t window,
    const char* name, const std::vector<uint32_t>& values) {
  const xcb_atom_t atom = InternAtom(connection, name);
  if (atom == XCB_ATOM_NONE) {
    return;
  }

  xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, atom,
      XCB_ATOM_CARDINAL, 32, values.size(), values.data());
}

/**
 * @brief Convert a logical length to native pixels, at least 0.
 *
 * @param value Logical length.
 * @param scale Device pixel ratio of the window.
 * @return Rounded native length.
 */
uint32_t ToNative(int value, qreal scale) {
  return static_cast<uint32_t>(qMax(0, qRound(value * scale)));
}

// The Wayland blur protocol only takes rectangles, so cut each area into
// rectangles that follow its rounded corners. Same as DMenuEffect.
QList<QRect> RoundedAreas(const QList<QRect>& areas, int radius) {
  if (radius <= 0) {
    return areas;
  }

  QList<QRect> rects;
  for (const QRect& area : areas) {
    const int r = qMin(radius, qMin(area.width(), area.height()) / 2);
    const int d = r * 2;
    QRegion region(area.adjusted(r, 0, -r, 0));
    region += area.adjusted(0, r, 0, -r);
    region += QRegion(area.x(), area.y(), d, d, QRegion::Ellipse) &
        QRect(area.x(), area.y(), r, r);
    region += QRegion(area.right() - d + 1, area.y(), d, d, QRegion::Ellipse) &
        QRect(area.right() - r + 1, area.y(), r, r);
    region +=
        QRegion(area.x(), area.bottom() - d + 1, d, d, QRegion::Ellipse) &
        QRect(area.x(), area.bottom() - r + 1, r, r);
    region += QRegion(area.right() - d + 1, area.bottom() - d + 1, d, d,
                  QRegion::Ellipse) &
        QRect(area.right() - r + 1, area.bottom() - r + 1, r, r);
    for (const QRect& rect : region) {
      rects.append(rect);
    }
  }

  return rects.isEmpty() ? areas : rects;
}

}  // namespace

bool SetBlurBehind(QWindow* window, const QList<QRect>& areas, int radius) {
  if (internal::IsWayland()) {
    if (radius <= 0 || !window) {
      return internal::SetWaylandBlur(window, areas);
    }

    const QList<QRect> rects = areas.isEmpty()
        ? QList<QRect>{QRect(QPoint(), window->size())}
        : areas;
    return internal::SetWaylandBlur(window, RoundedAreas(rects, radius));
  }

  xcb_connection_t* connection = X11Info::GetConnection();
  if (!window || !connection) {
    return false;
  }

  const xcb_window_t id = static_cast<xcb_window_t>(window->winId());
  const qreal scale = window->devicePixelRatio();
  const QList<QRect> rects =
      areas.isEmpty() ? QList<QRect>{QRect(QPoint(), window->size())} : areas;

  std::vector<uint32_t> deepin;
  std::vector<uint32_t> kde;
  for (const QRect& rect : rects) {
    // CARDINAL is unsigned. Keep the bits of negative positions.
    const uint32_t x = static_cast<uint32_t>(qRound(rect.x() * scale));
    const uint32_t y = static_cast<uint32_t>(qRound(rect.y() * scale));
    const uint32_t width = ToNative(rect.width(), scale);
    const uint32_t height = ToNative(rect.height(), scale);
    const uint32_t corner = ToNative(radius, scale);
    deepin.insert(deepin.end(), {x, y, width, height, corner, corner});
    kde.insert(kde.end(), {x, y, width, height});
  }

  SetCardinals(connection, id, kDeepinBlurAtom, deepin);
  SetCardinals(connection, id, kKdeBlurAtom, kde);
  xcb_flush(connection);
  return true;
}

bool ClearBlurBehind(QWindow* window) {
  if (internal::IsWayland()) {
    return internal::ClearWaylandBlur(window);
  }

  xcb_connection_t* connection = X11Info::GetConnection();
  if (!window || !connection || !window->handle()) {
    return false;
  }

  const xcb_window_t id = static_cast<xcb_window_t>(window->winId());
  for (const char* name : {kDeepinBlurAtom, kKdeBlurAtom}) {
    const xcb_atom_t atom = InternAtom(connection, name);
    if (atom != XCB_ATOM_NONE) {
      xcb_delete_property(connection, id, atom);
    }
  }

  xcb_flush(connection);
  return true;
}

bool SetWindowRadius(QWindow* window, int radius) {
  return internal::SetWaylandWindowRadius(window, radius);
}

bool SetNoTitleBar(QWindow* window, bool no_title_bar) {
  return internal::SetWaylandNoTitleBar(window, no_title_bar);
}

bool HasCompositingManager() {
  xcb_connection_t* connection = X11Info::GetConnection();
  if (!connection) {
    return false;
  }

  int screen_number = 0;
  if (DefaultRoot(connection, &screen_number) == XCB_WINDOW_NONE) {
    return false;
  }

  const QByteArray name = "_NET_WM_CM_S" + QByteArray::number(screen_number);
  const xcb_atom_t selection = InternAtom(connection, name.constData());
  if (selection == XCB_ATOM_NONE) {
    return false;
  }

  auto* reply = xcb_get_selection_owner_reply(connection,
      xcb_get_selection_owner(connection, selection), nullptr);
  if (!reply) {
    return false;
  }

  const bool owned = reply->owner != XCB_WINDOW_NONE;
  std::free(reply);
  return owned;
}

bool IsBlurBehindSupported() {
  if (internal::IsWayland()) {
    return internal::HasWaylandBlur();
  }

  xcb_connection_t* connection = X11Info::GetConnection();
  if (!connection) {
    return false;
  }

  const xcb_window_t root = DefaultRoot(connection, nullptr);
  if (root == XCB_WINDOW_NONE) {
    return false;
  }

  const xcb_atom_t deepin = InternAtom(connection, kDeepinBlurAtom);
  if (deepin != XCB_ATOM_NONE && IsNetSupported(connection, root, deepin)) {
    return true;
  }

  const xcb_atom_t kde = InternAtom(connection, kKdeBlurAtom);
  return kde != XCB_ATOM_NONE && HasProperty(connection, root, kde);
}

}  // namespace window_effects
}  // namespace bernese
