/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcafeedmanager_h
#define __mcafeedmanager_h

#include <QHash>

class QFileSystemWatcher;
class QAbstractItemModel;
class QSettings;
//class McaFeedPlugin;
class McaFeedPluginContainer;
class McaAggregatedServiceModel;
class McaSearchableContainer;
class McaSearchableFeed;

class McaFeedManager: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *serviceModel READ serviceModel);

public:
    static McaFeedManager *takeManager();
    static void releaseManager();

protected:
    McaFeedManager();

public:
    virtual ~McaFeedManager();

    // aggregated service model owned by the feed manager
    QAbstractItemModel *serviceModel();

    int createFeed(const QAbstractItemModel *serviceModel,
                                   const QString& name);
//    McaSearchableContainer *createSearchFeed(const QAbstractItemModel *serviceModel,
    int createSearchFeed(const QAbstractItemModel *serviceModel,
                                             const QString& name, const QString& searchText);
    QString serviceId(const QAbstractItemModel *serviceModel,
                      const QString& name);

signals:
    void searchFeedCreated(McaSearchableContainer *container, int uniqueRequestId);
    void feedCreated(QAbstractItemModel *model, int uniqueRequestId);

protected slots:
    void loadPlugins();
    void onLoadCompleted(McaFeedPluginContainer *plugin, const QString &absPath);
    void onLoadError(const QString &errorString);

protected:
    void addPlugin(McaFeedPluginContainer *plugin, const QString& abspath);
    QString getHash(QSettings *settings, const QString& path,
                    const QString& name, int pass);

private:
    QHash<McaFeedPluginContainer *, QString> m_pluginToPaths;
    McaAggregatedServiceModel *m_services;
    QFileSystemWatcher* m_watcher;
    QHash<const QAbstractItemModel*, McaFeedPluginContainer*> m_modelToPlugin;

    // map from pluginPath:serviceName to a unique hash
    QHash<QString, QString> m_idToHash;

    int m_requestIdCounter;
};

#endif  // __mcafeedmanager_h
