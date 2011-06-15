#include <QDBusPendingCall>
#include "searchmanagerproxy.h"

McaSearchManager::McaSearchManager(QObject *parent) :
    McaAbstractManager("newSearchManager", parent)
{
    m_searchText = "";
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
