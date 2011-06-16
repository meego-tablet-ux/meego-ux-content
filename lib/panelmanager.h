#ifndef PANELMANAGERPROXY_H
#define PANELMANAGERPROXY_H

#include <QStringList>
#include "abstractmanager.h"

class ServiceModelDbusProxy;

class McaPanelManager : public McaAbstractManager
{
    Q_OBJECT
    Q_PROPERTY(QSortFilterProxyModel *serviceModel READ serviceModel)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)
    Q_PROPERTY(QStringList categories READ categories WRITE setCategories NOTIFY categoriesChanged)
    Q_PROPERTY(bool servicesEnabledByDefault READ servicesEnabledByDefault WRITE setServicesEnabledByDefault)

public:
    McaPanelManager(QObject *parent = 0);

    // property functions - already accessible to QML
    virtual bool isEmpty();
    virtual QStringList categories();
    bool servicesEnabledByDefault();
    void setServicesEnabledByDefault(bool enabled);
    virtual QSortFilterProxyModel *serviceModel();

    Q_INVOKABLE bool isServiceEnabled(const QString& upid);
    Q_INVOKABLE void setServiceEnabled(const QString& upid, bool enabled);

signals:
    void isEmptyChanged(bool isEmpty);
    void categoriesChanged(const QStringList& categories);

public slots:
    virtual void initialize(const QString& managerData = QString());
    void setCategories(const QStringList& categories);

private slots:
    void feedRowsChanged();

private:
    QString m_panelName;
    QStringList m_categories;

    bool m_isEmpty;
    bool m_servicesEnabledByDefault;

    ServiceModelDbusProxy *m_dbusServiceModel;
};

#endif // PANELMANAGERPROXY_H
