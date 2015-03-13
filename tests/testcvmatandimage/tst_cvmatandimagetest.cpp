#include "cvmatandqimage.h"

#include <QtTest>
#include <QTemporaryFile>
#include <QDebug>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <vector>
#include <math.h>

using namespace QtOcv;

Q_DECLARE_METATYPE(QImage::Format)
Q_DECLARE_METATYPE(MatColorOrder)
Q_DECLARE_METATYPE(cv::Mat)
Q_DECLARE_METATYPE(cv::Vec4b)

static bool lenientCompare(const QImage &actual, const QImage &expected)
{
    if (expected.format() != actual.format()) {
        qWarning("Image format comparison failed: expected: %d, got %d",
                 expected.format(),actual.format());
        return false;
    }

    QImage expectedImage = expected.convertToFormat(QImage::Format_ARGB32);
    QImage actualImage = actual.convertToFormat(QImage::Format_ARGB32);

    if (expectedImage.size() != actualImage.size()) {
        qWarning("Image size comparison failed: expected: %dx%d, got %dx%d",
                 expectedImage.size().width(), expectedImage.size().height(),
                 actualImage.size().width(), actualImage.size().height());
        return false;
    }

    const int size = actual.width() * actual.height();
    const int threshold = 2;

    QRgb *a = (QRgb *)actualImage.bits();
    QRgb *e = (QRgb *)expectedImage.bits();
    for (int i = 0; i < size; ++i) {
        const QColor ca(a[i]);
        const QColor ce(e[i]);
        if (qAbs(ca.red() - ce.red()) > threshold
                || qAbs(ca.green() - ce.green()) > threshold
                || qAbs(ca.blue() - ce.blue()) > threshold
                || qAbs(ca.blue() - ce.blue()) > threshold) {
            qWarning("Color mismatch at pixel #%d: Expected: %d,%d,%d,%d got %d,%d,%d,%d",
                     i, ce.red(), ce.green(), ce.blue(), ce.alpha(),
                     ca.red(), ca.green(), ca.blue(), ca.alpha());
            return false;
        }
    }

    return true;
}

template<typename T>
static bool lenientCompare(const cv::Mat &actual, const cv::Mat &expected)
{
    if (expected.type() != actual.type()) {
        qWarning("cv::Mat comparison failed: expected: depth %d channels %d, got depth %d channels %d",
                 expected.depth(), expected.channels(),
                 actual.depth(), actual.channels());
        return false;
    }

    if (expected.rows != actual.rows || expected.cols != actual.cols) {
        qWarning("cv::Mat comparison failed: expected: %dX%d, got %dX%d",
                 expected.cols, expected.rows,
                 actual.cols, actual.rows);
        return false;
    }

    double threshold = 2.0e-1;

    std::vector<cv::Mat> actualPlanes(actual.channels());
    std::vector<cv::Mat> expectedPlanes(expected.channels());

    cv::split(actual, actualPlanes);
    cv::split(expected, expectedPlanes);

    for (int c=0; c<actual.channels(); ++c) {
        for (int i=0; i<actual.rows; ++i) {
            for (int j=0; j<actual.cols; ++j) {
                T a = actualPlanes[c].at<T>(i, j);
                T e = expectedPlanes[c].at<T>(i, j);
                if (qAbs(a-e) > threshold*(qAbs(a)+qAbs(e))) {
                    qWarning()<<QString("Value mismatch at channel %1 (%2 %3):Expected: %4, got %5")
                                .arg(c).arg(i).arg(j)
                                .arg(e).arg(a);
                    return false;
                }
            }
        }
    }

    return true;
}

class CvMatAndImageTest : public QObject
{
    Q_OBJECT
    
public:
    CvMatAndImageTest();
    ~CvMatAndImageTest();
    
private Q_SLOTS:
    void testMatChannelsOrder(); //Just for ...

    void testMat2QImage_data();
    void testMat2QImage();
    void testMat2QImageShared_data();
    void testMat2QImageShared();

    void testQImage2Mat_data();
    void testQImage2Mat();
    void testQImage2MatShared_data();
    void testQImage2MatShared();

private:
    cv::Mat mat_8UC1;
    cv::Mat mat_16UC1;
    cv::Mat mat_32FC1;

    cv::Mat mat_8UC3_rgb;
    cv::Mat mat_16UC3_rgb;
    cv::Mat mat_32FC3_rgb;
    cv::Mat mat_8UC3_bgr;
    cv::Mat mat_16UC3_bgr;
    cv::Mat mat_32FC3_bgr;

    cv::Mat mat_8UC4_rgba;
    cv::Mat mat_16UC4_rgba;
    cv::Mat mat_32FC4_rgba;
    cv::Mat mat_8UC4_argb;
    cv::Mat mat_16UC4_argb;
    cv::Mat mat_32FC4_argb;
    cv::Mat mat_8UC4_bgra;
    cv::Mat mat_16UC4_bgra;
    cv::Mat mat_32FC4_bgra;

    cv::Mat mat_8UC4_rgbx;
    cv::Mat mat_16UC4_rgbx;
    cv::Mat mat_32FC4_rgbx;
    cv::Mat mat_8UC4_xrgb;
    cv::Mat mat_16UC4_xrgb;
    cv::Mat mat_32FC4_xrgb;
    cv::Mat mat_8UC4_bgrx;
    cv::Mat mat_16UC4_bgrx;
    cv::Mat mat_32FC4_bgrx;

    QImage image_indexed8;
#if QT_VERSION >= 0x050500
    QImage image_grayscale8;
#endif
    QImage image_rgb32;
    QImage image_argb32;
#if QT_VERSION >= 0x040400
    QImage image_rgb888;
#endif
#if QT_VERSION >= 0x050200
    QImage image_rgbx8888;
    QImage image_rgba8888;
#endif
};

CvMatAndImageTest::CvMatAndImageTest()
{
    const int width = 200;
    const int height = 100;

    mat_8UC1 = cv::Mat(height, width, CV_8UC1);
    mat_16UC1 = cv::Mat(height, width, CV_16UC1);
    mat_32FC1 = cv::Mat(height, width, CV_32FC1);

    mat_8UC3_rgb = cv::Mat(height, width, CV_8UC3);
    mat_16UC3_rgb = cv::Mat(height, width, CV_16UC3);
    mat_32FC3_rgb = cv::Mat(height, width, CV_32FC3);
    mat_8UC3_bgr = cv::Mat(height, width, CV_8UC3);
    mat_16UC3_bgr = cv::Mat(height, width, CV_16UC3);
    mat_32FC3_bgr = cv::Mat(height, width, CV_32FC3);

    mat_8UC4_rgba = cv::Mat(height, width, CV_8UC4);
    mat_16UC4_rgba = cv::Mat(height, width, CV_16UC4);
    mat_32FC4_rgba = cv::Mat(height, width, CV_32FC4);
    mat_8UC4_argb = cv::Mat(height, width, CV_8UC4);
    mat_16UC4_argb = cv::Mat(height, width, CV_16UC4);
    mat_32FC4_argb = cv::Mat(height, width, CV_32FC4);
    mat_8UC4_bgra = cv::Mat(height, width, CV_8UC4);
    mat_16UC4_bgra = cv::Mat(height, width, CV_16UC4);
    mat_32FC4_bgra = cv::Mat(height, width, CV_32FC4);

    mat_8UC4_rgbx = cv::Mat(height, width, CV_8UC4);
    mat_16UC4_rgbx = cv::Mat(height, width, CV_16UC4);
    mat_32FC4_rgbx = cv::Mat(height, width, CV_32FC4);
    mat_8UC4_xrgb = cv::Mat(height, width, CV_8UC4);
    mat_16UC4_xrgb = cv::Mat(height, width, CV_16UC4);
    mat_32FC4_xrgb = cv::Mat(height, width, CV_32FC4);
    mat_8UC4_bgrx = cv::Mat(height, width, CV_8UC4);
    mat_16UC4_bgrx = cv::Mat(height, width, CV_16UC4);
    mat_32FC4_bgrx = cv::Mat(height, width, CV_32FC4);

    image_indexed8 = QImage(width, height, QImage::Format_Indexed8);
    QVector<QRgb> colorTable;
    for (int i=0; i<256; ++i)
        colorTable.append(qRgb(i, i, i));
    image_indexed8.setColorTable(colorTable);
#if QT_VERSION >= 0x050500
    image_grayscale8 = QImage(width, height, QImage::Format_Grayscale8);
#endif
    image_rgb32 = QImage(width, height, QImage::Format_RGB32);
    image_argb32 = QImage(width, height, QImage::Format_ARGB32);
#if QT_VERSION >= 0x040400
    image_rgb888 = QImage(width, height, QImage::Format_RGB888);
#endif
#if QT_VERSION >= 0x050200
    image_rgbx8888 = QImage(width, height, QImage::Format_RGBX8888);
    image_rgba8888 = QImage(width, height, QImage::Format_RGBA8888);
#endif
    for (int row=0; row<height; ++row) {
        for (int col=0; col<width; ++col) {
            uchar r = (row*col)%255;
            uchar g = r/2;
            uchar b = g/2;
            uchar a = b/2;
            mat_8UC1.at<uchar>(row, col) = r;
            mat_16UC1.at<quint16>(row, col) = r*255;
            mat_32FC1.at<float>(row, col) = r/255.0;

            mat_8UC3_rgb.at<cv::Vec3b>(row, col) = cv::Vec3b(r, g, b);
            mat_16UC3_rgb.at<cv::Vec3w>(row, col) = cv::Vec3w(r*255, g*255, b*255);
            mat_32FC3_rgb.at<cv::Vec3f>(row, col) = cv::Vec3f(r/255.0, g/255.0, b/255.0);

            mat_8UC3_bgr.at<cv::Vec3b>(row, col) = cv::Vec3b(b, g, r);
            mat_16UC3_bgr.at<cv::Vec3w>(row, col) = cv::Vec3w(b*255, g*255, r*255);
            mat_32FC3_bgr.at<cv::Vec3f>(row, col) = cv::Vec3f(b/255.0, g/255.0, r/255.0);

            mat_8UC4_rgba.at<cv::Vec4b>(row, col) = cv::Vec4b(r, g, b, a);
            mat_8UC4_argb.at<cv::Vec4b>(row, col) = cv::Vec4b(a, r, g, b);
            mat_8UC4_bgra.at<cv::Vec4b>(row, col) = cv::Vec4b(b, g, r, a);
            mat_8UC4_rgbx.at<cv::Vec4b>(row, col) = cv::Vec4b(r, g, b, 255);
            mat_8UC4_xrgb.at<cv::Vec4b>(row, col) = cv::Vec4b(255, r, g, b);
            mat_8UC4_bgrx.at<cv::Vec4b>(row, col) = cv::Vec4b(b, g, r, 255);

            mat_16UC4_rgba.at<cv::Vec4w>(row, col) = cv::Vec4w(r*255, g*255, b*255, a*255);
            mat_16UC4_argb.at<cv::Vec4w>(row, col) = cv::Vec4w(a*255, r*255, g*255, b*255);
            mat_16UC4_bgra.at<cv::Vec4w>(row, col) = cv::Vec4w(b*255, g*255, r*255, a*255);
            mat_16UC4_rgbx.at<cv::Vec4w>(row, col) = cv::Vec4w(r*255, g*255, b*255, 65535);
            mat_16UC4_xrgb.at<cv::Vec4w>(row, col) = cv::Vec4w(65535, r*255, g*255, b*255);
            mat_16UC4_bgrx.at<cv::Vec4w>(row, col) = cv::Vec4w(b*255, g*255, r*255, 65535);

            mat_32FC4_rgba.at<cv::Vec4f>(row, col) = cv::Vec4f(r/255.0, g/255.0, b/255.0, a/255.0);
            mat_32FC4_argb.at<cv::Vec4f>(row, col) = cv::Vec4f(a/255.0, r/255.0, g/255.0, b/255.0);
            mat_32FC4_bgra.at<cv::Vec4f>(row, col) = cv::Vec4f(b/255.0, g/255.0, r/255.0, a/255.0);
            mat_32FC4_rgbx.at<cv::Vec4f>(row, col) = cv::Vec4f(r/255.0, g/255.0, b/255.0, 1);
            mat_32FC4_xrgb.at<cv::Vec4f>(row, col) = cv::Vec4f(1, r/255.0, g/255.0, b/255.0);
            mat_32FC4_bgrx.at<cv::Vec4f>(row, col) = cv::Vec4f(b/255.0, g/255.0, r/255.0, 1);

            image_indexed8.setPixel(col, row, r);
#if QT_VERSION >= 0x050500
            //Note, unlike indexed8, this should be rgb value.
            image_grayscale8.setPixel(col, row, qRgb(r,r,r));
#endif
            image_rgb32.setPixel(col, row, qRgb(r, g, b));
            image_argb32.setPixel(col, row, qRgba(r, g, b, a));
#if QT_VERSION >= 0x040400
            image_rgb888.setPixel(col, row, qRgb(r, g, b));
#endif
#if QT_VERSION >= 0x050200
            image_rgbx8888.setPixel(col, row, qRgb(r, g, b));
            image_rgba8888.setPixel(col, row, qRgba(r, g, b, a));
#endif
        }
    }
}

CvMatAndImageTest::~CvMatAndImageTest()
{
}

void CvMatAndImageTest::testMatChannelsOrder()
{
#if QT_VERSION >= 0x040400
    //Save a QImage as a .png file, then load with highgui's method
    //note that the order is (B G R) instead of (R G B)
    const char* fileName1 = "tst_data1.png";
    image_rgb888.save(fileName1);

    cv::Mat mat = cv::imread(fileName1);
    QVERIFY(lenientCompare<uchar>(mat, mat_8UC3_bgr));
#endif

    //generate a (B G R A) OpenCV Image, then save as a .png file
    const char* fileName2 = "tst_data2.png";
    cv::imwrite(fileName2, mat_8UC4_bgra);

    QImage image(fileName2);
    QVERIFY(lenientCompare(image, image_argb32));
}

void CvMatAndImageTest::testMat2QImage_data()
{
    QTest::addColumn<cv::Mat>("mat");
    QTest::addColumn<MatColorOrder>("mcOrder");
    QTest::addColumn<QImage::Format>("formatHint");
    QTest::addColumn<QImage>("expect");

    //Test data: C1 ==> Indexed8
    QTest::newRow("8UC1_Invalid") << mat_8UC1 << MCO_BGR << QImage::Format_Invalid << image_indexed8;
    QTest::newRow("8UC1_Indexed8") << mat_8UC1 << MCO_BGR << QImage::Format_Indexed8 << image_indexed8;
    QTest::newRow("16UC1") << mat_16UC1 << MCO_BGR << QImage::Format_Indexed8 << image_indexed8;
    QTest::newRow("32FC1") << mat_32FC1 << MCO_BGR << QImage::Format_Indexed8 << image_indexed8;

#if QT_VERSION >= 0x050500
    //Test data: C1 ==> Grayscale8
    QTest::newRow("8UC1_Grayscale8") << mat_8UC1 << MCO_BGR << QImage::Format_Grayscale8 << image_grayscale8;
#endif

#if QT_VERSION >= 0x040400
    //Test data: C3 ==> RGB8888
    QTest::newRow("8UC3(RGB)_Invalid") << mat_8UC3_rgb << MCO_RGB << QImage::Format_Invalid << image_rgb888;
    QTest::newRow("8UC3(RGB)_RGB888") << mat_8UC3_rgb << MCO_RGB << QImage::Format_RGB888 << image_rgb888;
    QTest::newRow("16UC3(RGB)_RGB888") << mat_16UC3_rgb << MCO_RGB << QImage::Format_RGB888 << image_rgb888;
    QTest::newRow("32FC3(RGB)_RGB888") << mat_32FC3_rgb << MCO_RGB << QImage::Format_RGB888 << image_rgb888;
    QTest::newRow("8UC3(BGR)_Invalid") << mat_8UC3_bgr << MCO_BGR << QImage::Format_Invalid << image_rgb888;
    QTest::newRow("8UC3(BGR)_RGB888") << mat_8UC3_bgr << MCO_BGR << QImage::Format_RGB888 << image_rgb888;
    QTest::newRow("16UC3(BGR)_RGB888") << mat_16UC3_bgr << MCO_BGR << QImage::Format_RGB888 << image_rgb888;
    QTest::newRow("32FC3(BGR)_RGB888") << mat_32FC3_bgr << MCO_BGR << QImage::Format_RGB888 << image_rgb888;
#endif
    //Test data: C3 ==> RGB32
    QTest::newRow("8UC3(RGB)_RGB32") << mat_8UC3_rgb << MCO_RGB << QImage::Format_RGB32 << image_rgb32;
    QTest::newRow("16UC3(RGB)_RGB32") << mat_16UC3_rgb << MCO_RGB << QImage::Format_RGB32 << image_rgb32;
    QTest::newRow("32FC3(RGB)_RGB32") << mat_32FC3_rgb << MCO_RGB << QImage::Format_RGB32 << image_rgb32;
    QTest::newRow("8UC3(BGR)_RGB32") << mat_8UC3_bgr << MCO_BGR << QImage::Format_RGB32 << image_rgb32;
    QTest::newRow("16UC3(BGR)_RGB32") << mat_16UC3_bgr << MCO_BGR << QImage::Format_RGB32 << image_rgb32;
    QTest::newRow("32FC3(BGR)_RGB32") << mat_32FC3_bgr << MCO_BGR << QImage::Format_RGB32 << image_rgb32;

    //Test data: C4 ==> RGB32
    QTest::newRow("8UC4(BGRX)_RGB32") << mat_8UC4_bgrx << MCO_BGRA << QImage::Format_RGB32 << image_rgb32;
    QTest::newRow("16UC4(BGRX)_RGB32") << mat_16UC4_bgrx << MCO_BGRA << QImage::Format_RGB32 << image_rgb32;
    QTest::newRow("32FC4(BGRX)_RGB32") << mat_32FC4_bgrx << MCO_BGRA << QImage::Format_RGB32 << image_rgb32;

    QTest::newRow("8UC4(RGBX)_RGB32") << mat_8UC4_rgbx << MCO_RGBA << QImage::Format_RGB32 << image_rgb32;
    QTest::newRow("16UC4(RGBX)_RGB32") << mat_16UC4_rgbx << MCO_RGBA << QImage::Format_RGB32 << image_rgb32;
    QTest::newRow("32FC4(RGBX)_RGB32") << mat_32FC4_rgbx << MCO_RGBA << QImage::Format_RGB32 << image_rgb32;

    QTest::newRow("8UC4(XRGB)_RGB32") << mat_8UC4_xrgb << MCO_ARGB << QImage::Format_RGB32 << image_rgb32;
    QTest::newRow("16UC4(XRGB)_RGB32") << mat_16UC4_xrgb << MCO_ARGB << QImage::Format_RGB32 << image_rgb32;
    QTest::newRow("32FC4(XRGB)_RGB32") << mat_32FC4_xrgb << MCO_ARGB << QImage::Format_RGB32 << image_rgb32;

    //Test data: C4 ==> ARGB32
    QTest::newRow("8UC4(BGRA)_ARGB32_Invalid") << mat_8UC4_bgra << MCO_BGRA << QImage::Format_Invalid << image_argb32;
    QTest::newRow("8UC4(BGRA)_ARGB32") << mat_8UC4_bgra << MCO_BGRA << QImage::Format_ARGB32 << image_argb32;
    QTest::newRow("16UC4(BGRA)_ARGB32") << mat_16UC4_bgra << MCO_BGRA << QImage::Format_ARGB32 << image_argb32;
    QTest::newRow("32FC4(BGRA)_ARGB32") << mat_32FC4_bgra << MCO_BGRA << QImage::Format_ARGB32 << image_argb32;

    QTest::newRow("8UC4(RGBA)_ARGB32") << mat_8UC4_rgba << MCO_RGBA << QImage::Format_ARGB32 << image_argb32;
    QTest::newRow("16UC4(RGBA)_ARGB32") << mat_16UC4_rgba << MCO_RGBA << QImage::Format_ARGB32 << image_argb32;
    QTest::newRow("32FC4(RGBA)_ARGB32") << mat_32FC4_rgba << MCO_RGBA << QImage::Format_ARGB32 << image_argb32;

    QTest::newRow("8UC4(ARGB)_ARGB32_Invalid") << mat_8UC4_argb << MCO_ARGB << QImage::Format_Invalid << image_argb32;
    QTest::newRow("8UC4(ARGB)_ARGB32") << mat_8UC4_argb << MCO_ARGB << QImage::Format_ARGB32 << image_argb32;
    QTest::newRow("16UC4(ARGB)_ARGB32") << mat_16UC4_argb << MCO_ARGB << QImage::Format_ARGB32 << image_argb32;
    QTest::newRow("32FC4(ARGB)_ARGB32") << mat_32FC4_argb << MCO_ARGB << QImage::Format_ARGB32 << image_argb32;

#if QT_VERSION >= 0x050200
    //Test data: C4 ==> RGBA8888
    QTest::newRow("8UC4(RGBA)_RGBA8888") << mat_8UC4_rgba << MCO_RGBA << QImage::Format_RGBA8888 << image_rgba8888;
    QTest::newRow("16UC4(RGBA)_RGBA8888") << mat_16UC4_rgba << MCO_RGBA << QImage::Format_RGBA8888 << image_rgba8888;
    QTest::newRow("32FC4(RGBA)_RGBA8888") << mat_32FC4_rgba << MCO_RGBA << QImage::Format_RGBA8888 << image_rgba8888;
#endif
}

void CvMatAndImageTest::testMat2QImage()
{
    QFETCH(cv::Mat, mat);
    QFETCH(MatColorOrder, mcOrder);
    QFETCH(QImage::Format, formatHint);
    QFETCH(QImage, expect);

    QImage convertedImage = mat2Image(mat, mcOrder, formatHint);
    QVERIFY(lenientCompare(convertedImage, expect));
}


void CvMatAndImageTest::testMat2QImageShared_data()
{
    QTest::addColumn<cv::Mat>("mat");
    QTest::addColumn<QImage::Format>("formatHint");
    QTest::addColumn<QImage>("expect");

    //Test data: C1 ==> Indexed8
    QTest::newRow("8UC1_Invalid") << mat_8UC1 << QImage::Format_Invalid << image_indexed8;
    QTest::newRow("8UC1_Indexed8") << mat_8UC1 << QImage::Format_Indexed8 << image_indexed8;

#if QT_VERSION >= 0x050500
    //Test data: C1 ==> Grayscale8
    QTest::newRow("8UC1_Grayscale8") << mat_8UC1 << QImage::Format_Grayscale8 << image_grayscale8;
#endif

#if QT_VERSION >= 0x040400
    //Test data: C3 ==> RGB8888
    QTest::newRow("8UC3_Invalid") << mat_8UC3_rgb << QImage::Format_Invalid << image_rgb888;
#endif

    //Test data: C4 ==> ARGB32
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    QTest::newRow("8UC4_ARGB32_Invalid") << mat_8UC4_bgra << QImage::Format_Invalid << image_argb32;
    QTest::newRow("8UC4_ARGB32") << mat_8UC4_bgra << QImage::Format_ARGB32 << image_argb32;
#else
    QTest::newRow("8UC4_ARGB32_Invalid") << mat_8UC4_argb << QImage::Format_Invalid << image_argb32;
    QTest::newRow("8UC4_ARGB32") << mat_8UC4_argb << QImage::Format_ARGB32 << image_argb32;
#endif

#if QT_VERSION >= 0x050200
    //Test data: C4 ==> RGBA8888
    QTest::newRow("8UC4_RGBA8888") << mat_8UC4_rgba << QImage::Format_RGBA8888 << image_rgba8888;
#endif
}

void CvMatAndImageTest::testMat2QImageShared()
{
    QFETCH(cv::Mat, mat);
    QFETCH(QImage::Format, formatHint);
    QFETCH(QImage, expect);

    QImage convertedImage = mat2Image_shared(mat, formatHint);
    QVERIFY(lenientCompare(convertedImage, expect));
}

void CvMatAndImageTest::testQImage2Mat_data()
{
    QTest::addColumn<QImage>("image");
    QTest::addColumn<MatColorOrder>("order");
    QTest::addColumn<int>("matType");
    QTest::addColumn<cv::Mat>("expect");

    //Test data: Indexed8 ==> C1
    QTest::newRow("Indexed8_8UC1") << image_indexed8 << MCO_BGR << CV_8UC1 << mat_8UC1;
    QTest::newRow("Indexed8_16UC1") << image_indexed8 << MCO_BGR << CV_16UC1 << mat_16UC1;
    QTest::newRow("Indexed8_32FC1") << image_indexed8 << MCO_BGR << CV_32FC1 << mat_32FC1;

#if QT_VERSION >= 0x050500
    //Test data: Grayscale8 ==> C1
    QTest::newRow("Grayscale8_8UC1") << image_grayscale8 << MCO_BGR << CV_8UC1 << mat_8UC1;
#endif

#if QT_VERSION >= 0x040400
    //Test data: RGB888 ==> C3
    QTest::newRow("RGB888_8UC3(RGB)") << image_rgb888 << MCO_RGB << CV_8UC3 << mat_8UC3_rgb;
    QTest::newRow("RGB888_16UC3(RGB)") << image_rgb888 << MCO_RGB << CV_16UC3 << mat_16UC3_rgb;
    QTest::newRow("RGB888_32FC3(RGB)") << image_rgb888 << MCO_RGB << CV_32FC3 << mat_32FC3_rgb;
    QTest::newRow("RGB888_8UC3(BGR)") << image_rgb888 << MCO_BGR << CV_8UC3 << mat_8UC3_bgr;
    QTest::newRow("RGB888_16UC3(BGR)") << image_rgb888 << MCO_BGR << CV_16UC3 << mat_16UC3_bgr;
    QTest::newRow("RGB888_32FC3(BGR)") << image_rgb888 << MCO_BGR << CV_32FC3 << mat_32FC3_bgr;

    //Test data: RGB888 ==> C4
    QTest::newRow("RGB888_8UC4(RGBX)") << image_rgb888 << MCO_RGBA << CV_8UC4 << mat_8UC4_rgbx;
    QTest::newRow("RGB888_16UC4(RGBX)") << image_rgb888 << MCO_RGBA << CV_16UC4 << mat_16UC4_rgbx;
    QTest::newRow("RGB888_32FC4(RGBX)") << image_rgb888 << MCO_RGBA << CV_32FC4 << mat_32FC4_rgbx;
#endif

    //Test data: ARGB32 ==> C4
    QTest::newRow("ARGB32_8UC4(BGRA)") << image_argb32 << MCO_BGRA << CV_8UC4 << mat_8UC4_bgra;
    QTest::newRow("ARGB32_16UC4(BGRA)") << image_argb32 << MCO_BGRA << CV_16UC4 << mat_16UC4_bgra;
    QTest::newRow("ARGB32_32FC4(BGRA)") << image_argb32 << MCO_BGRA << CV_32FC4 << mat_32FC4_bgra;
    QTest::newRow("ARGB32_8UC4(RGBA)") << image_argb32 << MCO_RGBA << CV_8UC4 << mat_8UC4_rgba;
    QTest::newRow("ARGB32_16UC4(RGBA)") << image_argb32 << MCO_RGBA << CV_16UC4 << mat_16UC4_rgba;
    QTest::newRow("ARGB32_32FC4(RGBA)") << image_argb32 << MCO_RGBA << CV_32FC4 << mat_32FC4_rgba;
    QTest::newRow("ARGB32_8UC4(ARGB)") << image_argb32 << MCO_ARGB << CV_8UC4 << mat_8UC4_argb;
    QTest::newRow("ARGB32_16UC4(ARGB)") << image_argb32 << MCO_ARGB << CV_16UC4 << mat_16UC4_argb;
    QTest::newRow("ARGB32_32FC4(ARGB)") << image_argb32 << MCO_ARGB << CV_32FC4 << mat_32FC4_argb;

#if QT_VERSION >= 0x050200
    //Test data: RGBA8888 ==> C4
    QTest::newRow("RGBA8888_8UC4(RGBA)") << image_rgba8888 << MCO_RGBA << CV_8UC4 << mat_8UC4_rgba;
    QTest::newRow("RGBA8888_16UC4(RGBA)") << image_rgba8888 << MCO_RGBA << CV_16UC4 << mat_16UC4_rgba;
    QTest::newRow("RGBA8888_32FC4(RGBA)") << image_rgba8888 << MCO_RGBA << CV_32FC4 << mat_32FC4_rgba;

    QTest::newRow("RGBA8888_8UC4(BGRA)") << image_rgba8888 << MCO_BGRA << CV_8UC4 << mat_8UC4_bgra;
    QTest::newRow("RGBA8888_16UC4(BGRA)") << image_rgba8888 << MCO_BGRA << CV_16UC4 << mat_16UC4_bgra;
    QTest::newRow("RGBA8888_32FC4(BGRA)") << image_rgba8888 << MCO_BGRA << CV_32FC4 << mat_32FC4_bgra;
#endif
}

void CvMatAndImageTest::testQImage2Mat()
{
    QFETCH(QImage, image);
    QFETCH(MatColorOrder, order);
    QFETCH(int, matType);
    QFETCH(cv::Mat, expect);

    cv::Mat mat = image2Mat(image, matType, order);
    if (mat.depth() == CV_8U)
        QVERIFY(lenientCompare<uchar>(mat, expect));
    else if (mat.depth() == CV_16U)
        QVERIFY(lenientCompare<quint16>(mat, expect));
    else if (mat.depth() == CV_32F)
        QVERIFY(lenientCompare<float>(mat, expect));
    else
        QVERIFY(false);
}

void CvMatAndImageTest::testQImage2MatShared_data()
{
    QTest::addColumn<QImage>("image");
    QTest::addColumn<cv::Mat>("expect");

    //Test data: Indexed8 ==> C1
    QTest::newRow("Indexed8_8UC1") << image_indexed8 << mat_8UC1;

#if QT_VERSION >= 0x050500
    //Test data: Grayscale8 ==> C1
    QTest::newRow("Grayscale8_8UC1") << image_grayscale8 << mat_8UC1;
#endif

#if QT_VERSION >= 0x040400
    //Test data: RGB8888 ==> C3
    QTest::newRow("RGB8888_8UC3") << image_rgb888 << mat_8UC3_rgb;
#endif

    //Test data: ARGB32 ==> C4
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    QTest::newRow("ARGB32_8UC4") << image_argb32 << mat_8UC4_bgra;
#else
    QTest::newRow("ARGB32_8UC4") << image_argb32 << mat_8UC4_argb;
#endif

#if QT_VERSION >= 0x050200
    //Test data: RGBA8888 ==> C4
    QTest::newRow("RGBA8888_8UC4") << image_rgba8888 << mat_8UC4_rgba;
#endif
}

void CvMatAndImageTest::testQImage2MatShared()
{
    QFETCH(QImage, image);
    QFETCH(cv::Mat, expect);

    cv::Mat convertedMat = image2Mat_shared(image);
    QVERIFY(lenientCompare<uchar>(convertedMat, expect));
}

QTEST_MAIN(CvMatAndImageTest)

#include "tst_cvmatandimagetest.moc"
