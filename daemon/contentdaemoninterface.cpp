#include "contentdaemoninterface.h"

ContentDaemonInterface::ContentDaemonInterface(const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(CONTENT_DBUS_SERVICE, CONTENT_DBUS_OBJECT, CONTENT_DBUS_SERVICE, connection, parent)
{
}
