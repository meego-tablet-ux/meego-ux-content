#ifndef ACTIONSPROXY_H
#define ACTIONSPROXY_H

#include <QObject>
#include <QStringList>
#include <QMetaType>

class McaActionsProxy : public QObject
{
    Q_OBJECT
    // identifiers for custom actions (FUTURE)
    Q_PROPERTY(QStringList customActions READ customActions)
    // localized strings to show the user in a context menu (FUTURE)
    Q_PROPERTY(QStringList customDisplayActions READ customDisplayActions)
public:
    McaActionsProxy(QObject *parent = 0);

    Q_INVOKABLE virtual QStringList customActions();
    Q_INVOKABLE virtual QStringList customDisplayActions();

    void setCustomActions(const QStringList &ids, const QStringList &names);
    void setServiceUpid(const QString &upid);
    QString getServiceUpid();

public slots:
    void performStandardAction(QString action, QString uniqueid);
    void performCustomAction(QString action, QString uniqueid);

signals:
    void standardAction(QString action, QString uniqueid);
    void customAction(QString action, QString uniqueid);

private:
    QStringList m_ids;
    QStringList m_names;
    QString m_serviceUpid;

};

Q_DECLARE_METATYPE(McaActionsProxy*)

#endif // ACTIONSPROXY_H
