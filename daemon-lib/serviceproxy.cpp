/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "memoryleak.h"

#include <QDebug>

#include "serviceproxy.h"
#include "servicemodel.h"
#include "serviceadapter.h"
#include "panelmanager.h"

#include "memoryleak-defines.h"

//
// Overview of McaServiceProxy
//    - sorts and filters the service model
//    - adds enabled role to underlying model
//

//
// public methods
//

McaServiceProxy::McaServiceProxy(McaPanelManager *panelmgr, QAbstractItemModel *source,
                                 QObject *parent):
        QSortFilterProxyModel(parent)
{
    m_panelmgr = panelmgr;

    QHash<int, QByteArray> roles = source->roleNames();
    roles.insert(SystemEnabledRole, "enabled");
    // TODO: probably could eliminate aggregatedservicemodel now, move roles here?
    setRoleNames(roles);

    setSourceModel(source);
    m_source = source;
    m_filter = FilterCategories;  // categories empty, so model initially empty - good!

    connect(m_panelmgr, SIGNAL(serviceEnabledChanged(QString,bool)),
            this, SLOT(setServiceEnabled(QString,bool)));
}

McaServiceProxy::~McaServiceProxy()
{
}

int McaServiceProxy::filter()
{
    return m_filter;
}

void McaServiceProxy::setFilter(int filter)
{
    if (m_filter >= FilterLast) {
        qDebug() << "warning: invalid filter in McaServiceProxy::setFilter";
        return;
    }

    if (m_filter == filter)
        return;
    m_filter = filter;
    emit filterChanged(filter);

    // recalculate filtered rows
    invalidateFilter();
}

QStringList McaServiceProxy::categories()
{
    return m_categories;
}

void McaServiceProxy::setCategories(const QStringList &categories)
{
    if (m_categories == categories)
        return;
    m_categories = categories;
    emit categoriesChanged(categories);

    // force recalculation of filtered rows
    if (m_filter == FilterCategories)
        invalidateFilter();
}

QVariant McaServiceProxy::data(const QModelIndex &index, int role) const
{
    if (role == SystemEnabledRole) {
        QString upid = data(index, McaServiceAdapter::SystemUpidRole).toString();

        return m_panelmgr->isServiceEnabled(upid);
    }
    return QSortFilterProxyModel::data(index, role);
}

bool McaServiceProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)

    if (m_filter != FilterCategories)
        return true;

    QModelIndex sourceIndex = m_source->index(source_row, 0);
    QString category = m_source->data(sourceIndex, McaServiceModel::RequiredCategoryRole).toString();
    if (m_categories.contains(category))
        return true;
    return false;
}

//
// protected slots
//

void McaServiceProxy::setServiceEnabled(const QString &upid, bool enabled)
{
    Q_UNUSED(enabled)

    QModelIndex start = index(0, 0);
    QModelIndexList list = match(start, McaServiceAdapter::SystemUpidRole,
                                 upid, 1, Qt::MatchExactly);
    if (list.count() > 0)
        emit dataChanged(list[0], list[0]);
}
