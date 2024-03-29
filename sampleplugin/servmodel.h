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

    // As with any QAbstractListModel, at a minimum you need to override
    //   rowCount and data functions.
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    // Although the service model is a list model, meaning there is only
    //   one column of data, there are a number of roles defined in the
    //   servicemodel.h header file. You will need to consider which of
    //   them you need to provide in your implementation.
    QVariant data(const QModelIndex &index, int role) const;

    // If you have services that can by dynamically added or removed at
    //   runtime you will need to call beginInsertRows/endInsertRows, etc., 
    //   as in the QAbstractItemModel documentation

public slots:
    // If your model can ever return true for CommonConfigErrorRole, then
    //   you must provide an McaActions object through CommonActionsRole,
    //   and be ready to handle its standardAction signal, or override its
    //   performStandardAction function if you derive from it. The
    //   misconfigured status may be displayed to the user and they may be
    //   given the opportunity to configure your service. If that happens,
    //   you will receive the "configure" string for action, with the
    //   uniqueid being the one you provide in RequiredNameRole. You should
    //   then use the Settings API to open the settings application to the
    //   area for your service.

    // This performAction function is an example of how you could handle the
    //   signal and check for a configure request.
    void performAction(QString action, QString uniqueid);
    void configure(QString serviceName);

private:
    QStringList m_names;
    QStringList m_categories;
    QStringList m_displayNames;
    QStringList m_icons;
    McaActions *m_actions;
};

#endif  // __servmodel_h
