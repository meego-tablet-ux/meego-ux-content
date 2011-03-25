#
# Sample meegouxcontent plugin project
# 

# This include is just to get a version number
include(../common.pri)
TARGET = testplugin
TEMPLATE = lib

QT += network
CONFIG += plugin link_pkgconfig
PKGCONFIG += meego-ux-content

# use pkg-config paths for include in both g++ and moc
INCLUDEPATH += $$system(pkg-config --cflags meego-ux-content \
    | tr \' \' \'\\n\' | grep ^-I | cut -d 'I' -f 2-)

# this is only for building within the meego-ux-content source tree
INCLUDEPATH += ../lib

LIBS += -L../lib -lmeegouxcontent

OBJECTS_DIR = .obj
MOC_DIR = .moc

SOURCES += \
    plugin.cpp \
    servmodel.cpp \
    testmodel.cpp

HEADERS += \
    plugin.h \
    servmodel.h \
    testmodel.h

target.path = $$[QT_INSTALL_PLUGINS]/MeeGo/Content
INSTALLS += target
