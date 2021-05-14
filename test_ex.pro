
QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = testex
TEMPLATE = app
VERSION = 0.8.0.1
QMAKE_TARGET_PRODUCT = testex
QMAKE_TARGET_DESCRIPTION = testex

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/configuration.cpp \
    src/dialogadditem.cpp \
    src/commands.cpp \
    src/logger.cpp \
    src/dialogsettestitem.cpp \
    src/smartcard.cpp \
    src/dialognewseqpreset.cpp \
    src/qcustomplot/qcustomplot.cpp \
    src/dialogshowgraph.cpp \
    src/dialogshowdevattrib.cpp \
    src/commonfuncs.cpp \
    src/combinations.cpp \
    src/popup.cpp

HEADERS  += src/mainwindow.h \
    src/configuration.h \
    src/dialogadditem.h \
    src/commands.h \
    src/logger.h \
    src/dialogsettestitem.h \
    src/smartcard.h \
    src/dialognewseqpreset.h \
    src/qcustomplot/qcustomplot.h \
    src/dialogshowgraph.h \
    src/dialogshowdevattrib.h \
    src/commonfuncs.h \
    src/combinations.h \
    src/popup.h

FORMS    += ui/mainwindow.ui \
    ui/dialogadditem.ui \
    ui/dialogsettestitem.ui \
    ui/dialognewseqpreset.ui \
    ui/dialogshowgraph.ui \
    ui/dialogshowdevattrib.ui

DISTFILES += \
    back_debian/control \
    back_debian/rules \
    build_deb \
    .gitignore \
    device_conf.xml

RESOURCES += \
    test_ex.qrc

INCLUDEPATH += $$PWD/src

unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += libpcsclite
    LIBS += -ldl -lpcsclite
    QMAKE_LFLAGS += -rdynamic

    #VARIABLES
    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    BINDIR = $$PREFIX/bin
    DATADIR =$$PREFIX/share

    DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

    #MAKE INSTALL

    target.path =$$BINDIR
    INSTALLS += target

    desktop.path = $$DATADIR/applications/
    desktop.files += $${TARGET}.desktop
    INSTALLS += desktop

    config_file.path = $$DATADIR/$${TARGET}/
    config_file.files = device_apdu.cfg
    INSTALLS += config_file

    #ICONS
    app_icons16.path = $$DATADIR/icons/hicolor/16x16/apps/
    app_icons16.files = app_icons/16x16/*.*
    INSTALLS += app_icons16

    app_icons24.path = $$DATADIR/icons/hicolor/24x24/apps/
    app_icons24.files = app_icons/24x24/*.*
    INSTALLS += app_icons24

    app_icons32.path = $$DATADIR/icons/hicolor/32x32/apps/
    app_icons32.files = app_icons/32x32/*.*
    INSTALLS += app_icons32

    app_icons48.path = $$DATADIR/icons/hicolor/48x48/apps/
    app_icons48.files = app_icons/48x48/*.*
    INSTALLS += app_icons48

    app_icons128.path = $$DATADIR/icons/hicolor/128x128/apps/
    app_icons128.files = app_icons/128x128/*.*
    INSTALLS += app_icons128

    app_icons96.path = $$DATADIR/icons/hicolor/96x96/apps/
    app_icons96.files = app_icons/96x96/*.*
    INSTALLS += app_icons96
}

macx {
    LIBS = -L"/usr/local/opt/pcsc-lite/lib" -lpcsclite
    INCLUDEPATH += /usr/local/opt/pcsc-lite/include
    INCLUDEPATH += /usr/local/opt/pcsc-lite/include/PCSC
    QMAKE_LFLAGS += -framework PCSC
}

win32 {
#        QMAKE_CFLAGS_RELEASE += /MT
#        QMAKE_CXXFLAGS_RELEASE += /MT
#        QMAKE_LFLAGS_RELEASE += -static
#        CONFIG += static
#        CONFIG += release

        INCLUDEPATH += $$PWD/libs/include

        RC_ICONS += images/smartcard.ico
        LIBS += -lwinscard

    !contains(QMAKE_TARGET.arch, x86_64) {
        message("x86 build")
    } else {
        message("x86_64 build")
    }
}

