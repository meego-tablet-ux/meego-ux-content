/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __panelpage_h
#define __panelpage_h

#include <MApplicationPage>
#include <QPersistentModelIndex>
#include <QHash>

class MContentItem;
class McaPanelManager;
class MButton;

class PanelPage: public MApplicationPage
{
    Q_OBJECT

public:
    PanelPage(QGraphicsItem *parent = NULL);
    ~PanelPage();

    void addButtonIndexMapping(MContentItem *item, const QModelIndex& index);

public slots:
    void defaultAction();
    void openSettings();
    void openSearch();
    void toggleFreeze();
    void toggleHide();

private:
    McaPanelManager *m_panelmgr;
    QHash<MContentItem*, QPersistentModelIndex> m_map;
    MButton *m_freezeButton;
    MButton *m_hideButton;
    bool m_frozen;
    bool m_hiding;
};

#endif  // __panelpage_h
