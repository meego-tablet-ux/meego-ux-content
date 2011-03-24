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

    foreach(McaSearchableContainer* container, m_searchableContainers) {
        addSearchRequest(container, searchText);
    }
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

//void McaSearchManager::createFeedDone(QObject *containerObj, McaFeedAdapter *feedAdapter, int uniqueRequestId) {
//    McaSearchableContainer *container = qobject_cast<McaSearchableContainer*>(containerObj);
//    if (m_requestIds.keys().contains(uniqueRequestId) && 0 != container) {
//        QModelIndex index = m_serviceModel->index(m_requestIds[uniqueRequestId], 0);
//        QString name = m_serviceModel->data(index, McaServiceModel::RequiredNameRole).toString();
//        QAbstractListModel *model = qobject_cast<QAbstractListModel*>(m_serviceModel->data(index, McaAggregatedModel::SourceModelRole).value<QObject*>());
//        QString upid = m_feedmgr->serviceId(model, name);

//        connect(this, SIGNAL(searchTextChanged(QString)),
//                container, SLOT(setSearchText(QString)));
//        m_requestIds.remove(uniqueRequestId);

//        feedAdapter->setLimit(TemporaryFeedLimit);
//        FeedInfo *info = new FeedInfo;
//        info->upid = upid;
//        info->feed = feedAdapter;
//        m_upidToFeedInfo.insert(upid, info);
//        m_aggregator->addSourceModel(feedAdapter);
//    }
//}

void McaSearchManager::createFeedFinalise(QObject *containerObj, McaFeedAdapter *feedAdapter, FeedInfo *feedInfo)
{
    Q_UNUSED(feedInfo);

    McaSearchableContainer *container = qobject_cast<McaSearchableContainer*>(containerObj);

    connect(container, SIGNAL(searchDone()), this, SLOT(searchDone()));
    m_searchableContainers.push_back(container);
    if(!m_searchRequests.contains(container->thread())) {
        m_searchRequests[container->thread()] = new t_SearchRequestQueue();
    }
    addSearchRequest(container, m_searchText);

    feedAdapter->setLimit(TemporaryFeedLimit);
}

void McaSearchManager::searchDone()
{
    McaSearchableContainer *container = qobject_cast<McaSearchableContainer*>(sender());
    if(!container) {
        qDebug() << "ERROR: searchDone called from a QObject different from McaSearchableContainer";
        return;
    }

    QThread *containerThread = container->thread();
    if(!m_searchRequests.contains(containerThread)) {
        qDebug() << "CRITICAL ERROR: m_searchRequests does not contain entry for thread " << containerThread;
        return;
    }
    t_SearchRequestQueue *threadQueue = m_searchRequests[containerThread];
    m_processingRequests.removeOne(containerThread);
    qDebug() << "McaSearchManager::searchDone " << m_processingRequests;
    if(!threadQueue->isEmpty()) {
        t_SearchRequestEntry *searchRequest = threadQueue->front();
        threadQueue->pop_front();
        addSearchRequest(searchRequest->first, searchRequest->second);
        delete searchRequest;
    }
}

void McaSearchManager::removeFeedCleanup(const QString& upid) {
    qDebug() << "McaSearchManager::removeFeedCleanup NOT IMPLEMENTED, WILL CRASH ON REMOVE SERVICE";
    //This will crash if we do a search after a service is removed
    //TODO: remove the searchableContainer from m_searchableContainers
    //m_upidToFeedInfo[upid]->feed gives us the McaSearchableFeed, how do we get to McaSearchableContainer
}

void McaSearchManager::addSearchRequest(McaSearchableContainer *container, const QString &searchText)
{
    QThread *containerThread = container->thread();
    if(m_processingRequests.contains(containerThread)) {
        t_SearchRequestQueue *threadQueue = m_searchRequests.value(containerThread);
        t_SearchRequestEntry *requestEntry = 0;
        for(int index =0; index < threadQueue->count(); index++) {
            requestEntry = threadQueue->at(index);
            if(requestEntry->first == container) {
                qDebug() << container << requestEntry->second << searchText;
                threadQueue->removeOne(requestEntry);
            }
        }
        t_SearchRequestEntry *searchRequest = new t_SearchRequestEntry();
        searchRequest->first = container;
        searchRequest->second = searchText;
        threadQueue->push_back(searchRequest);
    } else {
        m_processingRequests.push_back(containerThread);
        qDebug() << "McaSearchManager::addSearchRequest " << m_processingRequests;
        QMetaObject::invokeMethod(container, "setSearchText", Q_ARG(QString, searchText));
    }
}
