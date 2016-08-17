include(../../opencv.pri)
include(../shared/shared.pri)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

SOURCES += main.cpp\
        dialog.cpp\
        cameradevice.cpp

HEADERS  += dialog.h \
            cameradevice.h

FORMS    += dialog.ui
