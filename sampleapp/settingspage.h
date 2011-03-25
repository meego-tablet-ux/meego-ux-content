/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef __settingspage_h
#define __settingspage_h

#include <QPersistentModelIndex>
#include <MApplicationPage>

class MList;
class MButton;
class McaPanelManager;

class SettingsPage: public MApplicationPage
{
    Q_OBJECT

public:
    SettingsPage(McaPanelManager *panelmgr, QGraphicsItem *parent = NULL);
    ~SettingsPage();

    void addButtonIndexMapping(MButton *button, const QModelIndex& index);

protected slots:
    void configure();
    void clearHistory();
    void toggleEnable(bool enabled);

private:
    MList *m_list;
    McaPanelManager *m_panelmgr;
    QMap<MButton *, QPersistentModelIndex> m_map;
};

#endif  // __settingspage_h
