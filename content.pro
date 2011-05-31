include(common.pri)
CONFIG += ordered
TEMPLATE = subdirs

system("touch $$OUT_PWD/.intree_build")

SUBDIRS += lib libqml
SUBDIRS += sampleplugin sampleqml

# Uncomment to build test app and plugin
#SUBDIRS += testapp testplugin

PROJECT_NAME = meego-ux-content

dist.commands += rm -fR $${PROJECT_NAME}-$${VERSION} &&
dist.commands += git clone . $${PROJECT_NAME}-$${VERSION} &&
dist.commands += rm -fR $${PROJECT_NAME}-$${VERSION}/.git &&
dist.commands += rm -f $${PROJECT_NAME}-$${VERSION}/.gitignore &&
dist.commands += tar jcpvf $${PROJECT_NAME}-$${VERSION}.tar.bz2 $${PROJECT_NAME}-$${VERSION} &&
dist.commands += rm -fR $${PROJECT_NAME}-$${VERSION} &&
dist.commands += echo; echo Created $${PROJECT_NAME}-$${VERSION}.tar.bz2
QMAKE_EXTRA_TARGETS += dist
