/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __emailmodel_h
#define __emailmodel_h

#include <feedmodel.h>

struct Email;

class EmailModel: public McaFeedModel
{
    Q_OBJECT

public:
    EmailModel(QObject *parent = NULL);
    ~EmailModel();

    // You need to override rowCount to provide the number of data items you
    //   are currently exposing through the model. See QAbstractItemModel
    //   documentation.
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    // Although the feed model is a list model, meaning there is only one
    //   column of data, there are a number of roles defined in the
    //   feedmodel.h header file. You will need to consider which of
    //   them you need to provide in your implementation.
    // Your data should be provided 
    QVariant data(const QModelIndex &index, int role) const;

protected slots:
    // There are three standard actions currently defined. You should always
    //   provide an McaActions object through the CommonActionsRole. You must
    //   at least handle the action "default". This will be called if the user
    //   clicks on your item. You should then launch the application to view
    //   this item in detail. The uniqueid given is the one you provided for
    //   the item in RequiredUniqueIdRole.
    // For normal data items, you provide the type "content" in
    //   RequiredTypeRole. You must then handle the "default" action.
    // For a data item associated with a picture or set of pictures, such as
    //   a photo upload event on a social networking site, if you can provide
    //   a thumbnail of the picture(s), instead use the "picture" type in
    //   RequiredTypeRole and a file:// URL for GenericPictureUrlRole. Again
    //   you must handle the "default" action.
    // The third defined type is "request", which can be used for presenting
    //   a decision to the user like a friend request. You can choose to
    //   provide localized text for two buttons with GenericAcceptTextRole and
    //   GenericRejectTextRole, or leave them blank to get our default text
    //   "Accept" or "Reject" (localized). When you provide a request, you
    //   must be ready to handle "accept" and "reject" actions. You may
    //   optionally handle the "default" action for these items as well, which
    //   may occur if the user clicks outside the button area.
    // You should test the action string and ignore any unknown ones.
    void performAction(QString action, QString uniqueid);
    void performCustomAction(QString action, QString uniqueid);

private:
    Email *m_emails;
    int m_size;
};

#endif  // __emailmodel_h
