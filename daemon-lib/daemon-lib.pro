include(../common.pri)
TARGET = meegouxcontent-daemon
TEMPLATE = lib
QT += dbus
CONFIG += threading
#CONFIG += threading-debug 
#CONFIG += memoryleak

threading {
    DEFINES *= THREADING
    threading-debug {
        DEFINES *= THREADING_DEBUG
        SOURCES += ../common/threadtest.cpp
    }
}

memoryleak {
    DEFINES *= MEMORY_LEAK_DETECTOR
    SOURCES += memoryleak.cpp
}

OBJECTS_DIR = .obj
MOC_DIR = .moc

SOURCES += \
    abstractmanagerdbus.cpp \
    actions.cpp \
    adapter.cpp \
    allocator.cpp \
    aggregatedmodel.cpp \
    feedadapter.cpp \
    feedfilter.cpp \
    feedmanager.cpp \
    feedplugincontainer.cpp \
    feedrelevance.cpp \
    panelmanagerdbus.cpp \
    searchmanagerdbus.cpp \
    searchablecontainer.cpp \
    serviceadapter.cpp \
    serviceproxy.cpp \
    settings.cpp \
    ../common/dbustypes.cpp

INSTALL_HEADERS += \
    abstractmanagerdbus.h \
    actions.h \
    ../common/feedmodel.h \
    ../common/feedplugin.h \
    panelmanagerdbus.h \
    searchmanagerdbus.h \
    ../common/servicemodel.h

HEADERS += \
    ../common/adapter.h \
    aggregatedmodel.h \
    allocator.h \
    ../common/feedadapter.h \
    feedfilter.h \
    feedmanager.h \
    feedplugincontainer.h \
    feedrelevance.h \
    memoryleak.h \
    memoryleak-defines.h \
    searchablecontainer.h \
    serviceproxy.h \
    settings.h \
    ../common/threadtest.h \
    $$INSTALL_HEADERS \
    ../common/dbustypes.h \
    ../common/serviceadapter.h \
    ../common/dbusdefines.h

INCLUDEPATH += ../common

system(sed 's/__library_version__/$${VERSION}/g' meego-ux-content.pc.in > meego-ux-content.pc)

target.path = $$INSTALL_ROOT/usr/lib
INSTALLS += target

headers.files += $$INSTALL_HEADERS
headers.path += $$INSTALL_ROOT/usr/include/meegouxcontent
INSTALLS += headers

pkgconfig.files += meego-ux-content.pc
pkgconfig.path += $$INSTALL_ROOT/usr/lib/pkgconfig
INSTALLS += pkgconfig
