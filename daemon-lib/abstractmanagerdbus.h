#ifndef ABSTRACTMANAGER_H
#define ABSTRACTMANAGER_H

#include <QObject>
#include <QModelIndex>

class McaFeedManager;
//class McaFeedCache;
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

class McaAbstractManagerDBus : public QObject
{
    Q_OBJECT

public:
    McaAbstractManagerDBus(QObject *parent = 0);
    virtual ~McaAbstractManagerDBus();

    //managerData will be panelName for PanelManagerDBus, searchText for SearchManagerDBus
    Q_INVOKABLE virtual void initialize(const QString& managerData = QString()) = 0;

    QString dbusObjectId();

public slots:
    QString feedModelPath();

protected:
    virtual void addFeed(const QModelIndex &index);
    virtual void removeFeed(const QModelIndex &index);
    virtual QString fullEnabledKey() { return ""; }
    void removeAllFeeds();

    static QString generateUniqueId();

signals:
    void updateCounts();

protected slots:
    virtual void rowsInserted(const QModelIndex &index, int start, int end);
    virtual void rowsAboutToBeRemoved(const QModelIndex &index, int start, int end);
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    virtual void createFeedDone(QObject *containerObj, McaFeedAdapter *feedAdapter, int uniqueRequestId);
    virtual void removeFeedCleanup(const QString& upid) = 0;
    void createFeedError(QString serviceName, int uniqueRequestId);

private:
    virtual QModelIndex serviceModelIndex(int row) = 0;
    virtual int serviceModelRowCount() = 0;
    virtual QVariant serviceModelData(const QModelIndex &index, int role) = 0;
    virtual QVariant serviceModelData(int row, int role) = 0;
    virtual bool dataChangedCondition(const QModelIndex &index) = 0;
    virtual bool dataChangedCondition(int row) = 0;

    virtual int createFeed(const QAbstractItemModel *serviceModel, const QString& name) = 0;
    virtual void createFeedFinalize(QObject *containerObj, McaFeedAdapter *feedAdapter, FeedInfo *feedInfo) = 0;

protected:
    McaFeedManager *m_feedmgr;
    McaAggregatedModel *m_aggregator;
    QHash<QString, FeedInfo *> m_upidToFeedInfo;
    QMap<int, int> m_requestIds;
    QString m_dbusObjectId;
};

#endif // ABSTRACTMANAGER_H
