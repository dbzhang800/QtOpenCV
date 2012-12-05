## About QtOpenCV

 * QtOpenCV provides some helper functions to converting cv::Mat from/to QImage.

 * QtOpenCV provides a opencv.pri file which can be used to integrate OpenCV2 to qmake-based project

Note: OpenCV2.2 or newer is needed.

## cv::Mat <==> QImage

 * Copy cvmatandqimage{.cpp .h} to your project's source tree.
 * Then take advantage of follow api functions.

```
namespace QtOcv {

//Standard convert
cv::Mat image2Mat(const QImage &img, int channels = 0, RgbOrder rgbOrder = BGR);
QImage mat2Image(const cv::Mat &mat, QImage::Format format = QImage::Format_RGB32, RgbOrder rgbOrder = BGR);

//Convert without data copy. note that, RgbOrder of cv::Mat must be R G B (3 channels) or B G R A(4 channels)
cv::Mat image2Mat_shared(const QImage &img);
QImage mat2Image_shared(const cv::Mat &mat);

} //namespace QtOcv
```

### Some thing you need to know

#### Channels order of OpenCV is `B G R`

The manual of OpenCV says that,

cv::imwrite()

Only 8-bit (or16-bit unsigned(CV_16U) in case of PNG,JPEG
2000,and TIFF) single-channel or 3-channel(with‘BGR’channelorder)images can be saved using this function.

cv::imread()

In the case of color images, the decoded images will have the channels stored in  B G R order.

#### Data bytes order of QImage is `B G R A` and `R G B`

 * For Little Endian System

```
    QImage::Format_RGB32  ==> B G R 255
    QImage::Format_ARGB32 ==> B G R A
    QImage::Format_RGB888 ==> R G B
```

#### Swap channels in OpenCV 

```
    cv::cvtColor(mat, mat, CV_BGR2RGB)
    cv::cvtColor(mat, mat, CV_BGRA2RGBA)
    ...
```

### Swap r and b channel of QImage

```
    QImage QImage::rgbSwapped();
```

## OpenCV2 Integration

```
    add_opencv_modules(modules [, version [, includepaths [, librarypaths]]])
```

### OpenCV installed in standard location

If OpenCV has been installed in the standard location all we need is

```
    include (opencv.pri)
    add_opencv_modules(core imgproc highgui)
```

[**For windows user**] If header files have been put in `%QTDIR%/include/opencv2/` and libraries have been copied to `%QTDIR%/lib`, this can be thought standard too.

### OpenCV install in non-standard location

If OpenCV2 doesn't installed in the standard directory, header files paths and library paths should be provided. There are more that four ways to do so.

 * set system environment variables to tell qmake or QtCreator.

```
   set OPENCV_VERSION=2.4.3
   set OPENCV_INCPATH=D:/opencv/build/include
   set OPENCV_LIBPATH=D:/opencv/build/x86/vc10/lib
   qmake
```

 * set qmake's persistent property to tell qmake or QtCreator

```
   qmake -set OPENCV_VERSION 2.4.3
   qmake -set OPENCV_INCPATH D:/opencv/build/include
   qmake -set OPENCV_LIBPATH D:/opencv/build/x86/vc10/lib
```

 * using the third and forth param of add_opencv_modules()

```
    add_opencv_modules(core imgproc highgui, 2.4.3, D:/opencv/build/include, D:/opencv/build/x86/vc10/lib)
```

[**Note that**, more than one paths can be provided, so you can set paths for linux/windows at the same time if you like]

 * set project variable before call add_opencv_modules

```
    OPENCV_VERSION = 2.4.3
    OPENCV_INCPATH += D:/opencv/build/include
    OPENCV_INCPATH += /home/debao/opencv/include
    OPENCV_LIBPATH = D:/opencv/build/x86/vc10/lib  /home/debao/opencv/lib
    add_opencv_modules(core imgproc highgui)
```

### Why we need to provided the version information?

[**Windows only**]

As the library name schame of OpenCV under windows is `libopencv_XXXX243.dll.a` or `opencv_XXXX243{d}.lib`, so the version information must be provided under Windows. Default is 2.4.3

```
    add_opencv_modules(core imgproc highgui, 2.4.3)
```

or

```
    OPENCV_VERSION = 2.4.3
    add_opencv_modules(core imgproc highgui)
```

or set environment variable or qmake's variable if you don't want to touch the project files.