/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>

#include <QSettings>

#include "allocator.h"
#include "feedmanager.h"
#include "servicemodel.h"
#include "aggregatedmodel.h"
#include "panelmanager.h"
#include "feedcache.h"
#include "feedfilter.h"
#include "feedmodel.h"
#include "serviceproxy.h"
#include "settings.h"

//
// Overview of McaPanelManager
//    - assembles pipeline to transform raw feeds (McaFeedModels from plugins)
//      - with hidden rows filtered out (McaFeedFilter)
//      - with a limited number of rows exposed (McaFeedAdapter)
//        - to a limit assigned by an allocation algorithm (McaAllocator)
//      - aggregated into one list model (McaAggregatedModel)
//      - with the ability to cache and freeze changes (McaFeedCache)
//    - responds to changes in the service model by adding or removing feeds
//

struct FeedInfo
{
    QString upid;
    QAbstractListModel *feed;
    McaFeedFilter *filter;
};

//
// public methods
//

McaPanelManager::McaPanelManager(QObject *parent):
        QObject(parent)
{
    m_allocator = new McaAllocator;
    m_feedmgr = McaFeedManager::takeManager();
    m_cache = new McaFeedCache(this);
    connect(m_cache, SIGNAL(frozenChanged(bool)),
            this, SIGNAL(frozenChanged(bool)));
    m_aggregator = new McaAggregatedModel(m_cache);
    m_cache->setSourceModel(m_aggregator);
    m_isEmpty = false;

    // sort the aggregated feed by timestamp
    m_feedProxy = new QSortFilterProxyModel(this);
    m_feedProxy->setSourceModel(m_cache);
    m_feedProxy->setSortRole(McaFeedModel::RequiredTimestampRole);
    m_feedProxy->sort(0, Qt::DescendingOrder);
    m_feedProxy->setDynamicSortFilter(true);

    m_serviceProxy = new McaServiceProxy(this, m_feedmgr->serviceModel(), this);

    m_servicesEnabledByDefault = true;

    connect(m_feedmgr, SIGNAL(feedCreated(QAbstractItemModel*,int)), this, SLOT(createFeedDone(QAbstractItemModel*,int)));
}

McaPanelManager::~McaPanelManager()
{
    rowsAboutToBeRemoved(QModelIndex(), 0, m_serviceProxy->rowCount() - 1);
    McaFeedManager::releaseManager();
}

void McaPanelManager::initialize(const QString& panelName)
{
    m_panelName = panelName;

    // proxy model should be empty, but seem to need to call rowCount to wake it up
    rowsInserted(QModelIndex(), 0, m_serviceProxy->rowCount() - 1);
    feedRowsChanged();

    connect(m_serviceProxy, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(rowsInserted(QModelIndex,int,int)));
    connect(m_serviceProxy, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(m_serviceProxy, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(dataChanged(QModelIndex,QModelIndex)));
    connect(m_serviceProxy, SIGNAL(categoriesChanged(QStringList)),
            this, SIGNAL(categoriesChanged(QStringList)));
    connect(m_feedProxy, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(feedRowsChanged()));
    connect(m_feedProxy, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(feedRowsChanged()));
}

QStringList McaPanelManager::categories()
{
    return m_serviceProxy->categories();
}

bool McaPanelManager::frozen()
{
    return m_cache->frozen();
}

bool McaPanelManager::servicesConfigured()
{
    // TODO: maybe need to really track just configured, not configured/enabled
    return m_upidToFeedInfo.count() != 0;
}

bool McaPanelManager::isEmpty()
{
    return m_isEmpty;
}

bool McaPanelManager::servicesEnabledByDefault()
{
    return m_servicesEnabledByDefault;
}

void McaPanelManager::setServicesEnabledByDefault(bool enabled)
{
    m_servicesEnabledByDefault = enabled;
}

QSortFilterProxyModel *McaPanelManager::serviceModel()
{
    return m_serviceProxy;
}

QSortFilterProxyModel *McaPanelManager::feedModel()
{
    return m_feedProxy;
}

bool McaPanelManager::isServiceEnabled(const QString& upid)
{
    bool enabled = m_servicesEnabledByDefault;

    if (!m_upidToEnabled.contains(upid)) {
        // check settings file
        QSettings settings(McaSettings::Organization, McaSettings::ApplicationCore);
        settings.beginGroup(upid);
        QVariant variant = settings.value(fullEnabledKey());
        if (variant.isValid())
            enabled = variant.toBool();
        settings.endGroup();
        m_upidToEnabled[upid] = enabled;
    }

    return m_upidToEnabled[upid];
}

void McaPanelManager::setServiceEnabled(const QString& upid, bool enabled)
{
    if (m_upidToEnabled.contains(upid) && m_upidToEnabled.value(upid) == enabled)
        return;
    m_upidToEnabled[upid] = enabled;

    // update the settings file
    QSettings settings(McaSettings::Organization, McaSettings::ApplicationCore);
    settings.beginGroup(upid);
    settings.setValue(fullEnabledKey(), enabled);
    settings.endGroup();

    emit serviceEnabledChanged(upid, enabled);
}

void McaPanelManager::clearHistory(const QString &upid, const QDateTime &datetime)
{
    FeedInfo *info = m_upidToFeedInfo.value(upid, NULL);
    if (info)
        info->filter->clearHistory(datetime);
    else
        qWarning() << "no matching feed while attempting to clear history";
}

void McaPanelManager::addDirectFeed(QAbstractItemModel *feed)
{
    m_aggregator->addSourceModel(feed);
}

void McaPanelManager::removeDirectFeed(QAbstractItemModel *feed)
{
    m_aggregator->removeSourceModel(feed);
}

//
// public slots
//

void McaPanelManager::setCategories(const QStringList &categories)
{
    m_serviceProxy->setCategories(categories);
}

void McaPanelManager::setFrozen(bool frozen)
{
    m_cache->setFrozen(frozen);
}

//
// protected slots
//

void McaPanelManager::rowsInserted(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index)

    for (int i=start; i<=end; i++) {
        QModelIndex qmi = m_serviceProxy->index(i, 0);
        if (m_serviceProxy->data(qmi, McaServiceModel::CommonConfigErrorRole).toBool() ||
            !m_serviceProxy->data(qmi, McaServiceProxy::SystemEnabledRole).toBool())
            continue;

        addFeed(qmi);
    }
}

void McaPanelManager::rowsAboutToBeRemoved(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index)

    for (int i=start; i<=end; i++) {
        QModelIndex qmi = m_serviceProxy->index(i, 0);
        removeFeed(qmi);
    }
}

void McaPanelManager::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    for (int i=topLeft.row(); i<=bottomRight.row(); i++) {
        QModelIndex qmi = m_serviceProxy->index(i, 0);
        QAbstractListModel *model = qobject_cast<QAbstractListModel*>(m_serviceProxy->data(qmi, McaAggregatedModel::SourceModelRole).value<QObject*>());
        QString name = m_serviceProxy->data(qmi, McaServiceModel::RequiredNameRole).toString();
        QString id = m_feedmgr->serviceId(model, name);

        if (m_upidToFeedInfo.contains(id)) {
            // if the feed now has a configuration error, remove it
            if (m_serviceProxy->data(qmi, McaServiceModel::CommonConfigErrorRole).toBool() ||
                !m_serviceProxy->data(qmi, McaServiceProxy::SystemEnabledRole).toBool())
                removeFeed(qmi);
        }
        else {
            // if the feed is now configured correctly, add it
            if (!m_serviceProxy->data(qmi, McaServiceModel::CommonConfigErrorRole).toBool() &&
                m_serviceProxy->data(qmi, McaServiceProxy::SystemEnabledRole).toBool())
                addFeed(qmi);
        }
    }
}

void McaPanelManager::feedRowsChanged()
{
    bool empty = m_feedProxy->rowCount() == 0;

    if (empty != m_isEmpty) {
        m_isEmpty = empty;
        emit isEmptyChanged(empty);
    }
}

//
// protected methods
//

void McaPanelManager::addFeed(const QModelIndex &index)
{
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(m_serviceProxy->data(index, McaAggregatedModel::SourceModelRole).value<QObject*>());

    QString name = m_serviceProxy->data(index, McaServiceModel::RequiredNameRole).toString();
/*
    QString displayName = m_serviceProxy->data(index, McaServiceModel::CommonDisplayNameRole).toString();
    QString iconUrl = m_serviceProxy->data(index, McaServiceModel::CommonIconUrlRole).toString();
    QString category = m_serviceProxy->data(index, McaServiceModel::RequiredCategoryRole).toString();
*/
    QString upid = m_feedmgr->serviceId(model, name);
    if (m_upidToFeedInfo.contains(upid)) {
        qWarning() << "warning: panel manager: unexpected duplicate add feed request";
        return;
    }

    m_requestIds[m_feedmgr->createFeed(model, name)] = index.row();

/*
    QAbstractItemModel *feed = m_feedmgr->createFeed(model, name);
    if (feed) {
        McaFeedFilter *filter = new McaFeedFilter(feed, upid);
        McaFeedAdapter *adapter = new McaFeedAdapter(filter, name, displayName, iconUrl, category);
        FeedInfo *info = new FeedInfo;
        info->upid = upid;
        info->feed = adapter;
        info->filter = filter;
        m_upidToFeedInfo.insert(upid, info);
        if (m_upidToFeedInfo.count() == 1)
            emit servicesConfiguredChanged(true);
        m_allocator->addFeed(upid, adapter);
        m_aggregator->addSourceModel(adapter);
    }
*/
}

void McaPanelManager::removeFeed(const QModelIndex &index)
{
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(m_serviceProxy->data(index, McaAggregatedModel::SourceModelRole).value<QObject*>());
    QString name = m_serviceProxy->data(index, McaServiceModel::RequiredNameRole).toString();
    QString upid = m_feedmgr->serviceId(model, name);

    FeedInfo *info = m_upidToFeedInfo.value(upid, NULL);
    if (info) {
        m_aggregator->removeSourceModel(info->feed);
        m_upidToFeedInfo.remove(upid);
        if (m_upidToFeedInfo.count() == 0)
            emit servicesConfiguredChanged(false);
        m_allocator->removeFeed(upid);
        delete info->feed;
        delete info;
    }
}

QString McaPanelManager::fullEnabledKey()
{
    // returns: the enabled key prefix with the suffix for this panel, if any
    QString key = McaSettings::KeyEnabledPrefix;
    if (!m_panelName.isEmpty())
        key.append(QString("-") + m_panelName);
    return key;
}

void McaPanelManager::createFeedDone(QAbstractItemModel *feed, int uniqueRequestId) {
    qDebug() << "McaSearchManager::createFeedDone " << uniqueRequestId << m_requestIds.keys();
    if (m_requestIds.keys().contains(uniqueRequestId) && 0 != feed) {
        qDebug() << "Key matched";
        QModelIndex index = m_serviceProxy->index(m_requestIds[uniqueRequestId], 0);
        QAbstractListModel *model = qobject_cast<QAbstractListModel*>(m_serviceProxy->data(index, McaAggregatedModel::SourceModelRole).value<QObject*>());

        QString name = m_serviceProxy->data(index, McaServiceModel::RequiredNameRole).toString();
        QString displayName = m_serviceProxy->data(index, McaServiceModel::CommonDisplayNameRole).toString();
        QString iconUrl = m_serviceProxy->data(index, McaServiceModel::CommonIconUrlRole).toString();
        QString category = m_serviceProxy->data(index, McaServiceModel::RequiredCategoryRole).toString();
        QString upid = m_feedmgr->serviceId(model, name);

        McaFeedFilter *filter = new McaFeedFilter(feed, upid);
        McaFeedAdapter *adapter = new McaFeedAdapter(filter, name, displayName, iconUrl, category);
        FeedInfo *info = new FeedInfo;
        info->upid = upid;
        info->feed = adapter;
        info->filter = filter;
        m_upidToFeedInfo.insert(upid, info);
        if (m_upidToFeedInfo.count() == 1)
            emit servicesConfiguredChanged(true);
        m_allocator->addFeed(upid, adapter);
        m_aggregator->addSourceModel(adapter);
    }
}

