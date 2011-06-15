#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QSortFilterProxyModel>

#include "abstractmanagerproxy.h"
#include "aggregatedmodelproxy.h"
#include "feedcache.h"
#include "feedmodel.h"
#include "servicemodel.h"
#include "dbustypes.h"

McaAbstractManagerProxy::McaAbstractManagerProxy(const QString &createMethodName, QObject *parent) :
    QObject(parent), m_servicesConfigured(0), m_servicesEnabled(0)
{
    registerDataTypes();

    m_dbusDaemonInterface = new QDBusInterface("com.meego.content", "/meegouxcontent");
    QDBusReply<QString> reply = m_dbusDaemonInterface->call(createMethodName);

    m_cache = new McaFeedCache(this);
    connect(m_cache, SIGNAL(frozenChanged(bool)),
            this, SIGNAL(frozenChanged(bool)));

    m_dbusManagerInterface = new QDBusInterface("com.meego.content", reply.value());
    connect(m_dbusManagerInterface, SIGNAL(updateCounts()), this, SLOT(updateCounts()));

    reply = m_dbusManagerInterface->call("feedModelPath");
    qDebug() << "AgreagatedModel dbus path: " << reply.value() << reply.error().message();
    m_dbusModelProxy = new McaAggregatedModelProxy("com.meego.content", reply.value());
    m_cache->setSourceModel(m_dbusModelProxy);

    m_feedProxy = new QSortFilterProxyModel(this);
    m_feedProxy->setSourceModel(m_cache);
    m_feedProxy->setSortRole(McaFeedModel::RequiredTimestampRole);
    m_feedProxy->sort(0, Qt::DescendingOrder);
    m_feedProxy->setDynamicSortFilter(true);

}

McaAbstractManagerProxy::~McaAbstractManagerProxy()
{
    if(0 == m_dbusDaemonInterface) return;

    QDBusReply<bool> reply = m_dbusDaemonInterface->call("release", QVariant(m_dbusManagerInterface->path()));
    qDebug() << "McaAbstractManagerProxy::~McaAbstractManagerProxy release " << m_dbusManagerInterface->path() << reply.value();

    if(0 != m_cache) {
        delete m_cache;
        m_cache = 0;
    }

    delete m_dbusModelProxy;
    m_dbusModelProxy = 0;
    delete m_dbusManagerInterface;
    m_dbusManagerInterface = 0;
    delete m_dbusDaemonInterface;
    m_dbusDaemonInterface = 0;
}

void McaAbstractManagerProxy::initialize(const QString& managerData)
{
    if(0 == m_dbusManagerInterface) return;
    m_dbusManagerInterface->call("initialize", QVariant(managerData));
}

bool McaAbstractManagerProxy::frozen()
{
    return m_cache->frozen();
}

int McaAbstractManagerProxy::servicesConfigured()
{
    return m_servicesConfigured;
}

int McaAbstractManagerProxy::servicesEnabled()
{
    return m_servicesEnabled;
}

void McaAbstractManagerProxy::setFrozen(bool frozen)
{
    m_cache->setFrozen(frozen);
}

QSortFilterProxyModel * McaAbstractManagerProxy::feedModel()
{
    return m_feedProxy;
}

void McaAbstractManagerProxy::updateCounts() {
    int count = serviceModelRowCount();

    int configured = 0;
    int enabled = 0;
    for (int i=0; i<count; i++) {
//        QModelIndex qmi = serviceModelIndex(i);
        if (!serviceModelData(i, McaServiceModel::CommonConfigErrorRole).toBool()) {
            configured++;
            if (dataChangedCondition(i))
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
}

int McaAbstractManagerProxy::serviceModelRowCount()
{
    if(0 == m_dbusManagerInterface) return 0;
    QDBusReply<int> reply = m_dbusManagerInterface->call("serviceModelRowCount");
    return reply.value();
}

QVariant McaAbstractManagerProxy::serviceModelData(int row, int role)
{
    if(0 == m_dbusManagerInterface) return QVariant();
    QDBusReply<QVariant> reply = m_dbusManagerInterface->call("serviceModelData", QVariant(row), QVariant(role));
    return reply.value();
}

bool McaAbstractManagerProxy::dataChangedCondition(int row)
{
    if(0 == m_dbusManagerInterface) return false;
    QDBusReply<bool> reply = m_dbusManagerInterface->call("dataChangedCondition", QVariant(row));
    return reply.value();
}
