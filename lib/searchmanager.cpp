/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>

#include <QSettings>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include "searchmanager.h"
#include "allocator.h"
#include "feedmanager.h"
#include "servicemodel.h"
#include "aggregatedmodel.h"
#include "feedcache.h"
#include "feedfilter.h"
#include "feedmodel.h"
#include "settings.h"
#include "searchablecontainer.h"

struct FeedInfo
{
    QString upid;
    QAbstractListModel *feed;
};

// TODO: remove this temporary solution to not really wanting limits on search
const int TemporaryFeedLimit = 100;

//
// public methods
//

McaSearchManager::McaSearchManager(QObject *parent):
        QObject(parent)
{
    m_feedmgr = McaFeedManager::takeManager();
    m_cache = new McaFeedCache(this);
    connect(m_cache, SIGNAL(frozenChanged(bool)),
            this, SIGNAL(frozenChanged(bool)));
    m_aggregator = new McaAggregatedModel(m_cache);
    m_cache->setSourceModel(m_aggregator);

    // sort the aggregated feed by timestamp
    m_feedProxy = new QSortFilterProxyModel(this);
    m_feedProxy->setSourceModel(m_cache);
    m_feedProxy->setSortRole(McaFeedModel::RequiredTimestampRole);
    m_feedProxy->sort(0, Qt::DescendingOrder);
    m_feedProxy->setDynamicSortFilter(true);

    m_serviceModel = m_feedmgr->serviceModel();
}

McaSearchManager::~McaSearchManager()
{
    rowsAboutToBeRemoved(QModelIndex(), 0, m_serviceModel->rowCount() - 1);
    McaFeedManager::releaseManager();
}

void McaSearchManager::initialize(const QString& searchText)
{
    m_searchText = searchText;

    rowsInserted(QModelIndex(), 0, m_serviceModel->rowCount() - 1);

    connect(m_serviceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(rowsInserted(QModelIndex,int,int)));
    connect(m_serviceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(m_serviceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(dataChanged(QModelIndex,QModelIndex)));
}

QString McaSearchManager::searchText()
{
    return m_searchText;
}

bool McaSearchManager::frozen()
{
    return m_cache->frozen();
}

QSortFilterProxyModel *McaSearchManager::feedModel()
{
    return m_feedProxy;
}

//
// public slots
//

void McaSearchManager::setSearchText(const QString& searchText)
{
    if (searchText == m_searchText)
        return;
    m_searchText = searchText;

    emit searchTextChanged(searchText);
}

void McaSearchManager::setFrozen(bool frozen)
{
    m_cache->setFrozen(frozen);
}

//
// protected slots
//

void McaSearchManager::rowsInserted(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index)

    for (int i=start; i<=end; i++) {
        QModelIndex qmi = m_serviceModel->index(i, 0);
        if (m_serviceModel->data(qmi, McaServiceModel::CommonConfigErrorRole).toBool())
            continue;
        addFeed(qmi);
    }
}

void McaSearchManager::rowsAboutToBeRemoved(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index)

    for (int i=start; i<=end; i++) {
        QModelIndex qmi = m_serviceModel->index(i, 0);
        removeFeed(qmi);
    }
}

void McaSearchManager::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    for (int i=topLeft.row(); i<=bottomRight.row(); i++) {
        QModelIndex qmi = m_serviceModel->index(i, 0);
        QAbstractListModel *model = qobject_cast<QAbstractListModel*>(m_serviceModel->data(qmi, McaAggregatedModel::SourceModelRole).value<QObject*>());
        QString name = m_serviceModel->data(qmi, McaServiceModel::RequiredNameRole).toString();
        QString id = m_feedmgr->serviceId(model, name);

        if (m_upidToFeedInfo.contains(id)) {
            // if the feed now has a configuration error, remove it
            if (m_serviceModel->data(qmi, McaServiceModel::CommonConfigErrorRole).toBool())
                removeFeed(qmi);
        }
        else {
            // if the feed is now configured correctly, add it
            if (!m_serviceModel->data(qmi, McaServiceModel::CommonConfigErrorRole).toBool())
                addFeed(qmi);
        }
    }
}

//
// protected methods
//

void McaSearchManager::addFeed(const QModelIndex &index)
{
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(m_serviceModel->data(index, McaAggregatedModel::SourceModelRole).value<QObject*>());
    QString name = m_serviceModel->data(index, McaServiceModel::RequiredNameRole).toString();
    QString displayName = m_serviceModel->data(index, McaServiceModel::CommonDisplayNameRole).toString();
    QString iconUrl = m_serviceModel->data(index, McaServiceModel::CommonIconUrlRole).toString();
    QString category = m_serviceModel->data(index, McaServiceModel::RequiredCategoryRole).toString();
    QString upid = m_feedmgr->serviceId(model, name);

    if (m_upidToFeedInfo.contains(upid)) {
        qWarning() << "warning: search manager: unexpected duplicate add feed request";
        return;
    }

    McaSearchableContainer *container = m_feedmgr->createSearchFeed(model, name, m_searchText);
    if (container) {
        connect(this, SIGNAL(searchTextChanged(QString)),
                container, SLOT(setSearchText(QString)));

        McaFeedAdapter *adapter = new McaFeedAdapter(container->feedModel(), name, displayName, iconUrl, category);
        adapter->setLimit(TemporaryFeedLimit);
        FeedInfo *info = new FeedInfo;
        info->upid = upid;
        info->feed = adapter;
        m_upidToFeedInfo.insert(upid, info);
        m_aggregator->addSourceModel(adapter);
    }
}

void McaSearchManager::removeFeed(const QModelIndex &index)
{
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(m_serviceModel->data(index, McaAggregatedModel::SourceModelRole).value<QObject*>());
    QString name = m_serviceModel->data(index, McaServiceModel::RequiredNameRole).toString();
    QString upid = m_feedmgr->serviceId(model, name);

    FeedInfo *info = m_upidToFeedInfo.value(upid, NULL);
    if (info) {
        m_aggregator->removeSourceModel(info->feed);
        m_upidToFeedInfo.remove(upid);
        delete info->feed;
        delete info;
    }
}

