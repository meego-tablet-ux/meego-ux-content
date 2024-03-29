/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QSortFilterProxyModel>
#include "actionsproxy.h"
#include "contentqml.h"
#include "panelmanager.h"
#include "searchmanager.h"
#include "servicemodeldbusproxy.h"

void McaContent::registerTypes(const char *uri)
{
    qmlRegisterType<McaActionsProxy>(uri, 0, 0, "McaActions");
    qmlRegisterType<McaPanelManager>(uri, 0, 0, "McaPanelManager");
    qmlRegisterType<McaSearchManager>(uri, 0, 0, "McaSearchManager");
    qmlRegisterType<ServiceModelDbusProxy>(uri, 0, 0, "ServiceModelDbusProxy");
    qmlRegisterType<QSortFilterProxyModel>(uri, 0, 0, "QSortFilterProxyModel");
}

Q_EXPORT_PLUGIN(McaContent);
