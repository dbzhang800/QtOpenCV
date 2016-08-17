###########################################################################
# Copyright (c) 2016 Debao Zhang <hello@debao.me>
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

## Note for Windows user:
##   you need to create an opencv.prf file,
##   then move the .prf file to %QTDIR%/mkspecs/features/
##   see README.md for more information.

unix{
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv
}
win32{
    # load(opencv) instead of CONFIG+=opencv used here
    !load(opencv):message("You must create an opencv.prf, and move it to $$[QT_INSTALL_PREFIX]/mkspecs/features/")
}

# silence msvc warning 4819
win32-msvc*:QMAKE_CXXFLAGS += -wd4819

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/cvmatandqimage.h

SOURCES += \
    $$PWD/cvmatandqimage.cpp 
