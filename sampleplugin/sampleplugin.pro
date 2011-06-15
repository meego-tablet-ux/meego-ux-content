#
# Sample meego-ux-content plugin project
# 

# This include is just to get a version number
include(../common.pri)
TARGET = sample_plugin
TEMPLATE = lib

CONFIG += plugin

exists($$OUT_PWD/../.intree_build) {
    include(../common.pri)

    INCLUDEPATH += ../daemon-lib ../common
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
    emailmodel.cpp \
    filter.cpp \
    plugin.cpp \
    servmodel.cpp \
    socialmodel.cpp

HEADERS += \
    emailmodel.h \
    filter.h \
    plugin.h \
    servmodel.h \
    socialmodel.h

target.path = $$[QT_INSTALL_PLUGINS]/MeeGo/Content
INSTALLS += target
