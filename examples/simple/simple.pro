include(../../QtOpenCV.pri)
include(../shared/shared.pri)
add_opencv_modules(core imgproc)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = simple
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp
