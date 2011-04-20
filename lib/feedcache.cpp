/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "memoryleak.h"

#include <QDebug>

#include "feedcache.h"
#include "feedadapter.h"
#include "feedmodel.h"
#include "actions.h"
#include "actionsproxy.h"

#include "memoryleak-defines.h"

static void connectToSource(McaFeedCache *cache, QAbstractListModel *model)
{
    QObject::connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            cache, SLOT(sourceRowsInserted(QModelIndex,int,int)), Qt::DirectConnection);
    QObject::connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            cache, SLOT(sourceRowsRemoved(QModelIndex,int,int)), Qt::DirectConnection);
    QObject::connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            cache, SLOT(sourceDataChanged(QModelIndex,QModelIndex)), Qt::DirectConnection);
}

static void disconnectFromSource(McaFeedCache *cache, QAbstractListModel *model)
{
    QObject::disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            cache, SLOT(sourceRowsInserted(QModelIndex,int,int)));
    QObject::disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            cache, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    QObject::disconnect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            cache, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
}

//
// instance methods
//

McaFeedCache::McaFeedCache(QObject *parent):
        QAbstractListModel(parent)
{
    m_source = NULL;
    m_state = StateThawed;

    // add feedmodel/feedadapter roles that have been lost through encapsulation
    QHash<int, QByteArray> roles = roleNames();
    roles.insert(McaFeedModel::RequiredTypeRole,      "type");
    roles.insert(McaFeedModel::RequiredUniqueIdRole,  "id");
    roles.insert(McaFeedModel::RequiredTimestampRole, "timestamp");
    roles.insert(McaFeedModel::CommonActionsRole,     "actions");
    roles.insert(McaFeedModel::CommonUuidRole,        "uuid");
    roles.insert(McaFeedModel::GenericTitleRole,      "title");
    roles.insert(McaFeedModel::GenericContentRole,    "content");
    roles.insert(McaFeedModel::GenericRelevanceRole,  "relevance");
    roles.insert(McaFeedModel::GenericAvatarUrlRole,  "avatar");
    roles.insert(McaFeedModel::GenericPictureUrlRole, "picture");
    roles.insert(McaFeedModel::GenericAcceptTextRole, "accept");
    roles.insert(McaFeedModel::GenericRejectTextRole, "reject");
    roles.insert(McaFeedAdapter::SystemServiceNameRole, "servicename");
    roles.insert(McaFeedAdapter::SystemServiceIconRole, "serviceicon");
    setRoleNames(roles);

    // QString roles
    m_cachedRoles.append(McaFeedModel::RequiredTypeRole);
    m_cachedRoles.append(McaFeedModel::RequiredUniqueIdRole);
    m_cachedRoles.append(McaFeedModel::CommonUuidRole);
    m_cachedRoles.append(McaFeedModel::GenericTitleRole);
    m_cachedRoles.append(McaFeedModel::GenericContentRole);
    m_cachedRoles.append(McaFeedModel::GenericAvatarUrlRole);
    m_cachedRoles.append(McaFeedModel::GenericPictureUrlRole);
    m_cachedRoles.append(McaFeedModel::GenericAcceptTextRole);
    m_cachedRoles.append(McaFeedModel::GenericRejectTextRole);
    m_cachedRoles.append(McaFeedAdapter::SystemServiceNameRole);
    m_cachedRoles.append(McaFeedAdapter::SystemServiceIconRole);

    // QDateTime role
    m_cachedRoles.append(McaFeedModel::RequiredTimestampRole);

    // McaActions* role
    m_cachedRoles.append(McaFeedModel::CommonActionsRole);
}

McaFeedCache::~McaFeedCache()
{
    if (m_source)
        disconnectFromSource(this, m_source);

    qDebug() << "Removing proxies: " << m_safeActions.count();

    foreach(McaActionsProxy *proxy, m_safeActions) {
        delete proxy;        
    }
    m_safeActions.clear();

    rowsRemoved(0, m_cache.count() - 1);
}

void McaFeedCache::setSourceModel(QAbstractListModel *model)
{
    if (m_source) {
        disconnectFromSource(this, m_source);
        rowsRemoved(0, m_source->rowCount() - 1);
    }

    m_source = model;

    if (m_source) {
        connectToSource(this, model);
        rowsInserted(0, model->rowCount() - 1);
    }
}

int McaFeedCache::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_source) {
        if (m_state == StateThawed)
            return m_source->rowCount();
        else
            return m_cache.count();
    }
    return 0;
}

QVariant McaFeedCache::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (m_source) {
        QVariant variant = m_cache.at(row)->value(role);
        if (role == McaFeedModel::CommonActionsRole) {
            if (!m_safeActions.contains(variant.value<McaActions*>())) {
                qWarning() << "warning: ignoring action (target destroyed)";
                return QVariant();
            }
            // Return the McaActionProxy instead
            McaActionsProxy *proxy = m_safeActions.value(variant.value<McaActions*>());
            return QVariant::fromValue(qobject_cast<McaActions*>(proxy));
        }
        return variant;
    }
    return QVariant();
}

bool McaFeedCache::frozen()
{
    return m_state != StateThawed;
}

//
// public slots
//

void McaFeedCache::setFrozen(bool frozen)
{
    bool frozenState = m_state != StateThawed;
    if (frozen == frozenState)
        return;

    emit frozenChanged(frozen);

    if (frozen) {
        m_state = StateFrozenClean;
        m_start = m_end = -1;
        m_updateStart = m_source->rowCount();
        m_updateEnd = -1;
    }
    else {
        switch (m_state) {
        case StateFrozenClean:
            // still need to send any data changes
            sendDataChanged();
            break;

        case StateFrozenInsert:
            // notify about row insertion and data changes
            rowsInserted(m_start, m_end);
            sendDataChanged();
            break;

        case StateFrozenRemove:
            // notify about row deletion and data changes
            rowsRemoved(m_start, m_end);
            sendDataChanged();
            break;

        case StateFrozenDirty:
            // reset the model by removing old rows, adding new, for now
            rowsRemoved(0, m_cache.count() - 1);
            rowsInserted(0, m_source->rowCount() - 1);
            break;

        default:
            break;
        }
        m_state = StateThawed;
    }
}

//
// protected slots
//

void McaFeedCache::sourceRowsInserted(const QModelIndex& parent,
                                      int start, int end)
{
    Q_UNUSED(parent)

    switch (m_state) {
    case StateThawed:
        // we're live so immediately update
        rowsInserted(start, end);
        break;

    case StateFrozenClean:
        // record this one transaction to perform when we thaw
        m_start = start;
        m_end = end;
        m_state = StateFrozenInsert;

        {
            int count = end - start + 1;
            if (m_updateStart >= start)
                m_updateStart += count;
            if (m_updateEnd >= start)
                m_updateEnd += count;
        }
        break;

    case StateFrozenDirty:
        // do nothing because we are already going to reset the model
        break;

    default:
        // changes are getting complicated so let's just reset the model
        m_state = StateFrozenDirty;
        break;
    }
}

void McaFeedCache::sourceRowsRemoved(const QModelIndex& parent,
                                     int start, int end)
{
    Q_UNUSED(parent)

    switch (m_state) {
    case StateThawed:
        // we're live so immediately update
        rowsRemoved(start, end);
        break;

    case StateFrozenClean:
        // record this one transaction to perform when we thaw
        m_start = start;
        m_end = end;
        m_state = StateFrozenRemove;

        {
            int count = end - start + 1;
            if (m_updateStart >= start) {
                if (m_updateStart <= end)
                    m_updateStart = start;
                else
                    m_updateStart -= count;
            }
            if (m_updateEnd >= start) {
                if (m_updateEnd <= end)
                    m_updateEnd = start - 1;
                else
                    m_updateEnd -= count;
            }
        }
        break;

    case StateFrozenDirty:
        // do nothing because we are already going to reset the model
        break;

    default:
        // changes are getting complicated so let's just reset the model
        m_state = StateFrozenDirty;
        break;
    }
}

void McaFeedCache::sourceDataChanged(const QModelIndex& topLeft,
                                     const QModelIndex& bottomRight)
{
    switch (m_state) {
    case StateThawed:
        for (int i = topLeft.row(); i <= bottomRight.row(); i++)
            updateRow(m_cache.at(i), i);
        emit dataChanged(index(topLeft.row()), index(bottomRight.row()));
        break;

    case StateFrozenDirty:
        // do nothing because we are already going to reset the model
        break;

    default:
        // add to list of changed rows
        if (topLeft.row() < m_updateStart)
            m_updateStart = topLeft.row();
        if (bottomRight.row() > m_updateEnd)
            m_updateEnd = bottomRight.row();
        break;
    }
}

void McaFeedCache::actionsDestroyed(QObject *object)
{    
    McaActionsProxy *proxy = m_safeActions.value(object);
    delete proxy;
    m_safeActions.remove(object);
}

//
// protected members
//

void McaFeedCache::rowsInserted(int start, int end)
{
    if (end < start)
        return;

    beginInsertRows(QModelIndex(), start, end);

    // should only get here when not frozen
    for (int i = end; i >= start; i--) {
        QMap<int,QVariant> *map = new QMap<int,QVariant>;
        updateRow(map, i);
        m_cache.insert(start, map);
    }

    endInsertRows();
}

void McaFeedCache::rowsRemoved(int start, int end)
{
    if (end < start)
        return;

    beginRemoveRows(QModelIndex(), start, end);

    // should only get here when not frozen
    int count = end - start + 1;
    QMap<int,QVariant> *map = 0;
    for (int i = 0; i < count; i++) {
        map = m_cache.at(start);
        m_cache.removeAt(start);
        delete map;
    }

    endRemoveRows();
}

void McaFeedCache::sendDataChanged() {
    if (m_updateEnd != -1)
        emit dataChanged(index(m_updateStart), index(m_updateEnd));
}

void McaFeedCache::updateRow(QMap<int,QVariant> *map, int row)
{
    // update all values for this row in the given cache map
    foreach (int role, m_cachedRoles)
        map->insert(role, m_source->data(m_source->index(row), role));

    McaActions *actions = map->value(McaFeedModel::CommonActionsRole).value<McaActions*>();
    if( actions && !m_safeActions.contains(actions) ) {
       McaActionsProxy *proxy = new McaActionsProxy(actions);
       m_safeActions.insert(qobject_cast<QObject*>(actions), proxy);
       connect(actions, SIGNAL(destroyed(QObject*)),
                this, SLOT(actionsDestroyed(QObject*)));
    }
}
