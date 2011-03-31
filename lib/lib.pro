include(../common.pri)
TARGET = meegouxcontent
TEMPLATE = lib
CONFIG += threading
CONFIG += threading-debug 
CONFIG += memoryleak

threading {
    DEFINES *= THREADING
    threading-debug {
        DEFINES *= THREADING_DEBUG
        SOURCES += threadtest.cpp
    }
}

memoryleak {
    DEFINES *= MEMORY_LEAK_DETECTOR
    SOURCES += memoryleak.cpp
}

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
    searchmanager.cpp \
    searchablecontainer.cpp \
    serviceadapter.cpp \
    serviceproxy.cpp \
    settings.cpp \
    feedplugincontainer.cpp \
    abstractmanager.cpp \
    actionsproxy.cpp \
    adapter.cpp

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
    searchablecontainer.h \
    serviceadapter.h \
    serviceproxy.h \
    settings.h \
    $$INSTALL_HEADERS \
    feedplugincontainer.h \
    abstractmanager.h \
    actionsproxy.h \
    threadtest.h \
    adapter.h \
    threadtest.h \
    memoryleak.h \
    memoryleak-defines.h

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

headers.files += $$INSTALL_HEADERS
headers.path += $$INSTALL_ROOT/usr/include/meegouxcontent
INSTALLS += headers

pkgconfig.files += meego-ux-content.pc
pkgconfig.path += $$[QT_INSTALL_LIBS]/pkgconfig
INSTALLS += pkgconfig

VERSION = 0.1.2
