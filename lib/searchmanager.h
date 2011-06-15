#ifndef SEARCHMANAGERPROXY_H
#define SEARCHMANAGERPROXY_H

#include "abstractmanager.h"

class McaSearchManager : public McaAbstractManager
{
    Q_OBJECT
public:
    McaSearchManager(QObject *parent = 0);

signals:

public slots:
    void setSearchText(const QString &searchText);
    virtual void initialize(const QString& managerData = QString());

private:
    QString m_searchText;
};

#endif // SEARCHMANAGERPROXY_H
