/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcaaggregatedmodel_h
#define __mcaaggregatedmodel_h

#include <QPersistentModelIndex>
#include <QDBusVariant>
#include "dbustypes.h"

class McaAdapter;
class McaFeedAdapter;

class McaAggregatedModel: public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(Roles)

public:
    McaAggregatedModel(QObject *parent = NULL);
    virtual ~McaAggregatedModel();

    enum Roles {
        // return the source model this row came from
        SourceModelRole = Qt::UserRole + 0
    };

    void addSourceModel(QAbstractItemModel *model);
    void removeSourceModel(QAbstractItemModel *model);

    int rowCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;

protected slots:
#ifdef THREADING
    void syncUpdate(McaAdapter *model, int start, int end);
    void syncRemoval(McaAdapter *model, int start, int end);
#endif
    void sourceRowsInserted(const QModelIndex& parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex& parent, int start, int end);
    void sourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void sourceModelAboutToBeReset();

protected:
    void rowsInserted(const QAbstractItemModel *model, int start, int end);
    void rowsRemoved(const QAbstractItemModel *model, int start, int end);

signals: // for dbus
    void ItemsAdded(ArrayOfMcaFeedItemStruct items);
    void ItemsChanged(ArrayOfMcaFeedItemStruct items);
    void ItemsRemoved(ArrayOfMcaFeedItemId items);

public slots: // for dbus
    void dataChanged ( const QModelIndex & topLeft, const QModelIndex & bottomRight );
    void rowsAboutToBeRemoved ( const QModelIndex & parent, int start, int end );
    void rowsInserted ( const QModelIndex & parent, int start, int end );

    void doStandardAction(const QString &action, const QString &itemId, const QString &serviceId);
    void doCustomAction(const QString &action, const QString &itemId, const QString &serviceId);

private:
    QList<QPersistentModelIndex> m_indexList;
    QHash<QString, QPersistentModelIndex> m_itemIdToIndex;
};

#endif  // __mcaaggregatedmodel_h
