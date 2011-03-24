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

class QSortFilterProxyModel;
class McaFeedManager;
class McaFeedCache;
class McaAggregatedModel;
class FeedInfo;
class McaSearchableContainer;

class McaSearchManager: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QSortFilterProxyModel *feedModel READ feedModel)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(bool frozen READ frozen WRITE setFrozen NOTIFY frozenChanged)

public:
    McaSearchManager(QObject *parent = NULL);
    virtual ~McaSearchManager();

    Q_INVOKABLE void initialize(const QString& searchText);

    virtual QString searchText();
    virtual bool frozen();
    virtual QSortFilterProxyModel *feedModel();

signals:
    void searchTextChanged(const QString& searchText);
    void frozenChanged(bool frozen);

public slots:
    void setSearchText(const QString& searchText);
    void setFrozen(bool frozen);

protected slots:
    void rowsInserted(const QModelIndex &index, int start, int end);
    void rowsAboutToBeRemoved(const QModelIndex &index, int start, int end);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    void createFeedDone(McaSearchableContainer *container, int uniqueRequestId);

protected:
    void addFeed(const QModelIndex &index);
    void removeFeed(const QModelIndex &index);
    QString fullEnabledKey();

private:
    McaFeedManager *m_feedmgr;
    McaFeedCache *m_cache;
    QSortFilterProxyModel *m_feedProxy;
    McaAggregatedModel *m_aggregator;
    QAbstractItemModel *m_serviceModel;

    QString m_searchText;
    QHash<QString, FeedInfo*> m_upidToFeedInfo;
    QMap<int, int> m_requestIds;
};

#endif  // __mcasearchmanager_h
