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

#include "emailmodel.h"

struct Email
{
    QDateTime timestamp;
    QString uuid;
    QString subject;
    QString body;
    QString address;
    McaActions actions;
};

EmailModel::EmailModel(QObject *parent): McaFeedModel(parent)
{
    // Most of this constructor is not very realistic because it is just
    //   generating fake data. You would most likely be initializing a
    //   connection to your back end here and connecting signals to notify
    //   about rows getting added and removed from your model.
    m_size = 15;
    m_emails = new Email[m_size];

    // ensure emails generated are always the same
    qsrand(0);

    QDateTime date = QDateTime::currentDateTime();
    for (int i=0; i<m_size; i++) {
        date = date.addSecs(-qrand() % 10000);
        m_emails[i].timestamp = date;

        // NOTE: Your unique ids should not be randomly generated but
        //   consistent across reboots, etc. If the system is rebooted and you
        //   are asked for a unique id for this same item again, you should
        //   be returning the same id.
        m_emails[i].uuid = QString::number(10000 + i);

        QStringList list;
        list << "Re: vacation plans" << "First Bank of Springfield" <<
                "special offer" << "lan party this weekend?";
        int msg = qrand() % list.size();
        m_emails[i].subject = list.at(msg);

        list.clear();
        list << "It was great to hear from you. I'm really hoping we'll be "
                "able to make it to Hawaii this year and see all of you."
                <<
                "Your payment for $124.13 was sent to Electric Company, Inc. "
                "on Friday"
                <<
                "Because you've bought martial arts DVDs in the past, we "
                "think you'll love the latest release from Golden Harvest"
                <<
                "Friday night is the time. My home is the place. You need to "
                "prepare yourself for the battle of the decade, and you need";
        m_emails[i].body = list.at(msg);

        list.clear();
        list << "cousin@family.org" << "alistair@firstbank.com" <<
                "dvdsales@awesomestuff.com" << "joe@buddy.org";
        m_emails[i].address = list.at(qrand() % list.size());

        // In this case, we are providing a different McaAction object for
        //   every item in our model. Another option is to provide the same
        //   one for every item and use the uniqueid argument to distinguish
        //   which one the signal was for. Another alternative is to derive
        //   from McaActions and pass an object that overrides the slots.
        connect(&m_emails[i].actions, SIGNAL(standardAction(QString,QString)),
                this, SLOT(performAction(QString,QString)));

        list.clear();
        list << "delete" << "reset" << "reload";
        for(int j = 0; j < list.size(); j++) {
            m_emails[i].actions.addCustomAction(list[j], list[j].toUpper());
        }
        connect(&m_emails[i].actions, SIGNAL(customAction(QString,QString)),
                this, SLOT(performCustomAction(QString,QString)));
    }
}

EmailModel::~EmailModel()
{
    delete [] m_emails;
}

int EmailModel::rowCount(const QModelIndex &parent) const
{
    // The current UI plans only show at most 30 items from your model in
    //   a panel, and more likely 10 or fewer. So if there are performance
    //   improvements you can make by pruning down the queries you make to
    //   your back end, that is a guideline. However, if setSearchText is
    //   called, then you should try to search as widely as you can for
    //   matching text.
    // If you are given limit hints with the setHints function, you can also
    //   use those to help tune how much information you bother exposing
    //   because that will tell you how much will really be used.
    // When there is no search text set, your model should also restrict its
    //   queries to items from the last 30 days, if that is helpful for
    //   performance. Older items may be ignored.
    Q_UNUSED(parent);
    return m_size;
}

QVariant EmailModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        qWarning() << "index invalid in email model" << role;
        return QVariant();
    }

    int row = index.row();
    if (row >= m_size)
        return QVariant();

    switch (role) {
    case RequiredTypeRole:
        // Types defined currently are "content", "picture", and "request";
        //   see emailmodel.h for more information.
        return QString("content");

    case RequiredUniqueIdRole:
        // RequiredUniqueIdRole is required, and the id you provide must be
        //   unique across this service and remain consistent for this data
        //   item across reboots, etc.
        // It doesn't have to be a true Uuid, although that would be fine.
        return m_emails[row].uuid;

    case RequiredTimestampRole:
        // Each item should have a timestamp associated, and the model
        //   should be sorted by timestamp, most recent first.
        return m_emails[row].timestamp;

    case GenericTitleRole:
        // The basic view for a content item contains two text fields,
        //   this one for the item title, plus the content preview (below).
        return m_emails[row].subject;

    case GenericContentRole:
        // The basic view for a content item contains two text fields,
        //   the item title (above), plus this content preview. The string
        //   should be limited to a few lines of text, maybe 160 or 256
        //   characters. More will be ignored.
        return m_emails[row].body;

    case GenericAvatarUrlRole:
        // This field is for a thumbnail image of the message sender.
        return QVariant();

    case CommonActionsRole:
        // This is required to respond to user actions on your data item.
        return QVariant::fromValue<McaActions*>(&m_emails[row].actions);

    default:
        // There are a few other optional roles... you can provide a
        //   float "relevance" of the item from 0.0 (low) to 1.0 (high);
        //   for future use. You can set custom text for accept/reject
        //   buttons (localized!) for a "request" type item. You can
        //   provide a "picture url" if there is picture associated with
        //   the item, such as a photo upload notification.
        return QVariant();
    };
}

void EmailModel::performAction(QString action, QString uniqueid)
{
    // This is a slot connected to the signal for all items, so we need to
    //   figure out which one it applies to based on the uniqueid paramater.
    //   Alternately, you could connect to a slot on an individual item, if
    //   you make it a QObject, and you don't need that parameter.
    qDebug() << "Action" << action << "called for email" << uniqueid;
}

void EmailModel::performCustomAction(QString action, QString uniqueid)
{
    qDebug() << "Custom action " << action << " called for social item" << uniqueid;
}
