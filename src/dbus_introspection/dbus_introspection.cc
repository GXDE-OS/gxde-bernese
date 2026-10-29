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

#include "dbus_introspection/dbus_introspection.h"

#include "QLatin1String"
#include "QSharedPointer"
#include "QString"
#include "QStringList"
#include "QXmlStreamAttributes"
#include "QXmlStreamReader"

namespace bernese {
namespace dbus_introspection {

namespace {

// Longest name that D-Bus accepts.
constexpr int kMaxNameLength = 255;

// Reads interfaces from XML. The flow of each step follows the QtDBus
// parser, so that bad input gives the same result in both.
class Parser {
 public:
  /**
   * @brief Start a parser on XML text.
   *
   * @param xml Introspection XML.
   */
  explicit Parser(const QString& xml) : xml_(xml) {}

  /**
   * @brief Read the whole document.
   *
   * Only start elements matter here. Interfaces may be at any node depth.
   *
   * @return Parsed interfaces.
   */
  Interfaces Run() {
    while (!xml_.atEnd()) {
      xml_.readNext();
      if (!xml_.isStartElement() || xml_.name() == QLatin1String("node")) {
        continue;
      }
      if (xml_.name() == QLatin1String("interface")) {
        ReadInterface();
      } else {
        xml_.skipCurrentElement();
      }
    }
    return interfaces_;
  }

 private:
  /**
   * @brief Read one interface element and add it to the result.
   *
   * The reader must be on the start of the interface element.
   */
  void ReadInterface() {
    const QString name = Attribute(xml_.attributes(), "name");
    if (!IsValidInterfaceName(name)) {
      return;
    }

    auto interface = QSharedPointer<Interface>::create();
    interface_ = interface.data();
    interface_->name = name;
    Write(
        QLatin1String("  <interface name=\"") + name + QLatin1String("\">\n"));

    while (xml_.readNextStartElement()) {
      if (xml_.name() == QLatin1String("method")) {
        Method method;
        if (ReadMethod(&method)) {
          interface_->methods.insert(method.name, method);
        }
      } else if (xml_.name() == QLatin1String("signal")) {
        Signal signal;
        if (ReadSignal(&signal)) {
          interface_->signal_map.insert(signal.name, signal);
        }
      } else if (xml_.name() == QLatin1String("property")) {
        Property property;
        if (ReadProperty(&property)) {
          interface_->properties.insert(property.name, property);
        }
      } else if (xml_.name() == QLatin1String("annotation")) {
        ReadAnnotation(&interface_->annotations, true);
        xml_.skipCurrentElement();
      } else {
        xml_.skipCurrentElement();
      }
    }

    Write(QLatin1String("  </interface>"));
    interfaces_.insert(name, interface);
    interface_ = nullptr;
  }

  /**
   * @brief Read one method element.
   *
   * Arguments with an unknown direction are left out.
   *
   * @param method Method to fill.
   * @return False if the method name is bad.
   */
  bool ReadMethod(Method* method) {
    method->name = Attribute(xml_.attributes(), "name");
    if (!IsValidMemberName(method->name)) {
      return false;
    }

    Write(QLatin1String("    <method name=\"") + method->name +
          QLatin1String("\""));
    ReadChildren(QLatin1String("method"), [&]() {
      if (xml_.name() == QLatin1String("annotation")) {
        ReadAnnotation(&method->annotations, false);
      } else if (xml_.name() == QLatin1String("arg")) {
        const QXmlStreamAttributes attributes = xml_.attributes();
        const bool has_direction =
            attributes.hasAttribute(QLatin1String("direction"));
        const QString direction = Attribute(attributes, "direction");
        if (!has_direction || direction == QLatin1String("in")) {
          method->input_args << ReadArgument(attributes);
        } else if (direction == QLatin1String("out")) {
          method->output_args << ReadArgument(attributes);
        }
      }
    });
    return true;
  }

  /**
   * @brief Read one signal element.
   *
   * Arguments with a direction other than "out" are left out.
   *
   * @param signal Signal to fill.
   * @return False if the signal name is bad.
   */
  bool ReadSignal(Signal* signal) {
    signal->name = Attribute(xml_.attributes(), "name");
    if (!IsValidMemberName(signal->name)) {
      return false;
    }

    Write(QLatin1String("    <signal name=\"") + signal->name +
          QLatin1String("\""));
    ReadChildren(QLatin1String("signal"), [&]() {
      if (xml_.name() == QLatin1String("annotation")) {
        ReadAnnotation(&signal->annotations, false);
      } else if (xml_.name() == QLatin1String("arg")) {
        const QXmlStreamAttributes attributes = xml_.attributes();
        if (!attributes.hasAttribute(QLatin1String("direction")) ||
            Attribute(attributes, "direction") == QLatin1String("out")) {
          signal->output_args << ReadArgument(attributes);
        }
      }
    });
    return true;
  }

  /**
   * @brief Read one property element.
   *
   * @param property Property to fill.
   * @return False if the name or the access value is bad.
   */
  bool ReadProperty(Property* property) {
    const QXmlStreamAttributes attributes = xml_.attributes();
    property->name = Attribute(attributes, "name");
    if (!IsValidMemberName(property->name)) {
      xml_.skipCurrentElement();
      return false;
    }

    property->type = Attribute(attributes, "type");
    const QString access = Attribute(attributes, "access");
    if (access == QLatin1String("read")) {
      property->access = Property::Access::kRead;
    } else if (access == QLatin1String("write")) {
      property->access = Property::Access::kWrite;
    } else if (access == QLatin1String("readwrite")) {
      property->access = Property::Access::kReadWrite;
    } else {
      // QtDBus does not skip the element here, so neither do we.
      return false;
    }

    Write(QLatin1String("    <property access=\"") + access +
          QLatin1String("\" type=\"") + property->type +
          QLatin1String("\" name=\"") + property->name + QLatin1String("\""));
    ReadChildren(QLatin1String("property"), [&]() {
      if (xml_.name() == QLatin1String("annotation")) {
        ReadAnnotation(&property->annotations, false);
      }
    });
    return xml_.isEndElement() && xml_.name() == QLatin1String("property");
  }

  /**
   * @brief Read the child elements of a member and close its XML text.
   *
   * The opening tag text must be written already, without its end. Every
   * child is skipped after read_child() looks at it.
   *
   * @param tag Name of the member element, like "method".
   * @param read_child Called once on the start of each child element.
   */
  template <typename Function>
  void ReadChildren(QLatin1String tag, Function read_child) {
    if (!xml_.readNextStartElement()) {
      Write(QLatin1String("/>\n"));
      return;
    }

    Write(QLatin1String(">\n"));
    do {
      read_child();
      xml_.skipCurrentElement();
    } while (xml_.readNextStartElement());
    Write(QLatin1String("    </") + tag + QLatin1String(">\n"));
  }

  /**
   * @brief Read one argument and add it to the introspection text.
   *
   * @param attributes Attributes of the arg element.
   * @return The argument.
   */
  Argument ReadArgument(const QXmlStreamAttributes& attributes) {
    Argument argument;
    argument.type = Attribute(attributes, "type");
    argument.name = Attribute(attributes, "name");

    QString text(QLatin1String("      <arg"));
    if (attributes.hasAttribute(QLatin1String("direction"))) {
      text += QLatin1String(" direction=\"") +
              Attribute(attributes, "direction") + QLatin1String("\"");
    }
    text += QLatin1String(" type=\"") + argument.type + QLatin1String("\"");
    if (!argument.name.isEmpty()) {
      text += QLatin1String(" name=\"") + argument.name + QLatin1String("\"");
    }
    Write(text + QLatin1String("/>\n"));
    return argument;
  }

  /**
   * @brief Read one annotation element.
   *
   * Annotations with a bad name are left out.
   *
   * @param annotations Map to add the annotation to.
   * @param of_interface True if the annotation belongs to the interface
   *     itself. Its text is then indented less.
   */
  void ReadAnnotation(Annotations* annotations, bool of_interface) {
    const QXmlStreamAttributes attributes = xml_.attributes();
    const QString name = Attribute(attributes, "name");
    if (!IsValidInterfaceName(name)) {
      return;
    }

    const QString value = Attribute(attributes, "value");
    annotations->insert(name, value);
    Write(QLatin1String(of_interface ? "" : "  ") +
          QLatin1String("    <annotation value=\"") + value.toHtmlEscaped() +
          QLatin1String("\" name=\"") + name + QLatin1String("\"/>\n"));
  }

  /**
   * @brief Get the value of an attribute as a string.
   *
   * @param attributes Attributes of the current element.
   * @param name Attribute name.
   * @return The value, or an empty string if the attribute is missing.
   */
  static QString Attribute(
      const QXmlStreamAttributes& attributes, const char* name) {
    return attributes.value(QLatin1String(name)).toString();
  }

  /**
   * @brief Add text to the introspection of the current interface.
   *
   * @param text Text to add.
   */
  void Write(const QString& text) {
    interface_->introspection += text;
  }

  QXmlStreamReader xml_;
  Interfaces interfaces_;
  Interface* interface_ = nullptr;  // Interface being read, or null.
};

}  // namespace

Interfaces ParseInterfaces(const QString& xml) {
  return Parser(xml).Run();
}

bool IsValidMemberName(const QString& name) {
  if (name.isEmpty() || name.size() > kMaxNameLength || name[0].isDigit()) {
    return false;
  }
  for (const QChar c : name) {
    const bool letter = (c >= QLatin1Char('a') && c <= QLatin1Char('z')) ||
                        (c >= QLatin1Char('A') && c <= QLatin1Char('Z'));
    const bool digit = c >= QLatin1Char('0') && c <= QLatin1Char('9');
    if (!letter && !digit && c != QLatin1Char('_')) {
      return false;
    }
  }
  return true;
}

bool IsValidInterfaceName(const QString& name) {
  if (name.isEmpty() || name.size() > kMaxNameLength) {
    return false;
  }
  const QStringList parts = name.split(QLatin1Char('.'));
  if (parts.size() < 2) {
    return false;
  }
  for (const QString& part : parts) {
    if (!IsValidMemberName(part)) {
      return false;
    }
  }
  return true;
}

}  // namespace dbus_introspection
}  // namespace bernese
