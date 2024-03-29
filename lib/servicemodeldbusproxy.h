#ifndef SERVICEMODELDBUSPROXY_H
#define SERVICEMODELDBUSPROXY_H

#include <QObject>
#include <QSortFilterProxyModel>
#include "dbustypes.h"
#include "modeldbusinterface.h"
#include "contentroles.h"
#include "dbusdefines.h"

class QDBusInterface;

class ServiceModelDbusProxy : public ModelDBusInterface
{
    Q_OBJECT
public:
    enum Roles {
        // provided by system
        SystemEnabledRole = McaContentRoles::SystemEnabledRole,  // bool
    };

    ServiceModelDbusProxy(const QString &service = CONTENT_DBUS_SERVICE);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;

    bool isServiceEnabled(const QString& upid);
    void setServiceEnabled(const QString& upid, bool enabled);
    void triggerSyncClients();

protected:
    void doOfflineChanged();

signals:
    void syncClients();

private slots:
    void onItemsAdded(ArrayOfMcaServiceItemStruct items);
    void onItemsChanged(ArrayOfMcaServiceItemStruct items);
    void onItemsRemoved(QStringList items);

private:
//    QDBusInterface *m_dbusModel;
    QList<McaServiceItemStruct*> m_feedItems;
};

#endif // SERVICEMODELDBUSPROXY_H
