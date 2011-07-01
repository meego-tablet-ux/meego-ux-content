#include <QtCore/QCoreApplication>
#include <QDebug>
#include <QtDBus>

#include "contentdaemon.h"
#include "dbusdefines.h"
#include "lockfile.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    qint64 pid = app.applicationPid();

    McaLockFile lockfile("/var/run/meego-ux-content.pid", pid);
    if(!lockfile.canContinue()) {
        qDebug() << "Daemon already running as " << pid;
        return 0;
    }

    ContentDaemon contentDaemon(&app);

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if(!sessionBus.isConnected()) {
        qDebug() << "Failed to connect to D-Bus session bus!";
        qDebug() << "Is the dbus-daemon running?";
        return 1;
    }

    ContentDaemonInterface contentDaemonInterface(sessionBus, &app);

    if (sessionBus.interface()->isServiceRegistered(CONTENT_DBUS_SERVICE)) {
        qDebug() << "registerService for " << CONTENT_DBUS_SERVICE << " failed!";
        qDebug() << CONTENT_DBUS_SERVICE << " service already registered. Is the daemon already running?";
        return 0;
    }

    if(!sessionBus.registerService(CONTENT_DBUS_SERVICE)) {
        qDebug() << "registerService for " << CONTENT_DBUS_SERVICE << " failed!";
        return 2;
    }

    if (!sessionBus.registerObject(CONTENT_DBUS_OBJECT, &app)) {
        qDebug() << "Failed to register D-Bus object " << CONTENT_DBUS_OBJECT;
        return 3;
    }

    return app.exec();
}
