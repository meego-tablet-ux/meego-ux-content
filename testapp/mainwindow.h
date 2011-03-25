/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QWidget>
#include <QLocalSocket>

class QLabel;
class QPushButton;
class QComboBox;
class QLineEdit;

class MainWindow: public QWidget
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

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

private:
    QLabel *m_label;
    QLocalSocket *m_socket;
    QPushButton *m_connectButton;
    QLineEdit *m_serviceNameEdit;
    QComboBox *m_serviceCombo;
    int m_serviceCount;
    int m_lastService;
};
