QMAKEVERSION = $$[QMAKE_VERSION]
ISQT5 = $$find(QMAKEVERSION, ^[2-9])
isEmpty( ISQT5 ) {
error("Use the qmake include with Qt4.4 or greater, on Debian that is qmake-qt4");
}

TEMPLATE = subdirs
SUBDIRS  = src

