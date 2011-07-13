/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "memoryleak.h"

#include <QDebug>

#include <QSettings>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include "searchmanagerdbus.h"
#include "allocator.h"
#include "feedmanager.h"
#include "servicemodel.h"
#include "aggregatedmodel.h"
#include "feedfilter.h"
#include "feedmodel.h"
#include "settings.h"
#include "searchablecontainer.h"
#include "serviceproxybase.h"

#include "memoryleak-defines.h"

// TODO: remove this temporary solution to not really wanting limits on search
const int TemporaryFeedLimit = 100;

//
// public methods
//

McaSearchManagerDBus::McaSearchManagerDBus(QObject *parent):
        McaAbstractManagerDBus(parent)
{
    m_serviceModel = m_feedmgr->serviceModel();

#ifndef THREADING
    m_searchTimer.setInterval(1);
    m_searchTimer.setSingleShot(true);
    connect(&m_searchTimer, SIGNAL(timeout()), this, SLOT(doNextSearch()));
    m_currentRequest = 0;
#endif
}

McaSearchManagerDBus::~McaSearchManagerDBus()
{
    removeAllFeeds();

    while(m_searchRequests.count()) {
        QThread *thread = m_searchRequests.keys().at(0);
        delete m_searchRequests.value(thread);
        m_searchRequests.remove(thread);
    }
}

void McaSearchManagerDBus::initialize(const QString& managerData)
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

QString McaSearchManagerDBus::searchText()
{
    return m_searchText;
}

QString McaSearchManagerDBus::serviceModelPath()
{
    qDebug() << "McaPanelManagerDBus::serviceModelPath";
    return m_dbusObjectId + SERVICEMODELPROXY_DBUS_NAME;
}

//
// public slots
//

void McaSearchManagerDBus::setSearchText(const QString& searchText)
{
    qDebug() << "McaSearchManagerDBus::setSearchText " << searchText << ", old= " << m_searchText;
    if (searchText == m_searchText)
        return;
    m_searchText = searchText;

//    emit searchTextChanged(searchText);

    foreach(McaSearchableContainer* container, m_searchableContainers) {
#ifdef THREADING
        container->searchable()->haltSearch();
#endif
        addSearchRequest(container, searchText);
    }
}

QModelIndex McaSearchManagerDBus::serviceModelIndex(int row)
{
    return m_serviceModel->index(row, 0);
}

int McaSearchManagerDBus::serviceModelRowCount()
{
    return m_serviceModel->rowCount();
}

QVariant McaSearchManagerDBus::serviceModelData(const QModelIndex &index, int role)
{
    return m_serviceModel->data(index, role);
}

//QVariant McaSearchManagerDBus::serviceModelData(int row, int role)
//{
//    return serviceModelData(serviceModelIndex(row), role);
//}

bool McaSearchManagerDBus::dataChangedCondition(const QModelIndex &index)
{
    Q_UNUSED(index);
    return true;
}

//bool McaSearchManagerDBus::dataChangedCondition(int row)
//{
//    return dataChangedCondition( serviceModelIndex(row));
//}

int McaSearchManagerDBus::createFeed(const QAbstractItemModel *serviceModel, const QString& name)
{
    return m_feedmgr->createSearchFeed(serviceModel, name, m_searchText);
}

//
// protected methods
//

void McaSearchManagerDBus::createFeedFinalize(QObject *containerObj, McaFeedAdapter *feedAdapter, FeedInfo *feedInfo)
{
    Q_UNUSED(feedInfo);

    McaSearchableContainer *container = qobject_cast<McaSearchableContainer*>(containerObj);

    qDebug() << "New container: " << container << container->searchable();

    connect(container, SIGNAL(searchDone()), this, SLOT(searchDone()));
    m_searchableContainers.push_back(container);
    if (!m_searchRequests.contains(container->thread())) {
        m_searchRequests[container->thread()] = new t_SearchRequestQueue();
    }
    addSearchRequest(container, m_searchText);

    feedAdapter->setLimit(TemporaryFeedLimit);
}

void McaSearchManagerDBus::searchDone()
{
    McaSearchableContainer *container = qobject_cast<McaSearchableContainer*>(sender());
    if (!container) {
        qDebug() << "ERROR: searchDone called from a QObject different from McaSearchableContainer";
        return;
    }

    QThread *containerThread = container->thread();
    if (!m_searchRequests.contains(containerThread)) {
        qDebug() << "CRITICAL ERROR: m_searchRequests does not contain entry for thread " << containerThread;
        return;
    }
    t_SearchRequestQueue *threadQueue = m_searchRequests[containerThread];
#ifdef THREADING
    m_processingRequests.removeOne(containerThread);
#endif
    if (!threadQueue->isEmpty()) {
        t_SearchRequestEntry *searchRequest = threadQueue->front();
        threadQueue->pop_front();
        addSearchRequest(searchRequest->first, searchRequest->second);
        delete searchRequest;
    }
}

void McaSearchManagerDBus::removeFeedCleanup(const QString& upid) {
    // TODO: Assuming these are unique containers for unique feeds?
    FeedInfo *info = m_upidToFeedInfo[upid];
    if (!info)
        return;

    McaFeedAdapter *adapter = qobject_cast<McaFeedAdapter*>(info->feed);
    if (adapter) {
        foreach(McaSearchableContainer *container, m_searchableContainers) {
            if( container->feedModel() == adapter->getSource() ) {
                m_searchableContainers.removeOne( container );

                //remove search requests for the removed feed
                t_SearchRequestQueue *threadQueue = m_searchRequests[container->thread()];
                t_SearchRequestEntry *requestEntry = 0;
                int index = 0;
                while (index < threadQueue->count()) {
                    requestEntry = threadQueue->at(index);
                    if (requestEntry->first == container) {
                        threadQueue->removeAt(index);
                        delete requestEntry;
                    }
                    else {
                        index++;
                    }
                }
                container->deleteLater();
                break;
            }
        }
    }
    else {
        qWarning() << "McaSearchManagerDBus::removeFeedCleanup: Requesting removal of non-McaFeedAdapter in search";
    }
}

void McaSearchManagerDBus::addSearchRequest(McaSearchableContainer *container, const QString &searchText)
{
    qDebug() << "McaSearchManagerDBus::addSearchRequest " << container << searchText << m_currentRequest;
    QThread *containerThread = container->thread();
#ifdef THREADING
    if (m_processingRequests.contains(containerThread)) {
#else
    if(0 != m_currentRequest) {
#endif
        t_SearchRequestQueue *threadQueue = m_searchRequests.value(containerThread);
        t_SearchRequestEntry *requestEntry = 0;
        for(int index =0; index < threadQueue->count(); index++) {
            requestEntry = threadQueue->at(index);
            if(requestEntry->first == container) {
                threadQueue->removeOne(requestEntry);
                delete requestEntry;
            }
        }
        t_SearchRequestEntry *searchRequest = new t_SearchRequestEntry();
        searchRequest->first = container;
        searchRequest->second = searchText;
        threadQueue->push_back(searchRequest);
    }
    else {        
#ifdef THREADING
        QMetaObject::invokeMethod(container, "setSearchText", Q_ARG(QString, searchText));
        m_processingRequests.push_back(containerThread);
        container->searchable()->resetSearchHalt();
#else
        m_currentRequest = new t_SearchRequestEntry;
        m_currentRequest->first = container;
        m_currentRequest->second = searchText;
        m_searchTimer.start();
#endif
    }
}

#ifndef THREADING
void McaSearchManagerDBus::doNextSearch()
{
    if(0 == m_currentRequest) {
        qDebug() << "McaSearchManagerDBus::doNextSearch: ERROR m_currentRequest is 0";
        return;
    }

    McaSearchableContainer *container = m_currentRequest->first;
    QString searchText = m_currentRequest->second;
    qDebug() << "McaSearchManagerDBus::doNextSearch: " << container << searchText;
    delete m_currentRequest;
    m_currentRequest = 0;
    container->setSearchText(searchText);
}
#endif
