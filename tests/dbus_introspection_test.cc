// Copyright 2026 CharOfString
// SPDX-License-Identifier: GPL-3.0-or-later
#include "dbus_introspection/dbus_introspection.h"

#include "QObject"
#include "QString"
#include "QtTest"

namespace bernese {

class DBusIntrospectionTest : public QObject {
  Q_OBJECT
 private slots:

  /** @brief Verify members, arguments and annotations are read. */
  void ParseMembers() {
    const dbus_introspection::Interfaces interfaces =
        dbus_introspection::ParseInterfaces(QStringLiteral(
            "<node><interface name=\"org.example.A\">"
            "<annotation name=\"org.example.Tag\" value=\"x\"/>"
            "<method name=\"Get\">"
            "<arg name=\"key\" type=\"s\"/>"
            "<arg name=\"value\" type=\"v\" direction=\"out\"/>"
            "<arg type=\"i\" direction=\"sideways\"/>"
            "</method>"
            "<method name=\"Get\"><arg type=\"i\"/></method>"
            "<signal name=\"Changed\"><arg type=\"s\"/>"
            "<arg type=\"i\" direction=\"in\"/></signal>"
            "<property name=\"Size\" type=\"u\" access=\"readwrite\"/>"
            "<property name=\"Bad\" type=\"u\" access=\"none\"/>"
            "</interface></node>"));
    QCOMPARE(interfaces.size(), 1);
    const auto& interface = *interfaces.value(QStringLiteral("org.example.A"));
    QCOMPARE(interface.annotations.value(QStringLiteral("org.example.Tag")),
        QStringLiteral("x"));
    QCOMPARE(interface.methods.count(QStringLiteral("Get")), 2);

    const QList<dbus_introspection::Method> methods =
        interface.methods.values(QStringLiteral("Get"));
    bool found_out = false;
    for (const dbus_introspection::Method& method : methods) {
      if (method.output_args.isEmpty()) {
        continue;
      }
      found_out = true;
      QCOMPARE(method.input_args.size(), 1);
      QCOMPARE(method.input_args[0].name, QStringLiteral("key"));
      QCOMPARE(method.output_args[0].type, QStringLiteral("v"));
    }
    QVERIFY(found_out);

    const dbus_introspection::Signal signal =
        interface.signal_map.value(QStringLiteral("Changed"));
    QCOMPARE(signal.output_args.size(), 1);
    QCOMPARE(signal.output_args[0].type, QStringLiteral("s"));

    QCOMPARE(interface.properties.size(), 1);
    QCOMPARE(interface.properties.first().access,
        dbus_introspection::Property::Access::kReadWrite);
  }

  /** @brief Verify the introspection text matches the QtDBus form. */
  void IntrospectionText() {
    const dbus_introspection::Interfaces interfaces =
        dbus_introspection::ParseInterfaces(QStringLiteral(
            "<interface name=\"org.example.B\">"
            "<annotation name=\"org.example.Note\" value=\"a&lt;b\"/>"
            "<method name=\"Ping\"/>"
            "<signal name=\"Done\"><arg name=\"ok\" type=\"b\"/></signal>"
            "<property name=\"Name\" type=\"s\" access=\"read\">"
            "<annotation name=\"org.example.Q\" value=\"1\"/></property>"
            "</interface>"));
    QCOMPARE(interfaces.value(QStringLiteral("org.example.B"))->introspection,
        QStringLiteral(
            "  <interface name=\"org.example.B\">\n"
            "    <annotation value=\"a&lt;b\" name=\"org.example.Note\"/>\n"
            "    <method name=\"Ping\"/>\n"
            "    <signal name=\"Done\">\n"
            "      <arg type=\"b\" name=\"ok\"/>\n"
            "    </signal>\n"
            "    <property access=\"read\" type=\"s\" name=\"Name\">\n"
            "      <annotation value=\"1\" name=\"org.example.Q\"/>\n"
            "    </property>\n"
            "  </interface>"));
  }

  /** @brief Verify bad interface and member names are skipped. */
  void SkipBadNames() {
    const dbus_introspection::Interfaces interfaces =
        dbus_introspection::ParseInterfaces(
            QStringLiteral("<node><interface name=\"single\"/>"
                           "<interface name=\"org.example.C\">"
                           "<property name=\"1st\" type=\"s\" access=\"read\"/>"
                           "<method name=\"Ok\"/></interface></node>"));
    QCOMPARE(interfaces.keys(), QStringList{QStringLiteral("org.example.C")});
    QCOMPARE(
        interfaces.first()->methods.keys(), QStringList{QStringLiteral("Ok")});
    QVERIFY(
        dbus_introspection::ParseInterfaces(QStringLiteral("<x")).isEmpty());
  }

  /** @brief Verify the D-Bus name rules. */
  void NameRules() {
    QVERIFY(dbus_introspection::IsValidMemberName(QStringLiteral("Get_2")));
    QVERIFY(!dbus_introspection::IsValidMemberName(QString()));
    QVERIFY(!dbus_introspection::IsValidMemberName(QStringLiteral("2x")));
    QVERIFY(!dbus_introspection::IsValidMemberName(QStringLiteral("a-b")));
    QVERIFY(!dbus_introspection::IsValidMemberName(QString(256, u'a')));
    QVERIFY(dbus_introspection::IsValidInterfaceName(QStringLiteral("a.b")));
    QVERIFY(!dbus_introspection::IsValidInterfaceName(QStringLiteral("ab")));
    QVERIFY(!dbus_introspection::IsValidInterfaceName(QStringLiteral("a..b")));
    QVERIFY(!dbus_introspection::IsValidInterfaceName(QStringLiteral("a.2b")));
  }
};

}  // namespace bernese

QTEST_GUILESS_MAIN(bernese::DBusIntrospectionTest)
#include "dbus_introspection_test.moc"
