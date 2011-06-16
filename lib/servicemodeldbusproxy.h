#ifndef SERVICEMODELDBUSPROXY_H
#define SERVICEMODELDBUSPROXY_H

#include <QObject>
#include <QSortFilterProxyModel>
#include "dbustypes.h"

class QDBusInterface;

class ServiceModelDbusProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    ServiceModelDbusProxy(const QString &service, const QString &objectPath);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;

    bool isServiceEnabled(const QString& upid);
    void setServiceEnabled(const QString& upid, bool enabled);

private slots:
    void onItemsAdded(ArrayOfMcaServiceItemStruct items);
    void onItemsChanged(ArrayOfMcaServiceItemStruct items);
    void onItemsRemoved(QStringList items);

private:
    QDBusInterface *m_dbusModel;
    QList<McaServiceItemStruct*> m_feedItems;
};

#endif // SERVICEMODELDBUSPROXY_H
