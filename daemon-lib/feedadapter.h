/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcafeedadapter_h
#define __mcafeedadapter_h

#include <QAbstractListModel>
#include "adapter.h"

class McaFeedAdapter: public McaAdapter
{
    Q_OBJECT
    Q_ENUMS(Roles)
    Q_PROPERTY(int limit READ limit WRITE setLimit NOTIFY limitChanged);

public:
    McaFeedAdapter(QAbstractItemModel *source, const QString &serviceId,
                   const QString &serviceName, const QString &serviceIcon,
                   const QString &serviceCategory, QObject *parent = NULL);
    virtual ~McaFeedAdapter();

    // NOTE: This is a bit confusing, but ServiceId below corresponds to
    //     the "ServiceName" from servicemodel.h, and ServiceName corresponds to
    //     the "ServiceDisplayName" from servicemodel.h
    //   The roles in servicemodel.h should probably be renamed sometime.
    enum Roles {
        // provided by system, from the service model
        SystemServiceIdRole =       Qt::UserRole + 1,  // QString
        SystemServiceNameRole =     Qt::UserRole + 2,  // QString
        SystemServiceIconRole =     Qt::UserRole + 3,  // QString
        SystemServiceCategoryRole = Qt::UserRole + 4,  // QString
    };

    virtual int limit();
    Q_INVOKABLE virtual void setLimit(int limit);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    virtual void fetchMore(const QModelIndex &parent = QModelIndex());

    QAbstractItemModel *getSource();
signals:
    void limitChanged();

    // source model updates have changed the number of rows provided
    void rowCountChanged();

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
    void rowsInserted(const QAbstractListModel *model, int start, int end);
    void rowsRemoved(const QAbstractListModel *model, int start, int end);

private:
    QAbstractItemModel *m_source;
    QString m_serviceId;
    QString m_serviceName;
    QString m_serviceIcon;
    QString m_serviceCategory;
    int m_limit;
    int m_queuedLimit;
    int m_rowCount;
    int m_lastRowCount;
    bool m_updating;    
};

#endif  // __mcafeedadapter_h
