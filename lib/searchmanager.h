#ifndef SEARCHMANAGERPROXY_H
#define SEARCHMANAGERPROXY_H

#include "abstractmanager.h"

class McaSearchManager : public McaAbstractManager
{
    Q_OBJECT
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

public:
    McaSearchManager(QObject *parent = 0);

    QString searchText();

signals:
    void searchTextChanged(const QString& searchText);

public slots:
    void setSearchText(const QString &searchText);
    virtual void initialize(const QString& managerData = QString());

protected:
    void serviceStateChanged(bool offline);

private:
    QString m_searchText;
    QString m_localSearchText;
};

#endif // SEARCHMANAGERPROXY_H
