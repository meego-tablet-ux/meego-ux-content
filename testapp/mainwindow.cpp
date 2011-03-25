/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include <QComboBox>
#include <QLineEdit>

#include "mainwindow.h"
#include "plugin.h"

MainWindow::MainWindow()
{
    m_serviceCount = 0;
    m_lastService = 0;

    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);

    m_label = new QLabel("Content Aggregator Tester");
    m_label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(m_label);

    QFrame *frame = new QFrame;
    frame->setFrameStyle(QFrame::Sunken);
    frame->setFrameShape(QFrame::HLine);
    layout->addWidget(frame);

    m_connectButton = new QPushButton("Connect");
    connect(m_connectButton, SIGNAL(clicked()), this, SLOT(tryConnect()));
    layout->addWidget(m_connectButton);

    // add/remove services

    frame = new QFrame;
    frame->setFrameStyle(QFrame::Sunken);
    frame->setFrameShape(QFrame::HLine);
    layout->addWidget(frame);

    QWidget *widget = new QWidget;
    layout->addWidget(widget);
    QBoxLayout *hlayout = new QBoxLayout(QBoxLayout::LeftToRight, widget);

    QLabel *label = new QLabel("Service name:");
    hlayout->addWidget(label);

    m_serviceNameEdit = new QLineEdit;
    connect(m_serviceNameEdit, SIGNAL(returnPressed()),
            this, SLOT(addService()));
    hlayout->addWidget(m_serviceNameEdit);

    QPushButton *button = new QPushButton("Add Service");
    connect(button, SIGNAL(clicked()), this, SLOT(addService()));
    hlayout->addWidget(button);

    // add/remove items

    frame = new QFrame;
    frame->setFrameStyle(QFrame::Sunken);
    frame->setFrameShape(QFrame::HLine);
    layout->addWidget(frame);

    widget = new QWidget;
    layout->addWidget(widget);
    hlayout = new QBoxLayout(QBoxLayout::LeftToRight, widget);

    label = new QLabel("Choose a service:");
    hlayout->addWidget(label);

    m_serviceCombo = new QComboBox;
    hlayout->addWidget(m_serviceCombo);

    button = new QPushButton("Remove Service");
    connect(button, SIGNAL(clicked()), this, SLOT(removeService()));
    layout->addWidget(button);

    button = new QPushButton("Add Content Item");
    connect(button, SIGNAL(clicked()), this, SLOT(addContentItem()));
    layout->addWidget(button);

    button = new QPushButton("Add Request Item");
    connect(button, SIGNAL(clicked()), this, SLOT(addRequestItem()));
    layout->addWidget(button);

    button = new QPushButton("Remove Item");
    connect(button, SIGNAL(clicked()), this, SLOT(removeItem()));
    layout->addWidget(button);

    m_socket = new QLocalSocket;
    connect(m_socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)),
            this, SLOT(socketError(QLocalSocket::LocalSocketError)));
    connect(m_socket, SIGNAL(stateChanged(QLocalSocket::LocalSocketState)),
            this, SLOT(socketStateChanged(QLocalSocket::LocalSocketState)));
}

MainWindow::~MainWindow()
{
}

void MainWindow::socketConnected()
{
    qDebug() << "MainWindow::socketConnected";
}

void MainWindow::socketDisconnected()
{
    qDebug() << "MainWindow::socketDisconnected";
}

void MainWindow::socketError(QLocalSocket::LocalSocketError error)
{
    Q_UNUSED(error)
    qDebug() << "MainWindow::socketError";
}

void MainWindow::socketStateChanged(QLocalSocket::LocalSocketState state)
{
    qDebug() << "MainWindow::socketStateChanged:";
    switch(state) {
    case QLocalSocket::UnconnectedState:
        qDebug() << "UNCONNECTED";
        m_connectButton->setText("Connect");
        break;
    case QLocalSocket::ConnectingState:
        qDebug() << "CONNECTING";
        m_connectButton->setText("Connecting...");
        break;
    case QLocalSocket::ConnectedState:
        qDebug() << "CONNECTED";
        m_connectButton->setText("Disconnect");
        break;
    case QLocalSocket::ClosingState:
        qDebug() << "CLOSING";
        m_connectButton->setText("Closing...");
        break;
    default:
        qDebug() << "UNEXPECTED SOCKET STATE";
        break;
    }
}

static void sendMessage(QLocalSocket *socket, int type, QString str = QString())
{
    qDebug() << "sendMessage:" << type << str;
    int len = str.length();
    if (len > 0)
        len++;  // account for null terminator

    int buflen = sizeof(Plugin::TestCommand) + len;
    char *buf = new char[buflen];
    Plugin::TestCommand *command = (Plugin::TestCommand *)buf;

    command->type = type;
    command->len = len;
    if (len > 0)
        qstrncpy(command->data, str.toAscii().data(), len);

    socket->write(buf, buflen);
}

void MainWindow::tryConnect()
{
    QString text = m_connectButton->text();
    if (text == "Connect") {
        m_socket->connectToServer("meego-ux-content-test");
        m_serviceNameEdit->clear();
        m_serviceCombo->clear();
        m_serviceCount = 0;
        m_lastService = 0;
    }
    else if (text == "Disconnect")
        m_socket->disconnectFromServer();
}

void MainWindow::addService()
{
    qDebug() << "ADD SERVICE";
    QString name = m_serviceNameEdit->text();
    if (name.isEmpty())
        return;

    sendMessage(m_socket, Plugin::TestCommandAddService, name);
    m_serviceCombo->addItem(name);
    m_serviceNameEdit->clear();
    m_serviceCount++;
}

void MainWindow::removeService()
{
    qDebug() << "REMOVE SERVICE";
    sendMessage(m_socket, Plugin::TestCommandRemoveService, m_serviceCombo->currentText());
    m_serviceCombo->removeItem(m_serviceCombo->currentIndex());
    m_serviceCount--;
}

void MainWindow::addContentItem()
{
    qDebug() << "ADD CONTENT ITEM";
    sendMessage(m_socket, Plugin::TestCommandAddContentItem, m_serviceCombo->currentText());
}

void MainWindow::addRequestItem()
{
    qDebug() << "ADD REQUEST ITEM";
    sendMessage(m_socket, Plugin::TestCommandAddRequestItem, m_serviceCombo->currentText());
}

void MainWindow::removeItem()
{
    qDebug() << "REMOVE SERVICE";
    sendMessage(m_socket, Plugin::TestCommandRemoveItem, m_serviceCombo->currentText());
}
