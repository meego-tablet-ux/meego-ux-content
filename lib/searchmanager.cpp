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

//struct FeedInfo
//{
//    QString upid;
//    QAbstractListModel *feed;
//};

// TODO: remove this temporary solution to not really wanting limits on search
const int TemporaryFeedLimit = 1000;

//
// public methods
//

McaSearchManager::McaSearchManager(QObject *parent):
        AbstractManager(parent)
{
    m_serviceModel = m_feedmgr->serviceModel();
}

McaSearchManager::~McaSearchManager()
{
    rowsAboutToBeRemoved(QModelIndex(), 0, m_serviceModel->rowCount() - 1);
}

void McaSearchManager::initialize(const QString& managerData)
{
    m_searchText = managerData;

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

QModelIndex McaSearchManager::serviceModelIndex(int row)
{
    return m_serviceModel->index(row, 0);
}

QVariant McaSearchManager::serviceModelData(const QModelIndex& index, int role)
{
    return m_serviceModel->data(index, role);
}

bool McaSearchManager::dataChangedCondition(const QModelIndex& index)
{
    Q_UNUSED(index);
    return true;
}

int McaSearchManager::createFeed(const QAbstractItemModel *serviceModel, const QString& name)
{
    return m_feedmgr->createSearchFeed(serviceModel, name, m_searchText);
}

//
// protected methods
//

void McaSearchManager::createFeedDone(QObject *containerObj, McaFeedAdapter *feedAdapter, int uniqueRequestId) {
    McaSearchableContainer *container = qobject_cast<McaSearchableContainer*>(containerObj);
    if (m_requestIds.keys().contains(uniqueRequestId) && 0 != container) {
        QModelIndex index = m_serviceModel->index(m_requestIds[uniqueRequestId], 0);
        QString name = m_serviceModel->data(index, McaServiceModel::RequiredNameRole).toString();
//        QString displayName = m_serviceModel->data(index, McaServiceModel::CommonDisplayNameRole).toString();
//        QString iconUrl = m_serviceModel->data(index, McaServiceModel::CommonIconUrlRole).toString();
//        QString category = m_serviceModel->data(index, McaServiceModel::RequiredCategoryRole).toString();

        QAbstractListModel *model = qobject_cast<QAbstractListModel*>(m_serviceModel->data(index, McaAggregatedModel::SourceModelRole).value<QObject*>());
        QString upid = m_feedmgr->serviceId(model, name);

        connect(this, SIGNAL(searchTextChanged(QString)),
                container, SLOT(setSearchText(QString)));
        m_requestIds.remove(uniqueRequestId);

//        McaFeedAdapter *adapter = new McaFeedAdapter(container->feedModel(), name, displayName, iconUrl, category);
        feedAdapter->setLimit(TemporaryFeedLimit);
        FeedInfo *info = new FeedInfo;
        info->upid = upid;
        info->feed = feedAdapter;
        m_upidToFeedInfo.insert(upid, info);
        m_aggregator->addSourceModel(feedAdapter);
    }
}
