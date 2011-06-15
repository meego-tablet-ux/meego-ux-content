/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcafeedcache_h
#define __mcafeedcache_h

#include <QAbstractListModel>
#include <QSet>
#include <QMap>

#if 0
class McaActions;
class McaActionsProxy;
#endif

class McaFeedCache: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool frozen READ frozen WRITE setFrozen NOTIFY frozenChanged);

public:
    McaFeedCache(QObject *parent = NULL);
    virtual ~McaFeedCache();

    virtual void setSourceModel(QAbstractListModel *model);

    virtual int rowCount(const QModelIndex& parent) const;
    virtual QVariant data(const QModelIndex& index, int role) const;

    virtual bool frozen();

    enum States {
        StateThawed,
        StateFrozenClean,
        StateFrozenInsert,
        StateFrozenRemove,
        StateFrozenDirty
    };

signals:
    void frozenChanged(bool frozen);

public slots:
    virtual void setFrozen(bool frozen);

protected slots:
    void sourceRowsInserted(const QModelIndex& parent, int start, int end);
    void sourceRowsRemoved(const QModelIndex& parent, int start, int end);
    void sourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void actionsDestroyed(QObject *object);

protected:
    void rowsInserted(int start, int end);
    void rowsRemoved(int start, int end);
    void sendDataChanged();
    void updateRow(QMap<int,QVariant> *map, int row);

private:
    QAbstractListModel *m_source;
    int m_state;
    int m_start;
    int m_end;
    int m_updateStart;
    int m_updateEnd;
    QList<QMap<int,QVariant> *> m_cache;
    QList<int> m_cachedRoles;
#if 0
    QMap<QObject*,McaActionsProxy*> m_safeActions;
#endif
};

#endif  // __mcafeedcache_h
