include(../common.pri)
PREFIX = /usr

TARGET = mtfcontent
TEMPLATE = app

CONFIG += meegotouch
PKGCONFIG += meegotouch

INCLUDEPATH += ../lib
LIBS += -L../lib -lmeegouxcontent

OBJECTS_DIR = .obj
MOC_DIR = .moc

SOURCES += \
    mtfcontent.cpp \
    panelpage.cpp \
    searchpage.cpp \
    settingspage.cpp

HEADERS += \
    panelpage.h \
    searchpage.h \
    settingspage.h

target.path = $$PREFIX/bin

INSTALLS += \
    target

MGEN_OUTDIR = .gen
mgen.output = $$GEN_DIR/gen_${QMAKE_FILE_BASE}data.cpp
mgen.clean += $$GEN_DIR/gen_*
