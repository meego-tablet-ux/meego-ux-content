#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include "servicemodeldbusproxy.h"
#include "servicemodel.h"
#include "serviceadapter.h"

ServiceModelDbusProxy::ServiceModelDbusProxy(const QString &service)
    : ModelDBusInterface(service)
{
    QHash<int, QByteArray> roles = roleNames();
    roles.insert(ServiceModelDbusProxy::SystemEnabledRole, "enabled");


    roles.insert(McaContentRoles::SystemServiceIdRole, "serviceid");
    roles.insert(McaContentRoles::SystemServiceNameRole, "servicename");
    roles.insert(McaContentRoles::SystemServiceIconRole, "serviceicon");
    roles.insert(McaContentRoles::SystemServiceCategoryRole, "servicecategory");

    roles.insert(McaServiceModel::RequiredNameRole,      "name");
    roles.insert(McaServiceModel::RequiredCategoryRole,  "category");
    roles.insert(McaServiceModel::CommonActionsRole,     "actions");
    roles.insert(McaServiceModel::CommonDisplayNameRole, "displayname");
    roles.insert(McaServiceModel::CommonIconUrlRole,     "icon");
    roles.insert(McaServiceModel::CommonConfigErrorRole, "configerror");
    roles.insert(McaServiceAdapter::SystemUpidRole,      "upid");

    setRoleNames(roles);
}

int ServiceModelDbusProxy::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_feedItems.count();
}

QVariant ServiceModelDbusProxy::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) return QVariant();

    int row = index.row();
    if(row >= m_feedItems.count()) {
        qDebug() << "ServiceModelDbusProxy::data: Invalid row requested" << row;
        return QVariant();
    }

    McaServiceItemStruct *feedItem = m_feedItems.at(row);
    QVariant result;

    switch(role) {
    case McaServiceModel::RequiredNameRole:
        result = QVariant::fromValue<QString>(feedItem->name);
        break;
    case McaServiceModel::RequiredCategoryRole:
        result = QVariant::fromValue<QString>(feedItem->category);
        break;
    case McaServiceModel::CommonDisplayNameRole:
        result = QVariant::fromValue<QString>(feedItem->displayName);
        break;
    case McaServiceModel::CommonIconUrlRole:
        result = QVariant::fromValue<QString>(feedItem->iconUrl);
        break;
    case McaServiceModel::CommonConfigErrorRole:
        result = QVariant::fromValue<bool>(feedItem->configError);
        break;
    case McaServiceAdapter::SystemUpidRole:
        result = QVariant::fromValue<QString>(feedItem->upid);
        break;
    case ServiceModelDbusProxy::SystemEnabledRole:
        result = QVariant::fromValue<bool>(feedItem->enabled);
        break;
    //TODO: remove this, it's here just for debug purposes
    case Qt::DisplayRole:
        result = QVariant::fromValue<QString>(feedItem->displayName + " - " + (feedItem->enabled ? "enabled" : "disabled"));
        break;
    default:
        if(role > Qt::UserRole) {
            qDebug() << "ServiceModelDbusProxy::data: Unhandled data role requested " << role << " for row " << row;
        }
        result = QVariant();
        break;
    }

    return result;
}

void ServiceModelDbusProxy::onItemsAdded(ArrayOfMcaServiceItemStruct items)
{
    qDebug() << "ServiceModelDbusProxy::onItemsAdded " << items.count();
    beginInsertRows(QModelIndex(), m_feedItems.count(), m_feedItems.count() + items.count() - 1);
    for(int row = 0; row < items.count(); row++) {
        McaServiceItemStruct *newItem = new McaServiceItemStruct(items.at(row));
        m_feedItems.append(newItem);
    }
    endInsertRows();
}

void ServiceModelDbusProxy::onItemsChanged(ArrayOfMcaServiceItemStruct items)
{
    qDebug() << "ServiceModelDbusProxy::onItemsChanged " << items.count();
    bool matched;

    QList<McaServiceItemStruct *> notOld;

    int row;
    foreach (McaServiceItemStruct feedItem, items) {
        matched = false;
        for (row = 0; row < m_feedItems.count(); ++row) {
            if (feedItem.upid == m_feedItems.at(row)->upid) {
                McaServiceItemStruct *oldItem = m_feedItems.at(row);
                *oldItem = feedItem;
                QModelIndex qmi = createIndex(row, 0, 0);
                matched = true;
                emit dataChanged(qmi, qmi);
                break;
            }
        }
        if(!matched) {
            McaServiceItemStruct *newItem = new McaServiceItemStruct(feedItem);
            notOld.append(newItem);
        }
    }

    // This will mostly be used by the synchronization
    // The performance be relatively poor for due to the earlier matcher.
    beginInsertRows(QModelIndex(), m_feedItems.count(), m_feedItems.count() + notOld.count() - 1);
    for(int row = 0; row < notOld.count(); row++) {
        m_feedItems.append(notOld.at(row));
    }
    endInsertRows();
}

void ServiceModelDbusProxy::onItemsRemoved(QStringList items)
{
    qDebug() << "ServiceModelDbusProxy::onItemsRemoved " << items.count();
    int i;
    foreach (QString itemId, items) {
        for (i = 0; i < m_feedItems.count(); i++) {
            struct McaServiceItemStruct *item = m_feedItems.at(i);
            if (itemId == item->upid) {
                beginRemoveRows(QModelIndex(), i, i);
                m_feedItems.removeAt(i);
                endRemoveRows();
                delete item;
                break;
            }
        }
    }
}

bool ServiceModelDbusProxy::isServiceEnabled(const QString& upid)
{
    for (int i = 0; i < m_feedItems.count(); i++) {
        struct McaServiceItemStruct *item = m_feedItems.at(i);
        qDebug() << "+++++++++++++++++++" << item->upid << item->enabled;
        if(upid == item->upid) return item->enabled;
    }
    return false;
}

void ServiceModelDbusProxy::triggerSyncClients()
{
    emit syncClients();
}

void ServiceModelDbusProxy::doOfflineChanged()
{
    if(!isOffline()) {
        beginRemoveRows(QModelIndex(), 0, m_feedItems.count() - 1);
        while(m_feedItems.count()) {
            struct McaServiceItemStruct *item = m_feedItems.at(0);
            m_feedItems.removeFirst();
            delete item;
        }
        endRemoveRows();

        connect(m_dbusModel, SIGNAL(ItemsAdded(ArrayOfMcaServiceItemStruct)),
                this, SLOT(onItemsAdded(ArrayOfMcaServiceItemStruct)));
        connect(m_dbusModel, SIGNAL(ItemsChanged(ArrayOfMcaServiceItemStruct)),
                this, SLOT(onItemsChanged(ArrayOfMcaServiceItemStruct)));
        connect(m_dbusModel, SIGNAL(ItemsRemoved(QStringList)),
                this, SLOT(onItemsRemoved(QStringList)));
        connect(this, SIGNAL(syncClients()),
                m_dbusModel, SLOT(syncClients()));
    }
}
