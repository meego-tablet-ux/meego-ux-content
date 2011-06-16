#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include "servicemodeldbusproxy.h"
#include "servicemodel.h"
#include "serviceadapter.h"

ServiceModelDbusProxy::ServiceModelDbusProxy(const QString &service, const QString &objectPath)
{
    QHash<int, QByteArray> roles = roleNames();
    roles.insert(ServiceModelDbusProxy::SystemEnabledRole, "enabled");

    roles.insert(McaServiceModel::RequiredNameRole,      "name");
    roles.insert(McaServiceModel::RequiredCategoryRole,  "category");
    roles.insert(McaServiceModel::CommonActionsRole,     "actions");
    roles.insert(McaServiceModel::CommonDisplayNameRole, "displayname");
    roles.insert(McaServiceModel::CommonIconUrlRole,     "icon");
    roles.insert(McaServiceModel::CommonConfigErrorRole, "configerror");
    roles.insert(McaServiceAdapter::SystemUpidRole,      "upid");

    setRoleNames(roles);

    m_dbusModel  = new QDBusInterface(service, objectPath);

    connect(m_dbusModel, SIGNAL(ItemsAdded(ArrayOfMcaServiceItemStruct)),
            this, SLOT(onItemsAdded(ArrayOfMcaServiceItemStruct)));
    connect(m_dbusModel, SIGNAL(ItemsChanged(ArrayOfMcaServiceItemStruct)),
            this, SLOT(onItemsChanged(ArrayOfMcaServiceItemStruct)));
    connect(m_dbusModel, SIGNAL(ItemsRemoved(QStringList)),
            this, SLOT(onItemsRemoved(QStringList)));
}

int ServiceModelDbusProxy::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_feedItems.count();
}

QVariant ServiceModelDbusProxy::data(const QModelIndex& index, int role) const
{
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
    int row;
    foreach (McaServiceItemStruct feedItem, items) {
        for (row = 0; row < m_feedItems.count(); ++row) {
            if (feedItem.upid == m_feedItems.at(row)->upid) {
                McaServiceItemStruct *oldItem = m_feedItems.at(row);
                *oldItem = feedItem;
                QModelIndex qmi = createIndex(row, 0, 0);
                emit dataChanged(qmi, qmi);
                break;
            }
        }
    }
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
        if(upid == item->upid) return item->enabled;
    }
    return false;
}