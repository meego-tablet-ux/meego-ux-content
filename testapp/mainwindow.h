/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QWidget>
#include <QLocalSocket>
#include <QTimer>

class QLabel;
class QPushButton;
class QComboBox;
class QLineEdit;

#define ADDACTIONSRATIO 5
#define REMOVEACTIONSRATIO 2

class MainWindow: public QWidget
{
    Q_OBJECT


public:
    MainWindow();
    ~MainWindow();

    enum Action {
        AddService,
        RemoveService,
        AddContentItem,
        AddRequestItem = AddContentItem + ADDACTIONSRATIO,
        RemoveRandomItem = AddRequestItem + ADDACTIONSRATIO,
        //Make sure this is the last item
        ActionsCount = RemoveRandomItem + REMOVEACTIONSRATIO
//        ActionsCount = RemoveService + 1
    };

protected slots:
    void socketConnected();
    void socketDisconnected();
    void socketError(QLocalSocket::LocalSocketError error);
    void socketStateChanged(QLocalSocket::LocalSocketState state);

    void tryConnect();
    void addService();
    void removeService();
    void addContentItem();
    void addRequestItem();
    void removeItem();

    void randomTimerHandler();
    void toggleRandomStuff();

private:
    QLabel *m_label;
    QLocalSocket *m_socket;
    QPushButton *m_connectButton;
    QLineEdit *m_serviceNameEdit;
    QComboBox *m_serviceCombo;
    int m_serviceCount;
    int m_lastService;

    QTimer m_randomStuffTimer;
};
