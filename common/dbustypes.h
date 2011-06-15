#ifndef DBUSTYPES_H
#define DBUSTYPES_H

#include <QMetaType>
#include <QList>
#include <QHash>
#include <QString>
#include <QtDBus>

struct McaFeedItemStruct {
    QString type;           // McaFeedModel::RequiredTypeRole
    QString uniqueId;       // McaFeedModel::RequiredUniqueIdRole
    QDateTime timestamp;    // McaFeedModel::RequiredTimestampRole
    QString uuid;           // McaFeedModel::CommonUuidRole
    QString title;          // McaFeedModel::GenericTitleRole
    QString content;        // McaFeedModel::GenericContentRole
};
typedef QList<McaFeedItemStruct> ArrayOfMcaFeedItemStruct;

struct McaServiceItemStruct {
    QString name;           // McaServiceModel::RequiredNameRole
    QString category;       // McaServiceModel::RequiredCategoryRole
    QString displayName;    // McaServiceModel::CommonDisplayNameRole
    QString iconUrl;        // McaServiceModel::CommonIconUrlRole
    bool configError;       // McaServiceModel::CommonConfigErrorRole
    QString upid;           // McaServiceAdapter::SystemUpidRole
};
typedef QList<McaServiceItemStruct> ArrayOfMcaServiceItemStruct;

// declare operators for marshalling or structs through DBus
QDBusArgument &operator<<(QDBusArgument &arg, const McaFeedItemStruct &item);
const QDBusArgument &operator>>(const QDBusArgument &arg, McaFeedItemStruct &item);

QDBusArgument &operator<<(QDBusArgument &arg, const McaServiceItemStruct &item);
const QDBusArgument &operator>>(const QDBusArgument &arg, McaServiceItemStruct &item);

inline void registerDataTypes()
{
    qDBusRegisterMetaType<McaFeedItemStruct>();
    qDBusRegisterMetaType<ArrayOfMcaFeedItemStruct>();
    qDBusRegisterMetaType<McaServiceItemStruct>();
    qDBusRegisterMetaType<ArrayOfMcaServiceItemStruct>();
}

Q_DECLARE_METATYPE(McaFeedItemStruct)
Q_DECLARE_METATYPE(ArrayOfMcaFeedItemStruct);
Q_DECLARE_METATYPE(McaServiceItemStruct)
Q_DECLARE_METATYPE(ArrayOfMcaServiceItemStruct);

#endif // DBUSTYPES_H
