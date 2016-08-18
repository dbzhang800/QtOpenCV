## About QtOpenCV

 * QtOpenCV provides some helper functions to converting cv::Mat from/to QImage.

 * QtOpenCV provides a opencv.pri file which can be used to integrate OpenCV2 or newer to qmake-based project.
 
## cv::Mat <==> QImage

 * Download and copy the `cvmatandqimage.cpp` `cvmatandqimage.h` and `opencv.pri` to your project's source tree.

 * Then take advantage of the following API to converting data between Cv::Mat and QImage.

```cpp
    namespace QtOcv {
        /* Convert QImage to/from cv::Mat
         *
         * - cv::Mat
         *   - Supported channels
         *     - 1 channel
         *     - 3 channels (B G R), (R G B)
         *     - 4 channels (B G R A), (R G B A), (A R G B)
         *   - Supported depth
         *     - CV_8U  [0, 255]
         *     - CV_16U [0, 65535]
         *     - CV_32F [0, 1.0]
         *
         * - QImage
         *   - All of the formats of QImage are supported.
         */
        cv::Mat image2Mat(const QImage &img, int matType = CV_8UC(0), MatColorOrder order=MCO_BGR);
        QImage mat2Image(const cv::Mat &mat, MatColorOrder order=MCO_BGR, QImage::Format formatHint = QImage::Format_Invalid);
    }
```

 * In addition, two other functions are provided which works more efficient when operating on `CV_8UC1`, `CV_8UC3(R G B)`
   `CV_8UC4(R G B A)`, `CV_8UC4(B G R A)` or `CV_8UC4(A R G B)`. 

```cpp
    namespace QtOcv {
        /* Convert QImage to/from cv::Mat without data copy
         *
         * - Supported QImage formats and cv::Mat types are:
         *   - QImage::Format_Indexed8               <==> CV_8UC1
         *   - QImage::Format_Alpha8                 <==> CV_8UC1
         *   - QImage::Format_Grayscale8             <==> CV_8UC1
         *   - QImage::Format_RGB888                 <==> CV_8UC3 (R G B)
         *   - QImage::Format_RGB32                  <==> CV_8UC4 (A R G B or B G R A)
         *   - QImage::Format_ARGB32                 <==> CV_8UC4 (A R G B or B G R A)
         *   - QImage::Format_ARGB32_Premultiplied   <==> CV_8UC4 (A R G B or B G R A)
         *   - QImage::Format_RGBX8888               <==> CV_8UC4 (R G B A)
         *   - QImage::Format_RGBA8888               <==> CV_8UC4 (R G B A)
         *   - QImage::Format_RGBA8888_Premultiplied <==> CV_8UC4 (R G B A)
         *
         * - For QImage::Format_RGB32 and QImage::Format_ARGB32, the
         *   color channel order of cv::Mat will be (B G R A) in little
         *   endian system or (A R G B) in big endian system.
         *
         * - User must make sure that the color channels order is the same as
         *   the color channels order required by QImage.
         */
        cv::Mat image2Mat_shared(const QImage &img, MatColorOrder *order=0);
        QImage mat2Image_shared(const cv::Mat &mat, QImage::Format formatHint = QImage::Format_Invalid);
    } //namespace QtOcv
```

## OpenCV2 Integration

If your want to use OpenCV in your qmake based project, you can download and put the source files to any directory you wanted,
then add following code to your .pro file.

    include (yourpath/yourpath/yourpath/opencv.pri)

or you can simply add following line to your .pro file:

    unix{
        CONFIG += link_pkgconfig
        PKGCONFIG += opencv
    }
    win32{
        # load(opencv) instead of CONFIG+=opencv used here
        !load(opencv):message("You must create an opencv.prf, and move it to $$[QT_INSTALL_PREFIX]/mkspecs/features/")
    }

As you can see, nothing else needed to do for non-windows users.

### Notes for Windows User

To make opencv.pri works for your, you need to create an `opencv.prf` file, then move the .prf file to `%QTDIR%/mkspecs/features/`. 

#### Method 1: Create and copy the opencv.prf by hand

You can use a textedit to create the .prf file. The contents of .prf file more or less looks like this:

    INCLUDEPATH += D:/opencv/opencv_build/include 
    LIBS += -LD:/opencv/opencv_build/x64/vc14/lib 
    CONFIG(debug, debug|release) {  
        LIBS += -lopencv_aruco310d 
        LIBS += -lopencv_bgsegm310d 
        LIBS += -lopencv_bioinspired310d 
        LIBS += -lopencv_xxxxxx
    } else {
        LIBS += -lopencv_aruco310 
        LIBS += -lopencv_bgsegm310 
        LIBS += -lopencv_bioinspired310 
        LIBS += -lopencv_xxxxxx
    }

Then you can copy it to `%QTDIR%/mkspecs/features/`.

#### Method 2: Take use of the helper script

If you have installed python, the helper script `opencv_prf_generator.py` can be used to generate and install the opencv.prf file.

    python opencv_prf_generator.py -q D:\Qt\QtOnline\5.6\msvc2015_x64\bin\qmake.exe D:\opencv\opencv_build\x64\vc14\lib

## Some thing you need to know

#### Channels order of OpenCV's image which used by highgui module is `B G R` and `B G R A`

The manual of OpenCV says that,

* cv::imwrite()

Only 8-bit (or 16-bit unsigned(`CV_16U`) in case of PNG,JPEG
2000,and TIFF) single-channel or **3-channel(with 'BGR' channel order)** images can be saved using this function.

It is possible to store PNG images with an alpha channel using this function. To do this, create 8-bit (or 16-bit) **4-chanel image BGRA**, where the alpha channel goes last.

* cv::imread()

In the case of color images, the decoded images will have the **channels stored in  B G R order** .

**Note:** If you don't care `opencv_highgui` module, you can always use the same channels order as QImage, which will be slightly fast.

#### Data bytes order of QImage

 * In Little Endian System

```
    QImage::Format_RGB32  ==> B G R 255
    QImage::Format_ARGB32 ==> B G R A

    QImage::Format_RGB888 ==> R G B
    QImage::Format_RGBX8888 ==> R G B 255
    QImage::Format_RGBA8888 ==> R G B A
```

 * Ins Big Endian System

```
    QImage::Format_RGB32  ==> 255 R G B
    QImage::Format_ARGB32 ==> A R G B

    QImage::Format_RGB888 ==> R G B
    QImage::Format_RGBX8888 ==> R G B 255
    QImage::Format_RGBA8888 ==> R G B A
```

#### How to swap channels?

 * In OpenCV 

```
    cv::cvtColor(mat, mat, CV_BGR2RGB)
    cv::cvtColor(mat, mat, CV_BGRA2RGBA)
    ...
```

 * In Qt

Swap r and b channel of QImage

```
    QImage QImage::rgbSwapped();
```

If the depth of the image is 32, the following function can be used too.

```
    void QImage::invertPixels(InvertMode mode = InvertRgb)
```

#### Common Image Data Range of OpenCV

```
    CV_8U   [0, 255]
    CV_16U  [0, 255*256]
    CV_32F  [0.0, 1.0]
```

