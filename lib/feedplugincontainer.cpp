#include <QAbstractItemModel>
#include <QDebug>
#include <QPluginLoader>

#include "feedplugincontainer.h"
#include "feedplugin.h"
#include "searchablecontainer.h"


McaFeedPluginContainer::McaFeedPluginContainer(QObject *parent)
      : QObject(parent),
        m_plugin(0),
        m_pluginPath(QString::null)
{    
}

McaFeedPluginContainer::~McaFeedPluginContainer()
{
    delete m_plugin;
}

void McaFeedPluginContainer::createFeedModel(const QString &service, int uniqueRequestId)
{
    qDebug() << "McaFeedPluginContainer::createFeedModel " << service << uniqueRequestId;
    emit feedModelCreated(m_plugin->createFeedModel(service), uniqueRequestId);
}

void McaFeedPluginContainer::createSearchModel(const QString &service, const QString &searchText, int uniqueRequestId)
{    
    qDebug() << "McaFeedPluginContainer::createSearchModel " << service << searchText << uniqueRequestId;
    McaSearchableContainer *container = McaSearchableContainer::create(m_plugin->createSearchModel(service, searchText));
    emit searchModelCreated(container, uniqueRequestId);
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
        emit this->loadError(loader.errorString());
}
