#
# Sample meegouxcontent plugin project
# 

# This include is just to get a version number
include(../common.pri)
TARGET = testplugin
TEMPLATE = lib

QT += network
CONFIG += plugin 

exists($$OUT_PWD/../.intree_build) {
    include(../common.pri)

    INCLUDEPATH += ../daemon-lib
    LIBS += -L../daemon-lib -lmeegouxcontent-daemon
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += meego-ux-content
    VERSION = 0.0.0

    # use pkg-config paths for include in both g++ and moc
    INCLUDEPATH += $$system(pkg-config --cflags meego-ux-content \
        | tr \' \' \'\\n\' | grep ^-I | cut -d 'I' -f 2-)
}

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
