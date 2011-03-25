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
#include <MTextEdit>
#include <MLabel>

#include <searchmanager.h>
#include <feedmodel.h>
#include <actions.h>

#include "searchpage.h"
#include "settingspage.h"

class SearchCellCreator: public MAbstractCellCreator<MContentItem>
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

    SearchPage *m_page;
};

SearchPage::SearchPage(QGraphicsItem *parent): MApplicationPage(parent)
{
    m_searchmgr = new McaSearchManager(this);
    m_searchmgr->initialize("");

    m_frozen = false;

    QGraphicsLinearLayout *linear = new QGraphicsLinearLayout(Qt::Vertical, centralWidget());

    QGraphicsLinearLayout *toolbar = new QGraphicsLinearLayout(Qt::Horizontal);
    linear->addItem(toolbar);

    m_freezeButton = new MButton("Freeze");
    connect(m_freezeButton, SIGNAL(clicked()), this, SLOT(toggleFreeze()));
    toolbar->addItem(m_freezeButton);

    MLabel *label = new MLabel("Search:");
    toolbar->addItem(label);

    m_editSearch = new MTextEdit;
    connect(m_editSearch, SIGNAL(textChanged()), this, SLOT(search()));
    connect(m_editSearch, SIGNAL(returnPressed()), this, SLOT(searchFinal()));
    m_editSearch->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    toolbar->addItem(m_editSearch);

    MList *list = new MList;
    linear->addItem(list);
    list->setPreferredWidth(800);
    list->setItemModel(m_searchmgr->feedModel());
    SearchCellCreator *cellCreator = new SearchCellCreator;
    cellCreator->m_page = this;
    list->setCellCreator(cellCreator);
}

SearchPage::~SearchPage()
{
}

void SearchPage::addButtonIndexMapping(MContentItem *item, const QModelIndex &index)
{
    m_map.insert(item, QPersistentModelIndex(index));
}

void SearchPage::defaultAction()
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
        actions->performStandardAction("default", uniqueid);
    }
}

void SearchPage::toggleFreeze()
{
    m_frozen = !m_frozen;
    m_searchmgr->setFrozen(m_frozen);
    m_freezeButton->setText(m_frozen ? "Thaw":"Freeze");
}

void SearchPage::search()
{
    QString text = m_editSearch->text();
    if (text.isEmpty())
        return;
    m_searchmgr->setSearchText(text);
}

void SearchPage::searchFinal()
{
    m_editSearch->clear();
}
