/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "defines.h"
#ifdef MEMORY_LEAK_DETECTOR
#include <base.h>
#endif

#include <QDebug>
#include "actions.h"

#ifdef MEMORY_LEAK_DETECTOR
#define __DEBUG_NEW__ new(__FILE__, __LINE__)
#define new __DEBUG_NEW__
#endif

McaActions::McaActions(QObject *parent):
        QObject(parent)
{
}

McaActions::~McaActions()
{
}

QStringList McaActions::customActions()
{
    return m_ids;
}

QStringList McaActions::customDisplayActions()
{
    return m_names;
}

void McaActions::addCustomAction(QString id, QString displayName)
{
    m_ids.append(id);
    m_names.append(displayName);
}

void McaActions::performStandardAction(QString action, QString uniqueid)
{
    emit standardAction(action, uniqueid);
}

void McaActions::performCustomAction(QString action, QString uniqueid)
{
    emit customAction(action, uniqueid);
}
