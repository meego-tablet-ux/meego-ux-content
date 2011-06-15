/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcafeedmodel_h
#define __mcafeedmodel_h

#include <QAbstractListModel>

class McaFeedModel: public QAbstractListModel
{
public:
    explicit McaFeedModel(QObject *parent): QAbstractListModel(parent) { }
    virtual ~McaFeedModel() { }

    enum Roles {
        // roles below Qt::UserRole + 10 are reserved for meego-ux-content!

        // "content", "picture", or "request" for now
        RequiredTypeRole      = Qt::UserRole + 10,  // QString

        // uniquely identify the item within this feed, persistent across time
        RequiredUniqueIdRole  = Qt::UserRole + 11,  // QString
        RequiredTimestampRole = Qt::UserRole + 12,  // QDateTime

        // McaActions object
        CommonActionsRole     = Qt::UserRole + 20,  // McaActions*

        // if available, this is a true uuid unique across all feeds and time
        CommonUuidRole        = Qt::UserRole + 21,  // QString

        // generic roles are used for fields in a standard view
        GenericTitleRole      = Qt::UserRole + 30,  // QString
        GenericContentRole    = Qt::UserRole + 31,  // QString

        GenericRelevanceRole  = Qt::UserRole + 32,  // qreal, scale from 0.0 to 1.0

        // thumbnail photo for message author
        GenericAvatarUrlRole  = Qt::UserRole + 33,  // QString

        // thumbnail for shared photo(s)
        GenericPictureUrlRole = Qt::UserRole + 34,  // QString

        // localized text for accept button of request
        GenericAcceptTextRole = Qt::UserRole + 35,  // QString

        // localized text for reject button of request
        GenericRejectTextRole = Qt::UserRole + 36,  // QString

        CustomRole            = Qt::UserRole + 50,  // defined by plugin
    };
};

// additional interface to implement for a search model
class McaSearchableFeed {
public:
    McaSearchableFeed(): m_haltSearch(false) {}

    virtual void setSearchText(const QString& text) = 0;
    void haltSearch() { m_haltSearch = true; }
    void resetSearchHalt() { m_haltSearch = false; } 

protected:
    bool isSearchHalted() { return m_haltSearch; }

private:
    bool m_haltSearch;
};

#endif  // __mcafeedmodel_h
