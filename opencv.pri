###########################################################################
# Copyright (c) 2012 Debao Zhang <hello@debao.me>
# All right reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
###########################################################################
#
#Usage:
#
#   add_opencv_modules(core imgproc highgui)
#
#if the opencv library doesn't exists in standard location, you can special them
#   add_opencv_modules(core imgproc highgui, 2.4.3, D:/opencv/build/include, D:/opencv/build/x86/vc10/lib)
#
#or define three variables before calling this function
#   OPENCV_VERSION = 2.4.3
#   OPENCV_INCPATH = D:/opencv/build/include  /home/debao/opencv/include
#   OPENCV_LIBPATH = D:/opencv/build/x86/vc10/lib  /home/debao/opencv/lib
#   add_opencv_modules(core imgproc highgui)
#
#or set environment variables before call qmake or QtCreator.
#   set OPENCV_VERSION=2.4.3
#   set OPENCV_INCPATH=D:/opencv/build/include
#   set OPENCV_LIBPATH=D:/opencv/build/x86/vc10/lib
#   qmake
#   nmake
#
#or set qmake variables
#   qmake -set OPENCV_INCPATH D:/opencv/build/include
#   qmake -set OPENCV_LIBPATH D:/opencv/build/x86/vc10/lib
defineTest(add_opencv_modules) {
    isEmpty(2) {
        isEmpty(OPENCV_VERSION):OPENCV_VERSION = $$(OPENCV_VERSION)
        isEmpty(OPENCV_VERSION):OPENCV_VERSION = $$[OPENCV_VERSION]
        isEmpty(OPENCV_VERSION):OPENCV_VERSION = 243
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
        isEmpty(OPENCV_INCPATH):OPENCV_INCPATH = $$[OPENCV_INCPATH]
    } else {
        OPENCV_INCPATH = $$3
    }
    isEmpty(4) {
        isEmpty(OPENCV_LIBPATH):OPENCV_LIBPATH = $$(OPENCV_LIBPATH)
        isEmpty(OPENCV_LIBPATH):OPENCV_LIBPATH = $$[OPENCV_LIBPATH]
    } else {
        OPENCV_LIBPATH = $$4
    }

    !isEmpty(OPENCV_INCPATH):INCLUDEPATH *= $$OPENCV_INCPATH
    !isEmpty(OPENCV_LIBPATH):LIBS *= -L$$OPENCV_LIBPATH

    export(INCLUDEPATH)
    export(LIBS)
}

