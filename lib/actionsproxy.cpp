/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>

#include "actionsproxy.h"
#include "actions.h"

McaActionsProxy::McaActionsProxy(McaActions *action, QObject *parent) :
         McaActions(parent), m_action(action)
{
    connect(this, SIGNAL(proxyAddCustomAction(QString,QString)),
            m_action, SLOT(addCustomAction(QString,QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(standardAction(QString,QString)),
            m_action, SLOT(performStandardAction(QString,QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(customAction(QString,QString)),
            m_action, SLOT(performCustomAction(QString,QString)), Qt::QueuedConnection);
}

McaActionsProxy::~McaActionsProxy()
{
}

QStringList McaActionsProxy::customActions()
{
    // TODO: These two might cause deadlocks in the ui thread, if the thread the Action exists
    // in has a deadlock.
    QStringList list;
    QMetaObject::invokeMethod(m_action, "customActions", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QStringList, list));
    return list;
}

QStringList McaActionsProxy::customDisplayActions()
{
    // TODO: These two might cause deadlocks in the ui thread, if the thread the Action exists
    // in has a deadlock.
    QStringList list;
    QMetaObject::invokeMethod(m_action, "customDisplayActions", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QStringList, list));
    return list;
}

void McaActionsProxy::addCustomAction(QString id, QString displayName)
{
    emit proxyAddCustomAction(id, displayName);
}

void McaActionsProxy::performStandardAction(QString action, QString uniqueid)
{
    qDebug() << "Proxy standard action: " << action << uniqueid;
    emit standardAction(action, uniqueid);
}

void McaActionsProxy::performCustomAction(QString action, QString uniqueid)
{
    qDebug() << "Proxy custom action: " << action << uniqueid;
    emit customAction(action, uniqueid);
}
