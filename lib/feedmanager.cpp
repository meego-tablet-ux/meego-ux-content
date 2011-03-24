/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>

#include <QCoreApplication>
#include <QPluginLoader>
#include <QDir>
#include <QFileSystemWatcher>
#include <QSettings>

#include "feedmanager.h"
//#include "feedplugin.h"
#include "feedplugincontainer.h"
#include "aggregatedservicemodel.h"
#include "settings.h"
#include "serviceadapter.h"
#include "searchablecontainer.h"
#include "pluginloaderthread.h"

//
// Overview of McaFeedManager
//    - loads content plugins
//    - assembles aggregation of service models from all plugins
//    - passes on feed requests to the right plugin and service
//    - generates unique ids for services and stores some info in QSettings
//

const char PLUGIN_RELPATH[] = "/MeeGo/Content";

//
// static members
//

static McaFeedManager *s_manager = NULL;
static int s_refCount = 0;

McaFeedManager *McaFeedManager::takeManager()
{
    if (!s_manager)
        s_manager = new McaFeedManager;
    s_refCount++;
    return s_manager;
}

void McaFeedManager::releaseManager()
{
    if (s_manager) {
        s_refCount--;
        if (s_refCount == 0) {
            delete s_manager;
            s_manager = NULL;
        }
    }
}

//
// members
//

McaFeedManager::McaFeedManager()
{
    m_requestIdCounter = 0;

    m_watcher = new QFileSystemWatcher;
    m_services = new McaAggregatedServiceModel;

    loadPlugins();

    // watch for new plugins getting installed and load them
    QStringList paths;
    paths << "/home/radu/work/meego/gits/content-build-meego/sampleplugin";
//    foreach (QString path, QCoreApplication::libraryPaths())
//        paths << path + PLUGIN_RELPATH;
    m_watcher->addPaths(paths);
    connect(m_watcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(loadPlugins()));

    // TODO: watch for plugin updates, removals too?
}

McaFeedManager::~McaFeedManager()
{
    delete m_watcher;
    delete m_services;
    foreach (McaFeedPluginContainer *plugin, m_pluginToPaths.keys()) {
        // Terminate the threads associated with the plugins
        plugin->thread()->terminate();
        // Block main thread for 10 seconds (or when thread finishes)
        if( !plugin->thread()->wait(10000) ) {
            qWarning() << "Plugin thread is not responding";
        }
        delete plugin;
    }
}

QAbstractItemModel *McaFeedManager::serviceModel()
{
    return m_services;
}

int McaFeedManager::createFeed(const QAbstractItemModel *serviceModel,
                                               const QString& name)
{
    // TODO: clean up destroyed models

    McaFeedPluginContainer *plugin = m_modelToPlugin.value(serviceModel);
    if (plugin) {
        QMetaObject::invokeMethod(plugin, "createFeedModel", Qt::QueuedConnection, Q_ARG(QString, name), Q_ARG(int, m_requestIdCounter));
        return ++m_requestIdCounter;
    }

    return -1;
}

//McaSearchableContainer *McaFeedManager::createSearchFeed(const QAbstractItemModel *serviceModel,
int McaFeedManager::createSearchFeed(const QAbstractItemModel *serviceModel,
                                                         const QString& name, const QString& searchText)
{
    // TODO: clean up destroyed models
    McaFeedPluginContainer *plugin = m_modelToPlugin.value(serviceModel);
    if (plugin) {
        QMetaObject::invokeMethod(plugin, "createSearchModel", Qt::QueuedConnection, Q_ARG(QString, name), Q_ARG(QString, searchText), Q_ARG(int, m_requestIdCounter));
//        m_requestIdCounter++;
        return ++m_requestIdCounter;
    }
    return -1;
}

QString McaFeedManager::getHash(QSettings *settings, const QString& path,
                                const QString& name, int pass)
{
    QString key = path + ":" + name;
    if (pass > 0)
        key.append(QString::number(pass));

    QString hash = QString::number(qHash(key), 16);
    settings->beginGroup(hash);
    QString settingsPath = settings->value(McaSettings::KeyPluginPath).toString();
    QString settingsName = settings->value(McaSettings::KeyServiceName).toString();

    if (settingsPath.isEmpty()) {
        // no such section, so create one
        settings->setValue(McaSettings::KeyPluginPath, path);
        settings->setValue(McaSettings::KeyServiceName, name);
    }
    else if (settingsPath != path || settingsName != name)
        // found non-matching section, return to iterate
        hash.clear();
    // otherwise, hash is good so return it

    settings->endGroup();
    return hash;
}

QString McaFeedManager::serviceId(const QAbstractItemModel *serviceModel,
                                  const QString& name)
{
    // returns: empty string on error, unique hash otherwise

    McaFeedPluginContainer *plugin = m_modelToPlugin.value(serviceModel);
    if (!plugin)
        return QString();

    QString pluginPath = m_pluginToPaths.value(plugin);
    QString serviceKey = pluginPath + ":" + name;
    QString hash = m_idToHash.value(serviceKey);

    if (hash.isEmpty()) {
        // haven't seen this service yet, so check QSettings file
        QSettings settings(McaSettings::Organization, McaSettings::ApplicationCore);
        int pass = 0;
        while (true) {
            hash = getHash(&settings, pluginPath, name, pass++);
            if (!hash.isEmpty())
                break;
        }
        m_idToHash[serviceKey] = hash;
    }

    return hash;
}

//
// protected slots
//

void McaFeedManager::loadPlugins()
{
    qDebug() << "McaFeedManager::loadPlugins()";
    // effects: checks plugin paths for any new plugins to load
//    foreach (QString path, QCoreApplication::libraryPaths()) {
//        QDir dir = QDir(path + PLUGIN_RELPATH);
        QDir dir = QDir("/home/radu/work/meego/gits/content-build-meego/sampleplugin");
        foreach (QString filename, dir.entryList(QStringList() << QString("*.so"))) {
            QString abspath = dir.absoluteFilePath(filename);
            if (m_pluginToPaths.values().contains(abspath))
                continue;

            McaFeedPluginContainer *pluginContainer = new McaFeedPluginContainer();
            pluginContainer->setPath(abspath);

            connect(pluginContainer, SIGNAL(loadCompleted(McaFeedPluginContainer*,QString)), this, SLOT(onLoadCompleted(McaFeedPluginContainer*,QString)));
            connect(pluginContainer, SIGNAL(loadError(QString)), this, SLOT(onLoadError(QString)));
            connect(pluginContainer, SIGNAL(searchModelCreated(McaSearchableContainer*,int)), this, SIGNAL(searchFeedCreated(McaSearchableContainer*,int)));
            connect(pluginContainer, SIGNAL(feedModelCreated(QAbstractItemModel*,int)), this, SIGNAL(feedCreated(QAbstractItemModel*,int)));

            QThread *pluginThread = new QThread(this);
            connect(pluginThread, SIGNAL(started()), pluginContainer, SLOT(load()));
            pluginContainer->moveToThread(pluginThread);
            pluginThread->start();
        }
//    }
}

void McaFeedManager::onLoadCompleted(McaFeedPluginContainer *plugin, const QString &absPath)
{
    if (plugin) {
        addPlugin(plugin, absPath);
    } else {
        qWarning() << "Error loading plugin: received null plugin from threaded loader!";
    }
}

void McaFeedManager::onLoadError(const QString &errorString)
{
    qWarning() << "Error loading plugin: " << errorString;
}

//
// protected members
//

void McaFeedManager::addPlugin(McaFeedPluginContainer *plugin, const QString& abspath)
{
    m_pluginToPaths.insert(plugin, abspath);
    QAbstractItemModel *model = plugin->serviceModel();
    McaServiceAdapter *adapter = new McaServiceAdapter(this, this);
    m_modelToPlugin.insert(adapter, plugin);
    adapter->setSourceModel(model);
    m_services->addSourceModel(adapter);
}
