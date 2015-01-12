include(../../QtOpenCV.pri)
add_opencv_modules(core imgproc)

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cannyedgedetector
TEMPLATE = app

SOURCES += main.cpp\
        dialog.cpp

HEADERS  += dialog.h

FORMS    += dialog.ui
