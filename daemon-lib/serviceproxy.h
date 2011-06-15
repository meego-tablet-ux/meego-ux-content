/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcaserviceproxy_h
#define __mcaserviceproxy_h

#include <QSortFilterProxyModel>
#include <QStringList>

class McaPanelManager;

class McaServiceProxy: public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(QStringList categories READ categories WRITE setCategories NOTIFY categoriesChanged)

public:
    enum Roles {
        // provided by system
        SystemEnabledRole = Qt::UserRole + 2,  // bool
    };

    enum {
        FilterCategories,
        FilterNone,  // i.e. let everything through
        FilterLast  // leave this in the last position
    };

    McaServiceProxy(McaPanelManager *panelmgr, QAbstractItemModel *source, QObject *parent = NULL);
    virtual ~McaServiceProxy();

    int filter();
    void setFilter(int filter);

    QStringList categories();
    void setCategories(const QStringList& categories);

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

signals:
    void filterChanged(int filter);
    void categoriesChanged(const QStringList& categories);

protected slots:
    void setServiceEnabled(const QString& upid, bool enabled);

private:
    McaPanelManager *m_panelmgr;
    QAbstractItemModel *m_source;
    int m_filter;
    QStringList m_categories;
};

#endif  // __mcaserviceproxy_h
