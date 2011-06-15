#include <QDBusPendingCall>
#include "panelmanager.h"

McaPanelManager::McaPanelManager(QObject *parent) :
    McaAbstractManager("newPanelManager", parent)
{

}

void McaPanelManager::initialize(const QString& managerData)
{
    m_panelName = managerData;
    McaAbstractManager::initialize(managerData);
}

void McaPanelManager::setCategories(const QStringList& categories)
{
    m_categories = categories;
    m_dbusManagerInterface->call("setCategories", QVariant(categories));
}
