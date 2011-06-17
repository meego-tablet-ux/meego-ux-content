#include <QDBusPendingCall>
#include "searchmanager.h"

McaSearchManager::McaSearchManager(QObject *parent) :
    McaAbstractManager("newSearchManager", parent)
{
    m_searchText = "";
    connect(this, SIGNAL(offlineChanged(bool)), this, SLOT(serviceStateChanged(bool)));
}

void McaSearchManager::setSearchText(const QString &searchText)
{
    m_dbusManagerInterface->asyncCall("setSearchText", QVariant(searchText));
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
