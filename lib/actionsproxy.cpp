#include "actionsproxy.h"

McaActionsProxy::McaActionsProxy(QObject *parent) :
    QObject(parent)
{
}

QStringList McaActionsProxy::customActions()
{
    return m_ids;
}

QStringList McaActionsProxy::customDisplayActions()
{
    return m_names;
}

void McaActionsProxy::performStandardAction(QString action, QString uniqueid)
{
    emit standardAction(action, uniqueid);
}

void McaActionsProxy::performCustomAction(QString action, QString uniqueid)
{
    emit customAction(action, uniqueid);
}


void McaActionsProxy::setCustomActions(const QStringList &ids, const QStringList &names)
{
    m_ids = ids;
    m_names = names;
}

void McaActionsProxy::setServiceUpid(const QString &upid)
{
    m_serviceUpid = upid;
}

QString McaActionsProxy::getServiceUpid()
{
    return m_serviceUpid;
}
