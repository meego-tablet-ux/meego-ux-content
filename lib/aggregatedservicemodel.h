/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __mcaaggregatedservicemodel_h
#define __mcaaggregatedservicemodel_h

#include "aggregatedmodel.h"

class McaAggregatedServiceModel: public McaAggregatedModel
{
    Q_OBJECT

public:
    McaAggregatedServiceModel(QObject *parent = NULL);
    virtual ~McaAggregatedServiceModel();
};

#endif  // __mcaaggregatedservicemodel_h
