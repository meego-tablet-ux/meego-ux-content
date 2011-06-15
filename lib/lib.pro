include(../common.pri)
TARGET = meegouxcontent
TEMPLATE = lib
QT += dbus

OBJECTS_DIR = .obj
MOC_DIR = .moc

SOURCES += \
    feedcache.cpp \
    abstractmanager.cpp \
    searchmanager.cpp \
    aggregatedmodelproxy.cpp \
    ../common/dbustypes.cpp \
    panelmanager.cpp

INSTALL_HEADERS += \
    abstractmanager.h \
    actions.h \
    feedmodel.h \
    feedplugin.h \
    panelmanager.h \
    searchmanager.h \
    servicemodel.h

HEADERS += \
    feedcache.h \
    abstractmanager.h \
    searchmanager.h \
    aggregatedmodelproxy.h \
    ../common/dbustypes.h \ 
    panelmanager.h

# Fix me, theres a few role refinition files in daemon-lib
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
