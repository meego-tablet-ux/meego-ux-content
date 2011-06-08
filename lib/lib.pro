include(../common.pri)
TARGET = meegouxcontent
TEMPLATE = lib
QT += dbus
CONFIG += threading
#CONFIG += threading-debug 
#CONFIG += memoryleak

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
    abstractmanager.cpp \
    actions.cpp \
    actionsproxy.cpp \
    adapter.cpp \
    allocator.cpp \
    aggregatedmodel.cpp \
    aggregatedservicemodel.cpp \
    feedadapter.cpp \
    feedcache.cpp \
    feedfilter.cpp \
    feedmanager.cpp \
    feedplugincontainer.cpp \
    feedrelevance.cpp \
    panelmanager.cpp \
    searchmanager.cpp \
    searchablecontainer.cpp \
    serviceadapter.cpp \
    serviceproxy.cpp \
    settings.cpp

INSTALL_HEADERS += \
    abstractmanager.h \
    actions.h \
    feedmodel.h \
    feedplugin.h \
    panelmanager.h \
    searchmanager.h \
    servicemodel.h

HEADERS += \
    actionsproxy.h \
    adapter.h \
    aggregatedmodel.h \
    aggregatedservicemodel.h \
    allocator.h \
    feedadapter.h \
    feedcache.h \
    feedfilter.h \
    feedmanager.h \
    feedplugincontainer.h \
    feedrelevance.h \
    memoryleak.h \
    memoryleak-defines.h \
    searchablecontainer.h \
    serviceadapter.h \
    serviceproxy.h \
    settings.h \
    threadtest.h \
    $$INSTALL_HEADERS

system(sed 's/__library_version__/$${VERSION}/g' meego-ux-content.pc.in > meego-ux-content.pc)

target.path = $$INSTALL_ROOT/usr/lib
INSTALLS += target

headers.files += $$INSTALL_HEADERS
headers.path += $$INSTALL_ROOT/usr/include/meegouxcontent
INSTALLS += headers

pkgconfig.files += meego-ux-content.pc
pkgconfig.path += $$INSTALL_ROOT/usr/lib/pkgconfig
INSTALLS += pkgconfig
