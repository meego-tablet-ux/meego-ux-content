#ifndef FEEDPLUGINCONTAINER_H
#define FEEDPLUGINCONTAINER_H

#include <QObject>
#include <QString>

//Qt classes
class QAbstractItemModel;
//Meego classes
class McaFeedPlugin;
class McaSearchableFeed;
class McaSearchableContainer;

class McaFeedPluginContainer : public QObject {
    Q_OBJECT;
public:
    explicit McaFeedPluginContainer( QObject *parent = 0 );
    ~McaFeedPluginContainer();

    Q_INVOKABLE void createFeedModel(const QString &service, int uniqueRequestId);
    Q_INVOKABLE void createSearchModel(const QString &service, const QString &searchText, int uniqueRequestId);
    Q_INVOKABLE QAbstractItemModel *serviceModel();

    void setPath(const QString& pluginPath);
signals:
    void loadCompleted( McaFeedPluginContainer *plugin, const QString &abspath );
    void loadError(const QString &errorString);

    void feedModelCreated(QAbstractItemModel *model, int uniqueRequestId);
    void searchModelCreated(McaSearchableContainer *container, int uniqueRequestId);

public slots:
    void load();

private:
    McaFeedPlugin *m_plugin;
    QString m_pluginPath;
};

#endif // FEEDPLUGINCONTAINER_H
