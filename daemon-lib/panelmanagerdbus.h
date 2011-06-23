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

#include "searchmanagerdbus.h"

class McaAllocator;
class McaServiceProxy;

class McaPanelManagerDBus: public McaAbstractManagerDBus
{
    Q_OBJECT

public:
    McaPanelManagerDBus(QObject *parent = NULL);
    virtual ~McaPanelManagerDBus();

    Q_INVOKABLE void initialize(const QString& managerData);

    // property functions - already accessible to QML
    void setServicesEnabledByDefault(bool enabled);
    virtual QSortFilterProxyModel *serviceModel();

    Q_INVOKABLE bool isServiceEnabled(const QString& upid);
    Q_INVOKABLE void setServiceEnabled(const QString& upid, bool enabled);

    Q_INVOKABLE void clearAllHistory(const QDateTime& datetime = QDateTime::currentDateTime());
    Q_INVOKABLE void clearHistory(const QString& upid,
                                  const QDateTime& datetime = QDateTime::currentDateTime());

    // temporary
    virtual void addDirectFeed(QAbstractItemModel *feed);
    virtual void removeDirectFeed(QAbstractItemModel *feed);

signals:
    void serviceEnabledChanged(const QString& upid, bool enabled);
    void servicesConfiguredChanged(int configured);

public slots:
    void setCategories(const QStringList& categories);
    virtual QString serviceModelPath();

protected:
    QString fullEnabledKey();

private:
    virtual QModelIndex serviceModelIndex(int row);
    virtual int serviceModelRowCount();
    virtual QVariant serviceModelData(const QModelIndex &index, int role);
//    virtual QVariant serviceModelData(int row, int role);
    virtual bool dataChangedCondition(const QModelIndex &index);
//    virtual bool dataChangedCondition(int row);

    virtual int createFeed(const QAbstractItemModel *serviceModel, const QString& name);
    virtual void removeFeedCleanup(const QString& upid);
    virtual void createFeedFinalize(QObject *containerObj, McaFeedAdapter *feedAdapter, FeedInfo *feedInfo);

private:
    McaAllocator *m_allocator;
    McaServiceProxy *m_serviceProxy;

    QString m_panelName;
    bool m_servicesEnabledByDefault;

    QHash<QString, bool> m_upidToEnabled;  // map from upid to enabled status
};

#endif  // __mcapanelmanager_h
