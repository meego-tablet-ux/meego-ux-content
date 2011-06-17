#include <QDBusPendingCall>
#include "panelmanager.h"
#include "dbustypes.h"
#include "aggregatedmodelproxy.h"
#include "servicemodeldbusproxy.h"
#include "dbusdefines.h"

McaPanelManager::McaPanelManager(QObject *parent) :
    McaAbstractManager("newPanelManager", parent)
{
    m_isEmpty = false;
    m_servicesEnabledByDefault = true;
}

void McaPanelManager::initialize(const QString& managerData)
{
    m_panelName = managerData;
    McaAbstractManager::initialize(managerData);
    feedRowsChanged();
}

void McaPanelManager::setCategories(const QStringList& categories)
{
    m_categories = categories;
    m_dbusManagerInterface->call("setCategories", QVariant(categories));
    emit categoriesChanged(categories);
}

void McaPanelManager::feedRowsChanged()
{
    if(isOffline()) return;
    bool empty = m_dbusModelProxy->rowCount() == 0;

    if (empty != m_isEmpty) {
        m_isEmpty = empty;
        emit isEmptyChanged(empty);
    }
}

bool McaPanelManager::isEmpty()
{
    return m_isEmpty;
}

QStringList McaPanelManager::categories()
{
    return m_categories;
}

bool McaPanelManager::servicesEnabledByDefault()
{
    return m_servicesEnabledByDefault;
}

void McaPanelManager::setServicesEnabledByDefault(bool enabled)
{
    m_servicesEnabledByDefault = enabled;
    m_dbusManagerInterface->call("setServicesEnabledByDefault", QVariant(enabled));
}

QAbstractListModel *McaPanelManager::serviceModel()
{
    qDebug() << "McaPanelManager::serviceModel";
    return m_dbusServiceModel;
}

bool McaPanelManager::isServiceEnabled(const QString& upid)
{
    return m_dbusServiceModel->isServiceEnabled(upid);
}

void McaPanelManager::setServiceEnabled(const QString& upid, bool enabled)
{
   m_dbusManagerInterface->call("setServiceEnabled", QVariant(upid), QVariant(enabled));
}

void McaPanelManager::serviceStateChanged(bool offline)
{
    qDebug() << "McaPanelManager::serviceStateChanged " << offline;
    if(!offline) {
        connect(m_dbusModelProxy, SIGNAL(rowsInserted(QModelIndex,int,int)),
               this, SLOT(feedRowsChanged()));
        connect(m_dbusModelProxy, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(feedRowsChanged()));

        QDBusReply<QString> reply = m_dbusManagerInterface->call("serviceModelPath");
        qDebug() << "ServiceModel dbus path: " << reply.value() << reply.error().message();
        m_dbusServiceModel = new ServiceModelDbusProxy(CONTENT_DBUS_SERVICE, reply.value());
    } else {
        //TODO
    }

    // TODO: send any local changes while offline to server
}
