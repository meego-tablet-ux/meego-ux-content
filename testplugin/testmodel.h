/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __testmodel_h
#define __testmodel_h

#include <feedmodel.h>

class McaActions;

class TestModel: public McaFeedModel
{
    Q_OBJECT

public:
    TestModel(QString prefix = QString(), QObject *parent = NULL);
    ~TestModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    void addRandomContentRow();
    void addRandomRequestRow();
    void removeRandomRow();

protected slots:
    void performAction(QString action, QString uniqueid);

private:
    QString m_prefix;
    QList<int> m_values;
    QList<int> m_types;
    QList<QDateTime> m_timestamps;
    McaActions *m_actions;
    int m_lastId;
};

#endif  // __testmodel_h
