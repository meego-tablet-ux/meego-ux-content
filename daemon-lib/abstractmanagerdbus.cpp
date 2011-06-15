#include "memoryleak.h"
#include "abstractmanagerdbus.h"

#include <QDebug>
#include <QUuid>
#include <QRegExp>
#include <QtDBus>

#include "feedmodel.h"
#include "servicemodel.h"
#include "feedmanager.h"
#include "aggregatedmodel.h"
#include "feedadapter.h"

#include "memoryleak-defines.h"

#define AGREGATEDMODEL_DBUS_NAME "/AggreagatedModel"

// generates unique ids to be used as dbus object paths
QString McaAbstractManagerDBus::generateUniqueId()
{
    QString id = QString("/") + QUuid::createUuid().toString();
    id.replace(QRegExp("[{}-]"),"");
    return id;
}

McaAbstractManagerDBus::McaAbstractManagerDBus(QObject *parent) :
    QObject(parent)
{
    m_dbusObjectId = generateUniqueId();

    QDBusConnection::sessionBus().registerObject(m_dbusObjectId, this, QDBusConnection::ExportAllContents);

    m_feedmgr = McaFeedManager::takeManager();
    m_aggregator = new McaAggregatedModel();
    QDBusConnection::sessionBus().registerObject(m_dbusObjectId + AGREGATEDMODEL_DBUS_NAME, m_aggregator, QDBusConnection::ExportAllContents);

    connect(m_feedmgr, SIGNAL(feedCreated(QObject*,McaFeedAdapter*,int)), this, SLOT(createFeedDone(QObject*,McaFeedAdapter*,int)), Qt::DirectConnection);
    connect(m_feedmgr, SIGNAL(createFeedError(QString,int)), this, SLOT(createFeedError(QString,int)));
}

McaAbstractManagerDBus::~McaAbstractManagerDBus()
{
    QDBusConnection::sessionBus().unregisterObject(m_dbusObjectId);

    QString modelObjectPath = m_dbusObjectId + AGREGATEDMODEL_DBUS_NAME;
    if(0 != QDBusConnection::sessionBus().objectRegisteredAt(modelObjectPath)) {
        QDBusConnection::sessionBus().unregisterObject(modelObjectPath);
    }

    m_requestIds.clear();
    McaFeedManager::releaseManager();
}

QString McaAbstractManagerDBus::feedModelPath()
{
    return m_dbusObjectId + AGREGATEDMODEL_DBUS_NAME;
}

QString McaAbstractManagerDBus::dbusObjectId()
{
    return m_dbusObjectId;
}

void McaAbstractManagerDBus::rowsAboutToBeRemoved(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index)

    for (int i=start; i<=end; i++) {
        QModelIndex qmi = serviceModelIndex(i);
        removeFeed(qmi);
    }

    emit updateCounts();
}

void McaAbstractManagerDBus::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
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

    emit updateCounts();
}

void McaAbstractManagerDBus::addFeed(const QModelIndex &index)
{
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(serviceModelData(index, McaAggregatedModel::SourceModelRole).value<QObject*>());
    QString name = serviceModelData(index, McaServiceModel::RequiredNameRole).toString();
    QString upid = m_feedmgr->serviceId(model, name);

    if (m_upidToFeedInfo.contains(upid)) {
        qWarning() << "warning: search manager: unexpected duplicate add feed request";
        return;
    }

    m_upidToFeedInfo.insert(upid, NULL);
    m_requestIds[createFeed(model, name)] = index.row();
}

void McaAbstractManagerDBus::removeFeed(const QModelIndex &index)
{
    QAbstractListModel *model = qobject_cast<QAbstractListModel*>(serviceModelData(index, McaAggregatedModel::SourceModelRole).value<QObject*>());
    QString name = serviceModelData(index, McaServiceModel::RequiredNameRole).toString();
    QString upid = m_feedmgr->serviceId(model, name);

    FeedInfo *info = m_upidToFeedInfo.value(upid, NULL);
    if (info) {
        m_aggregator->removeSourceModel(info->feed);
        removeFeedCleanup(upid);
        m_upidToFeedInfo.remove(upid);
        info->feed->deleteLater();
        delete info;
    }
}

void McaAbstractManagerDBus::removeAllFeeds()
{
    QHashIterator<QString, FeedInfo *> iter(m_upidToFeedInfo);
    while( iter.hasNext() ) {
        iter.next();
        FeedInfo *info = iter.value();
        QString upid = iter.key();
        if(info) {
            m_aggregator->removeSourceModel(info->feed);
            removeFeedCleanup(upid);
            info->feed->deleteLater();
            delete info;
        }
    }
    m_upidToFeedInfo.clear();
}

void McaAbstractManagerDBus::rowsInserted(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(index)

    for (int i=start; i<=end; i++) {
        QModelIndex qmi = serviceModelIndex(i);
            if (serviceModelData(qmi, McaServiceModel::CommonConfigErrorRole).toBool() || !dataChangedCondition(qmi)) {
            continue;
        }
        addFeed(qmi);
    }

    emit updateCounts();
}

void McaAbstractManagerDBus::createFeedDone(QObject *containerObj, McaFeedAdapter *feedAdapter, int uniqueRequestId) {
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
        createFeedFinalize(containerObj, feedAdapter, info);
        m_aggregator->addSourceModel(feedAdapter);
        qDebug() << "McaAbstractManagerDBus::createFeedDone " << name;
    }
}

void McaAbstractManagerDBus::createFeedError(QString serviceName, int uniqueRequestId) {
    if(m_requestIds.contains(uniqueRequestId)) {
        qDebug() << "CREATE Feed Error " << serviceName << " with request id " << uniqueRequestId;
        m_requestIds.remove(uniqueRequestId);
        //TODO: any aditional cleanup on feed creation error
    }
}
