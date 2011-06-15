/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "memoryleak.h"

#include "aggregatedservicemodel.h"
#include "servicemodel.h"
#include "serviceadapter.h"

#include "memoryleak-defines.h"

//
// instance methods
//

McaAggregatedServiceModel::McaAggregatedServiceModel(QObject *parent):
        McaAggregatedModel(parent)
{
    // add feedmodel/feedadapter roles that have been lost through encapsulation
    QHash<int, QByteArray> roles = roleNames();
    roles.insert(McaServiceModel::RequiredNameRole,      "name");
    roles.insert(McaServiceModel::RequiredCategoryRole,  "category");
    roles.insert(McaServiceModel::CommonActionsRole,     "actions");
    roles.insert(McaServiceModel::CommonDisplayNameRole, "displayname");
    roles.insert(McaServiceModel::CommonIconUrlRole,     "icon");
    roles.insert(McaServiceModel::CommonConfigErrorRole, "configerror");
    roles.insert(McaServiceAdapter::SystemUpidRole,      "upid");
    setRoleNames(roles);
}

McaAggregatedServiceModel::~McaAggregatedServiceModel()
{
}
