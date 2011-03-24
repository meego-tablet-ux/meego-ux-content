/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcasearchmanager_h
#define __mcasearchmanager_h

#include <QModelIndex>
#include <QStringList>

#include "abstractmanager.h"

class McaSearchManager: public AbstractManager
{
    Q_OBJECT
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

public:
    McaSearchManager(QObject *parent = NULL);
    virtual ~McaSearchManager();

    Q_INVOKABLE void initialize(const QString& managerData);

    virtual QString searchText();

signals:
    void searchTextChanged(const QString& searchText);

public slots:
    void setSearchText(const QString& searchText);

//protected slots:
//    void createFeedDone(QObject *containerObj, McaFeedAdapter *feedAdapter, int uniqueRequestId);

private:
    virtual QModelIndex serviceModelIndex(int row);
    virtual QVariant serviceModelData(const QModelIndex& index, int role);
    virtual bool dataChangedCondition(const QModelIndex& index);

    virtual int createFeed(const QAbstractItemModel *serviceModel, const QString& name);
    virtual void createFeedFinalise(QObject *containerObj, McaFeedAdapter *feedAdapter, FeedInfo *feedInfo);

private:
    QAbstractItemModel *m_serviceModel;
    QString m_searchText;
};

#endif  // __mcasearchmanager_h
