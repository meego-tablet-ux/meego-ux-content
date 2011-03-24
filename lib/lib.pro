include(../common.pri)
TARGET = meegouxcontent
TEMPLATE = lib

OBJECTS_DIR = .obj
MOC_DIR = .moc

SOURCES += \
    actions.cpp \
    allocator.cpp \
    aggregatedmodel.cpp \
    aggregatedservicemodel.cpp \
    feedadapter.cpp \
    feedcache.cpp \
    feedfilter.cpp \
    feedmanager.cpp \
    panelmanager.cpp \
    pluginloaderthread.cpp \
    searchmanager.cpp \
    searchablecontainer.cpp \
    serviceadapter.cpp \
    serviceproxy.cpp \
    settings.cpp \
    feedplugincontainer.cpp \
    abstractmanager.cpp

INSTALL_HEADERS += \
    actions.h \
    feedmodel.h \
    feedplugin.h \
    panelmanager.h \
    servicemodel.h \
    searchmanager.h

HEADERS += \
    aggregatedmodel.h \
    aggregatedservicemodel.h \
    allocator.h \
    feedadapter.h \
    feedcache.h \
    feedfilter.h \
    feedmanager.h \
    pluginloaderthread.h \
    searchablecontainer.h \
    serviceadapter.h \
    serviceproxy.h \
    settings.h \
    $$INSTALL_HEADERS \
    feedplugincontainer.h \
    abstractmanager.h \
    defines.h

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

headers.files += $$INSTALL_HEADERS
headers.path += $$INSTALL_ROOT/usr/include/meegouxcontent
INSTALLS += headers

pkgconfig.files += meego-ux-content.pc
pkgconfig.path += $$[QT_INSTALL_LIBS]/pkgconfig
INSTALLS += pkgconfig

VERSION = 0.1.2
