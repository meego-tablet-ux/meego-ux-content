/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "memoryleak.h"

#include <QDebug>
#include "allocator.h"

#include "memoryleak-defines.h"

//
// public methods
//

McaAllocator::McaAllocator()
{
    // default to 30 items
    m_total = 30;
    m_nextAlloc = 0;
}

McaAllocator::~McaAllocator()
{
}

void McaAllocator::addFeed(const QString& serviceId, McaFeedAdapter *adapter)
{
    qDebug() << "McaAllocator::addFeed() " << serviceId;
    FeedDescriptor fd;
    fd.adapter = adapter;
    fd.serviceId = serviceId;
    fd.target = 0;
    fd.actual = 0;
    m_feeds.append(fd);
    m_idToFeed.insert(serviceId, adapter);
    connect(adapter, SIGNAL(rowCountChanged()), this, SLOT(reallocateDynamic()));
    reallocate();
}

void McaAllocator::removeFeed(const QString& serviceId)
{    
    int count = m_feeds.count();
    for (int i = 0; i < count; i++) {
        if (m_feeds[i].serviceId == serviceId) {
            disconnect(m_feeds[i].adapter, SIGNAL(rowCountChanged()),
                       this, SLOT(reallocateDynamic()));
            m_feeds.removeAt(i);
            break;
        }
    }
    qDebug() << "McaAllocator::removeFeed() " << serviceId << " left " << m_feeds.count();
    m_idToFeed.remove(serviceId);
    reallocate();
}

int McaAllocator::total()
{
    return m_total;
}

void McaAllocator::setTotal(int total)
{
    if (m_total != total) {
        m_total = total;
        emit totalChanged();
        reallocate();
    }
}

//
// protected methods
//

int McaAllocator::allocateRemaining()
{
    // find how many slots were not filled by allocated models
    int remaining = m_total;

    // track which feeds may still have more data to provide
    QList<int> remainingFeeds;

    int count = m_feeds.count();
    int insertion = 0;
    for (int i = 0; i < count; i++) {
        FeedDescriptor& fd = m_feeds[i];
        fd.actual = fd.adapter->rowCount();
        remaining -= fd.actual;
        if (fd.request <= fd.actual) {
            // this feed may be able to provide more items
            if (i >= m_nextAlloc)
                remainingFeeds.insert(insertion++, i);
            else
                remainingFeeds.append(i);
        }
    }

    count = remainingFeeds.count();
    if (count <= 0)
        return -1;

    int origRemaining = remaining;
    int base = remaining / count;
    remaining -= count * base;

    for (int i = 0; i < count; i++) {
        int add = base;
        int feed = remainingFeeds[i];
        if (remaining > 0) {
            add++;
            remaining--;
            m_nextAlloc = (feed + 1) % m_feeds.count();
        }

        FeedDescriptor& fd = m_feeds[feed];
        if (add > 0) {
            fd.request += add;
            //fd.adapter->setLimit(fd.request);
            QMetaObject::invokeMethod(fd.adapter, "setLimit", Q_ARG(int, fd.request));
        }
    }

    return origRemaining;
}

void McaAllocator::reallocate()
{
    int count = m_feeds.count();
    if (count <= 0)
        return;

    int base = m_total / count;

    // simply divide up the available slots among the feeds
    int remain = m_total - count * base;
    m_nextAlloc = 0;
    for (int i = 0; i < count; i++) {
        int target = base;
        if (remain > 0) {
            target++;
            remain--;
            m_nextAlloc++;
        }
        m_feeds[i].target = target;
    }

    reallocateDynamic();
}

//
// protected slots
//

void McaAllocator::reallocateDynamic()
{
    int count = m_feeds.count();
    if (count <= 0)
        return;

    for (int i = 0; i < count; i++) {
        FeedDescriptor& fd = m_feeds[i];
        fd.request = fd.target;        
        //fd.adapter->setLimit(fd.request);
        QMetaObject::invokeMethod(fd.adapter, "setLimit", Q_ARG(int, fd.request));
    }

    int remaining = m_total;
    while (true) {
       int remain = allocateRemaining();
       // stop trying if there is an error, none remaining, or no change
       if (remain <= 0 || remain == remaining)
           break;
       remaining = remain;
    }
}
