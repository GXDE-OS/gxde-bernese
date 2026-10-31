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

#include "window_effects/wayland_effects.h"

#include <cstring>

#include "QEvent"
#include "QGuiApplication"
#include "QHash"
#include "QObject"
#include "QPlatformSurfaceEvent"
#include "QWindow"
#include "QtGlobal"

// QNativeInterface::QWaylandApplication is public since Qt 6.5.
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include "QtGui/qguiapplication_platform.h"
#include "qpa/qplatformnativeinterface.h"
#if QT_CONFIG(wayland)
#define BERNESE_WAYLAND_EFFECTS 1
#endif
#endif

#ifdef BERNESE_WAYLAND_EFFECTS
#include "wayland-client.h"

#include "protocols/blur-client-protocol.h"
#include "protocols/dde-shell-client-protocol.h"
#endif

namespace bernese {
namespace window_effects {
namespace internal {

#ifdef BERNESE_WAYLAND_EFFECTS
namespace {

// Compositor globals we use. They live as long as the process.
struct Globals {
  org_kde_kwin_blur_manager* blur = nullptr;
  dde_shell* shell = nullptr;
};

// Objects we made for the current surface of one window.
struct Effects {
  wl_surface* surface = nullptr;
  org_kde_kwin_blur* blur = nullptr;
  dde_shell_surface* shell_surface = nullptr;
};

/**
 * @brief Get the Wayland interface of Qt.
 *
 * @return The interface, or null when Qt does not use Wayland.
 */
QNativeInterface::QWaylandApplication* Native() {
  return qGuiApp
      ? qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>()
      : nullptr;
}

/**
 * @brief Bind the globals we need.
 *
 * @param data The Globals to fill.
 * @param registry Registry that sent the global.
 * @param name Name of the global.
 * @param interface Interface of the global.
 * @param version Version of the global.
 */
void HandleGlobal(void* data, wl_registry* registry, uint32_t name,
    const char* interface, uint32_t version) {
  auto* globals = static_cast<Globals*>(data);
  if (std::strcmp(interface, org_kde_kwin_blur_manager_interface.name) == 0) {
    globals->blur = static_cast<org_kde_kwin_blur_manager*>(wl_registry_bind(
        registry, name, &org_kde_kwin_blur_manager_interface, 1));
  } else if (std::strcmp(interface, dde_shell_interface.name) == 0 &&
      version >= 2) {
    // set_property needs version 2.
    globals->shell = static_cast<dde_shell*>(
        wl_registry_bind(registry, name, &dde_shell_interface, 2));
  }
}

/**
 * @brief Do nothing when a global goes away.
 *
 * @param data Not used.
 * @param registry Not used.
 * @param name Not used.
 */
void HandleGlobalRemove(void* data, wl_registry* registry, uint32_t name) {
  Q_UNUSED(data)
  Q_UNUSED(registry)
  Q_UNUSED(name)
}

/**
 * @brief Bind the globals on the display of Qt.
 *
 * We use our own event queue, so Qt does not see the registry events.
 * @return The globals. Missing ones are null.
 */
Globals BindGlobals() {
  Globals globals;
  QNativeInterface::QWaylandApplication* native = Native();
  wl_display* display = native ? native->display() : nullptr;
  if (!display) {
    return globals;
  }

  wl_event_queue* queue = wl_display_create_queue(display);
  auto* wrapper = static_cast<wl_display*>(wl_proxy_create_wrapper(display));
  wl_proxy_set_queue(reinterpret_cast<wl_proxy*>(wrapper), queue);
  wl_registry* registry = wl_display_get_registry(wrapper);
  wl_proxy_wrapper_destroy(wrapper);

  static const wl_registry_listener kRegistryListener = {
      HandleGlobal, HandleGlobalRemove};
  wl_registry_add_listener(registry, &kRegistryListener, &globals);
  wl_display_roundtrip_queue(display, queue);

  // Move the globals to the default queue, so we can delete our queue. Qt
  // drops events of objects that have no listener.
  for (void* proxy : {static_cast<void*>(globals.blur),
           static_cast<void*>(globals.shell)}) {
    if (proxy) {
      wl_proxy_set_queue(static_cast<wl_proxy*>(proxy), nullptr);
    }
  }
  wl_registry_destroy(registry);
  wl_event_queue_destroy(queue);
  return globals;
}

/**
 * @brief Get the globals. Binds them on first use.
 *
 * @return The globals.
 */
const Globals& GetGlobals() {
  static const Globals globals = BindGlobals();
  return globals;
}

/**
 * @brief Get the wl_surface of a window.
 *
 * Qt Wayland uses the wl_surface as the window id. We check the class name,
 * so we do nothing if that ever changes.
 * @param window The window.
 * @return The surface, or null when there is none.
 */
wl_surface* SurfaceOf(QWindow* window) {
  // Do not create the native window: a layer-shell role must be set first.
  if (!window->handle()) {
    return nullptr;
  }

  // QWindow::winId() is not the wl_surface on every Qt version: Qt 6.8
  // returns a dummy id (1), and passing it to wl_proxy_get_class() crashes.
  // Ask the platform plugin for the surface instead.
  if (QPlatformNativeInterface* native =
          QGuiApplication::platformNativeInterface()) {
    if (auto* surface = static_cast<wl_surface*>(
            native->nativeResourceForWindow("surface", window))) {
      return surface;
    }
  }

  // Fallback for platforms where the resource is not published: Qt Wayland
  // has used the wl_surface as the window id, so check the class first.
  const WId id = window->winId();
  if (id < 0x10000) {
    return nullptr;
  }
  auto* proxy = reinterpret_cast<wl_proxy*>(id);
  if (std::strcmp(wl_proxy_get_class(proxy), wl_surface_interface.name) != 0) {
    return nullptr;
  }
  return reinterpret_cast<wl_surface*>(proxy);
}

/**
 * @class Tracker
 * @brief Keeps the objects of each window for its current surface.
 *
 * Qt makes a new wl_surface when the native window is made again, and some
 * Qt versions also on each show. So we drop the objects on hide and before
 * the surface goes away.
 */
class Tracker : public QObject {
 public:
  /**
   * @brief Get the objects for the current surface of a window.
   *
   * @param window The window.
   * @return The objects, or null when the window has no surface.
   */
  Effects* Get(QWindow* window) {
    wl_surface* surface = SurfaceOf(window);
    if (!surface) {
      return nullptr;
    }

    auto it = effects_.find(window);
    if (it == effects_.end()) {
      it = effects_.insert(window, {});
      window->installEventFilter(this);
      connect(window, &QObject::destroyed, this, [this, window] {
        Remove(window);
      });
    }
    if (it->surface != surface) {
      Release(&it.value());
      it->surface = surface;
    }
    return &it.value();
  }

 protected:
  /**
   * @brief Drop the objects when the surface may go away.
   *
   * @param watched The window.
   * @param event Its event.
   * @return Always false, so Qt still gets the event.
   */
  bool eventFilter(QObject* watched, QEvent* event) override {
    const bool lost = event->type() == QEvent::PlatformSurface &&
        static_cast<QPlatformSurfaceEvent*>(event)->surfaceEventType() ==
            QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed;
    if (lost || event->type() == QEvent::Hide) {
      auto it = effects_.find(static_cast<QWindow*>(watched));
      if (it != effects_.end()) {
        Release(&it.value());
        it->surface = nullptr;
      }
    }
    return false;
  }

 private:
  /**
   * @brief Forget a window and drop its objects.
   *
   * @param window The window. It may be deleted already.
   */
  void Remove(QWindow* window) {
    auto it = effects_.find(window);
    if (it != effects_.end()) {
      Release(&it.value());
      effects_.erase(it);
    }
  }

  /**
   * @brief Destroy the objects of one window.
   *
   * @param effects The objects.
   */
  static void Release(Effects* effects) {
    if (effects->blur) {
      org_kde_kwin_blur_release(effects->blur);
      effects->blur = nullptr;
    }
    if (effects->shell_surface) {
      dde_shell_surface_destroy(effects->shell_surface);
      effects->shell_surface = nullptr;
    }
  }

  QHash<QWindow*, Effects> effects_;
};

/**
 * @brief Get the tracker. It lives as long as the process.
 *
 * @return The tracker.
 */
Tracker* GetTracker() {
  static Tracker* tracker = new Tracker;
  return tracker;
}

/**
 * @brief Send our requests and ask for a new frame.
 *
 * We never commit the surface, because Qt owns it. The compositor uses our
 * changes with the next frame.
 * @param window The window.
 */
void Flush(QWindow* window) {
  window->requestUpdate();
  if (QNativeInterface::QWaylandApplication* native = Native()) {
    wl_display_flush(native->display());
  }
}

}  // namespace

bool IsWayland() {
  return Native() != nullptr;
}

bool SetWaylandBlur(QWindow* window, const QList<QRect>& areas) {
  const Globals& globals = GetGlobals();
  wl_compositor* compositor = IsWayland() ? Native()->compositor() : nullptr;
  const bool ready = window && globals.blur && compositor;
  Effects* effects = ready ? GetTracker()->Get(window) : nullptr;
  if (!effects) {
    return false;
  }

  if (!effects->blur) {
    effects->blur =
        org_kde_kwin_blur_manager_create(globals.blur, effects->surface);
  }

  // A null region means the whole surface.
  wl_region* region = nullptr;
  if (!areas.isEmpty()) {
    region = wl_compositor_create_region(compositor);
    for (const QRect& rect : areas) {
      wl_region_add(region, rect.x(), rect.y(), rect.width(), rect.height());
    }
  }
  org_kde_kwin_blur_set_region(effects->blur, region);
  org_kde_kwin_blur_commit(effects->blur);
  if (region) {
    wl_region_destroy(region);
  }

  Flush(window);
  return true;
}

bool ClearWaylandBlur(QWindow* window) {
  const Globals& globals = GetGlobals();
  Effects* effects =
      window && globals.blur ? GetTracker()->Get(window) : nullptr;
  if (!effects) {
    return false;
  }

  org_kde_kwin_blur_manager_unset(globals.blur, effects->surface);
  if (effects->blur) {
    org_kde_kwin_blur_release(effects->blur);
    effects->blur = nullptr;
  }

  Flush(window);
  return true;
}

bool HasWaylandBlur() {
  return IsWayland() && GetGlobals().blur;
}

namespace {

// The dde_shell surface of a window, made on first use. Null when the
// compositor has no dde_shell or the window has no surface.
dde_shell_surface* ShellSurfaceOf(QWindow* window) {
  const Globals& globals = GetGlobals();
  Effects* effects =
      window && globals.shell ? GetTracker()->Get(window) : nullptr;
  if (!effects) {
    return nullptr;
  }

  if (!effects->shell_surface) {
    effects->shell_surface =
        dde_shell_get_shell_surface(globals.shell, effects->surface);
  }
  return effects->shell_surface;
}

}  // namespace

bool SetWaylandWindowRadius(QWindow* window, int radius) {
  dde_shell_surface* shell_surface = ShellSurfaceOf(window);
  if (!shell_surface) {
    return false;
  }

  // Two floats: x radius and y radius.
  wl_array value;
  wl_array_init(&value);
  if (auto* data = static_cast<float*>(
          wl_array_add(&value, sizeof(float) * 2))) {
    data[0] = static_cast<float>(radius);
    data[1] = static_cast<float>(radius);
  }
  dde_shell_surface_set_property(
      shell_surface, DDE_SHELL_PROPERTY_WINDOWRADIUS, &value);
  wl_array_release(&value);

  Flush(window);
  return true;
}

bool SetWaylandNoTitleBar(QWindow* window, bool no_title_bar) {
  dde_shell_surface* shell_surface = ShellSurfaceOf(window);
  if (!shell_surface) {
    return false;
  }

  // One int: 1 means the app draws the title bar.
  wl_array value;
  wl_array_init(&value);
  if (auto* data = static_cast<int32_t*>(
          wl_array_add(&value, sizeof(int32_t)))) {
    *data = no_title_bar ? 1 : 0;
  }
  dde_shell_surface_set_property(
      shell_surface, DDE_SHELL_PROPERTY_NOTITLEBAR, &value);
  wl_array_release(&value);

  Flush(window);
  return true;
}

#else  // BERNESE_WAYLAND_EFFECTS

// Without the Wayland interface of Qt we cannot get a wl_surface.

bool IsWayland() {
  return false;
}

bool SetWaylandBlur(QWindow* window, const QList<QRect>& areas) {
  Q_UNUSED(window)
  Q_UNUSED(areas)
  return false;
}

bool ClearWaylandBlur(QWindow* window) {
  Q_UNUSED(window)
  return false;
}

bool HasWaylandBlur() {
  return false;
}

bool SetWaylandWindowRadius(QWindow* window, int radius) {
  Q_UNUSED(window)
  Q_UNUSED(radius)
  return false;
}

bool SetWaylandNoTitleBar(QWindow* window, bool no_title_bar) {
  Q_UNUSED(window)
  Q_UNUSED(no_title_bar)
  return false;
}

#endif  // BERNESE_WAYLAND_EFFECTS

}  // namespace internal
}  // namespace window_effects
}  // namespace bernese
