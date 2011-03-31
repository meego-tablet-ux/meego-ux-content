/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "memoryleak.h"

#include <QDebug>
#include "feedadapter.h"
#include "feedmodel.h"
#include "feedfilter.h"
#include "threadtest.h"

#include "memoryleak-defines.h"

// enough display a full SMS or tweet
const int ContentMaxChars = 160;

//
// Overview of McaFeedAdapter
//    - limits number of rows exposed to value in limit property
//    - adds servicename and serviceicon roles to underlying model
//

//
// public methods
//

McaFeedAdapter::McaFeedAdapter(QAbstractItemModel *source, const QString &serviceId,
                               const QString &serviceName, const QString &serviceIcon,
                               const QString &serviceCategory, QObject *parent):
        McaAdapter(parent)
{
    m_source = source;
    m_serviceId = serviceId;
    m_serviceName = serviceName;
    m_serviceIcon = serviceIcon;
    m_serviceCategory = serviceCategory;
    m_limit = 0;
    m_queuedLimit = 0;
    m_rowCount = 0;
    m_lastRowCount = 0;
    m_updating = false;

    QHash<int, QByteArray> roles = source->roleNames();
    roles.insert(SystemServiceIdRole, "serviceid");
    roles.insert(SystemServiceNameRole, "servicename");
    roles.insert(SystemServiceIconRole, "serviceicon");
    roles.insert(SystemServiceCategoryRole, "servicecategory");
    setRoleNames(roles);

    connect(source, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
    connect(source, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
    connect(source, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(source, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    connect(source, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(source, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(sourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(source, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
    connect(source, SIGNAL(modelAboutToBeReset()),
            this, SLOT(sourceModelAboutToBeReset()));
    connect(source, SIGNAL(modelReset()),
            this, SLOT(sourceModelReset()));
}

McaFeedAdapter::~McaFeedAdapter()
{
    McaFeedFilter *feedFilter = qobject_cast<McaFeedFilter*>(m_source);
    if(feedFilter) {
        delete feedFilter;
    } else {
        delete m_source;
    }
}

int McaFeedAdapter::limit()
{
    return m_limit;
}

void McaFeedAdapter::setLimit(int limit)
{
    if (m_limit != limit) {
        if (m_updating) {
            qWarning() << "WARNING: limit changed during an update";
            m_queuedLimit = limit;
            return;
        }

        int oldLimit = m_limit;
        m_queuedLimit = m_limit = limit;
        emit limitChanged();

        if (limit > oldLimit)
            fetchMore();
        else if (limit < m_rowCount) {
            THREAD_SET_TEST(this);
            beginRemoveRows(QModelIndex(), limit, m_rowCount - 1);
            THREAD_UNSET_TEST(this);
            m_rowCount = limit;
            endRemoveRows();
        }
    }
}

int McaFeedAdapter::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_rowCount;
}

QVariant McaFeedAdapter::data(const QModelIndex &index, int role) const
{
    THREAD_PRINT_TEST(this);
    
    if (index.row() > m_rowCount) {
        if (m_limit == 0)
            qWarning() << "WARNING: limit zero in feed adapter data call";
        return QVariant();
    }

    if (role == SystemServiceIdRole)
        return m_serviceId;
    if (role == SystemServiceNameRole)
        return m_serviceName;
    if (role == SystemServiceIconRole)
        return m_serviceIcon;
    if (role == SystemServiceCategoryRole)
        return m_serviceCategory;

    QVariant var = m_source->data(m_source->index(index.row(), 0), role);

    // limit title and content strings, and condense whitespace
    if (role == McaFeedModel::GenericTitleRole ||
        role == McaFeedModel::GenericContentRole)
        return var.toString().left(ContentMaxChars).simplified();
    return var;
}

bool McaFeedAdapter::canFetchMore(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    if (m_limit > m_rowCount) {
        if (m_source->rowCount() > m_rowCount || m_source->canFetchMore(QModelIndex()))
            return true;
    }
    return false;
}

void McaFeedAdapter::fetchMore(const QModelIndex& parent)
{
    Q_UNUSED(parent)
    int count = m_source->rowCount();
    for (;;) {  // forever
        if (count >= m_limit)
            break;

        // fetch enough to reach our limit, if we can
        if (!m_source->canFetchMore(QModelIndex()))
            break;
        m_source->fetchMore(QModelIndex());

        // ensure we will terminate even if model is lying
        int newCount = m_source->rowCount();
        if (newCount == count)
            break;
        count = newCount;
    }

    if (count > m_limit)
        count = m_limit;
    if (count > m_rowCount) {
        beginInsertRows(QModelIndex(), m_rowCount, count - 1);
        m_rowCount = count;
        THREAD_SET_TEST(this);
        endInsertRows();
        THREAD_UNSET_TEST(this);
    }
}

//
// protected slots
//

void McaFeedAdapter::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)

    if (start >= m_limit)
        return;

    m_updating = true;
    m_lastRowCount = m_rowCount;

    if (end >= m_limit)
        end = m_limit - 1;
    int adding = end - start + 1;

    int addCount = m_rowCount + adding;
    if (addCount > m_limit) {
        // preemptively remove rows that will exceed the limit
        int remove = addCount - m_limit;
        THREAD_SET_TEST(this);
        beginRemoveRows(QModelIndex(), m_rowCount - remove, m_rowCount - 1);
        THREAD_UNSET_TEST(this);
        m_rowCount -= remove;
        endRemoveRows();
    }

    beginInsertRows(QModelIndex(), start, end);
    m_rowCount += adding;
}

void McaFeedAdapter::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(end)

    if (start >= m_limit)
        return;

    THREAD_SET_TEST(this);
    endInsertRows();
    THREAD_UNSET_TEST(this);

    if (m_rowCount != m_lastRowCount)
        emit rowCountChanged();

    m_updating = false;
    if (m_queuedLimit != m_limit)
        setLimit(m_queuedLimit);
}

void McaFeedAdapter::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)

    if (start >= m_limit)
        return;

    m_updating = true;
    m_lastRowCount = m_rowCount;

    if (end >= m_limit)
        end = m_limit - 1;
    THREAD_SET_TEST(this);
    beginRemoveRows(QModelIndex(), start, end);
    THREAD_UNSET_TEST(this);
}

void McaFeedAdapter::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)

    if (start >= m_limit)
        return;

    if (end >= m_limit)
        end = m_limit - 1;

    m_rowCount -= end - start + 1;
    endRemoveRows();
    fetchMore();

    if (m_rowCount != m_lastRowCount)
        emit rowCountChanged();

    m_updating = false;
    if (m_queuedLimit != m_limit)
        setLimit(m_queuedLimit);
}

void McaFeedAdapter::sourceRowsAboutToBeMoved(const QModelIndex &source, int start, int end,
                                              const QModelIndex &dest, int destStart)
{
    Q_UNUSED(source)
    Q_UNUSED(start)
    Q_UNUSED(end)
    Q_UNUSED(dest)
    Q_UNUSED(destStart)

    qWarning() << "WARNING: rows moved in feed adapter - resetting model";
    sourceModelAboutToBeReset();
}

void McaFeedAdapter::sourceRowsMoved(const QModelIndex &source, int start, int end,
                                     const QModelIndex &dest, int destStart)
{
    Q_UNUSED(source)
    Q_UNUSED(start)
    Q_UNUSED(end)
    Q_UNUSED(dest)
    Q_UNUSED(destStart)

    sourceModelReset();
}

void McaFeedAdapter::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(topLeft)
    Q_UNUSED(bottomRight)

    int top = topLeft.row();
    if (top >= m_rowCount)
        return;

    int bottom = bottomRight.row();
    if (bottom > m_rowCount)
        bottom = m_rowCount - 1;

    QModelIndex myTopLeft = index(top);
    QModelIndex myBottomRight = index(bottom);
    THREAD_SET_TEST(this);
    emit dataChanged(myTopLeft, myBottomRight);
    THREAD_UNSET_TEST(this);
}

void McaFeedAdapter::sourceModelAboutToBeReset()
{
    m_updating = true;
    m_lastRowCount = m_rowCount;
    beginResetModel();
}

void McaFeedAdapter::sourceModelReset()
{
    m_rowCount = 0;
    endResetModel();
    fetchMore();

    if (m_rowCount != m_lastRowCount)
        emit rowCountChanged();

    m_updating = false;
    if (m_queuedLimit != m_limit)
        setLimit(m_queuedLimit);
}

QAbstractItemModel * McaFeedAdapter::getSource() {
    return m_source;
}
