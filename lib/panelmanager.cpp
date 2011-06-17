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
    m_dbusServiceModel = new ServiceModelDbusProxy(CONTENT_DBUS_SERVICE);
}

McaPanelManager::~McaPanelManager()
{
    if(0 != m_dbusServiceModel) {
        delete m_dbusServiceModel;
        m_dbusServiceModel = 0;
    }
}

void McaPanelManager::initialize(const QString& managerData)
{
    m_panelName = managerData;
    feedRowsChanged();
    McaAbstractManager::initialize(managerData);
}

void McaPanelManager::setCategories(const QStringList& categories)
{
    if(isOffline()) {
        m_localCategories = categories;
        return;
    }
    bool changed = m_categories != categories;

    m_categories = categories;
    m_dbusManagerInterface->call("setCategories", QVariant(categories));
    if(changed) emit categoriesChanged(categories);
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
    qDebug() << "McaPanelManager::serviceStateChanged offline=" << offline;
    if(!offline) {
        connect(m_dbusModelProxy, SIGNAL(rowsInserted(QModelIndex,int,int)),
               this, SLOT(feedRowsChanged()));
        connect(m_dbusModelProxy, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(feedRowsChanged()));
        if(!m_dbusManagerInterface) {
            return; // Error state, handle me
        }
        if(!m_dbusServiceModel) {
            return; // Another error state
        }
        QDBusReply<QString> reply = m_dbusManagerInterface->call("serviceModelPath");
        qDebug() << "ServiceModel dbus path: " << reply.value() << reply.error().message();
        m_dbusServiceModel->setObjectPath(reply.value());
        m_dbusServiceModel->setOffline(offline);

        if(m_localCategories.count()) {
            initialize(m_panelName);
            QStringList categories = m_localCategories;
            m_localCategories.clear();
            setCategories(categories);
        }
    } else {
        m_dbusServiceModel->setOffline(offline);
        m_localCategories = m_categories;
    }

    // TODO: send any local changes while offline to server
}
