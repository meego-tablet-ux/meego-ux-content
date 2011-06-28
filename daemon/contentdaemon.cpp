#include <QDebug>

#include "contentdaemon.h"
#include "searchmanagerdbus.h"
#include "dbustypes.h"

#define CHECK_MANAGERS_INTERVAL 30000

ContentDaemon::ContentDaemon(QCoreApplication *application)
    : QDBusAbstractAdaptor(application)
{
    registerDataTypes();
}

QString ContentDaemon::newSearchManager(qint64 processPid)
{
    qDebug() << "ContentDaemon::newSearchManager called";
    McaSearchManagerDBus *searchManager = new McaSearchManagerDBus(this);

    return addObjectToList(searchManager, processPid);
}

QString ContentDaemon::newPanelManager(qint64 processPid)
{
    qDebug() << "ContentDaemon::newPanelManager called";
    McaPanelManagerDBus *panelManager = new McaPanelManagerDBus(this);

    return addObjectToList(panelManager, processPid);
}

QString ContentDaemon::addObjectToList(McaAbstractManagerDBus *dbusObject, qint64 processPid)
{
    if(m_managerList.count() != 0) {
        checkManagers();
    }

    QString objectPath = dbusObject->dbusObjectId();
    m_managerList[objectPath] = dbusObject;
    m_managerPids[objectPath] = processPid;

    return objectPath;
}

bool ContentDaemon::release(const QString &objectPath)
{
    qDebug() << "ContentDaemon::release " << objectPath;
    if(m_managerList.contains(objectPath)) {
        McaAbstractManagerDBus *pManager = m_managerList.value(objectPath);
        m_managerList.remove(objectPath);
        m_managerPids.remove(objectPath);

        delete pManager;
        return true;
    }
    qDebug() << "ERROR: ContentDaemon::release called with invalid manager object path " << objectPath;

    return false;
}

void ContentDaemon::checkManagers()
{
    QStringList objectsToRelease;
    QString procPath;
    foreach(QString objectId, m_managerList.keys()) {
        procPath = QString("/proc/%1").arg(m_managerPids.value(objectId));
        if(!QDir(procPath).exists()) {
            qDebug() << "procPath does not exist " << procPath;
            objectsToRelease.append(objectId);
        }
    }
    if(objectsToRelease.count()) {
        qDebug() << "ContentDaemon::checkManagers releasing unused managers " << objectsToRelease;
    }
    while(objectsToRelease.count()) {
        release(objectsToRelease.first());
        objectsToRelease.pop_front();
    }
}
