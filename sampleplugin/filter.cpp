/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>

#include <QSettings>
#include <QTimer>

#include "filter.h"
#include "actions.h"
#include "settings.h"

//
// public methods
//

Filter::Filter(QAbstractItemModel *source, QObject *parent):
        QAbstractItemModel(parent)
{
    m_source = source;

    // These aren't actually ever triggered in the sampleplugin
    // but quite likely would be in a live plugin
    connect(source, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
    connect(source, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
}

Filter::~Filter()
{
    delete m_source;
}

QModelIndex Filter::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    if (column == 0)
        return createIndex(row,column);
    else
        return QModelIndex();
}

QModelIndex Filter::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int Filter::rowCount(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return m_filteredList.count();
}

int Filter::columnCount(const QModelIndex& index) const
{
    Q_UNUSED(index);
    return 1;
}

QVariant Filter::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
        return m_filteredList.value(index.row()).data(role);
    else
        return QVariant();
}

void Filter::setSearchText(const QString &text)
{
    m_searchText = text;
    invalidateFilter();
}

void Filter::invalidateFilter()
{
    int i = 0, j = 0, count = m_source->rowCount();

    if (m_filteredList.count()) {
        // Reset model isn't currently supported
        beginRemoveRows(QModelIndex(), 0, m_filteredList.count() - 1);
        m_filteredList.clear();
        endRemoveRows();
    }

    if (m_searchText.isEmpty())
        return;

    while (!isSearchHalted() && i < count) {
        if (filterAcceptsRow(i)) {
            beginInsertRows(QModelIndex(), j, j);
            m_filteredList.push_back(m_source->index(i, 0));
            endInsertRows();
            j++;
        }
        i++;
    }
}

bool Filter::filterAcceptsRow(int source_row) const
{
    QModelIndex sourceIndex = m_source->index(source_row, 0);
    QString title = m_source->data(sourceIndex, McaFeedModel::GenericTitleRole).toString();
    if (title.contains(m_searchText, Qt::CaseInsensitive))
        return true;

    QString content = m_source->data(sourceIndex, McaFeedModel::GenericContentRole).toString();
    if (content.contains(m_searchText, Qt::CaseInsensitive))
        return true;

    return false;
}

//
// private slots
//

void Filter::sourceRowsInserted(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent);

    // Beware: Dragons Ahead
    // Untested sample, it compiles!
    int j = m_filteredList.count() - 1;
    while (!isSearchHalted() && start < end) {
        if (filterAcceptsRow(start)) {
            beginInsertRows(QModelIndex(), j, j);
            m_filteredList.push_back(m_source->index(start, 0));
            endInsertRows();
            j++;
        }
        start++;
    }
}

void Filter::sourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);

    // Beware: Dragons Ahead
    // Untested sample, it compiles!
    int row = 0;
    QMutableListIterator<QPersistentModelIndex> iter(m_filteredList);
    while (iter.hasNext()) {
        int sourceRow = iter.next().row();
        if (sourceRow >= start && sourceRow <= end) {
            beginRemoveRows(QModelIndex(), row, row);
            iter.remove();
            endRemoveRows();
        }
        else {
            row++;
        }
    }
}
