#ifndef ABSTRACTMANAGERPROXY_H
#define ABSTRACTMANAGERPROXY_H

#include <QDBusInterface>
#include <QModelIndex>
#include <QSortFilterProxyModel>

//class McaFeedCache;
class McaAggregatedModelProxy;

class McaAbstractManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QSortFilterProxyModel *feedModel READ feedModel)
    Q_PROPERTY(bool frozen READ frozen WRITE setFrozen NOTIFY frozenChanged)

    // this is the number of services that are configured, i.e. could be instantiated
    Q_PROPERTY(int servicesConfigured READ servicesConfigured NOTIFY servicesConfiguredChanged)

    // this is the number of enabled, instantiated feeds, i.e. <= servicesConfigured
    Q_PROPERTY(int servicesEnabled READ servicesEnabled NOTIFY servicesEnabledChanged)

protected:
    McaAbstractManager(const QString &createMethodName, QObject *parent = 0);
    virtual ~McaAbstractManager();

signals:
    void frozenChanged(bool frozen);
    void servicesConfiguredChanged(int servicesConfigured);
    void servicesEnabledChanged(int servicesEnabled);

public slots:
    virtual void initialize(const QString& managerData = QString());

    QSortFilterProxyModel *feedModel();
    virtual bool frozen();
    virtual int servicesConfigured();
    virtual int servicesEnabled();
    virtual void setFrozen(bool frozen);

protected slots:
    virtual void updateCounts();

protected:
    int serviceModelRowCount();
    QVariant serviceModelData(int row, int role);
    bool dataChangedCondition(int row);

protected:
    QDBusInterface *m_dbusManagerInterface;
    QDBusInterface *m_dbusDaemonInterface;

//    McaFeedCache *m_cache;
    QSortFilterProxyModel *m_feedProxy;
    McaAggregatedModelProxy *m_dbusModelProxy;

    int m_servicesConfigured;
    int m_servicesEnabled;
};

#endif // ABSTRACTMANAGERPROXY_H
