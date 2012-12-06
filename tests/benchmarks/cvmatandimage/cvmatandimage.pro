include (../../../opencv.pri)
INCLUDEPATH += ../../..
QT       += testlib

add_opencv_modules(core imgproc)

TARGET = tst_cvmatandimagetest_benchmark
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_cvmatandimagetest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
