/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcapanelmanager_h
#define __mcapanelmanager_h

#include <QModelIndex>
#include <QStringList>
#include <QDateTime>

class QSortFilterProxyModel;
class McaAllocator;
class McaFeedManager;
class McaFeedCache;
class McaAggregatedModel;
class McaServiceProxy;
class FeedInfo;

class McaPanelManager: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QSortFilterProxyModel *serviceModel READ serviceModel)
    Q_PROPERTY(QSortFilterProxyModel *feedModel READ feedModel)
    Q_PROPERTY(QStringList categories READ categories WRITE setCategories NOTIFY categoriesChanged)
    Q_PROPERTY(bool frozen READ frozen WRITE setFrozen NOTIFY frozenChanged)
    Q_PROPERTY(bool servicesConfigured READ servicesConfigured NOTIFY servicesConfiguredChanged)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)
    Q_PROPERTY(bool servicesEnabledByDefault READ servicesEnabledByDefault WRITE setServicesEnabledByDefault)

public:
    McaPanelManager(QObject *parent = NULL);
    virtual ~McaPanelManager();

    Q_INVOKABLE void initialize(const QString& panelName = QString());

    // property functions - already accessible to QML
    virtual QStringList categories();
    virtual bool frozen();
    virtual bool servicesConfigured();
    virtual bool isEmpty();
    bool servicesEnabledByDefault();
    void setServicesEnabledByDefault(bool enabled);
    virtual QSortFilterProxyModel *serviceModel();
    virtual QSortFilterProxyModel *feedModel();

    Q_INVOKABLE bool isServiceEnabled(const QString& upid);
    Q_INVOKABLE void setServiceEnabled(const QString& upid, bool enabled);

    Q_INVOKABLE void clearHistory(const QString& upid,
                                  const QDateTime& datetime = QDateTime::currentDateTime());

    // temporary
    virtual void addDirectFeed(QAbstractItemModel *feed);
    virtual void removeDirectFeed(QAbstractItemModel *feed);

signals:
    void categoriesChanged(const QStringList& categories);
    void frozenChanged(bool frozen);
    void servicesConfiguredChanged(bool servicesConfigured);
    void serviceEnabledChanged(const QString& upid, bool enabled);
    void isEmptyChanged(bool isEmpty);

public slots:
    void setCategories(const QStringList& categories);
    void setFrozen(bool frozen);

protected slots:
    void rowsInserted(const QModelIndex &index, int start, int end);
    void rowsAboutToBeRemoved(const QModelIndex &index, int start, int end);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void feedRowsChanged();

protected:
    void addFeed(const QModelIndex &index);
    void removeFeed(const QModelIndex &index);
    QString fullEnabledKey();

private:
    McaAllocator *m_allocator;
    McaFeedManager *m_feedmgr;
    McaServiceProxy *m_serviceProxy;
    QSortFilterProxyModel *m_feedProxy;
    McaAggregatedModel *m_aggregator;
    McaFeedCache *m_cache;
    bool m_isEmpty;

    QString m_panelName;
    bool m_servicesEnabledByDefault;

    QHash<QString, FeedInfo *> m_upidToFeedInfo;
    QHash<QString, bool> m_upidToEnabled;  // map from upid to enabled status
};

#endif  // __mcapanelmanager_h
