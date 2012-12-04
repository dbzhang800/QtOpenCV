include (../../QtOpenCV.pri)

QT       += testlib

TARGET = tst_testcvmatandimagetest

add_opencv_modules(core imgproc highgui)

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_testcvmatandimagetest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
