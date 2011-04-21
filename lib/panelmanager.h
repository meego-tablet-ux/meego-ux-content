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

#include "abstractmanager.h"

class McaAllocator;
class McaServiceProxy;

class McaPanelManager: public McaAbstractManager
{
    Q_OBJECT
    Q_PROPERTY(QSortFilterProxyModel *serviceModel READ serviceModel)
    Q_PROPERTY(QStringList categories READ categories WRITE setCategories NOTIFY categoriesChanged)
    Q_PROPERTY(bool servicesConfigured READ servicesConfigured NOTIFY servicesConfiguredChanged)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)
    Q_PROPERTY(bool servicesEnabledByDefault READ servicesEnabledByDefault WRITE setServicesEnabledByDefault)

public:
    McaPanelManager(QObject *parent = NULL);
    virtual ~McaPanelManager();

    Q_INVOKABLE void initialize(const QString& managerData);

    // property functions - already accessible to QML
    virtual QStringList categories();
    virtual bool servicesConfigured();
    virtual bool isEmpty();
    bool servicesEnabledByDefault();
    void setServicesEnabledByDefault(bool enabled);
    virtual QSortFilterProxyModel *serviceModel();

    Q_INVOKABLE bool isServiceEnabled(const QString& upid);
    Q_INVOKABLE void setServiceEnabled(const QString& upid, bool enabled);

    Q_INVOKABLE void clearHistory(const QString& upid,
                                  const QDateTime& datetime = QDateTime::currentDateTime());

    // temporary
    virtual void addDirectFeed(QAbstractItemModel *feed);
    virtual void removeDirectFeed(QAbstractItemModel *feed);

signals:
    void categoriesChanged(const QStringList& categories);
    void servicesConfiguredChanged(bool servicesConfigured);
    void serviceEnabledChanged(const QString& upid, bool enabled);
    void isEmptyChanged(bool isEmpty);

public slots:
    void setCategories(const QStringList& categories);

protected slots:
    void feedRowsChanged();
    //void createFeedDone(QObject *containerObj, McaFeedAdapter *feedAdapter, int uniqueRequestId);

protected:
    QString fullEnabledKey();

private:
    virtual QModelIndex serviceModelIndex(int row);
    virtual QVariant serviceModelData(const QModelIndex& index, int role);
    virtual bool dataChangedCondition(const QModelIndex& index);

    virtual int createFeed(const QAbstractItemModel *serviceModel, const QString& name);
    virtual void removeFeedCleanup(const QString& upid);
    virtual void createFeedFinalize(QObject *containerObj, McaFeedAdapter *feedAdapter, FeedInfo *feedInfo);

private:
    McaAllocator *m_allocator;
    McaServiceProxy *m_serviceProxy;
    bool m_isEmpty;

    QString m_panelName;
    bool m_servicesEnabledByDefault;

    QHash<QString, bool> m_upidToEnabled;  // map from upid to enabled status
};

#endif  // __mcapanelmanager_h
