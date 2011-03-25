/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __testplugin_h
#define __testplugin_h

#include <QHash>

#include <feedplugin.h>

class QLocalServer;
class McaServiceModel;
class McaFeedModel;
class ServiceModel;
class TestModel;

// An MCA plugin to implement the McaFeedPlugin interface
class Plugin: public QObject, public McaFeedPlugin
{
    Q_OBJECT
    Q_INTERFACES(McaFeedPlugin)

public:
    enum CommandType {
        TestCommandReset,
        TestCommandAddService,
        TestCommandRemoveService,
        TestCommandAddContentItem,
        TestCommandAddRequestItem,
        TestCommandRemoveItem,
    };

    struct TestCommand
    {
        int type;
        int id;
        int len;
        char data[];
    };

    explicit Plugin(QObject *parent = NULL);
    virtual ~Plugin();

    int capabilityFlags();

    QAbstractItemModel *serviceModel();
    QAbstractItemModel *createFeedModel(const QString& service);
    McaSearchableFeed *createSearchModel(const QString& service,
                                         const QString& searchText);

protected slots:
    void newConnection();
    void readyRead();
    void modelDestroyed(QObject *object);

private:
    ServiceModel *m_serviceModel;
    QHash<QString, TestModel*> m_serviceToModel;
    QLocalServer *m_server;
};

#endif  // __testplugin_h
