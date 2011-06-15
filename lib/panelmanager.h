#ifndef PANELMANAGERPROXY_H
#define PANELMANAGERPROXY_H

#include <QStringList>
#include "abstractmanager.h"

class McaPanelManager : public McaAbstractManager
{
    Q_OBJECT
public:
    McaPanelManager(QObject *parent = 0);

signals:

public slots:
    virtual void initialize(const QString& managerData = QString());
    void setCategories(const QStringList& categories);

private:
    QString m_panelName;
    QStringList m_categories;
};

#endif // PANELMANAGERPROXY_H
