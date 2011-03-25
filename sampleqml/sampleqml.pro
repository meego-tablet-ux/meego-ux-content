include(../common.pri)

TEMPLATE = subdirs

SOURCES += \
    meego-sample-search/main.qml \
    meego-sample-search/SearchItem.qml

qml.files += meego-sample-search
qml.path = /usr/share

INSTALLS += qml
