#include <QDebug>
#include <QDBusInterface>
#include "modeldbusinterface.h"

ModelDBusInterface::ModelDBusInterface(const QString &service) :
    m_dbusModel(0),
    m_objectOffline(true)
{
    m_service = service;
}

void ModelDBusInterface::setOffline(bool offline)
{
    m_objectOffline = offline;

    if(offline) {
        if(0 == m_dbusModel) {
            qDebug() << "WARNING:ModelDBusInterface::setOffline(false) called although we don't have a remote object";
            return;
        } else {
            delete m_dbusModel;
            m_dbusModel = 0;
        }
    } else {
        if(0 == m_dbusModel) {
            m_dbusModel = new QDBusInterface(m_service, m_objectPath);
            qDebug() << "ModelDBusInterface new interface created for " << m_objectPath << ", and isValid " << m_dbusModel->isValid();
        } else {
            qDebug() << "WARNING:ModelDBusInterface::setOffline(true) called although we already have a remote object";
            return;
        }
    }
    doOfflineChanged();
}

void ModelDBusInterface::setObjectPath(const QString &objectPath)
{
    m_objectPath = objectPath;
}

bool ModelDBusInterface::isOffline()
{
    return m_objectOffline;
}
