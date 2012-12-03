
#Usage:
#
#   add_opencv_modules(core imgproc highgui)
#
#if the opencv library doesn't exists in standard location, you can special them
#   add_opencv_modules(core imgproc highgui, 243, D:/opencv/build/include, D:/opencv/build/x86/vc10/lib)
#
#or define three variables before calling this function
#   OPENCV_VERSION = 243
#   OPENCV_INCPATH = D:/opencv/build/include  /home/debao/opencv/include
#   OPENCV_LIBPATH = D:/opencv/build/x86/vc10/lib  /home/debao/opencv/lib
#   add_opencv_modules(core imgproc highgui)
#
#or set environment variables when call qmake.
#   set OPENCV_VERSION=243
#   set OPENCV_INCPATH=D:/opencv/build/include
#   set OPENCV_LIBPATH=D:/opencv/build/x86/vc10/lib
#   qmake
#   nmake
defineTest(add_opencv_modules) {
    isEmpty(2) {
        isEmpty(OPENCV_VERSION) {
            !isEmpty($$(OPENCV_VERSION)):OPENCV_VERSION = $$(OPENCV_VERSION)
            else:OPENCV_VERSION = 243
        }
    } else {
        OPENCV_VERSION = $$2
    }
    OPENCV_VERSION ~= s,\.,,

    for (part, 1) {
        name = opencv_$$part
        win32 {
            name = $${name}$${OPENCV_VERSION}
            *-msvc*:CONFIG(debug, debug|release):name = $${name}d
            *-g++*:name = $${name}.dll
        }
        LIBS *= -l$$name
    }

    isEmpty(3) {
        isEmpty(OPENCV_INCPATH):OPENCV_INCPATH = $$(OPENCV_INCPATH)
    } else {
        OPENCV_INCPATH = $$3
    }
    isEmpty(4) {
        isEmpty(OPENCV_LIBPATH):OPENCV_LIBPATH = $$(OPENCV_LIBPATH)
    } else {
        OPENCV_LIBPATH = $$4
    }

    !isEmpty(OPENCV_INCPATH):INCLUDEPATH *= $$OPENCV_INCPATH
    !isEmpty(OPENCV_LIBPATH):LIBS *= -L$$OPENCV_LIBPATH

    export(INCLUDEPATH)
    export(LIBS)
}

