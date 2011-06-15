#include <QtCore/QCoreApplication>
#include <QDebug>
#include <QtDBus>

#include "contentdaemon.h"


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    ContentDaemon contentDaemon(&app);

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if(!sessionBus.isConnected()) {
        qDebug() << "Failed to connect to D-Bus session bus!";
        qDebug() << "Is the dbus-daemon running?";
        return 1;
    }

    ContentDaemonInterface contentDaemonInterface(sessionBus, &app);

    if (!sessionBus.registerService(CONTENT_DBUS_SERVICE)) {
        qDebug() << "registerService for " << CONTENT_DBUS_SERVICE << " failed!";
        qDebug() << "Is the daemon already running?";
        return 0;
    }

    if (!sessionBus.registerObject(CONTENT_DBUS_OBJECT, &app)) {
        qDebug() << "Failed to register D-Bus object " << CONTENT_DBUS_OBJECT;
        return 2;
    }

    return app.exec();
}
