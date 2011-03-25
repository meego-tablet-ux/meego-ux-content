/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>

#include <QAbstractItemModel>

#include "searchablecontainer.h"
#include "feedmodel.h"

//
// Overview of McaSearchableContainer
//   - wrapper for McaSearchable feed
//   - ensures feeds are valid, implementing both McaSearchableFeed and QAbstractItemModel
//   - provides QObject for slots (currently just setSearchText)
//

McaSearchableContainer *McaSearchableContainer::create(McaSearchableFeed *searchable)
{
    McaSearchableContainer *container = new McaSearchableContainer(searchable);
    if (container && container->isValid())
        return container;
    delete container;
    return NULL;
}

McaSearchableContainer::McaSearchableContainer(McaSearchableFeed *searchable, QObject *parent):
        QObject(parent)
{
    m_searchable = dynamic_cast<McaSearchableFeed*>(searchable);
    m_feedModel = dynamic_cast<QAbstractItemModel*>(searchable);
}

McaSearchableContainer::~McaSearchableContainer()
{
    delete m_searchable;
}

bool McaSearchableContainer::isValid()
{
    if (m_searchable && m_feedModel)
        return true;
    return false;
}

McaSearchableFeed *McaSearchableContainer::searchable()
{
    return m_searchable;
}

QAbstractItemModel *McaSearchableContainer::feedModel()
{
    return m_feedModel;
}

void McaSearchableContainer::setSearchText(const QString &text)
{
    m_searchable->setSearchText(text);
}
