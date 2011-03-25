/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>

#include <QtPlugin>
#include <QLocalServer>
#include <QLocalSocket>

#include "plugin.h"
#include "servmodel.h"
#include "testmodel.h"

Plugin::Plugin(QObject *parent): QObject(parent), McaFeedPlugin()
{
    m_server = new QLocalServer;
    connect(m_server, SIGNAL(newConnection()),
            this, SLOT(newConnection()));
    if (!m_server->listen("meego-ux-content-test"))
        qWarning() << "error: plugin not listening on server socket";

    m_serviceModel = new ServiceModel;
}

Plugin::~Plugin()
{
    delete m_serviceModel;
}

QAbstractItemModel *Plugin::serviceModel()
{
    return m_serviceModel;
}

QAbstractItemModel *Plugin::createFeedModel(const QString& service)
{
    qDebug() << "Plugin::createFeedModel: " << service;

    TestModel *model = new TestModel(m_serviceModel->displayNameFromId(service));
    m_serviceToModel.insert(service, model);
    connect(model, SIGNAL(destroyed(QObject*)), this, SLOT(modelDestroyed(QObject*)));
    return model;
}

McaSearchableFeed *Plugin::createSearchModel(const QString& service,
                                             const QString& searchText)
{
    qDebug() << "Plugin::createSearchModel: " << service << searchText;
    return NULL;
}

void Plugin::newConnection()
{
    qDebug() << "NEW CONNECTION";
    QLocalSocket *socket = m_server->nextPendingConnection();
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void Plugin::readyRead()
{
    qDebug() << "READING:";
    const int size = 256;
    char buf[size];

    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
    if (socket) {
        socket->read(buf, size);
        TestCommand *command = (TestCommand *)buf;
        switch (command->type) {
        case TestCommandAddService:
            qDebug() << "ADD SERVICE";
            if (command->len > 0)
                m_serviceModel->addService(command->data);
            else
                m_serviceModel->addService();
            break;

        case TestCommandRemoveService:
            qDebug() << "REMOVE SERVICE";
            if (command->len > 0)
                m_serviceModel->removeService(command->data);
            else
                m_serviceModel->removeService();
            break;

        case TestCommandAddContentItem:
            {
                qDebug() << "ADD CONTENT ITEM";
                QString id = m_serviceModel->idFromDisplayName(command->data);
                TestModel *model = m_serviceToModel.value(id);
                if (model)
                    model->addRandomContentRow();
                break;
            }

        case TestCommandAddRequestItem:
            {
                qDebug() << "ADD REQUEST ITEM";
                QString id = m_serviceModel->idFromDisplayName(command->data);
                TestModel *model = m_serviceToModel.value(id);
                if (model)
                    model->addRandomRequestRow();
                break;
            }

        case TestCommandRemoveItem:
            {
                qDebug() << "REMOVE ITEM";
                QString id = m_serviceModel->idFromDisplayName(command->data);
                TestModel *model = m_serviceToModel.value(id);
                if (model)
                    model->removeRandomRow();
                break;
            }

        default:
            qDebug() << "Unexpected command type";
            break;
        }
    }
}

void Plugin::modelDestroyed(QObject *object)
{
    // interestingly, can't qobject_cast here, it fails and returns null
    TestModel *model = static_cast<TestModel*>(object);
    QString key = m_serviceToModel.key(model);
    if (!key.isEmpty())
        m_serviceToModel.remove(key);
}

Q_EXPORT_PLUGIN2(sample_plugin, Plugin)
