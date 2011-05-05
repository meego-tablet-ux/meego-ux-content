#include "feedrelevance.h"
#include <QSettings>
#include <QStringList>
#include <QDebug>
#include <memory.h>
#include "settings.h"

//#define RELEVANCE_DEBUG

const int LOOKBACK_DAYS = 30;
const int POSITIVE_ACTION_SCORE = 2;
const int IGNORED_ACTION_SCORE = 1;
const int NEGATIVE_ACTION_SCORE = 0;

// static members

// QSettings keys
const char FeedRelevance::RelevanceKey[] = "meego-ux-content-relevance";
const char FeedRelevance::ScoreKey[] = "score";
const char FeedRelevance::SeenKey[] = "seen";
const char FeedRelevance::PositiveKey[] = "positive";
const char FeedRelevance::NegativeKey[] = "negative";
const char FeedRelevance::LastDateKey[] = "lastDate";

QMap<QString, FeedRelevance*> FeedRelevance::m_instances;
QMap<QString, int> FeedRelevance::m_refCounts;
QMap<int, int> FeedRelevance::m_relevanceWeights;

FeedRelevance *FeedRelevance::instance(const QString &panelName, const QString &serviceId)
{
    QString identifier = panelName + "." + serviceId;
    if (!m_instances.contains(identifier)) {
        m_instances[identifier] = new FeedRelevance(identifier);
    }
    m_refCounts[identifier] = m_refCounts[identifier] + 1;

    return m_instances[identifier];
}

void FeedRelevance::release(const QString &panelName, const QString &serviceId)
{
    QString identifier = panelName + "." + serviceId;
    if (m_instances.contains(identifier)) {
        m_refCounts[identifier] = m_refCounts[identifier] - 1;
        if (m_refCounts[identifier] == 0) {
            delete  m_instances[identifier];
            m_instances.remove(identifier);
            m_refCounts.remove(identifier);
        }
    }
}

//class members

FeedRelevance::FeedRelevance(const QString &identifier)
{
    m_numDays = LOOKBACK_DAYS;
    m_identifier = identifier;
    m_scoreByDay = new qreal[m_numDays];

    m_setNames << FeedRelevance::SeenKey << FeedRelevance::PositiveKey << FeedRelevance::NegativeKey;
    m_nameToSet[FeedRelevance::SeenKey] = &m_seen;
    m_nameToSet[FeedRelevance::PositiveKey] = &m_positive;
    m_nameToSet[FeedRelevance::NegativeKey] = &m_negative;

    if(m_relevanceWeights.count() == 0) {
        m_relevanceWeights.insert(1, 50); // 50% 0-1 days old
        m_relevanceWeights.insert(7, 35); // 35% 2-7 days old
        m_relevanceWeights.insert(m_numDays, 15); // 15% 8-30 days old
    }

    load();
}

FeedRelevance::~FeedRelevance()
{
    save();

    delete [] m_scoreByDay;
}

void FeedRelevance::load()
{
    clear();

    QSettings settings(McaSettings::Organization, FeedRelevance::RelevanceKey);
    settings.beginGroup(m_identifier);

    QDate date = settings.value(FeedRelevance::LastDateKey).toDate();
    if (!date.isValid()) {
        return;
    }
    m_lastDate = date;

    QString str;
    foreach(QString name, m_setNames) {
        str = settings.value(name).toString();
        if (!str.isEmpty()) {
            foreach (QString uuid, str.split(",", QString::SkipEmptyParts)) {
                m_nameToSet[name]->insert(uuid);                
            }
        }
    }    

    str = settings.value(FeedRelevance::ScoreKey).toString();
    if (!str.isEmpty()) {
        QStringList feedbackByDay = str.split(",", QString::SkipEmptyParts);
        if(feedbackByDay.size() == m_numDays)
            for(int index=0; index < m_numDays; index++) {
                m_scoreByDay[index] = feedbackByDay.at(index).toDouble();
            }
    }

    update();    
}

void FeedRelevance::save()
{
    QSettings settings(McaSettings::Organization, FeedRelevance::RelevanceKey);
    settings.beginGroup(m_identifier);
    settings.setValue(FeedRelevance::LastDateKey, m_lastDate);

    QStringList list;
    for(int i=0; i < m_setNames.length(); i++) {
        list.clear();
        foreach ( QString uuid, *(m_nameToSet.value(m_setNames[i])) ) {
            list << uuid;
        }
        settings.setValue(m_setNames[i], list.join(","));
    }

    list.clear();
    for (int i=0; i<m_numDays; i++) {
        list << QString::number(m_scoreByDay[i]);
    }
    settings.setValue(FeedRelevance::ScoreKey, list.join(","));
    settings.endGroup();
}

void FeedRelevance::clear(bool all)
{
    m_positive.clear();
    m_negative.clear();
    m_seen.clear();

    if(all) {
        memset(m_scoreByDay, 0, sizeof(double) * (m_numDays));
        m_lastDate = QDate::currentDate();
    }
}

void FeedRelevance::update()
{
    QDate current = QDate::currentDate();
    int days = m_lastDate.daysTo(current);
    if (days == 0) {
        return;
    }
    if (days < 0) {
        qWarning("WARNING FeedRelevance::update: new date earlier than expected");
        return;
    }

    if (days > m_numDays - 1) {
        clear();
        save();
        return;
    }

    for (int i = m_numDays - 1; i >= 0; i--) {
        if(i < days) {
            m_scoreByDay[i] = 0; // no feedback from this days
        } else {
            m_scoreByDay[i] = m_scoreByDay[i - days];
        }
    }
    m_scoreByDay[days - 1] = makeScore();
    m_lastDate = current;
    clear(false);
    save();
}

qreal FeedRelevance:: makeScore()
{
    QSet<QString> interacted = m_positive;
    interacted.unite(m_negative);
    int nrIgnored = m_seen.count() - interacted.count();
    int positiveScore = interacted.subtract(m_negative).count() * POSITIVE_ACTION_SCORE;
    int ignoredScore = nrIgnored * IGNORED_ACTION_SCORE;
    int negativeScore = m_negative.count() * NEGATIVE_ACTION_SCORE; //just for reference, NEGATIVE_ACTION_SCORE is 0
    qreal finalScore = 0;

    if(m_seen.count() != 0) {
        finalScore = (positiveScore  + negativeScore + ignoredScore) * 1.0 / m_seen.count();
    }

#ifdef RELEVANCE_DEBUG
    qDebug() << "Identifier " << m_identifier << m_lastDate;
    qDebug() << "Interacted " << interacted.count();
    qDebug() << "Ignored " << nrIgnored;
    qDebug() << "Ignored score " << ignoredScore;
    qDebug() << "Positive score " << positiveScore;
    qDebug() << "Negative score " << negativeScore;
    qDebug() << "Final score " << finalScore;
#endif

    return finalScore;
}

void FeedRelevance::positiveFeedback(const QString &recordId)
{
    if (QDate::currentDate() != m_lastDate) {
        update();
    }

    // make sure uuid is recorded as seen
    recordSeen(recordId);
    m_positive.insert(recordId);
    save();
}

void FeedRelevance::negativeFeedback(const QString &recordId)
{
    if (QDate::currentDate() != m_lastDate) {
        update();
    }

    // make sure uuid is recorded as seen
    recordSeen(recordId);
    m_negative.insert(recordId);
    save();
}

void FeedRelevance::recordSeen(const QString &recordId)
{
    if (QDate::currentDate() != m_lastDate) {
        update();
    }

    m_seen.insert(recordId);
    save();
}

qreal FeedRelevance::relevance()
{
    int oldIndex = 0;
    qreal intervalAverageSum = 0; // sum of interval avreges
    int weightsSum = 0; // sum of weights
    qreal totalScore = 0; // final score

    foreach(int step, m_relevanceWeights.keys()) {
        qreal sum = 0;
        for(int index=oldIndex; index < step; index++) {
            if(index == 0) {
                sum += makeScore();
            } else {
                sum += m_scoreByDay[index - 1];
            }
        }
        weightsSum += m_relevanceWeights[step];
        intervalAverageSum += m_relevanceWeights[step] * (sum / (step - oldIndex + 1));

        oldIndex = step + 1;
    }
    totalScore = intervalAverageSum / weightsSum;
#ifdef RELEVANCE_DEBUG
    qDebug() << m_identifier << ", intervalAverageSum " << intervalAverageSum << ", weightsSum " << weightsSum << ", score " << totalScore;
#endif

    return totalScore;
}
