#ifndef CONTENTDAEMONINTERFACE_H
#define CONTENTDAEMONINTERFACE_H

#include <QDBusAbstractInterface>

#define CONTENT_DBUS_SERVICE "com.meego.content"
#define CONTENT_DBUS_OBJECT "/meegouxcontent"

class ContentDaemonInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:
    ContentDaemonInterface(const QDBusConnection &connection, QObject *parent = 0);

signals:

public slots:

};

#endif // CONTENTDAEMONINTERFACE_H
