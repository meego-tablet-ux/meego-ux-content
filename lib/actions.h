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

class McaActions: public QObject
{
    Q_OBJECT
    // identifiers for custom actions (FUTURE)
    Q_PROPERTY(QStringList customActions READ customActions)
    // localized strings to show the user in a context menu (FUTURE)
    Q_PROPERTY(QStringList customDisplayActions READ customDisplayActions)

public:
    McaActions(QObject *parent = NULL);
    virtual ~McaActions();

    QStringList customActions();
    QStringList customDisplayActions();

    void addCustomAction(QString id, QString displayName);

signals:
    // standard actions are defined by the model
    void standardAction(QString action, QString uniqueid);
    void customAction(QString action, QString uniqueid);

public slots:
    // if you use the same actions object for more than one object, you should
    //   be prepared to potentially get called with a uniqueid of a row already
    //   removed from your model occasionally; you should either be able to
    //   still act on the removed content or gracefully throw away the action

    // standard actions are defined by the model
    void performStandardAction(QString action, QString uniqueid);
    void performCustomAction(QString action, QString uniqueid);

private:
    QStringList m_ids;
    QStringList m_names;
};

Q_DECLARE_METATYPE(McaActions*)

#endif  // __mcaactions_h
