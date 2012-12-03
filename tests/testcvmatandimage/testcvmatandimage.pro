include (../../QtOpenCV.pri)

QT       += testlib

TARGET = tst_testcvmatandimagetest
DESTDIR = $$PROJECT_APP_PATH

add_opencv_modules(core imgproc highgui)

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += tst_testcvmatandimagetest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
