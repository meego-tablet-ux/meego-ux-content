#ifndef SEARCHMANAGERPROXY_H
#define SEARCHMANAGERPROXY_H

#include "abstractmanager.h"

class ServiceModelDbusProxy;

class McaSearchManager : public McaAbstractManager
{
    Q_OBJECT
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(ServiceModelDbusProxy *serviceModel READ serviceModel)

public:
    McaSearchManager(QObject *parent = 0);
    ~McaSearchManager();

    QString searchText();
    virtual ServiceModelDbusProxy *serviceModel();

signals:
    void searchTextChanged(const QString& searchText);

public slots:
    void setSearchText(const QString &searchText);
    virtual void initialize(const QString& managerData = QString());

protected:
    void serviceStateChanged(bool offline);

    virtual QModelIndex serviceModelIndex(int row);
    virtual int serviceModelRowCount();
    virtual QVariant serviceModelData(const QModelIndex &index, int role);
    virtual bool dataChangedCondition(const QModelIndex &index);

private:
    QString m_searchText;
    QString m_localSearchText;

    ServiceModelDbusProxy *m_dbusServiceModel;
};

#endif // SEARCHMANAGERPROXY_H
