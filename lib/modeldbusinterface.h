#ifndef MODELDBUSINTERFACE_H
#define MODELDBUSINTERFACE_H

#include <QAbstractListModel>

class QDBusInterface;

class ModelDBusInterface : public QAbstractListModel
{
    Q_OBJECT
public:
    ModelDBusInterface(const QString &service);

    void setObjectPath(const QString &objectPath);

public slots:
    void setOffline(bool offline);

protected:
    bool isOffline();
    virtual void doOfflineChanged() = 0;

protected:
    QDBusInterface *m_dbusModel;
    bool m_objectOffline;
    QString m_service;
    QString m_objectPath;
};

#endif // MODELDBUSINTERFACE_H
