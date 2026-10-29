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

#ifndef WINDOW_EFFECTS_WINDOW_EFFECTS_H_
#define WINDOW_EFFECTS_WINDOW_EFFECTS_H_

#include "QList"
#include "QRect"
#include "QWindow"

#include "bernese/export.h"

namespace bernese {
namespace window_effects {

/**
 * @brief Blur what is behind a window.
 *
 * On X11 this writes the same window hints that D-XCB writes for
 * DBlurEffectWidget, so any QWindow can have the blur without DTK. It
 * writes the Deepin hint and the KDE hint, for Deepin WM, Flake WM and KWin.
 *
 * On Wayland this uses the org_kde_kwin_blur protocol, like
 * DBlurEffectWidget. The blur goes away when the window is hidden, so call
 * this again after each show. It works from the next frame, and the areas
 * have square corners.
 *
 * Call on the GUI thread. On X11 this creates the native window if needed.
 * On Wayland the window must be shown. The window needs an alpha channel.
 * @param window Top-level window, may be null. Not owned.
 * @param areas Areas to blur, in logical pixels in the window. Empty means
 *        the whole window.
 * @param radius Corner radius of the areas, in logical pixels. X11 only.
 * @return True when the blur was sent. False on other platforms, for a null
 *         window, or on Wayland when the window is hidden or the compositor
 *         has no blur.
 */
BERNESE_EXPORT bool SetBlurBehind(
    QWindow* window, const QList<QRect>& areas = {}, int radius = 0);

/**
 * @brief Remove the blur set by SetBlurBehind().
 *
 * Call on the GUI thread.
 * @param window Top-level window, may be null. Not owned.
 * @return True when the blur was removed. False on other platforms, for a
 *         null window, or when the window has no native window or surface.
 */
BERNESE_EXPORT bool ClearBlurBehind(QWindow* window);

/**
 * @brief Set the corner radius the Wayland compositor draws for a window.
 *
 * Replaces the dde_shell window radius that DBlurEffectWidget sets on
 * Wayland. Treeland reads 0 as "use the default", so use 1 for a square
 * window.
 *
 * Call on the GUI thread after the window is shown, and again after each
 * show, like SetBlurBehind().
 * @param window Top-level window, may be null. Not owned.
 * @param radius Corner radius in logical pixels.
 * @return True when the radius was sent. False off Wayland, for a null or
 *         hidden window, or when the compositor has no dde_shell.
 */
BERNESE_EXPORT bool SetWindowRadius(QWindow* window, int radius);

/**
 * @brief Check whether an X11 compositing manager is running.
 *
 * Replaces the D-XCB "hasComposite" platform function. It checks the owner
 * of the _NET_WM_CM_Sn selection of the default screen.
 *
 * Call on the GUI thread.
 * @return True when the selection has an owner. False off X11.
 */
BERNESE_EXPORT bool HasCompositingManager();

/**
 * @brief Check whether SetBlurBehind() has an effect.
 *
 * Replaces the D-XCB "hasBlurWindow" platform function. On X11, Deepin WM
 * lists its blur atom in _NET_SUPPORTED, and KWin puts a property with the
 * blur atom name on the root window. On Wayland the compositor must have
 * org_kde_kwin_blur_manager.
 *
 * Call on the GUI thread.
 * @return True when blur works. False on other platforms.
 */
BERNESE_EXPORT bool IsBlurBehindSupported();

}  // namespace window_effects
}  // namespace bernese

#endif  // WINDOW_EFFECTS_WINDOW_EFFECTS_H_
