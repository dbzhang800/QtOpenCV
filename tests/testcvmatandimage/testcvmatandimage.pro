
include (../../QtOpenCV.pri)

QT       += testlib

TARGET = tst_cvmatandimagetest

add_opencv_modules(core imgproc highgui)

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_cvmatandimagetest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
