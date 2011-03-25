#
# Sample meego-ux-content plugin project
# 

# This include is just to get a version number
include(../common.pri)
TARGET = sample_plugin
TEMPLATE = lib

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
