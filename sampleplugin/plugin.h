/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __sampleplugin_h
#define __sampleplugin_h

#include <QObject>
#include <QStringList>

#include <feedplugin.h>

class McaServiceModel;
class McaFeedModel;
class ServiceModel;
class McaSearchableFeed;

// An MCA plugin to implement the McaFeedPlugin interface
class Plugin: public QObject, public McaFeedPlugin
{
    Q_OBJECT
    Q_INTERFACES(McaFeedPlugin)

public:
    explicit Plugin(QObject *parent = NULL);
    virtual ~Plugin();

    // Your plugin must provide a service model, which is basically a list
    //   of different feeds you provide. Most plugins will only provide one
    //   "service", but you could provide one for each account, for example,
    //   if there is more than one.
    QAbstractItemModel *serviceModel();

    // Through the service model, you will provide a service id/name string
    //   for each item. These must be unique among the services your plugin
    //   provides. When this next function is called, you will be passed the
    //   unique service name and you create the actual feed model to provide
    //   its data.
    // You should be able to return multiple models for the same service.
    QAbstractItemModel *createFeedModel(const QString& service);

    // Through the service model, you will provide a service id/name string
    //   for each item. These must be unique among the services your plugin
    //   provides. When this next function is called, you will be passed the
    //   unique service name and you create the actual feed model to provide
    //   its data.
    // You should be able to return multiple models for the same service.
    McaSearchableFeed *createSearchModel(const QString& service,
                                         const QString& searchText);

private:
    ServiceModel *m_serviceModel;
};

#endif  // __sampleplugin_h
