#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QSortFilterProxyModel>

#include "abstractmanager.h"
#include "aggregatedmodelproxy.h"
#include "feedmodel.h"
#include "servicemodel.h"
#include "dbustypes.h"
#include "dbusdefines.h"

#define PING_DAEMON_TIME 5

McaAbstractManager::McaAbstractManager(const QString &createMethodName, QObject *parent) :
    QObject(parent),
    m_dbusManagerInterface(0),
    m_dbusDaemonInterface(0),
    m_servicesConfigured(0),
    m_servicesEnabled(0),
    m_createMethodName(createMethodName),
    m_dbusServiceWatcher(CONTENT_DBUS_SERVICE, QDBusConnection::sessionBus(),
        QDBusServiceWatcher::WatchForUnregistration | QDBusServiceWatcher::WatchForRegistration)
{
    m_isOffline = true;
    m_firstTimeinitialized = false;
    registerDataTypes();

    connect(&m_dbusServiceWatcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(serviceRegistered(QString)));
    connect(&m_dbusServiceWatcher, SIGNAL(serviceUnregistered(QString)),
            this, SLOT(serviceUnregistered(QString)));

    connect(this, SIGNAL(offlineChanged(bool)), this, SLOT(serviceStateChangedBase(bool)));

    m_dbusModelProxy = new McaAggregatedModelProxy(CONTENT_DBUS_SERVICE);
    connect(m_dbusModelProxy, SIGNAL(frozenChanged(bool)),
            this, SIGNAL(frozenChanged(bool)));

    m_feedProxy = new QSortFilterProxyModel(this);
    m_feedProxy->setSourceModel(m_dbusModelProxy);
    m_feedProxy->setSortRole(McaFeedModel::RequiredTimestampRole);
    m_feedProxy->sort(0, Qt::DescendingOrder);
    m_feedProxy->setDynamicSortFilter(true);

    m_stillAliveTimer.setInterval(PING_DAEMON_TIME);
    connect(&m_stillAliveTimer, SIGNAL(timeout()), this, SLOT(pingDaemon()));
}

McaAbstractManager::~McaAbstractManager()
{
    if(0 == m_dbusDaemonInterface) return;

    QDBusReply<bool> reply = m_dbusDaemonInterface->call("release", QVariant(m_dbusManagerInterface->path()));
    qDebug() << "McaAbstractManager::~McaAbstractManager release " << m_dbusManagerInterface->path() << reply.value();

    delete m_dbusModelProxy;
    m_dbusModelProxy = 0;

    if(0 != m_dbusManagerInterface) {
        delete m_dbusManagerInterface;
        m_dbusManagerInterface = 0;
    }

    if(0 != m_dbusDaemonInterface) {
        delete m_dbusDaemonInterface;
        m_dbusDaemonInterface = 0;
    }
}

void McaAbstractManager::initialize(const QString& managerData)
{
    if(!m_firstTimeinitialized) {
        m_firstTimeinitialized = true;
        m_isOffline = !QDBusConnection::sessionBus().interface()->isServiceRegistered(CONTENT_DBUS_SERVICE);
        setOffline(m_isOffline);
    }

    if(0 == m_dbusManagerInterface) return;
    m_dbusManagerInterface->call("initialize", QVariant(managerData));
}

bool McaAbstractManager::frozen()
{
    return m_dbusModelProxy->frozen();
}

int McaAbstractManager::servicesConfigured()
{
    return m_servicesConfigured;
}

int McaAbstractManager::servicesEnabled()
{
    return m_servicesEnabled;
}

void McaAbstractManager::setFrozen(bool frozen)
{
    m_dbusModelProxy->setFrozen(frozen);
}

QSortFilterProxyModel *McaAbstractManager::feedModel()
{
    return m_feedProxy;
}

void McaAbstractManager::updateCounts() {
    int count = serviceModelRowCount();

    qDebug() << "McaAbstractManager::updateCounts() Pre-count:" << count;
    int configured = 0;
    int enabled = 0;
    for (int i=0; i<count; i++) {
        QModelIndex qmi = serviceModelIndex(i);
        if (!serviceModelData(qmi, McaServiceModel::CommonConfigErrorRole).toBool()) {
            configured++;
            if (dataChangedCondition(qmi))
                enabled++;
        }
    }

    if (m_servicesConfigured != configured) {
        m_servicesConfigured = configured;
        emit servicesConfiguredChanged(configured);
    }

    if (m_servicesEnabled != enabled) {
        m_servicesEnabled = enabled;
        emit servicesEnabledChanged(enabled);
    }

    qDebug() << "McaAbstractManager::updateCounts() " << enabled << configured;
}

void McaAbstractManager::serviceRegistered(const QString & serviceName)
{
    Q_UNUSED(serviceName);
    setOffline(false);
}

void McaAbstractManager::serviceUnregistered(const QString & serviceName)
{
    Q_UNUSED(serviceName);
    setOffline(true);
}

void McaAbstractManager::setOffline(bool offline)
{
    m_isOffline = offline;
    emit offlineChanged(m_isOffline);
}

bool McaAbstractManager::isOffline()
{
    return m_isOffline;
}

void McaAbstractManager::serviceStateChangedBase(bool offline)
{
    qDebug() << "McaAbstractManager::serviceStateChangedBase offline =" << offline;

    if(!offline) {
        if(0 == m_dbusDaemonInterface) {
            qDebug() << "DBUS: Creating daemon interface";
            m_dbusDaemonInterface = new QDBusInterface(CONTENT_DBUS_SERVICE, CONTENT_DBUS_OBJECT);
            if(!m_dbusDaemonInterface->isValid()) {
                qDebug() << "DBUS: Failed to connect to daemon";
                delete m_dbusDaemonInterface;
                m_dbusDaemonInterface = 0;
                serviceStateChangedBase(true);
                return;
            } else {
                qDebug() << "DBUS: Daemon interface created";
            }
            QDBusReply<QString> reply = m_dbusDaemonInterface->call(m_createMethodName);
            // TODO: Error check

            qDebug() << "DBUS: Creating manager interface";
            m_dbusManagerInterface = new QDBusInterface(CONTENT_DBUS_SERVICE, reply.value());
            if(!m_dbusManagerInterface->isValid()) {
                qDebug() << "DBUS: Failed to connect to manager";
                delete m_dbusDaemonInterface;
                m_dbusDaemonInterface = 0;
                delete m_dbusManagerInterface;
                m_dbusManagerInterface = 0;
                serviceStateChangedBase(true);
                return;
            } else {
                qDebug() << "DBUS: Manager interface created";
            }
            connect(m_dbusManagerInterface, SIGNAL(updateCounts()), this, SLOT(updateCounts()));

            reply = m_dbusManagerInterface->call("feedModelPath");
            qDebug() << "AgreagatedModel dbus path: " << reply.value() << reply.error().message();
            m_dbusModelProxy->setObjectPath(reply.value());
            m_dbusModelProxy->setOffline(offline);

            m_stillAliveTimer.start();
        }
    } else {
        if(0 != m_dbusDaemonInterface) {
            m_dbusModelProxy->setOffline(offline);
            delete m_dbusDaemonInterface;
            m_dbusDaemonInterface = 0;
            delete m_dbusManagerInterface;
            m_dbusManagerInterface = 0;
        }

        m_stillAliveTimer.stop();
    }

    serviceStateChanged(offline);

    if(offline) {
        // restart meego-ux-daemon
        QDBusConnection::sessionBus().interface()->startService(CONTENT_DBUS_SERVICE);
    }
}

void McaAbstractManager::pingDaemon()
{
    if(!isOffline()) { // should not get here if offline, but check anyway
        m_dbusManagerInterface->asyncCall("ping");
    }
}
