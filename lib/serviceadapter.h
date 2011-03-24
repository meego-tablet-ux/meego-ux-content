/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcaserviceadapter_h
#define __mcaserviceadapter_h

#include <QAbstractListModel>
#include <QStringList>
#include "adapter.h"

class McaActions;
class McaFeedManager;

class McaServiceAdapter: public McaAdapter
{
    Q_OBJECT
    Q_ENUMS(Roles)

public:
    enum Roles {
        // provided by system
        SystemUpidRole = Qt::UserRole + 1,  // QString
    };

    McaServiceAdapter(McaFeedManager *feedmgr, QObject *parent = NULL);
    virtual ~McaServiceAdapter();

    virtual void setSourceModel(QAbstractItemModel *model);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    virtual void fetchMore(const QModelIndex &parent = QModelIndex());

protected slots:
    void sourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end);
    void sourceRowsInserted(const QModelIndex& parent, int start, int end);
    void sourceRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex& parent, int start, int end);
    void sourceRowsAboutToBeMoved(const QModelIndex &source, int start, int end,
                                  const QModelIndex &dest, int destStart);
    void sourceRowsMoved(const QModelIndex &source, int start, int end,
                         const QModelIndex &dest, int destStart);
    void sourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void sourceModelAboutToBeReset();
    void sourceModelReset();

protected:
    void rowsInserted(int start, int end);
    void rowsRemoved(int start, int end);

private:
    McaFeedManager *m_feedmgr;
    QAbstractItemModel *m_source;
    QStringList m_upids;  // unique persistent service ids
};

#endif  // __mcaserviceadapter_h
