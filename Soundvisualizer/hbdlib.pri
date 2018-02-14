PATH_TO_HBDLIB_SOURCES = $${PWD}/../HBDlib

defineReplace(qtLibraryName) {
   unset(LIBRARY_NAME)
   LIBRARY_NAME = $$1
   CONFIG(debug, debug|release): RET = $$member(LIBRARY_NAME, 0)d
   isEmpty(RET):RET = $$LIBRARY_NAME
   return($$RET)
}

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
    PATH_TO_HBDLIB_BINARIES = $${PATH_TO_HBDLIB_SOURCES}/bin/$${ARCHITECTURE}/$${COMPILER}
}
linux {
    DEFINES += TARGET_OS_LINUX
    PATH_TO_HBDLIB_BINARIES = $${PATH_TO_HBDLIB_SOURCES}/bin
}

message($${PATH_TO_HBDLIB_BINARIES})

LIBS += -L$${PATH_TO_HBDLIB_BINARIES} \
        -l$$qtLibraryName(hbdlib)

INCLUDEPATH += $${PATH_TO_HBDLIB_SOURCES}
