include (../../../opencv.pri)
INCLUDEPATH += ../../..
QT       += testlib

add_opencv_modules(core imgproc)

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_floatmatandqimagetest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
