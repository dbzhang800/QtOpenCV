## About QtOpenCV

 * QtOpenCV provides some helper functions to converting cv::Mat from/to QImage.

 * QtOpenCV provides a opencv.pri file which can be used to integrate OpenCV2 to qmake-based project

 **Note: OpenCV2.2 or newer is needed.**

## cv::Mat <==> QImage

 * Copy cvmatandqimage{.cpp .h} to your project's source tree.
 * Then take advantage of follow api functions to converting data between Cv::Mat ```C1 C3(RGB) C3(BGR) C4(RGBA) C4(BGRA)``` and QImage ```  Indexed8 RGB888 RGB32  ARGB32```

```
    namespace QtOcv {
        cv::Mat image2Mat(const QImage &img, int channels = 0, MatChannelOrder rgbOrder = MCO_BGR);
        QImage mat2Image(const cv::Mat &mat, QImage::Format format = QImage::Format_Invalid, MatChannelOrder rgbOrder = MCO_BGR);
    } //namespace QtOcv
```

 * In addition, two other functions are provided which works more efficient when operating on `CV_8UC1`、`CV_8UC3`(R G B)、`CV_8UC4`(A R G B or B G R A depending on system endian)

```
    namespace QtOcv {
    //Convert without data copy. note that, RgbOrder of cv::Mat must be R G B (3 channels) or B G R A(4 channels)
    cv::Mat image2Mat_shared(const QImage &img);
    QImage mat2Image_shared(const cv::Mat &mat);
    } //namespace QtOcv
```

### Some thing you need to know

#### Channels order of OpenCV's image which used by highgui module is `B G R` and `B G R A`

The manual of OpenCV says that,

* cv::imwrite()

Only 8-bit (or 16-bit unsigned(CV_16U) in case of PNG,JPEG
2000,and TIFF) single-channel or **3-channel(with‘BGR’channel order)** images can be saved using this function.

It is possible to store PNG images with an alpha channel using this function. To do this, create 8-bit (or 16-bit) **4-chanel image BGRA**, where the alpha channel goes last.

* cv::imread()

In the case of color images, the decoded images will have the **channels stored in  B G R order** .

**Note:** If you don't care opencv_highgui module, you can always use the same channels order as QImage.

#### Data bytes order of QImage is `B G R A`(little endian) `A R G B`(big endian) and `R G B`

 * In Little Endian System

```
    QImage::Format_RGB32  ==> B G R 255
    QImage::Format_ARGB32 ==> B G R A
    QImage::Format_RGB888 ==> R G B
```

 * Ins Big Endian System

```
    QImage::Format_RGB32  ==> 255 R G B
    QImage::Format_ARGB32 ==> A R G B
    QImage::Format_RGB888 ==> R G B
```

#### How to swap channels?

 * In OpenCV 

```
    cv::cvtColor(mat, mat, CV_BGR2RGB)
    cv::cvtColor(mat, mat, CV_BGRA2RGBA)
    ...
```

 * In Qt, swap r and b channel of QImage

```
    QImage QImage::rgbSwapped();
```

#### Common Image Data Range of OpenCV

```
    CV_8U   [0, 255]
    CV_16U  [0, 255*256]
    CV_32S  [0, 255*256]
    CV_32F  [0.0, 1.0]
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

[ **For windows user** ] If header files have been put in `%QTDIR%/include/opencv2/` and libraries have been copied to `%QTDIR%/lib`, this can be thought standard too.

### OpenCV install in non-standard location

If OpenCV2 doesn't installed in the standard directory, header files paths and library paths should be provided. There are more that four ways to do so.

 * set system environment variables

```
   set OPENCV_VERSION=2.4.3
   set OPENCV_INCPATH=D:/opencv/build/include
   set OPENCV_LIBPATH=D:/opencv/build/x86/vc10/lib
   qmake
```

 * set qmake's persistent property
```
   qmake -set OPENCV_VERSION 2.4.3
   qmake -set OPENCV_INCPATH D:/opencv/build/include
   qmake -set OPENCV_LIBPATH D:/opencv/build/x86/vc10/lib
```

 * using the third and forth param of add_opencv_modules()

```
    add_opencv_modules(core imgproc highgui, 2.4.3, D:/opencv/build/include, D:/opencv/build/x86/vc10/lib)
```

[ **Note that** , more than one paths can be provided, so you can set paths for linux/windows at the same time if you like]

 * set project variable before call add_opencv_modules

```
    OPENCV_VERSION = 2.4.3
    OPENCV_INCPATH += D:/opencv/build/include
    OPENCV_INCPATH += /home/debao/opencv/include
    OPENCV_LIBPATH = D:/opencv/build/x86/vc10/lib  /home/debao/opencv/lib
    add_opencv_modules(core imgproc highgui)
```

### Why we need to provided the version information?

[ **Windows only** ]

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
