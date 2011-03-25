/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>

#include <QGraphicsLinearLayout>
#include <QSortFilterProxyModel>

#include <MButton>
#include <MList>
#include <MAbstractCellCreator>
#include <MContentItem>

#include <panelmanager.h>
#include <feedmodel.h>
#include <actions.h>

#include "panelpage.h"
#include "settingspage.h"
#include "searchpage.h"

class PanelCellCreator: public MAbstractCellCreator<MContentItem>
{
public:
    void updateCell(const QModelIndex& index, MWidget *cell) const
    {
        MContentItem *contentItem = qobject_cast<MContentItem *>(cell);
        QObject::connect(contentItem, SIGNAL(clicked()), m_page, SLOT(defaultAction()),
                         Qt::UniqueConnection);
        contentItem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QString type = index.data(McaFeedModel::RequiredTypeRole).toString();
        QString name = index.data(McaFeedModel::GenericTitleRole).toString();
        QString body = index.data(McaFeedModel::GenericContentRole).toString();
        if (type == "content")
            name.prepend("Content: ");
        else if (type == "request") {
            name.prepend("Request: ");
            body.prepend("[Accept] [Decline] ");
        }
        contentItem->setTitle(name);
        contentItem->setSubtitle(body);
        m_page->addButtonIndexMapping(contentItem, index);
    }

    PanelPage *m_page;
};

PanelPage::PanelPage(QGraphicsItem *parent): MApplicationPage(parent)
{
    m_panelmgr = new McaPanelManager(this);
    m_panelmgr->setCategories(QStringList() << "email" << "social" << "im");
    m_panelmgr->initialize("sample");

    m_frozen = false;
    m_hiding = false;

    QGraphicsLinearLayout *linear = new QGraphicsLinearLayout(Qt::Vertical, centralWidget());

    QGraphicsLinearLayout *toolbar = new QGraphicsLinearLayout(Qt::Horizontal);
    linear->addItem(toolbar);

    MButton *button = new MButton("Search");
    connect(button, SIGNAL(clicked()), this, SLOT(openSearch()));
    toolbar->addItem(button);

    button = new MButton("Settings");
    connect(button, SIGNAL(clicked()), this, SLOT(openSettings()));
    toolbar->addItem(button);

    m_freezeButton = new MButton("Freeze");
    connect(m_freezeButton, SIGNAL(clicked()), this, SLOT(toggleFreeze()));
    toolbar->addItem(m_freezeButton);

    m_hideButton = new MButton("Mode: Default");
    connect(m_hideButton, SIGNAL(clicked()), this, SLOT(toggleHide()));
    toolbar->addItem(m_hideButton);

    MList *list = new MList;
    linear->addItem(list);
    list->setPreferredWidth(800);
    list->setItemModel(m_panelmgr->feedModel());
    PanelCellCreator *cellCreator = new PanelCellCreator;
    cellCreator->m_page = this;
    list->setCellCreator(cellCreator);
}

PanelPage::~PanelPage()
{
}

void PanelPage::addButtonIndexMapping(MContentItem *item, const QModelIndex &index)
{
    m_map.insert(item, QPersistentModelIndex(index));
}

void PanelPage::defaultAction()
{
    // find the itemthat was clicked
    MContentItem *item = qobject_cast<MContentItem *>(sender());

    // find the model index for the row to run action on
    QModelIndex index = m_map[item];

    // get the source model for this row before aggregation
    QVariant variant = index.data(McaFeedModel::CommonActionsRole);

    // cast to the original service model
    McaActions *actions = variant.value<McaActions*>();
    if (actions) {
        QString uniqueid = index.data(McaFeedModel::RequiredUniqueIdRole).toString();
        actions->performStandardAction(m_hiding ? "hide":"default", uniqueid);
    }
}

void PanelPage::openSettings()
{
    SettingsPage *settings = new SettingsPage(m_panelmgr);
    settings->appear(DestroyWhenDone);
}

void PanelPage::openSearch()
{
    SearchPage *search = new SearchPage;
    search->appear(DestroyWhenDone);
}

void PanelPage::toggleFreeze()
{
    m_frozen = !m_frozen;
    m_panelmgr->setFrozen(m_frozen);
    m_freezeButton->setText(m_frozen ? "Thaw":"Freeze");
}

void PanelPage::toggleHide()
{
    m_hiding = !m_hiding;
    m_hideButton->setText(m_hiding ? "Mode: Hide":"Mode: Default");
}
