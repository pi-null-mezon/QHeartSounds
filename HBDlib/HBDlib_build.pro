#-------------------------------------------------
#
# Project created by QtCreator 2018-02-14T16:27:07
#
#-------------------------------------------------
CONFIG -= qt
CONFIG += c++11

CONFIG(release, debug|release) {
    TARGET = hbdlib
} else {
    TARGET = hbdlibd
}

TEMPLATE = lib

DEFINES += HBDLIB_LIBRARY

SOURCES += \
        hbdlib.cpp

HEADERS += \
        hbdlib.h

#-------------------------- Where binaries should resides after build
win32-msvc2010: COMPILER = vc10
win32-msvc2012: COMPILER = vc11
win32-msvc2013: COMPILER = vc12
win32-msvc2015: COMPILER = vc14
win32-g++:      COMPILER = mingw
win32{
    contains(QMAKE_TARGET.arch, x86_64){
        ARCHITECTURE = x64
    } else {
        ARCHITECTURE = x86
    }
    DESTDIR = $${PWD}/bin/$${ARCHITECTURE}/$${COMPILER}
}
linux {
    DEFINES += TARGET_OS_LINUX
    DESTDIR = $${PWD}/bin
}



