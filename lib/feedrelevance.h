#ifndef FEEDRELEVANCE_H
#define FEEDRELEVANCE_H

#include <QObject>
#include <QMap>
#include <QSet>
#include <QDate>
#include <QStringList>

class FeedRelevance : public QObject
{
    Q_OBJECT;

public:
    static FeedRelevance *instance(const QString &panelName, const QString &serviceId);
    static void release(const QString &panelName, const QString &serviceId);

protected:
    FeedRelevance(const QString &identifier);
    virtual ~FeedRelevance();

public:
    void positiveFeedback(const QString &recordId);
    void negativeFeedback(const QString &recordId);
    void recordSeen(const QString &recordId);
    qreal relevance();


protected:
    void load();
    void save();
    void clear(bool all = true);
    void update();
    qreal makeScore();

private:
    QString m_identifier;
    QDate m_lastDate;
    int m_numDays;

    QSet<QString> m_positive;
    QSet<QString> m_negative;
    QSet<QString> m_seen;

    QStringList m_setNames;
    QMap< QString, QSet<QString>* > m_nameToSet;

    qreal *m_scoreByDay;

//static members
    static const char RelevanceKey[];
    static const char ScoreKey[];
    static const char LastDateKey[];
    static const char SeenKey[];
    static const char PositiveKey[];
    static const char NegativeKey[];

    static QMap<QString, FeedRelevance*> m_instances;
    static QMap<QString, int> m_refCounts;
    static QMap<int, int> m_relevanceWeights;
};

#endif // FEEDRELEVANCE_H
