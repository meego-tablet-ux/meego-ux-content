#-------------------------------------------------
#
# Project created by QtCreator 2011-06-09T14:08:25
#
#-------------------------------------------------

QT       += core dbus

QT       -= gui

TARGET = meego-ux-content
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    contentdaemon.cpp \
    contentdaemoninterface.cpp

HEADERS += \
    contentdaemon.h \
    contentdaemoninterface.h

INCLUDEPATH += ../daemon-lib ../common
LIBS += -L../daemon-lib -lmeegouxcontent-daemon

dbusservice.path=$$INSTALL_ROOT/usr/share/dbus-1/services
dbusservice.files=com.meego.content.service

target.path=$$INSTALL_ROOT/usr/bin

INSTALLS=target dbusservice
