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

#ifndef X11_INFO_X11_INFO_H_
#define X11_INFO_X11_INFO_H_

#include "QGuiApplication"
#include "QWindow"
#include "X11/Xlib.h"
#include "xcb/xcb.h"

#include "bernese/export.h"

namespace bernese {

class BERNESE_EXPORT X11Info {
 public:
  /**
   * @brief Check whether we're using Qt XCB platform.
   *
   * @note Note that @c XCB, @c D-XCB, and @c XWayland all counts.
   * @return True when we're using X11 platform.
   */
  static bool IsPlatformX11();

  /**
   * @brief Access the Xlib display connection.
   *
   * @note Must be called on the Qt GUI thread. The display connection will
   *       keep alive till Qt shuts down.
   * @warning Don't shut down the connection manually!!
   * @return The display borrowed, or @c nullptr when fails.
   */
  static Display* GetDisplay();

  /**
   * @brief Access the XCB connection we already have.
   *
   * @note Must be called on the Qt GUI thread. The display connection will
   *       keep alive till Qt shuts down.
   * @warning Don't shut down the connection manually!!
   * @return a XCB connection struct.
   */
  static xcb_connection_t* GetConnection();

  /**
   * @brief Get an X server timestamp.
   *
   * @note Must be called on the Qt GUI thread
   * @return X11 Server timestamp, or can be XCB CurrentTime if failed.
   */
  static Time GetTimestamp();
};

}  // namespace bernese

#endif  // X11_INFO_X11_INFO_H_
