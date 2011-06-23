/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcasearchmanager_h
#define __mcasearchmanager_h

#include <QModelIndex>
#include <QStringList>
#include <QQueue>
#include <QPair>

#include "abstractmanagerdbus.h"

class McaSearchableContainer;


class McaSearchManagerDBus: public McaAbstractManagerDBus
{
    Q_OBJECT

    typedef QPair<McaSearchableContainer*, QString> t_SearchRequestEntry;
    typedef QQueue<t_SearchRequestEntry*> t_SearchRequestQueue;
public:
    McaSearchManagerDBus(QObject *parent = NULL);
    virtual ~McaSearchManagerDBus();

    Q_INVOKABLE void initialize(const QString& managerData);

    virtual QString searchText();

public slots:
    void setSearchText(const QString& searchText);

protected slots:
//    void createFeedDone(QObject *containerObj, McaFeedAdapter *feedAdapter, int uniqueRequestId);
    void searchDone();
    QString serviceModelPath();

private:
    virtual QModelIndex serviceModelIndex(int row);
    virtual int serviceModelRowCount();
    virtual QVariant serviceModelData(const QModelIndex &index, int role);
//    virtual QVariant serviceModelData(int row, int role);
    virtual bool dataChangedCondition(const QModelIndex &index);
//    virtual bool dataChangedCondition(int row);

    virtual int createFeed(const QAbstractItemModel *serviceModel, const QString& name);
    virtual void createFeedFinalize(QObject *containerObj, McaFeedAdapter *feedAdapter, FeedInfo *feedInfo);

    void addSearchRequest(McaSearchableContainer *container, const QString &searchText);
    virtual void removeFeedCleanup(const QString& upid);

private:
    QAbstractItemModel *m_serviceModel;
    QString m_searchText;

    QMap<QThread*, t_SearchRequestQueue*> m_searchRequests;
    QList<QThread*> m_processingRequests;
    QList<McaSearchableContainer*> m_searchableContainers;
};

#endif  // __mcasearchmanager_h
