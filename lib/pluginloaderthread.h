/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef PLUGINLOADERTHREAD_H
#define PLUGINLOADERTHREAD_H

#include <QThread>
#include <QString>

class McaFeedPlugin;

class PluginLoaderThread : public QThread
{
    Q_OBJECT
public:
    explicit PluginLoaderThread(QObject *parent = 0);

    void run();

signals:
    void loadCompleted(McaFeedPlugin *plugin, QString absPath);
    void loadError(const QString &errorString);

public slots:
    void loadPlugin(QString absPath);

};

#endif // PLUGINLOADERTHREAD_H
