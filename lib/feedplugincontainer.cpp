#include "memoryleak.h"

#include <QAbstractItemModel>
#include <QDebug>
#include <QPluginLoader>

#include "feedplugincontainer.h"
#include "feedplugin.h"
#include "searchablecontainer.h"
#include "servicemodel.h"
#include "feedfilter.h"
#include "aggregatedmodel.h"
#include "feedadapter.h"

#include "memoryleak-defines.h"

McaFeedPluginContainer::McaFeedPluginContainer(QObject *parent)
      : QObject(parent),
        m_plugin(0),
        m_pluginPath(QString::null)
{
    THREAD_TEST_INIT
}

McaFeedPluginContainer::~McaFeedPluginContainer()
{
    delete m_plugin;
}

void McaFeedPluginContainer::createFeedModel(const QString &service, int uniqueRequestId, const QString &upid)
{
    qDebug() << "McaFeedPluginContainer::createFeedModel " << service << uniqueRequestId;

    QAbstractItemModel *feed = m_plugin->createFeedModel(service);
    if(0 == feed) {
        emit createFeedError(service, uniqueRequestId);
        return;
    }

    QString currentServiceName = "";
    QAbstractItemModel *model = m_plugin->serviceModel();
    int index = 0;
    QModelIndex modelIndex;
    while(service != currentServiceName && index < model->rowCount()) {
        modelIndex = model->index(index, 0);
        currentServiceName = model->data(modelIndex, McaServiceModel::RequiredNameRole).toString();
        index++;
    }

    QString name = model->data(modelIndex, McaServiceModel::RequiredNameRole).toString();
    QString displayName = model->data(modelIndex, McaServiceModel::CommonDisplayNameRole).toString();
    QString iconUrl = model->data(modelIndex, McaServiceModel::CommonIconUrlRole).toString();
    QString category = model->data(modelIndex, McaServiceModel::RequiredCategoryRole).toString();    

    McaFeedFilter *filter = new McaFeedFilter(feed, upid);
    McaFeedAdapter *adapter = new McaFeedAdapter(filter, name, displayName, iconUrl, category);

    THREAD_SET_TEST(this);
    emit feedModelCreated(qobject_cast<QObject*>(feed), adapter, uniqueRequestId);
    THREAD_UNSET_TEST(this);
}

void McaFeedPluginContainer::createSearchModel(const QString &service, const QString &searchText, int uniqueRequestId)
{    
    qDebug() << "McaFeedPluginContainer::createSearchModel " << service << searchText << uniqueRequestId;
    McaSearchableFeed *searchableFeed = m_plugin->createSearchModel(service, searchText);
    if(0 == searchableFeed) {
        emit createFeedError(service, uniqueRequestId);
        return;
    }
    McaSearchableContainer *container = McaSearchableContainer::create(searchableFeed);

    QString currentServiceName = "";
    QAbstractItemModel *model = m_plugin->serviceModel();
    int index = 0;
    QModelIndex modelIndex;
    while(service != currentServiceName && index < model->rowCount()) {
        modelIndex = model->index(index, 0);
        currentServiceName = model->data(modelIndex, McaServiceModel::RequiredNameRole).toString();
        index++;
    }

    QString name = model->data(modelIndex, McaServiceModel::RequiredNameRole).toString();
    QString displayName = model->data(modelIndex, McaServiceModel::CommonDisplayNameRole).toString();
    QString iconUrl = model->data(modelIndex, McaServiceModel::CommonIconUrlRole).toString();
    QString category = model->data(modelIndex, McaServiceModel::RequiredCategoryRole).toString();

    McaFeedAdapter *adapter = new McaFeedAdapter(container->feedModel(), name, displayName, iconUrl, category);

    THREAD_SET_TEST(this);
    emit feedModelCreated(qobject_cast<QObject*>(container), adapter, uniqueRequestId);
    THREAD_UNSET_TEST(this);
}

QAbstractItemModel *McaFeedPluginContainer::serviceModel()
{
    return m_plugin->serviceModel();
}

void McaFeedPluginContainer::setPath(const QString& pluginPath)
{
    m_pluginPath = pluginPath;
}

void McaFeedPluginContainer::load()
{
    if(m_pluginPath.isNull()) {
        qWarning() << "McaFeedPluginContainer::load(): WARNING: plugin path is empty";
        return;
    }

    QPluginLoader loader(m_pluginPath);
    m_plugin = qobject_cast<McaFeedPlugin *>(loader.instance());
    if (m_plugin)
        emit this->loadCompleted(this, m_pluginPath);
    else
        emit this->loadError(this, loader.errorString());
}
