/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __servmodel_h
#define __servmodel_h

#include <QStringList>

#include <servicemodel.h>

class McaActions;

class ServiceModel: public McaServiceModel
{
    Q_OBJECT

public:
    ServiceModel(QObject *parent = NULL);
    ~ServiceModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

    void addService(QString name = QString());
    void removeService(QString name = QString());

    void clear();

    QString displayNameFromId(QString id);
    QString idFromDisplayName(QString name);

public slots:
    void performAction(QString action, QString uniqueid);
    void configure(QString serviceName);

private:
    QStringList m_names;
    QStringList m_categories;
    QStringList m_displayNames;
    QStringList m_requestedNames;
    McaActions *m_actions;
    int m_lastId;
};

#endif  // __servmodel_h
