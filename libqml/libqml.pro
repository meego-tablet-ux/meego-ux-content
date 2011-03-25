include(../common.pri)
TARGET = Content
TEMPLATE = lib

CONFIG += plugin
QT += declarative

INCLUDEPATH += ../lib
LIBS += -L../lib -lmeegouxcontent

OBJECTS_DIR = .obj
MOC_DIR = .moc

SOURCES += \
    contentqml.cpp

HEADERS += \
    contentqml.h

target.path += $$[QT_INSTALL_IMPORTS]/MeeGo/Content
INSTALLS += target

qmldir.files += qmldir
qmldir.path += $$[QT_INSTALL_IMPORTS]/MeeGo/Content
INSTALLS += qmldir
