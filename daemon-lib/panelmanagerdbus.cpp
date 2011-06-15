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

#include "allocator.h"
#include "feedmanager.h"
#include "servicemodel.h"
#include "aggregatedmodel.h"
#include "panelmanagerdbus.h"
#include "feedfilter.h"
#include "feedmodel.h"
#include "serviceproxy.h"
#include "settings.h"

#include "memoryleak-defines.h"

//
// Overview of McaPanelManagerDBus
//    - assembles pipeline to transform raw feeds (McaFeedModels from plugins)
//      - with hidden rows filtered out (McaFeedFilter)
//      - with a limited number of rows exposed (McaFeedAdapter)
//        - to a limit assigned by an allocation algorithm (McaAllocator)
//      - aggregated into one list model (McaAggregatedModel)
//      - with the ability to cache and freeze changes (McaFeedCache)
//    - responds to changes in the service model by adding or removing feeds
//

//struct FeedInfo
//{
//    QString upid;
//    QAbstractListModel *feed;
//    McaFeedFilter *filter;
//};

//
// public methods
//

McaPanelManagerDBus::McaPanelManagerDBus(QObject *parent):
        McaAbstractManagerDBus(parent)
{
    m_allocator = new McaAllocator;
    m_isEmpty = false;
    m_serviceProxy = new McaServiceProxy(this, m_feedmgr->serviceModel(), this);
    m_servicesEnabledByDefault = true;
}

McaPanelManagerDBus::~McaPanelManagerDBus()
{
    removeAllFeeds();

    if(m_allocator) {
        delete m_allocator;
    }
}

void McaPanelManagerDBus::initialize(const QString& managerData)
{
    m_panelName = managerData;
    m_allocator->setPanelName(m_panelName);

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

    qDebug() << "TODO: move this to McaPanelManager";
//    connect(m_feedProxy, SIGNAL(rowsInserted(QModelIndex,int,int)),
//            this, SLOT(feedRowsChanged()));
//    connect(m_feedProxy, SIGNAL(rowsRemoved(QModelIndex,int,int)),
//            this, SLOT(feedRowsChanged()));
}

QStringList McaPanelManagerDBus::categories()
{
    return m_serviceProxy->categories();
}

bool McaPanelManagerDBus::isEmpty()
{
    return m_isEmpty;
}

bool McaPanelManagerDBus::servicesEnabledByDefault()
{
    return m_servicesEnabledByDefault;
}

void McaPanelManagerDBus::setServicesEnabledByDefault(bool enabled)
{
    m_servicesEnabledByDefault = enabled;
}

QSortFilterProxyModel *McaPanelManagerDBus::serviceModel()
{
    return m_serviceProxy;
}

bool McaPanelManagerDBus::isServiceEnabled(const QString& upid)
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

void McaPanelManagerDBus::setServiceEnabled(const QString& upid, bool enabled)
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

void McaPanelManagerDBus::clearAllHistory(const QDateTime &datetime)
{
    foreach (FeedInfo *info, m_upidToFeedInfo.values()) {
        if (info)
            info->filter->clearHistory(datetime);
    }
}

void McaPanelManagerDBus::clearHistory(const QString &upid, const QDateTime &datetime)
{
    FeedInfo *info = m_upidToFeedInfo.value(upid, NULL);
    if (info)
        info->filter->clearHistory(datetime);
    else
        qWarning() << "no matching feed while attempting to clear history";
}

void McaPanelManagerDBus::addDirectFeed(QAbstractItemModel *feed)
{
    m_aggregator->addSourceModel(feed);
}

void McaPanelManagerDBus::removeDirectFeed(QAbstractItemModel *feed)
{
    m_aggregator->removeSourceModel(feed);
}

//
// public slots
//

void McaPanelManagerDBus::setCategories(const QStringList &categories)
{
    m_serviceProxy->setCategories(categories);
}

QModelIndex McaPanelManagerDBus::serviceModelIndex(int row)
{
    return m_serviceProxy->index(row, 0);
}

int McaPanelManagerDBus::serviceModelRowCount()
{
    return m_serviceProxy->rowCount();
}

QVariant McaPanelManagerDBus::serviceModelData(const QModelIndex &index, int role)
{
    return m_serviceProxy->data(index, role);
}

QVariant McaPanelManagerDBus::serviceModelData(int row, int role)
{
    return serviceModelData(serviceModelIndex(row), role);
}

bool McaPanelManagerDBus::dataChangedCondition(const QModelIndex &index)
{
    return m_serviceProxy->data(index, McaServiceProxy::SystemEnabledRole).toBool();
}

bool McaPanelManagerDBus::dataChangedCondition(int row)
{
    return dataChangedCondition(serviceModelIndex(row));
}

void McaPanelManagerDBus::feedRowsChanged()
{
    qDebug() << "TODO: move this to McaPanelManager";
//    bool empty = m_feedProxy->rowCount() == 0;

//    if (empty != m_isEmpty) {
//        m_isEmpty = empty;
//        emit isEmptyChanged(empty);
//    }
}

int McaPanelManagerDBus::createFeed(const QAbstractItemModel *serviceModel, const QString& name)
{
    return m_feedmgr->createFeed(serviceModel, name);
}

//
// protected methods
//

void McaPanelManagerDBus::removeFeedCleanup(const QString& upid) {
    qDebug() << "TODO: move this to McaPanelManager";
//    if (m_upidToFeedInfo.count() == 0)
//        emit servicesConfiguredChanged(false);
//    m_allocator->removeFeed(upid);
}

QString McaPanelManagerDBus::fullEnabledKey()
{
    // returns: the enabled key prefix with the suffix for this panel, if any
    QString key = McaSettings::KeyEnabledPrefix;
    if (!m_panelName.isEmpty())
        key.append(QString("-") + m_panelName);
    return key;
}

void McaPanelManagerDBus::createFeedFinalize(QObject *containerObj, McaFeedAdapter *feedAdapter, FeedInfo *feedInfo)
{
    Q_UNUSED(containerObj);
    feedInfo->filter = qobject_cast<McaFeedFilter*>(feedAdapter->getSource());
    feedInfo->filter->setPanelName(m_panelName);

    qDebug() << "TODO: move this to McaPanelManager";
//    if (m_upidToFeedInfo.count() == 1) {
//        emit servicesConfiguredChanged(true);
//    }

    m_allocator->addFeed(feedInfo->upid, feedAdapter);
}
