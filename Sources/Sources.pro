#-------------------------------------------------
#
# Project created by QtCreator 2014-04-19T22:19:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += multimedia

TARGET = HeartSounds
VERSION = 1.0.0.2
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        qplot.cpp \
    qsoundprocessor.cpp \
    qharmonicprocessor.cpp


HEADERS  += mainwindow.h \
    qplot.h \
    qsoundprocessor.h \
    qharmonicprocessor.h

RC_ICONS = $$PWD/../Resources/images/QHeartSounds.ico

win32-g++: DEFINES += MINGW_COMPILER

#------------MANUAL_CHOOSE_THE_PLATFORM_TO_COMPILE----------------------
#This option is managed automatically when you chose the Kit for compilation, manual settings are listed below
#win32-msvc*:QMAKE_TARGET.arch = x86_64
#win32-msvc*:QMAKE_TARGET.arch = $$QMAKE_HOST.arch

#-----------------------------FFTW--------------------------------------
#for MSVC compiller
win32-msvc*{
        win32-msvc*:contains(QMAKE_TARGET.arch, x86_64):{
            message( "Building for 64 bit, mscv compiller" )
            LIBS += -L$$PWD/../Resources/fftw3-64/ -llibfftw3-3
        } else {
            message( "Building for 32 bit, mscv compiller" )
            LIBS += -L$$PWD/../Resources/fftw3-32/ -llibfftw3-3
        }
      }
#for MinGW compiller
win32-g++ {
        win32-g++:contains(QMAKE_HOST.arch, x86_64):{
            message( "Building for 64 bit, MinGW compiller" )
            LIBS += -L$$PWD/../Resources/fftw3-64/ -llibfftw3-3
        } else {
            message( "Building for 32 bit, MinGW compiller" )
            LIBS += -L$$PWD/../Resources/fftw3-32/ -llibfftw3-3
        }
      }

INCLUDEPATH += $$PWD/../Resources
DEPENDPATH += $$PWD/../Resources
#----------------------------------------------------------------------

