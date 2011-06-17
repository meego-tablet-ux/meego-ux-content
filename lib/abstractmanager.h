#ifndef ABSTRACTMANAGERPROXY_H
#define ABSTRACTMANAGERPROXY_H

#include <QDBusInterface>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QDBusServiceWatcher>

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
    void offlineChanged(bool offline);

public slots:
    virtual void initialize(const QString& managerData = QString());

    QSortFilterProxyModel *feedModel();
    virtual bool frozen();
    virtual int servicesConfigured();
    virtual int servicesEnabled();
    virtual void setFrozen(bool frozen);

protected slots:
    virtual void updateCounts();

    void serviceRegistered(const QString & serviceName);
    void serviceUnregistered(const QString & serviceName);

    virtual void serviceStateChangedBase(bool offline);

protected:
    int serviceModelRowCount();
    QVariant serviceModelData(int row, int role);
    bool dataChangedCondition(int row);
    bool isOffline();
    virtual void serviceStateChanged(bool offline) = 0;

private:
    void setOffline(bool offline);

protected:
    QDBusInterface *m_dbusManagerInterface;
    QDBusInterface *m_dbusDaemonInterface;

    QSortFilterProxyModel *m_feedProxy;
    McaAggregatedModelProxy *m_dbusModelProxy;

    int m_servicesConfigured;
    int m_servicesEnabled;

    bool m_isOffline;
    QString m_createMethodName;
    QDBusServiceWatcher m_dbusServiceWatcher;

    bool m_firstTimeinitialized;
};

#endif // ABSTRACTMANAGERPROXY_H
