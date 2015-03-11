
include(../../QtOpenCV.pri)
include(../shared/shared.pri)
add_opencv_modules(core imgproc)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = imageprocess
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    recentfiles.cpp \
    convert.cpp

HEADERS  += mainwindow.h \
    recentfiles.h \
    convert.h

FORMS    += mainwindow.ui
