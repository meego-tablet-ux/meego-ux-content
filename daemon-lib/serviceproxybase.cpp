/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "memoryleak.h"

#include <QDebug>

#include "serviceproxybase.h"
#include "servicemodel.h"
#include "serviceadapter.h"
#include "panelmanagerdbus.h"

#include "memoryleak-defines.h"

//
// Overview of McaServiceProxyBase
//    - sorts and filters the service model
//    - adds enabled role to underlying model
//

//
// public methods
//

McaServiceProxyBase::McaServiceProxyBase(QAbstractItemModel *source, QObject *parent):
        QSortFilterProxyModel(parent)
{
    setSourceModel(source);
    m_source = source;

    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(dataChangedProxy(QModelIndex,QModelIndex)));
    connect(this, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(rowsAboutToBeRemovedProxy(QModelIndex,int,int)));
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(rowsInsertedProxy(QModelIndex,int,int)));
}

McaServiceProxyBase::~McaServiceProxyBase()
{
}

bool McaServiceProxyBase::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)
    Q_UNUSED(source_row)
    return true;
}
void McaServiceProxyBase::dataChangedProxy ( const QModelIndex & topLeft, const QModelIndex & bottomRight )
{
    qDebug() << "McaServiceProxyBase::dataChangedProxy ";
    ArrayOfMcaServiceItemStruct itemsArray;
    int topRow = topLeft.row();
    int bottomRow = bottomRight.row();

    McaServiceItemStruct item;
    for(int row = topRow; row <= bottomRow; row++) {
        item.name = data(index(row, 0), McaServiceModel::RequiredNameRole).toString();
        item.category = data(index(row, 0), McaServiceModel::RequiredCategoryRole).toString();
        item.displayName = data(index(row, 0), McaServiceModel::CommonDisplayNameRole).toString();
        item.iconUrl = data(index(row, 0), McaServiceModel::CommonIconUrlRole).toString();
        item.configError = data(index(row, 0), McaServiceModel::CommonConfigErrorRole).toBool();
        item.upid = data(index(row, 0), McaServiceAdapter::SystemUpidRole).toString();
        item.enabled = true;
        itemsArray.append(item);
    }

    emit ItemsChanged(itemsArray);
}

void McaServiceProxyBase::rowsAboutToBeRemovedProxy ( const QModelIndex & parent, int start, int end )
{
    qDebug() << "McaServiceProxyBase::rowsAboutToBeRemovedProxy ";
    Q_UNUSED(parent);
    QStringList itemIds;
    int topRow = start;
    int bottomRow = end;

    QString id;
    for(int row = topRow; row <= bottomRow; row++) {
        id = data(index(row, 0), McaServiceAdapter::SystemUpidRole).toString();
        itemIds.append(id);
    }

    emit ItemsRemoved(itemIds);
}

void McaServiceProxyBase::rowsInsertedProxy ( const QModelIndex & parent, int start, int end )
{
    qDebug() << "McaServiceProxyBase::rowsInsertedProxy ";
    Q_UNUSED(parent);
    ArrayOfMcaServiceItemStruct itemsArray;
    int topRow = start;
    int bottomRow = end;

    McaServiceItemStruct item;
    for(int row = topRow; row <= bottomRow; row++) {
        item.name = data(index(row, 0), McaServiceModel::RequiredNameRole).toString();
        item.category = data(index(row, 0), McaServiceModel::RequiredCategoryRole).toString();
        item.displayName = data(index(row, 0), McaServiceModel::CommonDisplayNameRole).toString();
        item.iconUrl = data(index(row, 0), McaServiceModel::CommonIconUrlRole).toString();
        item.configError = data(index(row, 0), McaServiceModel::CommonConfigErrorRole).toBool();
        item.upid = data(index(row, 0), McaServiceAdapter::SystemUpidRole).toString();
        item.enabled = true;
        itemsArray.append(item);
    }

    emit ItemsAdded(itemsArray);
}
