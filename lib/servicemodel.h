/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcaservicemodel_h
#define __mcaservicemodel_h

#include <QAbstractListModel>

class McaServiceModel: public QAbstractListModel
{
public:
    McaServiceModel(QObject *parent): QAbstractListModel(parent) { }
    virtual ~McaServiceModel() { }

    enum Roles {
        // roles below Qt::UserRole + 10 are reserved for meego-ux-content!

        // [immutable]
        // unique name/id for this service/account (not user-visible)
        RequiredNameRole        = Qt::UserRole + 10,  // QString

        // [immutable]
        // "email", "social", "im", "media", or "other" for now
        RequiredCategoryRole    = Qt::UserRole + 11,  // QString

        // McaActions object
        CommonActionsRole       = Qt::UserRole + 20,  // McaActions*

        // localized display name for this service/account
        CommonDisplayNameRole   = Qt::UserRole + 21,  // QString

        // url for the service icon
        CommonIconUrlRole       = Qt::UserRole + 22,  // QString

        // true if there is a configuration error with this service/account
        CommonConfigErrorRole   = Qt::UserRole + 23,  // bool

        // capability flags
        CommonCapFlagsRole      = Qt::UserRole + 24,  // flags from Capabilities enum
    };

    enum Capabilities {
        ProvidesFeed   = 0x01,
        ProvidesSearch = 0x02
    };
};

#endif  // __mcaservicemodel_h
