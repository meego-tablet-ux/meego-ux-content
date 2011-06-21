include(../common.pri)
TARGET = meegouxcontent
TEMPLATE = lib
QT += dbus
CONFIG += debug

OBJECTS_DIR = .obj
MOC_DIR = .moc

SOURCES += \
    abstractmanager.cpp \
    searchmanager.cpp \
    aggregatedmodelproxy.cpp \
    panelmanager.cpp \
    servicemodeldbusproxy.cpp \
    modeldbusinterface.cpp \
    ../common/dbustypes.cpp \
    actionsproxy.cpp

INSTALL_HEADERS += \
    abstractmanager.h \
    panelmanager.h \
    searchmanager.h

HEADERS += \
    $$INSTALL_HEADERS \
    aggregatedmodelproxy.h \
    servicemodeldbusproxy.h \
    modeldbusinterface.h \
    ../common/dbusdefines.h \
    ../common/dbustypes.h \
    actionsproxy.h

# Fix me, theres a few role definitions files in daemon-lib
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
