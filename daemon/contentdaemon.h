#ifndef CONTENTDAEMON_H
#define CONTENTDAEMON_H

#include <QtDBus>
#include <QtCore/QCoreApplication>
#include <QMap>
#include <QModelIndex>

#include "contentdaemoninterface.h"
#include "searchmanagerdbus.h"
#include "panelmanagerdbus.h"

class McaSearchManagerAdaptor;

class ContentDaemon : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.meego.content"); // build complains when using CONTENT_DBUS_SERVICE macro
public:
    ContentDaemon(QCoreApplication *application);

public slots:
//    QString newPanelManager();
    QString newSearchManager();

    bool release(const QString &objectPath);


private:
    static QString uniqueId();

private:
    QMap<QString, McaPanelManagerDBus*> m_panelManagers;
    QMap<QString, McaSearchManagerDBus*> m_searchManagers;
};

#endif // CONTENTDAEMON_H
