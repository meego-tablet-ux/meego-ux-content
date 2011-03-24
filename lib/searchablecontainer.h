/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcasearchablecontainer_h
#define __mcasearchablecontainer_h

#include <QObject>

class QAbstractItemModel;
class McaSearchableFeed;

class McaSearchableContainer: public QObject
{
    Q_OBJECT

public:
    static McaSearchableContainer *create(McaSearchableFeed *searchable);

protected:
    McaSearchableContainer(McaSearchableFeed *searchable, QObject *parent = NULL);

public:
    virtual ~McaSearchableContainer();

    bool isValid();

    McaSearchableFeed *searchable();
    QAbstractItemModel *feedModel();

signals:
    void searchDone();

public slots:
    Q_INVOKABLE void setSearchText(const QString& text);

private:
    McaSearchableFeed *m_searchable;
    QAbstractItemModel *m_feedModel;
};

#endif  // __mcasearchablecontainer_h
