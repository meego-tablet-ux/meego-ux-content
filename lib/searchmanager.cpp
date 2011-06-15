#include <QDBusPendingCall>
#include "searchmanagerproxy.h"

McaSearchManagerProxy::McaSearchManagerProxy(QObject *parent) :
    McaAbstractManagerProxy("newSearchManager", parent)
{
    m_searchText = "";
}

void McaSearchManagerProxy::setSearchText(const QString &searchText)
{
    m_dbusManagerInterface->asyncCall("setSearchText", QVariant(searchText));
}

void McaSearchManagerProxy::initialize(const QString& managerData)
{
    m_searchText = managerData;
    McaAbstractManagerProxy::initialize(managerData);
}
