#include <QDBusPendingCall>
#include <QDebug>
#include "searchmanager.h"

McaSearchManager::McaSearchManager(QObject *parent) :
    McaAbstractManager("newSearchManager", parent)
{
    m_searchText = "";
    connect(this, SIGNAL(offlineChanged(bool)), this, SLOT(serviceStateChanged(bool)));
}

void McaSearchManager::setSearchText(const QString &searchText)
{
    if(m_dbusManagerInterface) {
        m_dbusManagerInterface->asyncCall("setSearchText", QVariant(searchText));
    } else {
        qDebug() << "Offline search triggered?";
    }
}

void McaSearchManager::initialize(const QString& managerData)
{
    m_searchText = managerData;
    McaAbstractManager::initialize(managerData);
}

void McaSearchManager::serviceStateChanged(bool offline)
{
    // TODO: send the current search text to the service
}
