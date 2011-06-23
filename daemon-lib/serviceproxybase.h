/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcaserviceproxybase_h
#define __mcaserviceproxybase_h

#include <QSortFilterProxyModel>
#include <QStringList>

#include "dbustypes.h"

class McaPanelManagerDBus;

class McaServiceProxyBase: public QSortFilterProxyModel
{
    Q_OBJECT

public:
    McaServiceProxyBase(QAbstractItemModel *source, QObject *parent = NULL);
    virtual ~McaServiceProxyBase();

    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    void synchronizeClients();

signals: // for dbus
    void ItemsAdded(ArrayOfMcaServiceItemStruct items);
    void ItemsChanged(ArrayOfMcaServiceItemStruct items);
    void ItemsRemoved(QStringList items);

public slots: // for dbus
    void dataChangedProxy ( const QModelIndex & topLeft, const QModelIndex & bottomRight );
    void rowsAboutToBeRemovedProxy ( const QModelIndex & parent, int start, int end );
    void rowsInsertedProxy ( const QModelIndex & parent, int start, int end );

protected:
    QAbstractItemModel *m_source;
};

#endif  // __mcaserviceproxy_h
