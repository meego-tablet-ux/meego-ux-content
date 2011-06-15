#ifndef AGGREGATEDMODELPROXY_H
#define AGGREGATEDMODELPROXY_H

#include <QObject>
#include <QAbstractListModel>
#include "dbustypes.h"

class QDBusInterface;

class McaAggregatedModelProxy : public QAbstractListModel
{
    Q_OBJECT
public:
    McaAggregatedModelProxy(const QString &service, const QString &objectPath);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;

private slots:
    void onItemsAdded(ArrayOfMcaFeedItemStruct items);
    void onItemsChanged(ArrayOfMcaFeedItemStruct items);
    void onItemsRemoved(QStringList items);

private:
    QDBusInterface *m_dbusModel;
    QList<McaFeedItemStruct*> m_feedItems;
};

#endif // AGGREGATEDMODELPROXY_H
