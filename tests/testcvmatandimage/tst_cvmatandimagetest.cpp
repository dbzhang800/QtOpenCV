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
    QImage expectedImage = expected.convertToFormat(QImage::Format_RGB32);
    QImage actualImage = actual.convertToFormat(QImage::Format_RGB32);

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
            || qAbs(ca.blue() - ce.blue()) > threshold) {
            qWarning("Color mismatch at pixel #%d: Expected: %d,%d,%d, got %d,%d,%d",
                     i, ce.red(), ce.green(), ce.blue(), ca.red(), ca.green(), ca.blue());
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
    void testQImageDataBytesOrder(); //Just for ...
    void testMatChannelsOrder(); //Just for ...
    void testSimpleGrayValue();

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

    cv::Mat mat_8UC3;
    cv::Mat mat_16UC3;
    cv::Mat mat_32FC3;

    cv::Mat mat_8UC4_rgba;
    cv::Mat mat_16UC4_rgba;
    cv::Mat mat_32FC4_rgba;
    cv::Mat mat_8UC4_argb;
    cv::Mat mat_16UC4_argb;
    cv::Mat mat_32FC4_argb;
    cv::Mat mat_8UC4_bgra;
    cv::Mat mat_16UC4_bgra;
    cv::Mat mat_32FC4_bgra;

    QImage image_indexed8;
    QImage image_rgb888;
    QImage image_rgb32;
    QImage image_argb32;
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

    mat_8UC3 = cv::Mat(height, width, CV_8UC3);
    mat_16UC3 = cv::Mat(height, width, CV_16UC3);
    mat_32FC3 = cv::Mat(height, width, CV_32FC3);

    mat_8UC4_rgba = cv::Mat(height, width, CV_8UC4);
    mat_16UC4_rgba = cv::Mat(height, width, CV_16UC4);
    mat_32FC4_rgba = cv::Mat(height, width, CV_32FC4);
    mat_8UC4_argb = cv::Mat(height, width, CV_8UC4);
    mat_16UC4_argb = cv::Mat(height, width, CV_16UC4);
    mat_32FC4_argb = cv::Mat(height, width, CV_32FC4);
    mat_8UC4_bgra = cv::Mat(height, width, CV_8UC4);
    mat_16UC4_bgra = cv::Mat(height, width, CV_16UC4);
    mat_32FC4_bgra = cv::Mat(height, width, CV_32FC4);

    image_indexed8 = QImage(width, height, QImage::Format_Indexed8);
    QVector<QRgb> colorTable;
    for (int i=0; i<256; ++i)
        colorTable.append(qRgb(i, i, i));
    image_indexed8.setColorTable(colorTable);
    image_rgb888 = QImage(width, height, QImage::Format_RGB888);
    image_rgb32 = QImage(width, height, QImage::Format_RGB32);
    image_argb32 = QImage(width, height, QImage::Format_ARGB32);
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

            mat_8UC3.at<cv::Vec3b>(row, col) = cv::Vec3b(r, g, b);
            mat_16UC3.at<cv::Vec3w>(row, col) = cv::Vec3w(r*255, g*255, b*255);
            mat_32FC3.at<cv::Vec3f>(row, col) = cv::Vec3f(r/255.0, g/255.0, b/255.0);

            mat_8UC4_rgba.at<cv::Vec4b>(row, col) = cv::Vec4b(r, g, b, a);
            mat_8UC4_argb.at<cv::Vec4b>(row, col) = cv::Vec4b(a, r, g, b);
            mat_8UC4_bgra.at<cv::Vec4b>(row, col) = cv::Vec4b(b, g, r, a);

            mat_16UC4_rgba.at<cv::Vec4w>(row, col) = cv::Vec4w(r*255, g*255, b*255, a*255);
            mat_16UC4_argb.at<cv::Vec4w>(row, col) = cv::Vec4w(a*255, r*255, g*255, b*255);
            mat_16UC4_bgra.at<cv::Vec4w>(row, col) = cv::Vec4w(b*255, g*255, r*255, a*255);

            mat_32FC4_rgba.at<cv::Vec4f>(row, col) = cv::Vec4f(r/255.0, g/255.0, b/255.0, a/255.0);
            mat_32FC4_argb.at<cv::Vec4f>(row, col) = cv::Vec4f(a/255.0, r/255.0, g/255.0, b/255.0);
            mat_32FC4_bgra.at<cv::Vec4f>(row, col) = cv::Vec4f(b/255.0, g/255.0, r/255.0, a/255.0);

            image_indexed8.setPixel(col, row, r);
            image_rgb888.setPixel(col, row, qRgb(r, g, b));
            image_rgb32.setPixel(col, row, qRgb(r, g, b));
            image_argb32.setPixel(col, row, qRgba(r, g, b, a));
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

void CvMatAndImageTest::testQImageDataBytesOrder()
{       
    //Image data bytes order: R G B
    QImage redImage_rgb888(400, 300, QImage::Format_RGB888);
    //redImage_rgb888.fill(qRgb(254,1,0)); Though works under Qt5, but not Qt4
    redImage_rgb888.fill(QColor(254,1,0));
    QCOMPARE(QByteArray(reinterpret_cast<char*>(redImage_rgb888.bits()), 6), QByteArray("\xfe\x01\x00\xfe\x01\x00", 6));

    //Image data bytes order: B G R X(little endian) or X R G B(big endian)
    QImage redImage_rgb32(400, 300, QImage::Format_RGB32);
    redImage_rgb32.fill(QColor(254,1,0));
    QByteArray target = (QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? QByteArray("\x00\x01\xfe\xff", 4) : QByteArray("\xff\xfe\x01\x00", 4);
    QCOMPARE(QByteArray(reinterpret_cast<char*>(redImage_rgb32.bits()), 4), target);

    //Image data bytes order: B G R A(little endian) or A R G B(big endian)
    QImage redImage_argb32(400, 300, QImage::Format_ARGB32);
    redImage_argb32.fill(QColor(254,1,0, 128));
    QByteArray target2 = (QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? QByteArray("\x00\x01\xfe\x80", 4) : QByteArray("\x80\xfe\x01\x00", 4);
    QCOMPARE(QByteArray(reinterpret_cast<char*>(redImage_argb32.bits()), 4), target2);
}

void CvMatAndImageTest::testMatChannelsOrder()
{
    //(1)
    //generate a red-color image, then save as a png format image file
    QImage redImage(400, 300, QImage::Format_RGB888);
    redImage.fill(QColor(254,1,0)); // * R G B *
    const char* redImage_filename = "tst_data_testchannelsorder1.png";
    redImage.save(redImage_filename);

    //load this image with highgui's method, note that the order is B G R instead of R G B
    cv::Mat redMat = cv::imread(redImage_filename);
    QVERIFY(redMat.channels() == 3);
    QCOMPARE(redMat.at<cv::Vec3b>(1,1), cv::Vec3b(0,1,254)); // * B G R *
    QCOMPARE(redMat.at<cv::Vec3b>(1,1)[2], uchar(254));

    QCOMPARE(QByteArray(reinterpret_cast<char*>(redImage.bits()), 6), QByteArray("\xfe\x01\x00\xfe\x01\x00", 6));
    QCOMPARE(QByteArray(reinterpret_cast<char*>(redMat.data),     6), QByteArray("\x00\x01\xfe\x00\x01\xfe", 6));

    //(2)
    //generate a B G R A OpenCV Image, then save as a png format image file
    const char* alphaImage_filename = "tst_data_testchannelsorder2.png";
    cv::Mat alphaMat(400, 300, CV_8UC4, cv::Scalar_<uchar>(0,1,254,128));
    cv::imwrite(alphaImage_filename, alphaMat);

    QImage alphaImage(alphaImage_filename);
    QCOMPARE(alphaImage.format(), QImage::Format_ARGB32);
    QCOMPARE(alphaImage.pixel(1,1), qRgba(254,1,0,128));
}

void CvMatAndImageTest::testSimpleGrayValue()
{
    const int r = 254;
    const int g = 1;
    const int b = 0;

    int gray_Qt = qGray(r, g, b);

    cv::Mat mat_helper(1,1,CV_8UC3,cv::Scalar_<uchar>(r,g,b));
    cv::cvtColor(mat_helper, mat_helper, CV_RGB2GRAY);
    int gray_Ocv = mat_helper.at<uchar>(0,0);

    int gray_my1 = (r * 9798 + g * 19238 + b * 3728)/32768;
    int gray_my2 = cv::saturate_cast<int>(r * 0.299 + g * 0.587 + b * 0.114);

    qWarning()<<gray_Qt<<gray_Ocv<<gray_my1<<gray_my2;
//    QVERIFY(abs(gray_Qt - gray_Ocv) < 3);
    QVERIFY(abs(gray_my1 - gray_Ocv) < 3);
    QVERIFY(abs(gray_my2 - gray_Ocv) < 3);
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

    //Test data: C3 ==> RGB8888
    QTest::newRow("8UC3_Invalid") << mat_8UC3 << MCO_RGB << QImage::Format_Invalid << image_rgb888;
    QTest::newRow("8UC3_RGB888") << mat_8UC3 << MCO_RGB << QImage::Format_RGB888 << image_rgb888;
    QTest::newRow("16UC3") << mat_16UC3 << MCO_RGB << QImage::Format_RGB888 << image_rgb888;
    QTest::newRow("32FC3") << mat_32FC3 << MCO_RGB << QImage::Format_RGB888 << image_rgb888;

    //Test data: C4 ==> ARGB32
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        QTest::newRow("8UC4_ARGB32_Invalid") << mat_8UC4_bgra << MCO_BGRA << QImage::Format_Invalid << image_argb32;
        QTest::newRow("8UC4_ARGB32") << mat_8UC4_bgra << MCO_BGRA << QImage::Format_ARGB32 << image_argb32;
        QTest::newRow("16UC4_ARGB32") << mat_16UC4_bgra << MCO_BGRA << QImage::Format_ARGB32 << image_argb32;
        QTest::newRow("32FC4_ARGB32") << mat_32FC4_bgra << MCO_BGRA << QImage::Format_ARGB32 << image_argb32;
    } else {
        QTest::newRow("8UC4_ARGB32_Invalid") << mat_8UC4_argb << MCO_ARGB << QImage::Format_Invalid << image_argb32;
        QTest::newRow("8UC4_ARGB32") << mat_8UC4_argb << MCO_ARGB << QImage::Format_ARGB32 << image_argb32;
        QTest::newRow("16UC4_ARGB32") << mat_16UC4_argb << MCO_ARGB << QImage::Format_ARGB32 << image_argb32;
        QTest::newRow("32FC4_ARGB32") << mat_32FC4_argb << MCO_ARGB << QImage::Format_ARGB32 << image_argb32;
    }

#if QT_VERSION >= 0x050200
    //Test data: C4 ==> RGBA8888
    QTest::newRow("8UC4_RGBA8888") << mat_8UC4_rgba << MCO_RGBA << QImage::Format_RGBA8888 << image_rgba8888;
    QTest::newRow("16UC4_RGBA8888") << mat_16UC4_rgba << MCO_RGBA << QImage::Format_RGBA8888 << image_rgba8888;
    QTest::newRow("32FC4_RGBA8888") << mat_32FC4_rgba << MCO_RGBA << QImage::Format_RGBA8888 << image_rgba8888;
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

    //Test data: C3 ==> RGB8888
    QTest::newRow("8UC3_Invalid") << mat_8UC3 << QImage::Format_Invalid << image_rgb888;

    //Test data: C4 ==> ARGB32
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        QTest::newRow("8UC4_ARGB32_Invalid") << mat_8UC4_bgra << QImage::Format_Invalid << image_argb32;
        QTest::newRow("8UC4_ARGB32") << mat_8UC4_bgra << QImage::Format_ARGB32 << image_argb32;
    } else {
        QTest::newRow("8UC4_ARGB32_Invalid") << mat_8UC4_argb << QImage::Format_Invalid << image_argb32;
        QTest::newRow("8UC4_ARGB32") << mat_8UC4_argb << QImage::Format_ARGB32 << image_argb32;
    }

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
    QTest::addColumn<int>("matDepth");
    QTest::addColumn<cv::Mat>("expect");

    //Test data: Indexed8 ==> C1
    QTest::newRow("Indexed8_8UC1") << image_indexed8 << CV_8U << mat_8UC1;
    QTest::newRow("Indexed8_16UC1") << image_indexed8 << CV_16U << mat_16UC1;
    QTest::newRow("Indexed8_32FC1") << image_indexed8 << CV_32F << mat_32FC1;

    //Test data: RGB888 ==> C3
    QTest::newRow("RGB8888_8UC3") << image_rgb888 << CV_8U << mat_8UC3;
    QTest::newRow("RGB8888_16UC3") << image_rgb888 << CV_16U << mat_16UC3;
    QTest::newRow("RGB8888_32FC3") << image_rgb888 << CV_32F << mat_32FC3;

    //Test data: ARGB32 ==> C4
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        QTest::newRow("ARGB32_8UC4") << image_argb32 << CV_8U << mat_8UC4_bgra;
        QTest::newRow("ARGB32_16UC4") << image_argb32 << CV_16U << mat_16UC4_bgra;
        QTest::newRow("ARGB32_32FC4") << image_argb32 << CV_32F << mat_32FC4_bgra;
    } else {
        QTest::newRow("ARGB32_8UC4") << image_argb32 << CV_8U << mat_8UC4_argb;
        QTest::newRow("ARGB32_16UC4") << image_argb32 << CV_16U << mat_16UC4_argb;
        QTest::newRow("ARGB32_32FC4") << image_argb32 << CV_32F << mat_32FC4_argb;
    }

#if QT_VERSION >= 0x050200
    //Test data: RGBA8888 ==> C4
    QTest::newRow("RGBA8888_8UC4") << image_rgba8888 << CV_8U << mat_8UC4_rgba;
    QTest::newRow("RGBA8888_16UC4") << image_rgba8888 << CV_16U << mat_16UC4_rgba;
    QTest::newRow("RGBA8888_32FC4") << image_rgba8888 << CV_32F << mat_32FC4_rgba;
#endif
}

void CvMatAndImageTest::testQImage2Mat()
{
    QFETCH(QImage, image);
    QFETCH(int, matDepth);
    QFETCH(cv::Mat, expect);

    cv::Mat mat = image2Mat(image, matDepth);
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

    //Test data: RGB8888 ==> C3
    QTest::newRow("RGB8888_8UC3") << image_rgb888 << mat_8UC3;

    //Test data: ARGB32 ==> C4
    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        QTest::newRow("ARGB32_8UC4") << image_argb32 << mat_8UC4_bgra;
    } else {
        QTest::newRow("ARGB32_8UC4") << image_argb32 << mat_8UC4_argb;
    }

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
