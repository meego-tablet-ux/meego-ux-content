#ifndef ABSTRACTMANAGER_H
#define ABSTRACTMANAGER_H

#include <QObject>
#include <QModelIndex>

class McaFeedManager;
class McaFeedCache;
class McaAggregatedModel;
class McaFeedFilter;
class McaFeedAdapter;
class McaAdapter;

class QSortFilterProxyModel;

struct FeedInfo
{
    QString upid;
    McaAdapter *feed;
    McaFeedFilter *filter;
};

class McaAbstractManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QSortFilterProxyModel *feedModel READ feedModel)
    Q_PROPERTY(bool frozen READ frozen WRITE setFrozen NOTIFY frozenChanged)

public:
    McaAbstractManager(QObject *parent = 0);
    virtual ~McaAbstractManager();

    //managerData will be panelName for PanelManager, searchText for SearchManager
    Q_INVOKABLE virtual void initialize(const QString& managerData = QString()) = 0;

    virtual bool frozen();
    virtual QSortFilterProxyModel *feedModel();

protected:
    virtual void addFeed(const QModelIndex &index);
    virtual void removeFeed(const QModelIndex &index);
    virtual QString fullEnabledKey() { return ""; }
    void removeAllFeeds();

signals:
    void frozenChanged(bool frozen);

public slots:
    virtual void setFrozen(bool frozen);

protected slots:
    virtual void rowsInserted(const QModelIndex &index, int start, int end);
    virtual void rowsAboutToBeRemoved(const QModelIndex &index, int start, int end);
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    virtual void createFeedDone(QObject *containerObj, McaFeedAdapter *feedAdapter, int uniqueRequestId);
    virtual void removeFeedCleanup(const QString& upid) = 0;
    void createFeedError(QString serviceName, int uniqueRequestId);

private:
    virtual QModelIndex serviceModelIndex(int row) = 0;
    virtual QVariant serviceModelData(const QModelIndex& index, int role) = 0;
    virtual bool dataChangedCondition(const QModelIndex& index) = 0;

    virtual int createFeed(const QAbstractItemModel *serviceModel, const QString& name) = 0;
    virtual void createFeedFinalize(QObject *containerObj, McaFeedAdapter *feedAdapter, FeedInfo *feedInfo) = 0;

protected:
    McaFeedManager *m_feedmgr;
    McaFeedCache *m_cache;
    QSortFilterProxyModel *m_feedProxy;
    McaAggregatedModel *m_aggregator;
    QHash<QString, FeedInfo *> m_upidToFeedInfo;
    QMap<int, int> m_requestIds;
};

#endif // ABSTRACTMANAGER_H
