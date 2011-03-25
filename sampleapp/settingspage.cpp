/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>

#include <QGraphicsLinearLayout>
#include <MList>
#include <MAbstractCellCreator>
#include <MLabel>
#include <MButton>

#include <servicemodel.h>
#include <panelmanager.h>
#include <actions.h>

#include "settingspage.h"
#include "serviceproxy.h"
#include "serviceadapter.h"

class SettingsCellCreator: public MAbstractCellCreator<MWidgetController>
{
public:
    void updateCell(const QModelIndex& index, MWidget *cell) const
    {
        QGraphicsLinearLayout *linear = dynamic_cast<QGraphicsLinearLayout *>(cell->layout());
        MLabel *label;
        MButton *configure, *history, *enable;
        if (!linear) {
            linear = new QGraphicsLinearLayout;

            cell->setLayout(linear);
            label = new MLabel;
            linear->addItem(label);

            linear->addStretch();

            configure = new MButton("Configure");
            configure->setPreferredWidth(200);
            QObject::connect(configure, SIGNAL(clicked()), m_page, SLOT(configure()));
            linear->addItem(configure);

            history = new MButton("Clear History");
            history->setPreferredWidth(200);
            QObject::connect(history, SIGNAL(clicked()), m_page, SLOT(clearHistory()));
            linear->addItem(history);

            enable = new MButton("Enable");
            enable->setPreferredWidth(200);
            enable->setViewType(MButton::toggleType);
            enable->setCheckable(true);
            QObject::connect(enable, SIGNAL(clicked(bool)), m_page, SLOT(toggleEnable(bool)));
            linear->addItem(enable);
        }
        else {
            label = dynamic_cast<MLabel *>(linear->itemAt(0));
            configure = dynamic_cast<MButton *>(linear->itemAt(1));
            history = dynamic_cast<MButton *>(linear->itemAt(2));
            enable = dynamic_cast<MButton *>(linear->itemAt(3));
        }

        label->setText(index.data(McaServiceModel::CommonDisplayNameRole).toString());
        m_page->addButtonIndexMapping(configure, index);
        m_page->addButtonIndexMapping(history, index);
        m_page->addButtonIndexMapping(enable, index);
        enable->setChecked(index.model()->data(index, McaServiceProxy::SystemEnabledRole).toBool());
    }

    QSizeF cellSize() const
    {
        return QSizeF(800, 70);
    }

    SettingsPage *m_page;
};

SettingsPage::SettingsPage(McaPanelManager *panelmgr, QGraphicsItem *parent):
        MApplicationPage(parent)
{
    m_panelmgr = panelmgr;

    MList *list = new MList(centralWidget());
    list->setPreferredWidth(800);
    list->setItemModel(m_panelmgr->serviceModel());

    SettingsCellCreator *cellCreator = new SettingsCellCreator;
    cellCreator->m_page = this;
    list->setCellCreator(cellCreator);
}

SettingsPage::~SettingsPage()
{
}

void SettingsPage::addButtonIndexMapping(MButton *button, const QModelIndex &index)
{
    m_map.insert(button, QPersistentModelIndex(index));
}

void SettingsPage::configure()
{
    // find the button that was clicked
    MButton *button = qobject_cast<MButton *>(sender());

    // find the model index for the row to run configure on
    QModelIndex index = m_map[button];

    // get the source model for this row before aggregation
    QVariant variant = index.data(McaServiceModel::CommonActionsRole);

    // cast to the original service model
    McaActions *actions = variant.value<McaActions*>();
    if (actions) {
        QString uniqueid = index.data(McaServiceModel::RequiredNameRole).toString();
        actions->performStandardAction("configure", uniqueid);
    }
}

void SettingsPage::clearHistory()
{
    // find the button that was clicked
    MButton *button = qobject_cast<MButton *>(sender());

    // find the model index for the row to run configure on
    QModelIndex index = m_map[button];

    QString upid = index.data(McaServiceAdapter::SystemUpidRole).toString();
    m_panelmgr->clearHistory(upid);
}

void SettingsPage::toggleEnable(bool enabled)
{
    // find the button that was clicked
    MButton *button = qobject_cast<MButton *>(sender());

    // find the model index for the row to run configure on
    QModelIndex index = m_map[button];

    QString upid = index.data(McaServiceAdapter::SystemUpidRole).toString();
    m_panelmgr->setServiceEnabled(upid, enabled);
}
