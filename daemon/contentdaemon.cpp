#include <QDebug>

#include "contentdaemon.h"
#include "searchmanagerdbus.h"
#include "dbustypes.h"

#define CHECK_MANAGERS_INTERVAL 30000

ContentDaemon::ContentDaemon(QCoreApplication *application)
    : QDBusAbstractAdaptor(application)
{
    registerDataTypes();
    m_checkManagersTimer.setInterval(CHECK_MANAGERS_INTERVAL);
    connect(&m_checkManagersTimer, SIGNAL(timeout()), this, SLOT(checkManagers()));
}

QString ContentDaemon::newSearchManager()
{
    qDebug() << "ContentDaemon::newSearchManager called";
    McaSearchManagerDBus *searchManager = new McaSearchManagerDBus(this);

    return addObjectToList(searchManager);
}

QString ContentDaemon::newPanelManager()
{
    qDebug() << "ContentDaemon::newPanelManager called";
    McaPanelManagerDBus *panelManager = new McaPanelManagerDBus(this);

    return addObjectToList(panelManager);
}

QString ContentDaemon::addObjectToList(McaAbstractManagerDBus *dbusObject)
{
    QString objectPath = dbusObject->dbusObjectId();
    m_managerList[objectPath] = dbusObject;

    if(!m_checkManagersTimer.isActive()) {
        m_checkManagersTimer.start();
    }

    return objectPath;
}

bool ContentDaemon::release(const QString &objectPath)
{
    qDebug() << "ContentDaemon::release " << objectPath;
    if(m_managerList.contains(objectPath)) {
        McaAbstractManagerDBus *pManager = m_managerList.value(objectPath);
        m_managerList.remove(objectPath);
        delete pManager;
        return true;
    }
    qDebug() << "ERROR: ContentDaemon::release called with invalid manager object path " << objectPath;

    if(m_managerList.count() == 0) {
        m_checkManagersTimer.stop();
    }

    return false;
}

void ContentDaemon::checkManagers()
{
    QStringList objectsToRelease;
    qint64 time = QDateTime::currentMSecsSinceEpoch();
    foreach(QString objectId, m_managerList.keys()) {
        McaAbstractManagerDBus *manager = m_managerList.value(objectId);
        if(time - manager->lastTime() > CHECK_MANAGERS_INTERVAL) {
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
