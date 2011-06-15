/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __feedplugin_h
#define __feedplugin_h

#include <QString>

class QAbstractItemModel;
class McaSearchableFeed;

class McaFeedPlugin
{
public:
    McaFeedPlugin() {}
    virtual ~McaFeedPlugin() {}

    // service model owned by the plugin, should be destroyed on unload
    // return a QAbstractItemModel that implements McaServiceModel roles
    // only column 0 matters
    virtual QAbstractItemModel *serviceModel() = 0;

    // the returned model is owned by the caller
    // return NULL if unsupported
    // return a QAbstractItemModel that implements McaFeedModel roles
    // only column 0 matters
    virtual QAbstractItemModel *createFeedModel(const QString& service) = 0;

    // the returned model is owned by the caller
    // return a QAbstractItemModel that also inherits McaSearchableFeed
    //   or NULL if unsupported
    virtual McaSearchableFeed *createSearchModel(const QString& service,
                                                 const QString& searchText) = 0;
};

Q_DECLARE_INTERFACE(McaFeedPlugin, "com.meego.content.FeedPlugin/0.3")

#endif  // __feedplugin_h
