include(../common.pri)
PREFIX = /usr

TARGET = catester
TEMPLATE = app

QT += gui network

INCLUDEPATH += ../testplugin ../lib ../common

OBJECTS_DIR = .obj
MOC_DIR = .moc

SOURCES += \
    catester.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

target.path = $$PREFIX/bin

INSTALLS += \
    target

MGEN_OUTDIR = .gen
mgen.output = $$GEN_DIR/gen_${QMAKE_FILE_BASE}data.cpp
mgen.clean += $$GEN_DIR/gen_*
