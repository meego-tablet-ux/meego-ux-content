/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcaactions_h
#define __mcaactions_h

#include <QStringList>
#include <QMetaType>
#include <QMap>

class McaActions: public QObject
{
    Q_OBJECT

public:
    McaActions(QObject *parent = NULL);
    virtual ~McaActions();

    virtual QStringList customActions();
    virtual QStringList customDisplayActions();
    bool actionType(const QString &action); // true = positive, false = negative

signals:
    // standard actions are defined by the model
    void standardAction(QString action, QString uniqueid);
    void customAction(QString action, QString uniqueid);

public slots:
    // if you use the same actions object for more than one object, you should
    //   be prepared to potentially get called with a uniqueid of a row already
    //   removed from your model occasionally; you should either be able to
    //   still act on the removed content or gracefully throw away the action

    // custom actions are defined by the model
    virtual void addCustomAction(QString id, QString displayName, bool actionType = true);

    virtual void performStandardAction(QString action, QString uniqueid);
    virtual void performCustomAction(QString action, QString uniqueid);

private:
    QStringList m_ids;
    QStringList m_names;
    QMap<QString, bool> m_actionTypes; // true = positive, false = negative
};

Q_DECLARE_METATYPE(McaActions*)

#endif  // __mcaactions_h
