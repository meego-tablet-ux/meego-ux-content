/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __searchpage_h
#define __searchpage_h

#include <MApplicationPage>
#include <QPersistentModelIndex>
#include <QHash>

class MContentItem;
class McaSearchManager;
class MButton;
class MTextEdit;

class SearchPage: public MApplicationPage
{
    Q_OBJECT

public:
    SearchPage(QGraphicsItem *parent = NULL);
    ~SearchPage();

    void addButtonIndexMapping(MContentItem *item, const QModelIndex& index);

public slots:
    void defaultAction();
    void toggleFreeze();
    void search();
    void searchFinal();

private:
    McaSearchManager *m_searchmgr;
    QHash<MContentItem*, QPersistentModelIndex> m_map;
    MButton *m_freezeButton;
    MTextEdit *m_editSearch;
    bool m_frozen;
};

#endif  // __searchpage_h
