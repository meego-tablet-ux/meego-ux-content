/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>

#include <actions.h>

#include "servmodel.h"

ServiceModel::ServiceModel(QObject *parent): McaServiceModel(parent)
{
    // Your service model might actually be this simple, or simpler
    //   if you are only providing one service. If your services can come
    //   and go as new accounts are configured, etc, then it may be more
    //   complex and need to dynamically manage this data.
    m_names << "email1" << "network1";
    m_categories << "email" << "social";
    m_displayNames << "Email" << "Social Network";
    m_icons << "file://tmp/checkbox.png" << "file://tmp/fish.png";
    m_actions = new McaActions;

    // If you are going to use the CommonConfigErrorRole to notify the
    //   meego-ux-content software that there is a configuration error with
    //   your service, then you must implement a handler for the "configure"
    //   action as we are doing here, and it should launch the settings
    //   for your service.
    connect(m_actions, SIGNAL(standardAction(QString,QString)),
            this, SLOT(performAction(QString,QString)));
}

ServiceModel::~ServiceModel()
{
    delete m_actions;
}

int ServiceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_names.size();
}

QVariant ServiceModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row >= m_names.size())
        return QVariant();

    switch (role) {
    case CommonDisplayNameRole:
        // This display name is a localized name for your service -- if you
        //   provide more than one service, each should have a distinct name
        //   for example, identifying the account. But it should be a title,
        //   preferably under ~32 characters.
        return m_displayNames.at(row);

    case CommonIconUrlRole:
        // Here you can provide a small icon that identifies the service.
        //   This icon would probably be the same for each account if you
        //   provide multiple accounts as separate "services".
        return m_icons.at(row);

    case RequiredCategoryRole:
        // Currently we define three categories: "social", "email", and "im"
        //   that will be pulled into the Friends panel. In the future we
        //   may extend the categories for other panels and purposes.
        return m_categories.at(row);

    case RequiredNameRole:
        // This field is a unique name for the service within this plugin
        //   If you provide multiple accounts each should have its own
        //   unique id. This is not user-visible.
        return m_names.at(row);

    case CommonActionsRole:
        // This is required if you will ever return true for
        //   CommonConfigErrorRole.
        return QVariant::fromValue<McaActions*>(m_actions);

    default:
        // There is also the CommonConfigErrorRole which is a bool and just
        //   requests the UI to alert the user to misconfiguration, and gives
        //   them an opportunity to configure your service. If you return
        //   true here, you must also provide the CommonActionsRole above
        //   with a handler watching for a "configure" action.
        qWarning() << "Unhandled data role requested!";
        return QVariant();
    }
}

void ServiceModel::performAction(QString action, QString uniqueid)
{
    // The mtfcontent sample application provides Configure buttons for each
    //   service you report so you can test that you are receiving the
    //   configure signal properly. In the real application, we plan to only
    //   provide this option to the user if you report that there is a
    //   configuration error through CommonConfigErrorRole.
    if (action == "configure")
        configure(uniqueid);
}

void ServiceModel::configure(QString serviceName)
{
    qDebug() << "Configure called for :" << serviceName;
}
