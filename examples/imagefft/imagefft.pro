include(../../QtOpenCV.pri)
add_opencv_modules(core imgproc)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = imagefft
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp

HEADERS  += dialog.h

FORMS    += dialog.ui
