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
    m_searchText = searchText;
    if(isOffline()) {
        m_localSearchText = searchText;
    } else {
        m_dbusManagerInterface->asyncCall("setSearchText", QVariant(searchText));
    }
}

void McaSearchManager::initialize(const QString& managerData)
{
    m_searchText = managerData;
    McaAbstractManager::initialize(managerData);
}

void McaSearchManager::serviceStateChanged(bool offline)
{
    if(offline) {
        m_localSearchText = m_searchText;
    } else {
        if(m_localSearchText.isEmpty()) {
            QString text = m_localSearchText;
            m_localSearchText = "";
            setSearchText(text);
        }
    }
}
