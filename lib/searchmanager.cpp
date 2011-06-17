#include <QDBusPendingCall>
#include <QDebug>
#include "searchmanager.h"

McaSearchManager::McaSearchManager(QObject *parent) :
    McaAbstractManager("newSearchManager", parent)
{
    m_searchText = "";
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
    if(isOffline()) {
        m_localSearchText = managerData;
    }
    m_searchText = managerData;
    McaAbstractManager::initialize(managerData);
}

void McaSearchManager::serviceStateChanged(bool offline)
{
    qDebug() << "McaSearchManager::serviceStateChanged " << offline;
    if(offline) {
        m_localSearchText = m_searchText;
    } else {        
        QString text = m_localSearchText;
        m_localSearchText = "";
        initialize(text);
    }
}
