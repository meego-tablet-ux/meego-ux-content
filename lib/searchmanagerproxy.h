#ifndef SEARCHMANAGERPROXY_H
#define SEARCHMANAGERPROXY_H

#include "abstractmanagerproxy.h"

class McaSearchManagerProxy : public McaAbstractManagerProxy
{
    Q_OBJECT
public:
    McaSearchManagerProxy(QObject *parent = 0);

signals:

public slots:
    void setSearchText(const QString &searchText);
    virtual void initialize(const QString& managerData = QString());

private:
    QString m_searchText;
};

#endif // SEARCHMANAGERPROXY_H
