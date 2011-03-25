/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "pluginloaderthread.h"
#include "feedplugin.h"
#include <QPluginLoader>

PluginLoaderThread::PluginLoaderThread(QObject *parent) :
    QThread(parent)
{
}

void PluginLoaderThread::run()
{
    exec();
}

void PluginLoaderThread::loadPlugin(QString absPath)
{
    QPluginLoader loader(absPath);
    McaFeedPlugin *plugin = qobject_cast<McaFeedPlugin *>(loader.instance());
    if (plugin)
        emit this->loadCompleted(plugin, absPath);
    else
        emit this->loadError(loader.errorString());
}
