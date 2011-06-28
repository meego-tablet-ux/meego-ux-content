#ifndef CONTENTDAEMON_H
#define CONTENTDAEMON_H

#include <QtDBus>
#include <QtCore/QCoreApplication>
#include <QMap>
#include <QModelIndex>

#include "contentdaemoninterface.h"
#include "searchmanagerdbus.h"
#include "panelmanagerdbus.h"

class McaSearchManagerAdaptor;

class ContentDaemon : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.meego.content"); // build complains when using CONTENT_DBUS_SERVICE macro
public:
    ContentDaemon(QCoreApplication *application);

public slots:
    QString newPanelManager(qint64 processPid);
    QString newSearchManager(qint64 processPid);
    bool release(const QString &objectPath);

private slots:
    void checkManagers();
private:
    QString addObjectToList(McaAbstractManagerDBus *dbusObject, qint64 processPid);

private:
    QMap<QString, McaAbstractManagerDBus*> m_managerList;
    QMap<QString,qint64> m_managerPids;
};

#endif // CONTENTDAEMON_H
