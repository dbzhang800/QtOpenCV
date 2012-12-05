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
#if the opencv library doesn't exists in standard location,
#you can special them by environment variables
#   set OPENCV_VERSION=2.4.3
#   set OPENCV_INCPATH=D:/opencv/build/include
#   set OPENCV_LIBPATH=D:/opencv/build/x86/vc10/lib
#
#or by qmake's persistent property
#   qmake -set OPENCV_VERSION 2.4.3
#   qmake -set OPENCV_INCPATH D:/opencv/build/include
#   qmake -set OPENCV_LIBPATH D:/opencv/build/x86/vc10/lib
#
#or taking advantage of the third and forth params of add_opencv_modules()
#   add_opencv_modules(core imgproc highgui, 2.4.3, D:/opencv/build/include, D:/opencv/build/x86/vc10/lib)
#
#or defining project variables before calling add_opencv_modules()
#   OPENCV_VERSION = 2.4.3
#   OPENCV_INCPATH = D:/opencv/build/include  /home/debao/opencv/include
#   OPENCV_LIBPATH = D:/opencv/build/x86/vc10/lib  /home/debao/opencv/lib
#   add_opencv_modules(core imgproc highgui)

defineTest(add_opencv_modules) {
    isEmpty(2) {
        isEmpty(OPENCV_VERSION):OPENCV_VERSION = $$(OPENCV_VERSION)
        isEmpty(OPENCV_VERSION):OPENCV_VERSION = $$[OPENCV_VERSION]
        isEmpty(OPENCV_VERSION):OPENCV_VERSION = 243
    } else {
        OPENCV_VERSION = $$2
    }
    OPENCV_VERSION ~= s,\\.,,

    for(part, 1) {
        name = opencv_$$part
        win32 {
            name = $${name}$${OPENCV_VERSION}
            *-msvc*:CONFIG(debug, debug|release):name = $${name}d
            *-g++*:name = $${name}.dll
        }
        LIBS *= -l$$name
    }

    win32:SEP = ;
    else:SEP = :
    isEmpty(3) {
        isEmpty(OPENCV_INCPATH):OPENCV_INCPATH = $$(OPENCV_INCPATH)
        OPENCV_INCPATH = $$split(OPENCV_INCPATH, $$SEP)
        isEmpty(OPENCV_INCPATH):OPENCV_INCPATH = $$[OPENCV_INCPATH]
    } else {
        OPENCV_INCPATH = $$3
    }
    isEmpty(4) {
        isEmpty(OPENCV_LIBPATH):OPENCV_LIBPATH = $$(OPENCV_LIBPATH)
        OPENCV_LIBPATH = $$split(OPENCV_LIBPATH, $$SEP)
        isEmpty(OPENCV_LIBPATH):OPENCV_LIBPATH = $$[OPENCV_LIBPATH]
    } else {
        OPENCV_LIBPATH = $$4
    }

    !isEmpty(OPENCV_INCPATH):INCLUDEPATH *= $$OPENCV_INCPATH
    !isEmpty(OPENCV_LIBPATH):LIBS *= -L$$OPENCV_LIBPATH

    export(INCLUDEPATH)
    export(LIBS)
}

