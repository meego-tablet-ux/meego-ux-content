/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "memoryleak.h"

#include <QDebug>

#include "aggregatedmodel.h"
#include "adapter.h"

#include "memoryleak-defines.h"

//
// instance methods
//

McaAggregatedModel::McaAggregatedModel(QObject *parent):
        QAbstractListModel(parent)
{
    QHash<int, QByteArray> hash = roleNames();
    hash.insert(SourceModelRole, "sourcemodel");
    setRoleNames(hash);
}

McaAggregatedModel::~McaAggregatedModel()
{
}

void McaAggregatedModel::addSourceModel(QAbstractItemModel *model)
{
#ifdef THREADING
    McaAdapter *adapter_model = qobject_cast<McaAdapter*>(model);
    if (adapter_model) {
        // Alternative: Pass this object as a argument, use invokeMethod,
        connect(adapter_model, SIGNAL(syncUpdate(McaAdapter*,int,int)),
                this, SLOT(syncUpdate(McaAdapter*,int,int)), Qt::BlockingQueuedConnection);
        connect(adapter_model, SIGNAL(syncRemoval(McaAdapter*,int,int)),
                this, SLOT(syncRemoval(McaAdapter*,int,int)), Qt::BlockingQueuedConnection);

        // TODO: Make sure this doesn't cause issues if it gets between begin/end updates
        // as the handling is split in McaAdapter.
        QMetaObject::invokeMethod(adapter_model, "triggerSyncUpdate", Qt::QueuedConnection);
    } else {
        qWarning() << "McaAggregatedModel: Recieved an unknown model, threading might break.";
        rowsInserted(model, 0, model->rowCount() - 1);

        connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(sourceRowsInserted(QModelIndex,int,int)), Qt::BlockingQueuedConnection);
        connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                this, SLOT(sourceRowsRemoved(QModelIndex,int,int)), Qt::BlockingQueuedConnection);
        connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)), Qt::BlockingQueuedConnection);
        connect(model, SIGNAL(modelAboutToBeReset()),
                this, SLOT(sourceModelAboutToBeReset()), Qt::BlockingQueuedConnection);
    }
#else
    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
    connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
    connect(model, SIGNAL(modelAboutToBeReset()),
            this, SLOT(sourceModelAboutToBeReset()));

    rowsInserted(model, 0, model->rowCount() - 1);
#endif

    // TODO: handle model reset
}

void McaAggregatedModel::removeSourceModel(QAbstractItemModel *model)
{    
#ifdef THREADING
    McaAdapter *adapter_model = qobject_cast<McaAdapter*>(model);
    if (adapter_model) {
        // TODO: Make sure this doesn't cause issues if it gets between begin/end updates
        // as the handling is split in McaAdapter.
        QMetaObject::invokeMethod(adapter_model, "triggerSyncRemoval", Qt::QueuedConnection);
    } else {
        qWarning() << "McaAggregatedModel: Recieved an unknown model, threading might break.";

        disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
               this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
               this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
        disconnect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
               this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));

        rowsRemoved(model, 0, model->rowCount() - 1);
    }
#else
    rowsRemoved(model, 0, model->rowCount() - 1);
#endif
}

int McaAggregatedModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_indexList.count();
}

QVariant McaAggregatedModel::data(const QModelIndex &index, int role) const
{
    // TODO: fix the const_cast hack here
    int row = index.row();
    if (row < m_indexList.count()) {
        QAbstractItemModel *model = const_cast<QAbstractItemModel *>(m_indexList.at(row).model());
        if (role == SourceModelRole)
            return QVariant::fromValue<QObject *>(model);
        return m_indexList.at(row).data(role);
    }

    return QVariant();
}

#ifdef THREADING

void McaAggregatedModel::syncUpdate(McaAdapter *model, int start, int end)
{
    rowsInserted(model, start, end);

    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(sourceRowsInserted(QModelIndex,int,int)), Qt::BlockingQueuedConnection);
    connect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsRemoved(QModelIndex,int,int)), Qt::BlockingQueuedConnection);
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)), Qt::BlockingQueuedConnection);
    connect(model, SIGNAL(modelAboutToBeReset()),
            this, SLOT(sourceModelAboutToBeReset()), Qt::BlockingQueuedConnection);
}

void McaAggregatedModel::syncRemoval(McaAdapter *model, int start, int end)
{
    rowsRemoved(model, start, end);

    disconnect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
           this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
    disconnect(model, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
           this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    disconnect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
           this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));

    disconnect(model, SIGNAL(syncUpdate(McaAdapter*,int,int)),
            this, SLOT(syncUpdate(McaAdapter*,int,int)));
    disconnect(model, SIGNAL(syncRemoval(McaAdapter*,int,int)),
            this, SLOT(syncRemoval(McaAdapter*,int,int)));
}
#endif

//
// helper functions
//

static QList< QPair<int,int> > findAffectedRows(const QList<QPersistentModelIndex> &list,
                                                const QAbstractItemModel *sourceModel,
                                                int start, int end)
{
    // effects: identifies items in list that come from rows between start and
    //            end (inclusive) in the model matching sourceModel
    // returns: a list of pairs of first row and last row of blocks of
    //            consecutive affected rows in the list

    // affected rows in order they are found
    QList<int> affectedRows;

    int i=0;
    foreach (QModelIndex sourceIndex, list) {
        if (sourceIndex.model() == sourceModel) {
            // index is from the affected model, check to see if row affected
            int row = sourceIndex.row();
            if (row >= start && row <= end)
                affectedRows.append(i);
        }
        i++;
    }

    QList< QPair<int,int> > affectedBlocks;
    QPair<int,int> block(-1, -1);

    foreach (int row, affectedRows) {
        // first time through, just initialize
        if (block.first == -1) {
            block.first = row;
            block.second = row;
            continue;
        }

        // if this row is consecutive, just bump last
        if (row == block.second + 1) {
            block.second = row;
            continue;
        }

        // otherwise record block and start next one
        affectedBlocks.append(block);
        block.first = block.second = row;
    }

    // append final block
    if (block.first != -1)
        affectedBlocks.append(block);

    return affectedBlocks;
}

//
// protected slots
//

void McaAggregatedModel::sourceRowsInserted(const QModelIndex& parent,
                                            int start, int end)
{
    Q_UNUSED(parent)

    QAbstractListModel *model = qobject_cast<QAbstractListModel *>(sender());
    if (model)
        rowsInserted(model, start, end);
}

void McaAggregatedModel::sourceRowsRemoved(const QModelIndex& parent,
                                           int start, int end)
{
    Q_UNUSED(parent)

    QAbstractListModel *model = qobject_cast<QAbstractListModel *>(sender());
    if (model)
        rowsRemoved(model, start, end);
}

void McaAggregatedModel::sourceDataChanged(const QModelIndex& topLeft,
                                           const QModelIndex& bottomRight)
{    
    QList< QPair<int,int> > affectedRows;
    affectedRows = findAffectedRows(m_indexList, topLeft.model(),
                                    topLeft.row(), bottomRight.row());

    // emit dataChanged for each affected block
    for (int i = affectedRows.count() - 1; i >= 0; i--) {
        QPair<int,int> block = affectedRows[i];
        QModelIndex first = index(block.first);
        QModelIndex last = index(block.second);
        emit dataChanged(first, last);
    }
}


void McaAggregatedModel::sourceModelAboutToBeReset()
{
    QAbstractListModel *model = qobject_cast<QAbstractListModel *>(sender());

    rowsRemoved(model, 0, model->rowCount() - 1);
}

//
// protected members
//

void McaAggregatedModel::rowsInserted(const QAbstractItemModel *sourceModel,
                                      int start, int end)
{
    if (end < start)
        return;

    // simply append the new source rows to our list in one block
    int myCount = m_indexList.count();
    int newRows = end - start + 1;

    beginInsertRows(QModelIndex(), myCount, myCount + newRows - 1);
    for (int i=start; i<=end; i++) {
        QModelIndex sourceIndex = sourceModel->index(i, 0);
        m_indexList.append(sourceIndex);
    }
    endInsertRows();
}

void McaAggregatedModel::rowsRemoved(const QAbstractItemModel *sourceModel,
                                     int start, int end)
{

    if (end < start)
        return;

    QList< QPair<int,int> > affectedRows;
    affectedRows = findAffectedRows(m_indexList, sourceModel, start, end);

    // walk through rows in reverse order so removing won't affect indices
    for (int i = affectedRows.count() - 1; i >= 0; i--) {
        QPair<int,int> block = affectedRows[i];
        beginRemoveRows(QModelIndex(), block.first, block.second);
        for (int j = block.first; j <= block.second; j++)
            m_indexList.removeAt(block.first);
        endRemoveRows();
    }
}
