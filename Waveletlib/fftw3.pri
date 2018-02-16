win32 {

    PATH_TO_FFTW3 = $${PWD}/../../3rdParties/fftw3

    INCLUDEPATH += $${PATH_TO_FFTW3}

    win32:contains(QMAKE_TARGET.arch, x86_64) {
        LIBS += -L$${PATH_TO_FFTW3}/fftw3-64
    } else {
        LIBS += -L$${PATH_TO_FFTW3}/fftw3-32
    }

    LIBS += -llibfftw3f-3
}

