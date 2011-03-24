#include "abstractmanager.h"

#include <QSortFilterProxyModel>
#include <QDebug>

#include "feedmodel.h"
#include "servicemodel.h"
#include "feedmanager.h"
#include "feedcache.h"
#include "aggregatedmodel.h"
#include "feedadapter.h"

AbstractManager::AbstractManager(QObject *parent) :
    QObject(parent)
{
    m_feedmgr = McaFeedManager::takeManager();
    m_cache = new McaFeedCache(this);
    connect(m_cache, SIGNAL(frozenChanged(bool)),
            this, SIGNAL(frozenChanged(bool)));
    m_aggregator = new McaAggregatedModel(m_cache);
    m_cache->setSourceModel(m_aggregator);

    m_feedProxy = new QSortFilterProxyModel(this);
    m_feedProxy->setSourceModel(m_cache);
    m_feedProxy->setSortRole(McaFeedModel::RequiredTimestampRole);
    m_feedProxy->sort(0, Qt::DescendingOrder);
    m_feedProxy->setDynamicSortFilter(true);

    connect(m_feedmgr, SIGNAL(feedCreated(QObject*,McaFeedAdapter*,int)), this, SLOT(createFeedDone(QObject*,McaFeedAdapter*,int)));
    connect(m_feedmgr, SIGNAL(createFeedError(QString,int)), this, SLOT(createFeedError(QString,int)));
}

AbstractManager::~AbstractManager()
{
    delete m_cache;

    McaFeedManager::releaseManager();
}

bool AbstractManager::frozen()
{
    return m_cache->frozen();
}

QSortFilterProxyModel *AbstractManager::feedModel()
{
    return m_feedProxy;
}

void AbstractManager::setFrozen(bool frozen)
{
    m_cache->setFrozen(frozen);
}

void AbstractManager::rowsAboutToBeRemoved(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index)

    for (int i=start; i<=end; i++) {
        QModelIndex qmi = serviceModelIndex(i);
        removeFeed(qmi);
    }
}

void AbstractManager::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    for (int i=topLeft.row(); i<=bottomRight.row(); i++) {
        QModelIndex qmi = serviceModelIndex(i);//m_serviceProxy->index(i, 0);
        QAbstractListModel *model = qobject_cast<QAbstractListModel*>(serviceModelData(qmi, McaAggregatedModel::SourceModelRole).value<QObject*>());
        QString name = serviceModelData(qmi, McaServiceModel::RequiredNameRole).toString();
        QString id = m_feedmgr->serviceId(model, name);

        if (m_upidToFeedInfo.contains(id)) {
            // if the feed now has a configuration error, remove it
            if (serviceModelData(qmi, McaServiceModel::CommonConfigErrorRole).toBool() || !dataChangedCondition(qmi)) {
                removeFeed(qmi);
            }
        }
        else {
            // if the feed is now configured correctly, add it
            if (!serviceModelData(qmi, McaServiceModel::CommonConfigErrorRole).toBool() && dataChangedCondition(qmi)) {
                addFeed(qmi);
            }
        }
    }
}

void AbstractManager::addFeed(const QModelIndex &index)
{
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(serviceModelData(index, McaAggregatedModel::SourceModelRole).value<QObject*>());
    QString name = serviceModelData(index, McaServiceModel::RequiredNameRole).toString();
    QString upid = m_feedmgr->serviceId(model, name);

    if (m_upidToFeedInfo.contains(upid)) {
        qWarning() << "warning: search manager: unexpected duplicate add feed request";
        return;
    }

    m_requestIds[createFeed(model, name)] = index.row();
}

void AbstractManager::removeFeed(const QModelIndex &index)
{
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(serviceModelData(index, McaAggregatedModel::SourceModelRole).value<QObject*>());
    QString name = serviceModelData(index, McaServiceModel::RequiredNameRole).toString();
    QString upid = m_feedmgr->serviceId(model, name);

    FeedInfo *info = m_upidToFeedInfo.value(upid, NULL);
    if (info) {
        m_aggregator->removeSourceModel(info->feed);
        m_upidToFeedInfo.remove(upid);
        removeFeedCleanup(upid);
        //delete info->feed;
        info->feed->deleteLater();
        delete info;
    }
}

void AbstractManager::rowsInserted(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index)

    for (int i=start; i<=end; i++) {
        QModelIndex qmi = serviceModelIndex(i);
            if (serviceModelData(qmi, McaServiceModel::CommonConfigErrorRole).toBool() || !dataChangedCondition(qmi)) {
            continue;
        }
        addFeed(qmi);
    }
}

void AbstractManager::createFeedDone(QObject *containerObj, McaFeedAdapter *feedAdapter, int uniqueRequestId) {
    if (m_requestIds.keys().contains(uniqueRequestId) && 0 != containerObj) {
        QModelIndex index = serviceModelIndex(m_requestIds[uniqueRequestId]);
        QString name = serviceModelData(index, McaServiceModel::RequiredNameRole).toString();
        QAbstractListModel *model = qobject_cast<QAbstractListModel*>(serviceModelData(index, McaAggregatedModel::SourceModelRole).value<QObject*>());
        QString upid = m_feedmgr->serviceId(model, name);

        m_requestIds.remove(uniqueRequestId);

        FeedInfo *info = new FeedInfo;
        info->upid = upid;
        info->feed = feedAdapter;
        m_upidToFeedInfo.insert(upid, info);
        createFeedFinalise(containerObj, feedAdapter, info);
        m_aggregator->addSourceModel(feedAdapter);
    }
}

void AbstractManager::createFeedError(QString serviceName, int uniqueRequestId) {
    if(m_requestIds.contains(uniqueRequestId)) {
        qDebug() << "CREATE Feed Error " << serviceName << " with request id " << uniqueRequestId;
        m_requestIds.remove(uniqueRequestId);
        //TODO: any aditional cleanup on feed creation error
    }
}
