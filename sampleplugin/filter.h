/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __filter_h
#define __filter_h

#include <QList>
#include <QPersistentModelIndex>

#include <feedmodel.h>

class McaActions;

class Filter: public QAbstractItemModel, public McaSearchableFeed
{
    Q_OBJECT

public:
    Filter(QAbstractItemModel *source, QObject *parent = NULL);
    ~Filter();

    QModelIndex index(int row, int column, const QModelIndex& parent) const;
    QModelIndex parent(const QModelIndex& index) const;
    int rowCount(const QModelIndex& index) const;
    int columnCount(const QModelIndex& index) const;
    QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const;

    void setSearchText(const QString& text);
    void invalidateFilter();
    bool filterAcceptsRow(int source_row) const;

private slots:
    void sourceRowsInserted(const QModelIndex &parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex &parent, int start, int end);

private:
    QAbstractItemModel *m_source;
    QList<QPersistentModelIndex> m_filteredList;
    QString m_searchText;
};

#endif  // __filter_h
