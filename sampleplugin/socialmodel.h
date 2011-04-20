/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __socialmodel_h
#define __socialmodel_h

#include <feedmodel.h>

struct Social;

class SocialModel: public McaFeedModel
{
    Q_OBJECT

public:
    SocialModel(QObject *parent = NULL);
    ~SocialModel();

    // Developers: Please see comments in the emailmodel.h for help on how
    //   to implement your own feed model. This one is no different.
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

protected slots:
    void performAction(QString action, QString uniqueid);
    void performCustomAction(QString action, QString uniqueid);

private:
    Social *m_socials;
    int m_size;
};

#endif  // __socialmodel_h
