#include <QDBusPendingCall>
#include "panelmanager.h"
#include "dbustypes.h"
#include "aggregatedmodelproxy.h"
#include "servicemodeldbusproxy.h"

McaPanelManager::McaPanelManager(QObject *parent) :
    McaAbstractManager("newPanelManager", parent)
{
    m_isEmpty = false;
    m_servicesEnabledByDefault = true;

    connect(m_dbusModelProxy, SIGNAL(rowsInserted(QModelIndex,int,int)),
           this, SLOT(feedRowsChanged()));
    connect(m_dbusModelProxy, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(feedRowsChanged()));

    QDBusReply<QString> reply = m_dbusManagerInterface->call("serviceModelPath");
    qDebug() << "ServiceModel dbus path: " << reply.value() << reply.error().message();
    m_dbusServiceModel = new ServiceModelDbusProxy("com.meego.content", reply.value());
}

void McaPanelManager::initialize(const QString& managerData)
{
    m_panelName = managerData;
    feedRowsChanged();
    McaAbstractManager::initialize(managerData);
}

void McaPanelManager::setCategories(const QStringList& categories)
{
    //TODO: store m_categories locally in local McaServiceProxy and return them from there
    m_categories = categories;
    m_dbusManagerInterface->call("setCategories", QVariant(categories));
    emit categoriesChanged(categories);
}

void McaPanelManager::feedRowsChanged()
{
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
    //TODO: store m_categories locally in local McaServiceProxy and return them from there
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

QSortFilterProxyModel *McaPanelManager::serviceModel()
{
    return m_dbusServiceModel;
}
