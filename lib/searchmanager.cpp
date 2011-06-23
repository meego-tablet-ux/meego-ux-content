#include <QDBusPendingCall>
#include <QDebug>
#include "searchmanager.h"
#include "servicemodeldbusproxy.h"

McaSearchManager::McaSearchManager(QObject *parent) :
    McaAbstractManager("newSearchManager", parent)
{
    m_searchText = "";
    m_dbusServiceModel = new ServiceModelDbusProxy(CONTENT_DBUS_SERVICE);
}

McaSearchManager::~McaSearchManager()
{
    if(0 != m_dbusServiceModel) {
        delete m_dbusServiceModel;
    }
}

void McaSearchManager::setSearchText(const QString &searchText)
{
    bool changed = m_searchText != searchText;
    m_searchText = searchText;
    if(isOffline()) {
        m_localSearchText = searchText;
    } else {
        m_dbusManagerInterface->asyncCall("setSearchText", QVariant(searchText));
    }
    if(changed) {
        emit searchTextChanged(searchText);
    }
}

ServiceModelDbusProxy *McaSearchManager::serviceModel()
{
    qDebug() << "McaPanelManager::serviceModel";
    return m_dbusServiceModel;
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
        m_dbusServiceModel->setOffline(offline);
    } else {
        if(!m_dbusManagerInterface) {
            return; // Error state, handle me
        }

        m_dbusServiceModel->setObjectPath(CONTENT_DBUS_RAWSERVICE_OBJECT);
        m_dbusServiceModel->setOffline(offline);

        m_dbusServiceModel->triggerSyncClients();

        QString text = m_localSearchText;
        m_localSearchText = "";
        initialize(text);
    }
}

QString McaSearchManager::searchText()
{
    return isOffline() ? m_localSearchText : m_searchText;
}

int McaSearchManager::serviceModelRowCount()
{
    qDebug() << "McaSearchManager::serviceModelRowCount";
    return m_dbusServiceModel->rowCount();
}

QVariant McaSearchManager::serviceModelData(const QModelIndex &index, int role)
{
    qDebug() << "McaSearchManager::serviceModelData";
    return m_dbusServiceModel->data(index, role);
}

bool McaSearchManager::dataChangedCondition(const QModelIndex &index)
{
    qDebug() << "McaSearchManager::dataChangedCondition";
    return true;
}

QModelIndex McaSearchManager::serviceModelIndex(int row)
{
    qDebug() << "McaSearchManager::serviceModelIndex";
    return m_dbusServiceModel->index(row);
}
