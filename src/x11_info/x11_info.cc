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

// Must load the Qt types before Xlib defines macros.
#include "x11_info/x11_info.h"

#include <qlatin1stringview.h>

#include <QGuiApplication>
#include <QString>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <xcb/xcb.h>

namespace bernese {

bool X11Info::IsPlatformX11() {
  auto* cur_app = static_cast<QGuiApplication *>(QCoreApplication::instance());
  if (cur_app != nullptr) {
    return (cur_app->platformName() == QLatin1String("xcb")
        || cur_app->platformName() == QLatin1String("dxcb"));
  }

  return false;
}

Display* X11Info::GetDisplay() {
  if (!IsPlatformX11()) {
    return nullptr;
  }

  // Keep the connection alive till Qt shuts down.
  static Display* const kDisplay = XOpenDisplay(nullptr);
  return kDisplay;
}

xcb_connection_t* X11Info::GetConnection() {
  if (!IsPlatformX11()) {
    return nullptr;
  }

  static xcb_connection_t* const kResult = xcb_connect(nullptr, nullptr);
  return xcb_connection_has_error(kResult) ? nullptr : kResult;
}

Time X11Info::GetTimestamp() {
  if (!IsPlatformX11()) {
    return CurrentTime;
  }

  Display* display = XOpenDisplay(nullptr);
  if (!display) {
    return CurrentTime;
  }

  const Window window = XCreateSimpleWindow(
      display, DefaultRootWindow(display), 0, 0, 1, 1, 0, 0, 0);
  XSelectInput(display, window, PropertyChangeMask);

  const Atom property = XInternAtom(display, "_GXDE_TIMESTAMP", False);
  const unsigned char value = 0;
  XChangeProperty(
      display, window, property, XA_INTEGER, 8, PropModeReplace, &value, 1);

  XEvent event;
  XWindowEvent(display, window, PropertyChangeMask, &event);
  const Time time = event.xproperty.time;

  XDestroyWindow(display, window);
  XCloseDisplay(display);
  return time;
}

}  // namespace bernese
