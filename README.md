## About QtOpenCV

 * QtOpenCV provides some helper functions to converting cv::Mat from/to QImage.

 * QtOpenCV provides a opencv.pri file which can be used to integrate OpenCV2 to qmake-based project

## cv::Mat <==> QImage

 * Copy cvmatandqimage{.cpp .h} to project's source tree.

## OpenCV2 Integration

 * If OpenCV has been installed in the standard location, all we need is

```
include (opencv.pri)
add_opencv_modules(core imgproc highgui)
```

 * As the library name of OpenCV under windows in format libopencv_XXXX243.dll.a or opencv_XXXX243{d}.lib, so version information should be provided

```
add_opencv_modules(core imgproc highgui, 2.4.3)
```

or

```
OPENCV_VERSION = 2.4.3
add_opencv_modules(core imgproc highgui)
```

 * If OpenCV2 doesn't installed in the standard directory.

```
OPENCV_VERSION = 243
OPENCV_INCPATH += D:/opencv/build/include
OPENCV_INCPATH += /home/debao/opencv/include
OPENCV_LIBPATH = D:/opencv/build/x86/vc10/lib  /home/debao/opencv/lib
add_opencv_modules(core imgproc highgui)
```

or set environment variables when call qmake.

```
   set OPENCV_VERSION=243
   set OPENCV_INCPATH=D:/opencv/build/include
   set OPENCV_LIBPATH=D:/opencv/build/x86/vc10/lib
   qmake
   make
```