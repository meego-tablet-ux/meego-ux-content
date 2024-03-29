#ifndef AGGREGATEDMODELPROXY_H
#define AGGREGATEDMODELPROXY_H

#include <QObject>
#include <QAbstractListModel>
#include "dbustypes.h"
#include "modeldbusinterface.h"
#include "dbusdefines.h"

class QDBusInterface;
class McaActionsProxy;

class McaAggregatedModelProxy : public ModelDBusInterface
{
    Q_OBJECT
    Q_PROPERTY(bool frozen READ frozen WRITE setFrozen NOTIFY frozenChanged);

public:
    McaAggregatedModelProxy(const QString &service = CONTENT_DBUS_SERVICE);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;

    bool frozen();
public slots:
    void setFrozen(bool frozen);

signals:
    void frozenChanged(bool frozen);

private slots:
    void onItemsAdded(ArrayOfMcaFeedItemStruct items);
    void onItemsChanged(ArrayOfMcaFeedItemStruct items);
    void onItemsRemoved(ArrayOfMcaFeedItemId items);

    void doStandardAction(const QString& action, const QString& uniqueId);
    void doCustomAction(const QString& action, const QString& uniqueId);

protected:
    void doOfflineChanged();

private:
    void onItemsAddedInternal(ArrayOfMcaFeedItemStruct *items);
    void onItemsChangedInternal(ArrayOfMcaFeedItemStruct *items);
    void onItemsRemovedInternal(ArrayOfMcaFeedItemId *items);

    enum feeditem_event_enum_e {
        FEEDITEM_ADD,
        FEEDITEM_CHANGE,
        FEEDITEM_REMOVE
    };

    struct feeditem_event_s {
        enum feeditem_event_enum_e type;
        union payload_u {
            ArrayOfMcaFeedItemStruct *addchange_items;
            ArrayOfMcaFeedItemId *remove_list;
        } u;
    };

    bool m_frozen;
//    QDBusInterface *m_dbusModel;
    QQueue<struct feeditem_event_s *> m_frozenQueue;
    QList<McaFeedItemStruct*> m_feedItems;
    QList<McaActionsProxy*> m_feedActions;
};

#endif // AGGREGATEDMODELPROXY_H
