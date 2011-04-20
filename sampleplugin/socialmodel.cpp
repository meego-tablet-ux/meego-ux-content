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

#include "socialmodel.h"

struct Social
{
    QDateTime timestamp;
    QString uuid;
    QString subject;
    QString body;
    QString type;
    McaActions actions;
};

// Developers: Please see annotations in emailmodel.cpp for help on how to
//   implement your own feed model. This one is no different.
SocialModel::SocialModel(QObject *parent): McaFeedModel(parent)
{
    m_size = 15;
    m_socials = new Social[m_size];

    // ensure Socials generated are always the same
    qsrand(0);

    QDateTime date = QDateTime::currentDateTime();
    for (int i=0; i<m_size; i++) {
        date = date.addSecs(-qrand() % 10000);
        m_socials[i].timestamp = date;

        // use consistent uuids for testing
        m_socials[i].uuid = QString::number(20000 + i);

        QStringList list;
        list << "Friend Request" << "New friend" <<
                "Sarah is listening" << "Photo album uploaded";
        int msg = qrand() % list.size();
        m_socials[i].subject = list.at(msg);

        list.clear();
        list << "Barney would like you to be his friend"
                <<
                "Alex has accepted your friend request"
                <<
                "Sarah is listening to Beethoven's 5th Symphony"
                <<
                "Photos from the last week of my road trip to Paris";
        m_socials[i].body = list.at(msg);
        m_socials[i].type = (msg == 0) ? "request":"content";

        connect(&m_socials[i].actions, SIGNAL(standardAction(QString,QString)),
                this, SLOT(performAction(QString,QString)));

        list.clear();
        list << "delete" << "reset" << "reload";        
        for(int j = 0; j < list.size(); j++) {
            m_socials[i].actions.addCustomAction(list[j], list[j].toUpper());
        }
        connect(&m_socials[i].actions, SIGNAL(customAction(QString,QString)),
                this, SLOT(performCustomAction(QString,QString)));
    }
}

SocialModel::~SocialModel()
{
    delete [] m_socials;
}

int SocialModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_size;
}

QVariant SocialModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        qWarning() << "index invalid in social model" << role;
        return QVariant();
    }

    int row = index.row();
    if (row >= m_size)
        return QVariant();

    switch (role) {
    case RequiredTypeRole:
        return m_socials[row].type;

    case RequiredUniqueIdRole:
        return m_socials[row].uuid;

    case RequiredTimestampRole:
        return m_socials[row].timestamp;

    case GenericTitleRole:
        return m_socials[row].subject;

    case GenericContentRole:
        return m_socials[row].body;

    case GenericAvatarUrlRole:
        return QVariant();

    case CommonActionsRole:
        return QVariant::fromValue<McaActions*>(&m_socials[row].actions);

    default:
        return QVariant();
    };
}

void SocialModel::performAction(QString action, QString uniqueid)
{
    qDebug() << "Action" << action << "called for social item" << uniqueid;
}

void SocialModel::performCustomAction(QString action, QString uniqueid)
{
    qDebug() << "Custom action " << action << " called for social item" << uniqueid;
}

