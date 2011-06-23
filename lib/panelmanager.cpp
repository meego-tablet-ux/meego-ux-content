#include <QDBusPendingCall>
#include "panelmanager.h"
#include "dbustypes.h"
#include "aggregatedmodelproxy.h"
#include "servicemodeldbusproxy.h"
#include "dbusdefines.h"
#include "contentroles.h"

McaPanelManager::McaPanelManager(QObject *parent) :
    McaAbstractManager("newPanelManager", parent)
{
    qDebug() << "McaPanelManager::McaPanelManager";
    m_isEmpty = false;
    m_servicesEnabledByDefault = true;
    m_dbusServiceModel = new ServiceModelDbusProxy(CONTENT_DBUS_SERVICE);
    m_categoriesChanged = false;
}

McaPanelManager::~McaPanelManager()
{
    if(0 != m_dbusServiceModel) {
        delete m_dbusServiceModel;
    }
}

void McaPanelManager::initialize(const QString& managerData)
{
    qDebug() << " McaPanelManager::initialize";
    m_panelName = managerData;
    McaAbstractManager::initialize(managerData);
    feedRowsChanged();
}

void McaPanelManager::setCategories(const QStringList& categories)
{
    bool changed = m_categories != categories;
    m_categories = categories;
    if(isOffline()) {
        qDebug() << "McaPanelManager::setCategories offline set local categories " << categories;
        m_categoriesChanged = true;
        return;
    }    
    m_categoriesChanged = false;

    m_dbusManagerInterface->call("setCategories", QVariant(categories));
    if(changed) emit categoriesChanged(categories);
}

void McaPanelManager::feedRowsChanged()
{    
    bool empty = m_dbusModelProxy->rowCount() == 0;
    qDebug() << "McaPanelManager::feedRowsChanged empty=" << empty;

    if (empty != m_isEmpty) {
        m_isEmpty = empty;
        emit isEmptyChanged(empty);
    }
}

bool McaPanelManager::isEmpty()
{
qDebug() << "McaPanelManager::isEmpty";
    return m_isEmpty;
}

QStringList McaPanelManager::categories()
{
qDebug() << "McaPanelManager::categories";
    return m_categories;
}

bool McaPanelManager::servicesEnabledByDefault()
{
qDebug() << "McaPanelManager::servicesEnabledByDefault";
    return m_servicesEnabledByDefault;
}

void McaPanelManager::setServicesEnabledByDefault(bool enabled)
{
qDebug() << "McaPanelManager::setServicesEnabledByDefault";
    m_servicesEnabledByDefault = enabled;
    if(!isOffline()) {
        m_dbusManagerInterface->call("setServicesEnabledByDefault", QVariant(enabled));
    }
}

ServiceModelDbusProxy *McaPanelManager::serviceModel()
{
    qDebug() << "McaPanelManager::serviceModel";
    return m_dbusServiceModel;
}

bool McaPanelManager::isServiceEnabled(const QString& upid)
{
qDebug() << "McaPanelManager::isServiceEnabled";
    return m_dbusServiceModel->isServiceEnabled(upid);
}

void McaPanelManager::setServiceEnabled(const QString& upid, bool enabled)
{
qDebug() << " McaPanelManager::setServiceEnabled";
    if(!isOffline()) {
        m_dbusManagerInterface->call("setServiceEnabled", QVariant(upid), QVariant(enabled));
    } else {
        qDebug() << "TODO: McaPanelManager::setServiceEnabled";
    }
}

void McaPanelManager::serviceStateChanged(bool offline)
{
    qDebug() << "McaPanelManager::serviceStateChanged offline=" << offline;
    if(!offline) {
        connect(m_dbusModelProxy, SIGNAL(rowsInserted(QModelIndex,int,int)),
               this, SLOT(feedRowsChanged()));
        connect(m_dbusModelProxy, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(feedRowsChanged()));
        connect(m_dbusManagerInterface, SIGNAL(servicesConfiguredChanged(int)), this, SIGNAL(servicesConfiguredChanged(int)));

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

        qDebug() << "McaPanelManager::serviceStateChanged Local categories " << m_categories;
        if(m_categoriesChanged) {
            initialize(m_panelName);            
            setCategories(m_categories);
        }

        setServicesEnabledByDefault(m_servicesEnabledByDefault);
    } else {
        m_dbusServiceModel->setOffline(offline);
        m_categoriesChanged = true;
    }

    // TODO: send any local changes while offline to server
}

void McaPanelManager::clearAllHistory(const QDateTime& datetime)
{
    if(isOffline()) {
        qDebug() << "TODO: McaPanelManager::clearAllHistory while daemon is down";
    } else {
        m_dbusManagerInterface->asyncCall("clearAllHistory", QVariant(datetime));
    }
}

void McaPanelManager::clearHistory(const QString& upid, const QDateTime& datetime)
{
    if(isOffline()) {
        qDebug() << "TODO: McaPanelManager::clearHistory while daemon is down";
    } else {
        m_dbusManagerInterface->asyncCall("clearHistory", QVariant(upid), QVariant(datetime));
    }
}

int McaPanelManager::serviceModelRowCount()
{
    qDebug() << "McaPanelManager::serviceModelRowCount";
    return m_dbusServiceModel->rowCount();
}

QVariant McaPanelManager::serviceModelData(const QModelIndex &index, int role)
{
    qDebug() << "McaPanelManager::serviceModelData";
    return m_dbusServiceModel->data(index, role);
}

bool McaPanelManager::dataChangedCondition(const QModelIndex &index)
{
    qDebug() << "McaPanelManager::dataChangedCondition";
    QString upid = m_dbusServiceModel->data(index, McaContentRoles::SystemUpidRole).toString();
    return isServiceEnabled(upid);
}

QModelIndex McaPanelManager::serviceModelIndex(int row)
{
    qDebug() << "McaPanelManager::serviceModelIndex";
    return m_dbusServiceModel->index(row);
}
