
#include "dbustypes.h"

QDBusArgument &operator<<(QDBusArgument &arg, const McaFeedItemStruct &item)
{
    arg.beginStructure();
    arg << item.type << item.uniqueId << item.timestamp << item.uuid << item.title << item.content;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, McaFeedItemStruct &item)
{
    arg.beginStructure();
    arg >> item.type >> item.uniqueId >> item.timestamp >> item.uuid >> item.title >> item.content;
    arg.endStructure();
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const McaServiceItemStruct &item)
{
    arg.beginStructure();
    arg << item.name << item.category << item.displayName << item.iconUrl << item.configError << item.upid << item.enabled;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, McaServiceItemStruct &item)
{
    arg.beginStructure();
    arg >> item.name >> item.category >> item.displayName >> item.iconUrl >> item.configError >> item.upid >> item.enabled;
    arg.endStructure();
    return arg;
}
