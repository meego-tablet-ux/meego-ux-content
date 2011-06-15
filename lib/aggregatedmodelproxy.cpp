#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include "aggregatedmodelproxy.h"
#include "feedmodel.h"

McaAggregatedModelProxy::McaAggregatedModelProxy(const QString &service, const QString &objectPath)
{
    m_dbusModel  = new QDBusInterface(service, objectPath);

    connect(m_dbusModel, SIGNAL(ItemsAdded(ArrayOfMcaFeedItemStruct)),
            this, SLOT(onItemsAdded(ArrayOfMcaFeedItemStruct)));
    connect(m_dbusModel, SIGNAL(ItemsChanged(ArrayOfMcaFeedItemStruct)),
            this, SLOT(onItemsChanged(ArrayOfMcaFeedItemStruct)));
    connect(m_dbusModel, SIGNAL(ItemsRemoved(QStringList)),
            this, SLOT(onItemsRemoved(QStringList)));
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
    default:
        qDebug() << "McaAggregatedModelProxy::data: Unhandled data role requested " << role << " for row " << row;
        result = QVariant();
        break;
    }

    return result;
}

void McaAggregatedModelProxy::onItemsAdded(ArrayOfMcaFeedItemStruct items)
{
    qDebug() << "McaAggregatedModelProxy::onItemsAdded " << items.count();
    beginInsertRows(QModelIndex(), m_feedItems.count(), m_feedItems.count() + items.count() - 1);
    for(int row = 0; row < items.count(); row++) {
        McaFeedItemStruct *newItem = new McaFeedItemStruct(items.at(row));
        m_feedItems.append(newItem);
    }
    endInsertRows();
}

void McaAggregatedModelProxy::onItemsChanged(ArrayOfMcaFeedItemStruct items)
{
    qDebug() << "TODO: implement data change - McaAggregatedModelProxy::onItemsChanged";
}

void McaAggregatedModelProxy::onItemsRemoved(QStringList items)
{
    qDebug() << "McaAggregatedModelProxy::onItemsRemoved " << items.count();
    int i;
    foreach (QString itemId, items) {
        for (i = 0; i < m_feedItems.count(); i++) {
            struct McaFeedItemStruct *item = m_feedItems.at(i);
            if (itemId == item->uuid) {
                beginRemoveRows(QModelIndex(), i, i);
                m_feedItems.removeAt(i);
                endRemoveRows();
                delete item;
                break;
            }
        }
    }
}
