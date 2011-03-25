/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __allocator_h
#define __allocator_h

#include "feedadapter.h"

class McaFeedAdapter;

struct FeedDescriptor
{
    McaFeedAdapter *adapter;
    QString serviceId;
    int target;
    int request;
    int actual;
};

class McaAllocator: public QObject
{
    Q_OBJECT
    Q_PROPERTY(int total READ total WRITE setTotal NOTIFY totalChanged)

public:
    McaAllocator();
    virtual ~McaAllocator();

    virtual void addFeed(const QString& serviceId, McaFeedAdapter *adapter);
    virtual void removeFeed(const QString& serviceId);

    virtual int total();
    virtual void setTotal(int total);

signals:
    void totalChanged();

protected:
    virtual int allocateRemaining();
    virtual void reallocate();

protected slots:
    void reallocateDynamic();

private:
    int m_total;
    QList<FeedDescriptor> m_feeds;
    QHash<QString, McaFeedAdapter*> m_idToFeed;
    int m_nextAlloc;
};

#endif  // __allocator_h
