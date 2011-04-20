/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcafeedfilter_h
#define __mcafeedfilter_h

#include <QSortFilterProxyModel>
#include <QDate>
#include <QSet>

class McaActions;

class McaFeedFilter: public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int lookback READ lookback)

public:
    McaFeedFilter(QAbstractItemModel *source, QString serviceId, QObject *parent = NULL);
    virtual ~McaFeedFilter();

    int lookback();

    void hide(QString uniqueid);
    bool isHidden(QString uniqueid) const;

    void clearHistory(QDateTime datetime = QDateTime::currentDateTime());

    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

protected:
    void clear();
    void load();
    void save();
    void update();

protected slots:
    void performStandardAction(QString action, QString uniqueid);    
    void saveNow();

private:
    QAbstractItemModel *m_source;

    QString m_serviceId;
    QDate m_lastDate;
    int m_lastIndex;
    int m_numDays;
    bool m_dirty;

    // arrays with a bucket for each of the last m_numDays with set of ids
    //   hidden from the user on that date
    QSet<QString> *m_hiddenByDate;

    // contains the set of all items in the m_hiddenByDate array
    QSet<QString> m_hidden;

    // earliest time for which to show items
    QDateTime m_earliestTime;
};

#endif  // __mcafeedfilter_h
