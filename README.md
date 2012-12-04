## About QtOpenCV

 * QtOpenCV provides some helper functions to converting cv::Mat from/to QImage.

 * QtOpenCV provides a opencv.pri file which can be used to integrate OpenCV2 to qmake-based project

## cv::Mat <==> QImage

 * Copy cvmatandqimage{.cpp .h} to project's source tree.

```
namespace QtOcv {

cv::Mat image2Mat(const QImage &img, int channels = 0);
QImage mat2Image(const cv::Mat &mat, QImage::Format format = QImage::Format_RGB32);

//Convert without data copy
cv::Mat image2Mat_shared(const QImage &img);
QImage mat2Image_shared(const cv::Mat &mat);

} //namespace QtOcv
```

## OpenCV2 Integration

```
add_opencv_modules(modules [, version [, includepaths [, librarypaths]]])
```

 * If OpenCV has been installed in the standard location, all we need is

```
include (opencv.pri)
add_opencv_modules(core imgproc highgui)
```

 * As the library name schame of OpenCV under windows is libopencv_XXXX243.dll.a or opencv_XXXX243{d}.lib, so version information must be provided. Default is 2.4.3

```
add_opencv_modules(core imgproc highgui, 2.4.3)
```

or

```
OPENCV_VERSION = 2.4.3
add_opencv_modules(core imgproc highgui)
```

 * If OpenCV2 doesn't installed in the standard directory, header files paths and library paths should be provided.[Note that, more than one paths can be provided, so you can set paths for linux/windows at the same time if you like]

```
add_opencv_modules(core imgproc highgui, 243, D:/opencv/build/include, D:/opencv/build/x86/vc10/lib)
```

or

```
OPENCV_VERSION = 2.4.3
OPENCV_INCPATH += D:/opencv/build/include
OPENCV_INCPATH += /home/debao/opencv/include
OPENCV_LIBPATH = D:/opencv/build/x86/vc10/lib  /home/debao/opencv/lib
add_opencv_modules(core imgproc highgui)
```

or set environment variables when call qmake.

```
   set OPENCV_VERSION=2.4.3
   set OPENCV_INCPATH=D:/opencv/build/include
   set OPENCV_LIBPATH=D:/opencv/build/x86/vc10/lib
   qmake
   make
```