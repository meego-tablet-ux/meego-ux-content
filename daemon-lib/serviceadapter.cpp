/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "memoryleak.h"

#include <QDebug>
#include <QThread>

#include "serviceadapter.h"
#include "feedmanager.h"
#include "servicemodel.h"
#include "threadtest.h"

#include "memoryleak-defines.h"

//
// Overview of McaServiceAdapter
//    - adds unique persistent service id (upid) role to underlying model
//

//
// public methods
//

static void connectToSource(McaServiceAdapter *adapter, QAbstractItemModel *source)
{
    QObject::connect(source, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            adapter, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
    QObject::connect(source, SIGNAL(rowsInserted(QModelIndex,int,int)),
            adapter, SLOT(sourceRowsInserted(QModelIndex,int,int)));
    QObject::connect(source, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            adapter, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    QObject::connect(source, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            adapter, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    QObject::connect(source, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            adapter, SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    QObject::connect(source, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            adapter, SLOT(sourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    QObject::connect(source, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            adapter, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
    QObject::connect(source, SIGNAL(modelAboutToBeReset()),
            adapter, SLOT(sourceModelAboutToBeReset()));
    QObject::connect(source, SIGNAL(modelReset()),
            adapter, SLOT(sourceModelReset()));
}

static void disconnectFromSource(McaServiceAdapter *adapter, QAbstractItemModel *source)
{
    QObject::disconnect(source, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            adapter, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
    QObject::disconnect(source, SIGNAL(rowsInserted(QModelIndex,int,int)),
            adapter, SLOT(sourceRowsInserted(QModelIndex,int,int)));
    QObject::disconnect(source, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            adapter, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    QObject::disconnect(source, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            adapter, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    QObject::disconnect(source, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            adapter, SLOT(sourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    QObject::disconnect(source, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            adapter, SLOT(sourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    QObject::disconnect(source, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            adapter, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
    QObject::disconnect(source, SIGNAL(modelAboutToBeReset()),
            adapter, SLOT(sourceModelAboutToBeReset()));
    QObject::disconnect(source, SIGNAL(modelReset()),
            adapter, SLOT(sourceModelReset()));
}

McaServiceAdapter::McaServiceAdapter(McaFeedManager *feedmgr, QObject *parent):
        McaAdapter(parent)
{
    m_feedmgr = feedmgr;
    m_source = NULL;

    qDebug() << "Service Adapter Threadid: " << QThread::currentThreadId();
}

McaServiceAdapter::~McaServiceAdapter()
{
}

void McaServiceAdapter::setSourceModel(QAbstractItemModel *model)
{
    if (m_source) {
        disconnectFromSource(this, m_source);
        int count = m_source->rowCount();
        if (count > 0) {
            THREAD_SET_TEST(this);
            beginRemoveRows(QModelIndex(), 0, count - 1);
            THREAD_UNSET_TEST(this);
            rowsRemoved(0, count - 1);
            endRemoveRows();
        }
    }

    m_source = model;

    if (m_source) {
        connectToSource(this, model);
        int count = model->rowCount();
        if (count > 0) {
            beginInsertRows(QModelIndex(), 0, count - 1);
            rowsInserted(0, count - 1);
            THREAD_SET_TEST(this);
            endInsertRows();
            THREAD_UNSET_TEST(this);
        }

        QHash<int, QByteArray> roles = m_source->roleNames();
        roles.insert(SystemUpidRole, "upid");
        setRoleNames(roles);
    }
}

int McaServiceAdapter::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_source->rowCount();
}

QVariant McaServiceAdapter::data(const QModelIndex &index, int role) const
{
    THREAD_PRINT_TEST(this);
    
    int row = index.row();
    if (!index.isValid() || row > m_source->rowCount())
        return QVariant();

    if (role == SystemUpidRole)
        return m_upids.at(row);
    return m_source->data(m_source->index(row, 0), role);
}

bool McaServiceAdapter::canFetchMore(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_source->canFetchMore(QModelIndex());
}

void McaServiceAdapter::fetchMore(const QModelIndex& parent)
{
    Q_UNUSED(parent)
    m_source->fetchMore(QModelIndex());
}

//
// protected slots
//

void McaServiceAdapter::sourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)

    beginInsertRows(QModelIndex(), start, end);
}

void McaServiceAdapter::sourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)
    rowsInserted(start, end);
    THREAD_SET_TEST(this);
    endInsertRows();
    THREAD_UNSET_TEST(this);
}

void McaServiceAdapter::sourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)
    THREAD_SET_TEST(this);
    beginRemoveRows(QModelIndex(), start, end);
    THREAD_UNSET_TEST(this);
}

void McaServiceAdapter::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)

    rowsRemoved(start, end);
    endRemoveRows();
}

void McaServiceAdapter::sourceRowsAboutToBeMoved(const QModelIndex &source, int start, int end,
                                                 const QModelIndex &dest, int destStart)
{
    Q_UNUSED(source)
    Q_UNUSED(dest)

    beginMoveRows(QModelIndex(), start, end, QModelIndex(), destStart);
}

void McaServiceAdapter::sourceRowsMoved(const QModelIndex &source, int start, int end,
                                        const QModelIndex &dest, int destStart)
{
    Q_UNUSED(source)
    Q_UNUSED(dest)

    QStringList moved;
    for (int i = start; i < end; i++)
        moved.append(m_upids.takeAt(start));
    for (int i = start; i < end; i++)
        m_upids.insert(destStart++, moved.takeAt(0));
    endMoveRows();
}

void McaServiceAdapter::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QModelIndex top = index(topLeft.row(), 0);
    QModelIndex bottom = index(bottomRight.row(), 0);
    THREAD_SET_TEST(this);
    emit dataChanged(top, bottom);
    THREAD_UNSET_TEST(this);
    // service name should never change, not updating m_upids
}

void McaServiceAdapter::sourceModelAboutToBeReset()
{
    beginResetModel();
}

void McaServiceAdapter::sourceModelReset()
{
    m_upids.clear();
    int count = m_source->rowCount();
    rowsInserted(0, count - 1);
    endResetModel();
}

//
// protected methods
//

void McaServiceAdapter::rowsInserted(int start, int end)
{
    for (int i = start; i <= end; i++) {
        QModelIndex sourceIndex = m_source->index(i, 0);
        QString name = m_source->data(sourceIndex, McaServiceModel::RequiredNameRole).toString();
        m_upids.insert(i, m_feedmgr->serviceId(this, name));
    }
}

void McaServiceAdapter::rowsRemoved(int start, int end)
{
    for (int i = start; i <= end; i++)
        m_upids.removeAt(start);
}
