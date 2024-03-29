#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include "aggregatedmodelproxy.h"
#include "feedmodel.h"
#include "contentroles.h"
#include "actionsproxy.h"

McaAggregatedModelProxy::McaAggregatedModelProxy(const QString &service)
    : ModelDBusInterface(service)
{
    m_frozen = false;

    QHash<int, QByteArray> roles = roleNames();
    roles.insert(McaFeedModel::RequiredTypeRole,      "type");
    roles.insert(McaFeedModel::RequiredUniqueIdRole,  "id");
    roles.insert(McaFeedModel::RequiredTimestampRole, "timestamp");
    roles.insert(McaFeedModel::CommonActionsRole,     "actions");
    roles.insert(McaFeedModel::CommonUuidRole,        "uuid");
    roles.insert(McaFeedModel::GenericTitleRole,      "title");
    roles.insert(McaFeedModel::GenericContentRole,    "content");
    roles.insert(McaFeedModel::GenericRelevanceRole,  "relevance");
    roles.insert(McaFeedModel::GenericAvatarUrlRole,  "avatar");
    roles.insert(McaFeedModel::GenericPictureUrlRole, "picture");
    roles.insert(McaFeedModel::GenericAcceptTextRole, "accept");
    roles.insert(McaFeedModel::GenericRejectTextRole, "reject");
    roles.insert(McaContentRoles::SystemServiceNameRole, "servicename");
    roles.insert(McaContentRoles::SystemServiceIconRole, "serviceicon");
    setRoleNames(roles);
}

int McaAggregatedModelProxy::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_feedItems.count();
}

QVariant McaAggregatedModelProxy::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if(row >= m_feedItems.count()) {
        qDebug() << "McaAggregatedModelProxy::data: Invalid row requested" << row;
        return QVariant();
    }

    McaFeedItemStruct *feedItem = m_feedItems.at(row);
    QVariant result;

    switch(role) {
    case McaFeedModel::RequiredTypeRole:
        result = QVariant::fromValue<QString>(feedItem->type);
        break;
    case McaFeedModel::RequiredUniqueIdRole:
        result = QVariant::fromValue<QString>(feedItem->uniqueId);
        break;
    case McaFeedModel::RequiredTimestampRole:
        result = QVariant::fromValue<QDateTime>(feedItem->timestamp);
        break;
    case McaFeedModel::CommonUuidRole:
        result = QVariant::fromValue<QString>(feedItem->uuid);
        break;
    case McaFeedModel::GenericTitleRole:
        result = QVariant::fromValue<QString>(feedItem->title);
        break;
    case McaFeedModel::GenericContentRole:
        result = QVariant::fromValue<QString>(feedItem->content);
        break;
    case McaContentRoles::SystemServiceNameRole:
        result = QVariant::fromValue<QString>(feedItem->serviceName);
        break;
    case McaContentRoles::SystemServiceIconRole:
        result = QVariant::fromValue<QString>(feedItem->serviceIcon);
        break;
    case McaFeedModel::CommonActionsRole:
        result = QVariant::fromValue<McaActionsProxy*>(m_feedActions.at(row));
        break;
    case McaFeedModel::GenericAvatarUrlRole:
        result = QVariant::fromValue<QString>(feedItem->avatar);
        break;
    default:
//        qDebug() << "McaAggregatedModelProxy::data: Unhandled data role requested " << role << " for row " << row;
        result = QVariant();
        break;
    }

    return result;
}

bool McaAggregatedModelProxy::frozen()
{
    return m_frozen;
}

void McaAggregatedModelProxy::setFrozen(bool frozen)
{
    if(m_frozen == frozen) return;

    if(!frozen) {
        // Thaw
        m_frozen = frozen;
        while(!m_frozenQueue.isEmpty()) {
            struct feeditem_event_s *item = m_frozenQueue.dequeue();
            switch(item->type) {
            case FEEDITEM_ADD:
                onItemsAddedInternal(item->u.addchange_items);
                delete item->u.addchange_items;
                break;
            case FEEDITEM_CHANGE:
                onItemsChangedInternal(item->u.addchange_items);
                delete item->u.addchange_items;
                break;
            case FEEDITEM_REMOVE:
                onItemsRemovedInternal(item->u.remove_list);
                delete item->u.remove_list;
                break;
            default:
                break;
            }
            delete item;
        }
        emit frozenChanged(m_frozen);
    } else {
        m_frozen = frozen;
        emit frozenChanged(m_frozen);
    }
}

void McaAggregatedModelProxy::onItemsAdded(ArrayOfMcaFeedItemStruct items)
{
    qDebug() << "McaAggregatedModelProxy::onItemsAdded " << items.count();

    if(m_frozen) {
        struct feeditem_event_s *item = new struct feeditem_event_s;
        ArrayOfMcaFeedItemStruct *payload = new ArrayOfMcaFeedItemStruct(items);
        item->type = FEEDITEM_ADD;
        item->u.addchange_items = payload;
        m_frozenQueue.enqueue(item);
    } else {
        onItemsAddedInternal(&items);
    }
}

void McaAggregatedModelProxy::onItemsChanged(ArrayOfMcaFeedItemStruct items)
{
    if(m_frozen) {
        struct feeditem_event_s *item = new struct feeditem_event_s;
        ArrayOfMcaFeedItemStruct *payload = new ArrayOfMcaFeedItemStruct(items);
        item->type = FEEDITEM_CHANGE;
        item->u.addchange_items = payload;
        m_frozenQueue.enqueue(item);
    } else {
        onItemsChangedInternal(&items);
    }
}

void McaAggregatedModelProxy::onItemsRemoved(ArrayOfMcaFeedItemId items)
{
    qDebug() << "McaAggregatedModelProxy::onItemsRemoved " << items.count();

    if(m_frozen) {
        struct feeditem_event_s *item = new struct feeditem_event_s;
        ArrayOfMcaFeedItemId *payload = new ArrayOfMcaFeedItemId(items);
        item->type = FEEDITEM_REMOVE;
        item->u.remove_list = payload;
        m_frozenQueue.enqueue(item);
    } else {
        onItemsRemovedInternal(&items);
    }
}

void McaAggregatedModelProxy::onItemsAddedInternal(ArrayOfMcaFeedItemStruct *items)
{
    qDebug() << "McaAggregatedModelProxy::onItemsAdded " << items->count();

    beginInsertRows(QModelIndex(), m_feedItems.count(), m_feedItems.count() + items->count() - 1);
    for(int row = 0; row < items->count(); row++) {
        McaFeedItemStruct *newItem = new McaFeedItemStruct(items->at(row));
        m_feedItems.append(newItem);

        McaActionsProxy *actions = new McaActionsProxy();
        actions->setServiceUpid(newItem->serviceUpid);
        actions->setCustomActions(newItem->customActions, newItem->customDisplayActions);
        m_feedActions.append(actions);

        connect(actions, SIGNAL(standardAction(QString,QString)), this, SLOT(doStandardAction(QString, QString)));
        connect(actions, SIGNAL(customAction(QString,QString)), this, SLOT(doCustomAction(QString, QString)));
    }
    endInsertRows();
}

void McaAggregatedModelProxy::onItemsChangedInternal(ArrayOfMcaFeedItemStruct *items)
{
    int row;
    foreach (McaFeedItemStruct feedItem, *items) {
        for (row = 0; row < m_feedItems.count(); ++row) {
            McaFeedItemStruct *oldItem = m_feedItems.at(row);
            if (oldItem->serviceUpid == feedItem.serviceUpid && oldItem->uniqueId == feedItem.uniqueId) {
                McaFeedItemStruct *oldItem = m_feedItems.at(row);
                *oldItem = feedItem;
                m_feedActions.at(row)->setCustomActions(feedItem.customActions, feedItem.customDisplayActions);
                QModelIndex qmi = createIndex(row, 0, 0);
                emit dataChanged(qmi, qmi);
                break;
            }
        }
    }
}

void McaAggregatedModelProxy::onItemsRemovedInternal(ArrayOfMcaFeedItemId *items)
{
    qDebug() << "McaAggregatedModelProxy::onItemsRemoved " << items->count();
    int i;
    foreach (McaFeedItemId itemId, *items) {
        for (i = 0; i < m_feedItems.count(); i++) {
            struct McaFeedItemStruct *item = m_feedItems.at(i);
            if (itemId.itemId == item->uniqueId && itemId.serviceId == item->serviceUpid) {
                beginRemoveRows(QModelIndex(), i, i);
                m_feedItems.removeAt(i);
                McaActionsProxy *actions = m_feedActions.at(i);
                m_feedActions.removeAt(i);
                endRemoveRows();
                delete item;
                delete actions;
                break;
            }
        }
    }
}

void McaAggregatedModelProxy::doOfflineChanged()
{
    if(!isOffline()) {
        //TODO: figure out what to do if state frozen
        beginRemoveRows(QModelIndex(), 0, m_feedItems.count() - 1);
        while(m_feedItems.count()) {
            struct McaFeedItemStruct *item = m_feedItems.at(0);
            m_feedItems.removeFirst();
            delete item;
        }
        endRemoveRows();

        connect(m_dbusModel, SIGNAL(ItemsAdded(ArrayOfMcaFeedItemStruct)),
                this, SLOT(onItemsAdded(ArrayOfMcaFeedItemStruct)));
        connect(m_dbusModel, SIGNAL(ItemsChanged(ArrayOfMcaFeedItemStruct)),
                this, SLOT(onItemsChanged(ArrayOfMcaFeedItemStruct)));
        connect(m_dbusModel, SIGNAL(ItemsRemoved(ArrayOfMcaFeedItemId)),
                this, SLOT(onItemsRemoved(ArrayOfMcaFeedItemId)));
    }
}

void McaAggregatedModelProxy::doStandardAction(const QString& action, const QString& uniqueId)
{
    McaActionsProxy *actions = qobject_cast<McaActionsProxy*>(sender());
    if(actions) {
        if(isOffline()) {
            qDebug() << "TODO: OFFLINE McaAggregatedModelProxy::doStandardAction "
                     << action << uniqueId << actions->getServiceUpid();
        } else {
            m_dbusModel->asyncCall("doStandardAction", QVariant(action), QVariant(uniqueId), QVariant(actions->getServiceUpid()));
        }
    }
}

void McaAggregatedModelProxy::doCustomAction(const QString& action, const QString& uniqueId)
{
    McaActionsProxy *actions = qobject_cast<McaActionsProxy*>(sender());
    if(actions) {
        if(isOffline()) {
            qDebug() << "TODO: OFFLINE McaAggregatedModelProxy::doCustomAction "
                     << action << uniqueId << actions->getServiceUpid();
        } else {
            m_dbusModel->asyncCall("doCustomAction", QVariant(action), QVariant(uniqueId), QVariant(actions->getServiceUpid()));
        }
    }
}
