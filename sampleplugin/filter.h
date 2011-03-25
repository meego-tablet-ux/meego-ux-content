/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __filter_h
#define __filter_h

#include <QSortFilterProxyModel>

#include <feedmodel.h>

class McaActions;

class Filter: public QSortFilterProxyModel, public McaSearchableFeed
{
    Q_OBJECT

public:
    Filter(QAbstractItemModel *source, QObject *parent = NULL);
    ~Filter();

    void setSearchText(const QString& text);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    QAbstractItemModel *m_source;
    QString m_searchText;
};

#endif  // __filter_h
