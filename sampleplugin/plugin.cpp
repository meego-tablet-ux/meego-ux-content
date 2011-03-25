/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>

#include <QtPlugin>

#include "plugin.h"
#include "emailmodel.h"
#include "socialmodel.h"
#include "servmodel.h"
#include "filter.h"

Plugin::Plugin(QObject *parent): QObject(parent), McaFeedPlugin()
{
    m_serviceModel = new ServiceModel;
}

Plugin::~Plugin()
{
    delete m_serviceModel;
}

QAbstractItemModel *Plugin::serviceModel()
{
    return m_serviceModel;
}

QAbstractItemModel *Plugin::createFeedModel(const QString& service)
{
    qDebug() << "Plugin::createFeedModel: " << service;

    if (service == "email1")
        return new EmailModel;
    else if (service == "network1")
        return new SocialModel;
    return NULL;
}

McaSearchableFeed *Plugin::createSearchModel(const QString& service,
                                             const QString& searchText)
{
    Q_UNUSED(searchText)
    qDebug() << "Plugin::createSearchModel: " << service;
    if (service == "email1")
        return new Filter(new EmailModel);
    else if (service == "network1")
        return new Filter(new SocialModel);
    return NULL;
}

Q_EXPORT_PLUGIN2(sample_plugin, Plugin)
