/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "memoryleak.h"

#include <QDebug>
#include <QSettings>
#include <QTimer>

#include "feedfilter.h"
#include "feedmodel.h"
#include "actions.h"
#include "settings.h"
#include "feedrelevance.h"
#include "actionsproxy.h"

#include "memoryleak-defines.h"

//
// Overview of McaFeedFilter
//    - records ids of hidden rows and filters them out of the model
//

const int LookbackDaysDefault = 30;
const int SaveDelayMS = 1 * 1000;  // slight delay to batch rapid requests

//
// public methods
//

McaFeedFilter::McaFeedFilter(QAbstractItemModel *source, QString serviceId, QObject *parent):
        QSortFilterProxyModel(parent)
{
    setSourceModel(source);

    m_source = source;
    m_feedRelevance = 0;

    m_serviceId = serviceId;
    m_numDays = LookbackDaysDefault;
    m_hiddenByDate = new QSet<QString>[m_numDays];
    m_dirty = false;
    load();
}

McaFeedFilter::~McaFeedFilter()
{    
    if(0 != m_feedRelevance) {
        m_feedRelevance->release(m_panelName, m_serviceId);
        m_feedRelevance = 0;
    }
    delete m_source;
    delete []m_hiddenByDate;
}

int McaFeedFilter::lookback()
{
    return m_numDays;
}

void McaFeedFilter::hide(QString uniqueid)
{
    // handle date change while we're running
    if (QDate::currentDate() != m_lastDate)
        update();

    m_hiddenByDate[m_lastIndex].insert(uniqueid);
    m_hidden.insert(uniqueid);
    invalidateFilter();
    save();
}

bool McaFeedFilter::isHidden(QString uniqueid) const
{
    return m_hidden.contains(uniqueid);
}

void McaFeedFilter::clearHistory(QDateTime datetime)
{
    if (datetime < m_earliestTime)
        return;
    m_earliestTime = datetime;
    invalidateFilter();
    save();
}

bool McaFeedFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)

    // filter out hidden items
    QModelIndex sourceIndex = m_source->index(source_row, 0);
    QDateTime timestamp = m_source->data(sourceIndex, McaFeedModel::RequiredTimestampRole).toDateTime();
    if (timestamp < m_earliestTime) {
        static bool first = true;
        if (first) {
            qDebug() << "Ignoring some items from service" << m_serviceId << "due to age";
            first = false;
        }

        return false;
    }

    return !isHidden(m_source->data(sourceIndex, McaFeedModel::RequiredUniqueIdRole).toString());
}

QVariant McaFeedFilter::data(const QModelIndex &index, int role) const
{
    if (role == McaFeedModel::CommonActionsRole) {
        QVariant variantActions = m_source->data(index, McaFeedModel::CommonActionsRole);
        McaActions *actions = variantActions.value<McaActions*>();

        if (!actions) return variantActions;

        // return our own McaActions to catch all actions
        McaActions *proxyActions = actions->findChild<McaActions*>("proxyAction");
        if(0 == proxyActions) {
            // set parent to real actions object and let Qt delete this
            proxyActions = new McaActions();
            proxyActions->moveToThread(actions->thread());
            proxyActions->setParent(actions);
            proxyActions->setObjectName("proxyAction");

            // copy any custom actions exist
            // TODO: what happens if more actions are added to the original object?
            QStringList actionNames = actions->customDisplayActions();
            QStringList actionIds = actions->customActions();
            for(int i=0; i < actionNames.length(); i++) {
                proxyActions->addCustomAction(actionIds.at(i), actionNames.at(i), actions->actionType(actionIds.at(i)));
            }

            connect(proxyActions, SIGNAL(standardAction(QString,QString)),
                    this, SLOT(performStandardAction(QString,QString)));
            connect(proxyActions, SIGNAL(customAction(QString,QString)),
                    this, SLOT(performCustomAction(QString,QString)));
        }
        return QVariant::fromValue<McaActions*>(proxyActions);
    }

    return QSortFilterProxyModel::data(index, role);
}

void McaFeedFilter::setPanelName(const QString &panelName)
{
    if(m_panelName == panelName) return;

    if(0 != m_feedRelevance) {
        m_feedRelevance->release(m_panelName, m_serviceId);
    }
    m_panelName = panelName;
    m_feedRelevance = FeedRelevance::instance(m_panelName, m_serviceId);
}

//
// protected methods
//

void McaFeedFilter::clear()
{
    m_lastIndex = 0;
    m_lastDate = QDate::currentDate();

    for (int i = 0; i < m_numDays; i++)
        m_hiddenByDate[i].clear();
    m_hidden.clear();
}

void McaFeedFilter::load()
{
    // load hidden item data from QSettings file
    clear();

    QSettings settings(McaSettings::Organization, McaSettings::ApplicationHide);
    settings.beginGroup(m_serviceId);

    QDateTime datetime = QDateTime::currentDateTime().addDays(-LookbackDaysDefault);
    QVariant variant = settings.value(McaSettings::KeyEarliestTime);
    if (variant.isValid()) {
        QDateTime dt = variant.toDateTime();
        if (dt > datetime)
            datetime = dt;
    }
    m_earliestTime = datetime;

    QDate date = settings.value(McaSettings::KeyLastDate).toDate();
    if (!date.isValid())
        return;

    m_lastDate = date;
    m_lastIndex = 0;

    QString hidden = McaSettings::KeyHiddenPrefix;
    for (int i = 0; i < m_numDays; i++) {
        int index = (m_lastIndex + m_numDays - i) % m_numDays;
        QString str = settings.value(hidden + QString::number(i)).toString();
        if (!str.isEmpty()) {
            foreach (QString uniqueid, str.split(",", QString::SkipEmptyParts)) {
                m_hiddenByDate[index].insert(uniqueid);
                m_hidden.insert(uniqueid);
            }
        }
        date = date.addDays(-1);
    }
    settings.endGroup();

    update();
}

void McaFeedFilter::save()
{
    if (!m_dirty) {
        QTimer::singleShot(SaveDelayMS, this, SLOT(saveNow()));
        m_dirty = true;
    }
}

void McaFeedFilter::saveNow()
{
    // save hidden item data to QSettings file
    m_dirty = false;

    QSettings settings(McaSettings::Organization, McaSettings::ApplicationHide);
    settings.beginGroup(m_serviceId);
    settings.setValue(McaSettings::KeyLastDate, m_lastDate);
    settings.setValue(McaSettings::KeyEarliestTime, m_earliestTime);

    // hidden0 contains today's items, hidden1 contains yesterday's, etc.
    QString hidden = McaSettings::KeyHiddenPrefix;
    for (int i=0; i<m_numDays; i++) {
        int index = (m_lastIndex + m_numDays - i) % m_numDays;

        // add the hidden lines
        QStringList list;
        foreach (QString uniqueid, m_hiddenByDate[index])
            list << uniqueid;
        settings.setValue(hidden + QString::number(i), list.join(","));
    }
    settings.endGroup();
}

void McaFeedFilter::update()
{
    // handle date rollover since the last change to hidden item data
    QDate current = QDate::currentDate();

    int days = m_lastDate.daysTo(current);
    if (days == 0)
        return;
    if (days < 0) {
        qWarning("new date earlier than expected");
        return;
    }

    if (days > m_numDays - 1) {
        clear();
        save();
        return;
    }

    QDate earliest = current.addDays(-m_numDays + 1);

    for (int i=0; i<days; i++) {
        m_lastIndex = (m_lastIndex + 1) % m_numDays;
        m_hiddenByDate[m_lastIndex].clear();
    }

    m_lastDate = current;
    save();
}

//
// protected slots
//

void McaFeedFilter::performStandardAction(QString action, QString uniqueid)
{    
    if (action == "hide") {        
        hide(uniqueid);
        m_feedRelevance->negativeFeedback(uniqueid);
        return;
    }

    if (action == "setViewed") {
        m_feedRelevance->recordSeen(uniqueid);
        return;
    }
    m_feedRelevance->positiveFeedback(uniqueid);

    if(sender()) {
        McaActions *realActions = qobject_cast<McaActions*>(sender()->parent());
        if(0 != realActions) {
            realActions->performStandardAction(action, uniqueid);
        }
    }
}

void McaFeedFilter::performCustomAction(QString action, QString uniqueid)
{
    if(sender()) {
        McaActions *realActions = qobject_cast<McaActions*>(sender()->parent());
        if(0 != realActions) {
            if(realActions->actionType(action)) {
                m_feedRelevance->positiveFeedback(uniqueid);
            } else {
                m_feedRelevance->negativeFeedback(uniqueid);
            }
            realActions->performCustomAction(action, uniqueid);
        }
    }
}
