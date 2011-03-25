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
    m_lastId = 0;
    m_actions = new McaActions;
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
        return m_displayNames.at(row);

    case RequiredCategoryRole:
        return m_categories.at(row);

    case RequiredNameRole:
        return m_names.at(row);

    case CommonActionsRole:
        return QVariant::fromValue<McaActions*>(m_actions);

    default:
        qWarning() << "Unhandled data role requested!";
        return QVariant();
    }
}

void ServiceModel::addService(QString name)
{
    int row = qrand() % (m_names.size() + 1);
    qDebug() << "ADD SERVICE: ROW " << row;

    beginInsertRows(QModelIndex(), row, row);
    QString num = QString::number(++m_lastId);
    m_names.insert(row, QString("service") + num);

    QString category;
    int rand = qrand() % 3;
    if (rand == 0)
        category = "email";
    else if (rand == 1)
        category = "social";
    else
        category = "im";
    m_categories.insert(row, category);

    QString displayName = QString("Service ") + num;
    if (!name.isEmpty()) {
        displayName += ": ";
        displayName += name;
    }
    m_displayNames.insert(row, displayName);
    m_requestedNames.insert(row, name);
    endInsertRows();
}

void ServiceModel::removeService(QString name)
{
    int row = m_requestedNames.indexOf(name);
    if (row == -1) {
        int size = m_names.size();
        if (size <= 0) {
            qWarning() << "error: attempted to remove invalid service:" << name;
            return;
        }
        row = qrand() % size;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_names.removeAt(row);
    m_categories.removeAt(row);
    m_displayNames.removeAt(row);
    m_requestedNames.removeAt(row);
    endRemoveRows();
}

void ServiceModel::clear()
{
    foreach (QString name, m_names)
        removeService(name);
}

QString ServiceModel::displayNameFromId(QString id)
{
    int row = m_names.indexOf(id);
    if (row != -1)
        return m_requestedNames.at(row);
    qWarning() << "error: display name requested for invalid id:" << id;
    return QString();
}

QString ServiceModel::idFromDisplayName(QString name)
{
    int row = m_requestedNames.indexOf(name);
    if (row != -1)
        return m_names.at(row);
    qWarning() << "error: id requested for invalid display name:" << name;
    return QString();
}

void ServiceModel::performAction(QString action, QString uniqueid)
{
    if (action == "configure")
        configure(uniqueid);
}

void ServiceModel::configure(QString serviceName)
{
    qDebug() << "Configure called for :" << serviceName;
}
