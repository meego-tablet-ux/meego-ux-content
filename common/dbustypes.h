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

QDBusArgument &operator<<(QDBusArgument &arg, const McaFeedItemStruct &item);
const QDBusArgument &operator>>(const QDBusArgument &arg, McaFeedItemStruct &item);

inline void registerDataTypes()
{
    qDBusRegisterMetaType<McaFeedItemStruct>();
    qDBusRegisterMetaType<ArrayOfMcaFeedItemStruct>();
}

Q_DECLARE_METATYPE(McaFeedItemStruct)
Q_DECLARE_METATYPE(ArrayOfMcaFeedItemStruct);

#endif // DBUSTYPES_H
