/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcaactionsproxy_h
#define __mcaactionsproxy_h

#include "actions.h"

#include <QStringList>
#include <QMetaType>

class McaActionsProxy: public McaActions
{
    Q_OBJECT
    // identifiers for custom actions (FUTURE)
    Q_PROPERTY(QStringList customActions READ customActions)
    // localized strings to show the user in a context menu (FUTURE)
    Q_PROPERTY(QStringList customDisplayActions READ customDisplayActions)

public:
    McaActionsProxy(McaActions *action, QObject *parent = 0);
    virtual ~McaActionsProxy();

    QStringList customActions();
    QStringList customDisplayActions();

    void addCustomAction(QString id, QString displayName);

signals:
    // standard actions are defined by the model
    void standardAction(QString action, QString uniqueid);
    void customAction(QString action, QString uniqueid);
    void proxyAddCustomAction(QString id, QString displayName);

public slots:
    // if you use the same actions object for more than one object, you should
    //   be prepared to potentially get called with a uniqueid of a row already
    //   removed from your model occasionally; you should either be able to
    //   still act on the removed content or gracefully throw away the action

    // standard actions are defined by the model
    void performStandardAction(QString action, QString uniqueid);
    void performCustomAction(QString action, QString uniqueid);
private:
    McaActions *m_action;
};

Q_DECLARE_METATYPE(McaActionsProxy*)

#endif  // __mcaactions_h
