#include <QDebug>

#include "contentdaemon.h"
#include "searchmanager.h"
#include "dbustypes.h"

ContentDaemon::ContentDaemon(QCoreApplication *application)
    : QDBusAbstractAdaptor(application)
{
    registerDataTypes();
}

QString ContentDaemon::newSearchManager()
{
    qDebug() << "ContentDaemon::newSearchManager called";    
    McaSearchManager *searchManager = new McaSearchManager(this);
    QString objectPath = searchManager->dbusObjectId();
    m_searchManagers[objectPath] = searchManager;
    qDebug() << "registered new SearchManager object at path " << objectPath;

    return objectPath;
}

//QString ContentDaemon::newPanelManager()
//{
//    qDebug() << "ContentDaemon::newPanelManager called";
//    QString objectPath = QString(CONTENT_DBUS_OBJECT) + "/PanelManager_" + uniqueId();
//    McaPanelManager *panelManager = new McaPanelManager(this);

//    m_panelManagers[objectPath] = panelManager;
//    QDBusConnection::sessionBus().registerObject(objectPath, panelManager, QDBusConnection::ExportAllContents);

//    qDebug() << "registered new PanelManager object at path " << objectPath;

//    return objectPath;
//}

bool ContentDaemon::release(const QString &objectPath)
{
    if(m_searchManagers.contains(objectPath)) {
        McaSearchManager *searchManager = m_searchManagers[objectPath];
        m_searchManagers.remove(objectPath);
        delete searchManager;
        return true;
    }

    if(m_panelManagers.contains(objectPath)) {
        McaPanelManager *panelManager = m_panelManagers[objectPath];
        m_panelManagers.remove(objectPath);
        QDBusConnection::sessionBus().unregisterObject(objectPath);
        delete panelManager;
        return true;
    }

    return false;
}
