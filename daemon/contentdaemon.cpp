#include <QDebug>

#include "contentdaemon.h"
#include "searchmanagerdbus.h"
#include "dbustypes.h"

ContentDaemon::ContentDaemon(QCoreApplication *application)
    : QDBusAbstractAdaptor(application)
{
    registerDataTypes();
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

    return false;
}
