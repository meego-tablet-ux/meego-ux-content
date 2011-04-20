include(../common.pri)

TEMPLATE = subdirs

SOURCES += \
    main.qml \
    SampleItem.qml \
    ServiceItem.qml

qml.files += $$SOURCES
qml.path = /usr/share/meego-sample-content

INSTALLS += qml
