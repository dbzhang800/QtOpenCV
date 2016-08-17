include(../../opencv.pri)

QT       += testlib

TARGET = tst_cvmatandimagetest

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_cvmatandimagetest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
