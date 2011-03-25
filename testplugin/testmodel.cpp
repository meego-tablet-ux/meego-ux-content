/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>

#include <QDateTime>

#include <actions.h>

#include "testmodel.h"

enum {
    TypeContent,
    TypeRequest
};

TestModel::TestModel(QString prefix, QObject *parent): McaFeedModel(parent)
{
    m_prefix = prefix;

    m_lastId = 0;

    m_actions = new McaActions;
    connect(m_actions, SIGNAL(standardAction(QString,QString)),
            this, SLOT(performAction(QString,QString)));
}

TestModel::~TestModel()
{
}

int TestModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_values.count();
}

QVariant TestModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= m_values.count())
        return QVariant();

    QString strval = QString::number(m_values[row]);

    switch (role) {
    case RequiredTypeRole:
        if (m_types[row] == TypeRequest)
            return QString("request");
        else if (m_types[row] != TypeContent)
            qWarning() << "TestModel: unexpected type found";
        return QString("content");

    case RequiredUniqueIdRole:
        return QString("row") + strval;

    case RequiredTimestampRole:
        return m_timestamps[row];

    case GenericTitleRole:
        return m_prefix + " title" + strval;

    case GenericContentRole:
        return m_prefix + " content" + strval;

    case CommonActionsRole:
        return QVariant::fromValue<McaActions*>(m_actions);

    default:
        return QVariant();
    };
}

void TestModel::addRandomContentRow()
{
    int row = qrand() % (m_values.count() + 1);
    beginInsertRows(QModelIndex(), row, row);
    qDebug() << "ADDED CONTENT" << row << m_lastId;
    m_types.insert(row, TypeContent);
    m_values.insert(row, m_lastId++);
    m_timestamps.insert(row, QDateTime::currentDateTime());
    endInsertRows();
}

void TestModel::addRandomRequestRow()
{
    int row = qrand() % (m_values.count() + 1);
    beginInsertRows(QModelIndex(), row, row);
    qDebug() << "ADDED REQUEST" << row << m_lastId;
    m_types.insert(row, TypeRequest);
    m_values.insert(row, m_lastId++);
    m_timestamps.insert(row, QDateTime::currentDateTime());
    endInsertRows();
}

void TestModel::removeRandomRow()
{
    int count = m_values.count();
    if (count <= 0)
        return;

    int row = qrand() % count;
    beginRemoveRows(QModelIndex(), row, row);
    qDebug() << "REMOVED" << row << m_values.at(row);
    m_types.removeAt(row);
    m_values.removeAt(row);
    m_timestamps.removeAt(row);
    endRemoveRows();
}

void TestModel::performAction(QString action, QString uniqueid)
{
    qDebug() << "Action" << action << "called for " << uniqueid;
}
