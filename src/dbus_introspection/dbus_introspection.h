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

#ifndef DBUS_INTROSPECTION_DBUS_INTROSPECTION_H_
#define DBUS_INTROSPECTION_DBUS_INTROSPECTION_H_

#include "QList"
#include "QMap"
#include "QMultiMap"
#include "QSharedPointer"
#include "QString"

#include "bernese/export.h"

namespace bernese {
namespace dbus_introspection {

// Annotation values by annotation name. A later annotation with the same
// name replaces the earlier one.
using Annotations = QMap<QString, QString>;

// One argument of a method or a signal.
struct Argument {
  QString type;  // D-Bus type signature, like "a{sv}".
  QString name;  // May be empty.
};

using Arguments = QList<Argument>;

// One method of an interface.
struct Method {
  QString name;
  Arguments input_args;
  Arguments output_args;
  Annotations annotations;
};

// One signal of an interface. Signals only have output arguments.
struct Signal {
  QString name;
  Arguments output_args;
  Annotations annotations;
};

// One property of an interface.
struct Property {
  enum class Access { kRead, kWrite, kReadWrite };

  QString name;
  QString type;  // D-Bus type signature.
  Access access = Access::kRead;
  Annotations annotations;
};

// One interface. The maps are sorted by member name. Overloaded methods and
// signals share one name, so they use multi maps.
struct Interface {
  QString name;
  // The interface written back as XML, in the same text form that QtDBus
  // makes. Generated adaptors store it in their D-Bus Introspection
  // class info.
  QString introspection;
  Annotations annotations;
  QMultiMap<QString, Method> methods;
  // Not named "signals" because that word is a Qt keyword macro.
  QMultiMap<QString, Signal> signal_map;
  QMap<QString, Property> properties;
};

// Interfaces by interface name.
using Interfaces = QMap<QString, QSharedPointer<const Interface>>;

/**
 * @brief Read all interfaces from D-Bus introspection XML.
 *
 * It replaces QDBusIntrospection::parseInterfaces() and gives the same
 * members, in the same order, and the same introspection text. Bad parts
 * are skipped silently, like QtDBus does when its debug log is off.
 *
 * @param xml Introspection XML, with or without a root node element.
 * @return Parsed interfaces; empty if none are valid.
 */
BERNESE_EXPORT Interfaces ParseInterfaces(const QString& xml);

/**
 * @brief Check a D-Bus member name, like "GetAll".
 *
 * It replaces QDBusUtil::isValidMemberName().
 *
 * @param name Name to check.
 * @return True if the name has 1 to 255 ASCII letters, digits or
 *     underscores and does not start with a digit.
 */
BERNESE_EXPORT bool IsValidMemberName(const QString& name);

/**
 * @brief Check a D-Bus interface name, like "org.freedesktop.DBus".
 *
 * It replaces QDBusUtil::isValidInterfaceName().
 *
 * @param name Name to check.
 * @return True if the name is at most 255 characters and has two or more
 *     dot separated parts that are each a valid member name.
 */
BERNESE_EXPORT bool IsValidInterfaceName(const QString& name);

}  // namespace dbus_introspection
}  // namespace bernese

#endif  // DBUS_INTROSPECTION_DBUS_INTROSPECTION_H_
