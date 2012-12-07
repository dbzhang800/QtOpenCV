#include "cvmatandqimage.h"
#include <QString>
#include <QtTest>
#include <QTemporaryFile>
#include <QDebug>
#include <vector>
#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace QtOcv;

Q_DECLARE_METATYPE(MatChannelOrder)
Q_DECLARE_METATYPE(QImage::Format)
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
    const int threshold = 1;

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
        qWarning("cv::Mat comparison failed: expected: %d C%d, got %d C%d",
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

    double threshold = 2.0e-2;

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
    
private Q_SLOTS:
    void testQImageDataBytesOrder(); //Just for ...
    void testMatChannelsOrder(); //Just for ...
    void testSimpleGrayValue();

    void testMat2QImage();
    void testMat2QImageShared();
    void testMat2QImageChannelsOrder_data();
    void testMat2QImageChannelsOrder();

    void testQImage2Mat();
    void testQImage2MatShared();
    void testQImage2MatChannelsOrder_data();
    void testQImage2MatChannelsOrder();
};

CvMatAndImageTest::CvMatAndImageTest()
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

void CvMatAndImageTest::testMat2QImage()
{
    cv::Mat mat_8UC1(100, 200, CV_8UC1);
    for (int i=0; i<100; ++i) {
        for (int j=0; j<200; ++j)
            mat_8UC1.at<uchar>(i,j) = (i*j)%255;
    }

    //CV_8UC1 to QImage::Format_Index8
    QImage img1_index8 = mat2Image(mat_8UC1);
    QCOMPARE(img1_index8.pixelIndex(50,2), 100);

    //CV_8UC1 to QImage::Format_RGB32
    QImage img1_rgb32 = mat2Image(mat_8UC1, QImage::Format_RGB32);
    QCOMPARE(img1_rgb32.pixel(50,2), qRgb(100, 100, 100));

    cv::Mat mat_8UC3_bgr(100, 200, CV_8UC3, cv::Scalar_<uchar>(0, 1, 254));
    cv::Mat mat_8UC3_gray;
    cv::cvtColor(mat_8UC3_bgr, mat_8UC3_gray, CV_BGR2GRAY, 1);

    //CV_8UC3 to QImage::Format_Index8
    QImage img3_index8 = mat2Image(mat_8UC3_bgr, QImage::Format_Indexed8);
    QCOMPARE(img3_index8.pixelIndex(1,1), int(mat_8UC3_gray.at<uchar>(1,1)));

    //CV_8UC3 to QImage::Format_RGB32
    QImage img3_rgb32 = mat2Image(mat_8UC3_bgr, QImage::Format_RGB32, MCO_BGR);
    QCOMPARE(img3_rgb32.pixel(1,1), qRgb(254, 1, 0));

    cv::Mat mat_16UC3_bgr;
    mat_8UC3_bgr.convertTo(mat_16UC3_bgr, CV_16UC3, 65535.0/255.0);
    QImage img3_16UC3_rgb32 = mat2Image(mat_16UC3_bgr, QImage::Format_RGB32, MCO_BGR);

    QVERIFY(lenientCompare(img3_rgb32, img3_16UC3_rgb32));

    cv::Mat mat_32FC3_bgr;
    mat_8UC3_bgr.convertTo(mat_32FC3_bgr, CV_32FC3, 1.0/255.0);
    QImage img3_32FC3_rgb32 = mat2Image(mat_32FC3_bgr, QImage::Format_RGB32, MCO_BGR);

    QVERIFY(lenientCompare(img3_rgb32, img3_32FC3_rgb32));
}

void CvMatAndImageTest::testMat2QImageShared()
{
    cv::Mat mat_8UC1(100, 200, CV_8UC1);
    for (int i=0; i<100; ++i) {
        for (int j=0; j<200; ++j)
            mat_8UC1.at<uchar>(i,j) = (i*j)%255;
    }

    //CV_8UC1 share data with QImage::Format_Index8
    QImage img1_index8 = mat2Image_shared(mat_8UC1);
    QCOMPARE(img1_index8.pixelIndex(3,50), 150);
    QCOMPARE(img1_index8, mat2Image(mat_8UC1, QImage::Format_Indexed8));

    cv::Mat mat_8UC3;
    std::vector<cv::Mat> channels;
    channels.push_back(mat_8UC1);
    channels.push_back(mat_8UC1);
    channels.push_back(mat_8UC1);
    cv::merge(channels, mat_8UC3);

    //CV_8UC3(R G B) share data with QImage::Format_RGB888
    QImage img1_rgb888 = mat2Image_shared(mat_8UC3);
    QCOMPARE(img1_rgb888.format(), QImage::Format_RGB888);
    QCOMPARE(img1_rgb888.pixel(3, 50), qRgb(150, 150, 150));

    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
        //CV_8UC4(B G R A) share data with QImage::Format_ARGB32
        cv::Mat mat_8UC4;
        channels.push_back(cv::Mat(100, 200, CV_8UC1, cv::Scalar_<uchar>(128)));
        cv::merge(channels, mat_8UC4);

        QImage img1_argb32 = mat2Image_shared(mat_8UC4);
        QCOMPARE(img1_argb32.format(), QImage::Format_ARGB32);
        QCOMPARE(img1_argb32.pixel(3, 50), qRgba(150, 150, 150, 128));
    }

    cv::Size2i size(4096, 4096);
    const int channel = 3;
    cv::Mat mat_rgb(size, CV_32FC(channel), cv::Scalar_<float>(0.9f, 0.01f, 0.0f, 0.5f));

    QImage img0 = mat2Image(mat_rgb, QImage::Format_RGB888, QtOcv::MCO_RGB);
    mat_rgb.convertTo(mat_rgb, CV_8UC(channel), 255);
    QImage img1 = mat2Image_shared(mat_rgb);

    QVERIFY(lenientCompare(img0, img1));
}

void CvMatAndImageTest::testMat2QImageChannelsOrder_data()
{
    QTest::addColumn<int>("channels");
    QTest::addColumn<MatChannelOrder>("rgbOrder");
    QTest::addColumn<QImage::Format>("format");
    QTest::addColumn<unsigned int>("expect");

    const int gray = cv::saturate_cast<int>(254 * 0.299 + 1 * 0.587 + 0 * 0.114);

    QTest::newRow("C1 ==> QImage::Format_ARGB32")<<1<<MCO_BGR<<QImage::Format_ARGB32<<qRgba(gray,gray,gray,255);
    QTest::newRow("C1 ==> QImage::Format_RGB32")<<1<<MCO_BGR<<QImage::Format_RGB32<<qRgb(gray,gray,gray);
    QTest::newRow("C1 ==> QImage::Format_RGB888")<<1<<MCO_BGR<<QImage::Format_RGB888<<qRgb(gray,gray,gray);
    QTest::newRow("C1 ==> QImage::Format_Indexed8")<<1<<MCO_BGR<<QImage::Format_Indexed8<<qRgb(gray,gray,gray);

    QTest::newRow("C3_BGR ==> QImage::Format_ARGB32")<<3<<MCO_BGR<<QImage::Format_ARGB32<<qRgba(254,1,0,255);
    QTest::newRow("C3_RGB ==> QImage::Format_ARGB32")<<3<<MCO_RGB<<QImage::Format_ARGB32<<qRgba(254,1,0,255);
    QTest::newRow("C3_BGR ==> QImage::Format_RGB32")<<3<<MCO_BGR<<QImage::Format_RGB32<<qRgb(254,1,0);
    QTest::newRow("C3_RGB ==> QImage::Format_RGB32")<<3<<MCO_RGB<<QImage::Format_RGB32<<qRgb(254,1,0);
    QTest::newRow("C3_BGR ==> QImage::Format_RGB888")<<3<<MCO_BGR<<QImage::Format_RGB888<<qRgb(254,1,0);
    QTest::newRow("C3_RGB ==> QImage::Format_RGB888")<<3<<MCO_RGB<<QImage::Format_RGB888<<qRgb(254,1,0);
    QTest::newRow("C3_BGR ==> QImage::Format_Indexed8")<<3<<MCO_BGR<<QImage::Format_Indexed8<<qRgb(gray,gray,gray);
    QTest::newRow("C3_RGB ==> QImage::Format_Indexed8")<<3<<MCO_RGB<<QImage::Format_Indexed8<<qRgb(gray,gray,gray);

    QTest::newRow("C4_BGRA ==> QImage::Format_ARGB32")<<4<<MCO_BGR<<QImage::Format_ARGB32<<qRgba(254,1,0,128);
    QTest::newRow("C4_RGBA ==> QImage::Format_ARGB32")<<4<<MCO_RGB<<QImage::Format_ARGB32<<qRgba(254,1,0,128);
    QTest::newRow("C4_BGRA ==> QImage::Format_RGB32")<<4<<MCO_BGR<<QImage::Format_RGB32<<qRgb(254,1,0);
    QTest::newRow("C4_RGBA ==> QImage::Format_RGB32")<<4<<MCO_RGB<<QImage::Format_RGB32<<qRgb(254,1,0);
    QTest::newRow("C4_BGRA ==> QImage::Format_RGB888")<<4<<MCO_BGR<<QImage::Format_RGB888<<qRgb(254,1,0);
    QTest::newRow("C4_RGBA ==> QImage::Format_RGB888")<<4<<MCO_RGB<<QImage::Format_RGB888<<qRgb(254,1,0);
    QTest::newRow("C4_BGRA ==> QImage::Format_Indexed8")<<4<<MCO_BGR<<QImage::Format_Indexed8<<qRgb(gray,gray,gray);
    QTest::newRow("C4_RGBA ==> QImage::Format_Indexed8")<<4<<MCO_RGB<<QImage::Format_Indexed8<<qRgb(gray,gray,gray);
}

void CvMatAndImageTest::testMat2QImageChannelsOrder()
{
    QFETCH(int, channels);
    QFETCH(MatChannelOrder, rgbOrder);
    QFETCH(QImage::Format, format);
    QFETCH(unsigned int, expect);

    const int gray = cv::saturate_cast<int>(254 * 0.299 + 1 * 0.587 + 0 * 0.114);
    //(1) CV_8U
    cv::Scalar_<uchar> vec_8u;
    if (channels==1)
        vec_8u = cv::Scalar_<uchar>(gray);
    else if(rgbOrder == MCO_BGR)
        vec_8u =  cv::Scalar_<uchar>(0, 1, 254, 128);
    else
        vec_8u = cv::Scalar_<uchar>(254, 1, 0, 128);

    //This is a red image, when channles is 4, the alpha is 128.
    const cv::Mat mat(400, 300, CV_8UC(channels), vec_8u);
    QImage image = mat2Image(mat, format, rgbOrder);
    QCOMPARE(image.pixel(1,1), expect);

    //(2) CV_16U
    cv::Scalar_<quint16> vec_16u;
    if (channels==1)
        vec_16u = cv::Scalar_<quint16>(gray*65535/255);
    else if(rgbOrder == MCO_BGR)
        vec_16u =  cv::Scalar_<quint16>(0, 65535/255, 254*65535/255, 128*65535/255);
    else
        vec_16u = cv::Scalar_<quint16>(254*65535/255, 65535/255, 0, 128*65535/255);

    const cv::Mat mat_16U(400, 300, CV_16UC(channels), vec_16u);
    QImage image_16U = mat2Image(mat_16U, format, rgbOrder);
    QCOMPARE(image_16U.pixel(1,1), expect);

    //(3) CV_32F
    cv::Scalar_<float> vec_32f;
    if (channels==1)
        vec_32f = cv::Scalar_<float>(gray/255.0);
    else if(rgbOrder == MCO_BGR)
        vec_32f = cv::Scalar_<float>(0.0f, float(1/255.0), float(254/255.0), float(128/255.0));
    else
        vec_32f = cv::Scalar_<float>(float(254/255.0), float(1/255.0), 0.0f, float(128/255.0));

    const cv::Mat mat_32F(400, 300, CV_32FC(channels), vec_32f);
    QImage image_32F = mat2Image(mat_32F, format, rgbOrder);
    QCOMPARE(image_32F.pixel(1,1), expect);
}

void CvMatAndImageTest::testQImage2Mat()
{
    QImage img_rgb32 = QImage(100, 100, QImage::Format_RGB32);
    img_rgb32.fill(QColor(254, 1, 0));

    // QImage::Format_RGB32 will be convert to CV_8UC4(B G R 255) by default
    cv::Mat mat = image2Mat(img_rgb32);
    QCOMPARE(mat.type(), CV_8UC4);
    QCOMPARE(mat.at<cv::Vec4b>(1,1), cv::Vec4b(0,1,254,255));

    mat = image2Mat(img_rgb32, CV_8UC1);
    QCOMPARE(mat.type(), CV_8UC1);

    QImage img_rgb888 = QImage(100, 100, QImage::Format_RGB888);
    img_rgb888.fill(QColor(254, 1, 0));

    // QImage::Format_RGB888 will be convert to CV_8UC3(B G R) by default
    mat = image2Mat(img_rgb888);
    QCOMPARE(mat.type(), CV_8UC3);
    QCOMPARE(mat.at<cv::Vec3b>(1,1), cv::Vec3b(0,1,254));

    mat = image2Mat(img_rgb888, CV_8UC1);
    QCOMPARE(mat.type(), CV_8UC1);

    cv::Mat mat_8UC4 = image2Mat(img_rgb32);
    QCOMPARE(mat_8UC4.type(), CV_8UC4);
    cv::Mat mat_16UC4 = image2Mat(img_rgb32, CV_16UC4);
    QCOMPARE(mat_16UC4.type(), CV_16UC4);
    cv::Mat mat_32FC4 = image2Mat(img_rgb32, CV_32FC4);
    QCOMPARE(mat_32FC4.type(), CV_32FC4);

    QCOMPARE(mat_8UC4.at<cv::Vec4b>(1,1)[2], uchar(254));
    QCOMPARE(mat_16UC4.at<cv::Vec4w>(1,1)[2], unsigned short(254*(65535./255.)));
    QCOMPARE(mat_32FC4.at<cv::Vec4f>(1,1)[2], float(254/255.0));
}


void CvMatAndImageTest::testQImage2MatShared()
{
    //QImage::Format_RGB32 share data with CV_8UC4(B G R 255, little endian system)
    //or CV_8UC4(255 R G B, big endian system)
    QImage img_rgb32 = QImage(100, 100, QImage::Format_RGB32);
    img_rgb32.fill(QColor(254, 1, 0));
    cv::Mat mat_8UC4 = image2Mat_shared(img_rgb32);
    QCOMPARE(mat_8UC4.type(), CV_8UC4);
    cv::Vec4b target = (QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? cv::Vec4b(0,1,254,255) :
                                                                         cv::Vec4b(255,254,1,0);
    QCOMPARE(mat_8UC4.at<cv::Vec4b>(1,1), target);

    //QImage::Format_ARGB32 share data with CV_8UC4(B G R A, little endian system)
    //or CV_8UC4(A R G B, big endian system)
    QImage img_argb32 = QImage(100, 100, QImage::Format_ARGB32);
    img_argb32.fill(QColor(254, 1, 0, 128));
    mat_8UC4 = image2Mat_shared(img_argb32);
    QCOMPARE(mat_8UC4.type(), CV_8UC4);
    cv::Vec4b target2 = (QSysInfo::ByteOrder == QSysInfo::LittleEndian) ? cv::Vec4b(0,1,254,128) :
                                                                         cv::Vec4b(128,254,1,0);
    QCOMPARE(mat_8UC4.at<cv::Vec4b>(1,1), target2);

    //QImage::Format_RGB888 share data with CV_8UC3(R G B)
    QImage img_rgb888 = QImage(100, 100, QImage::Format_RGB888);
    img_rgb888.fill(QColor(254, 1, 0));
    cv::Mat mat_8UC3 = image2Mat_shared(img_rgb888);
    QCOMPARE(mat_8UC3.type(), CV_8UC3);
    QCOMPARE(mat_8UC3.at<cv::Vec3b>(1,1), cv::Vec3b(254,1,0));
}

void CvMatAndImageTest::testQImage2MatChannelsOrder_data()
{
    QTest::addColumn<int>("channels");
    QTest::addColumn<MatChannelOrder>("rgbOrder");
    QTest::addColumn<QImage::Format>("format");
    QTest::addColumn<unsigned int>("fillValue");

    QTest::newRow("QImage::Format_ARGB32 ==> C1")<<1<<MCO_BGR<<QImage::Format_ARGB32<<qRgba(254,1,0,128);
    QTest::newRow("QImage::Format_ARGB32 ==> C3_BGR")<<3<<MCO_BGR<<QImage::Format_ARGB32<<qRgba(254,1,0,128);
    QTest::newRow("QImage::Format_ARGB32 ==> C3_RGB")<<3<<MCO_RGB<<QImage::Format_ARGB32<<qRgba(254,1,0,128);
    QTest::newRow("QImage::Format_ARGB32 ==> C4_BGRA")<<4<<MCO_BGR<<QImage::Format_ARGB32<<qRgba(254,1,0,128);
    QTest::newRow("QImage::Format_ARGB32 ==> C4_RGBA")<<4<<MCO_RGB<<QImage::Format_ARGB32<<qRgba(254,1,0,128);

    QTest::newRow("QImage::Format_RGB32 ==> C1")<<1<<MCO_BGR<<QImage::Format_RGB32<<qRgb(254,1,0);
    QTest::newRow("QImage::Format_RGB32 ==> C3_BGR")<<3<<MCO_BGR<<QImage::Format_RGB32<<qRgb(254,1,0);
    QTest::newRow("QImage::Format_RGB32 ==> C3_RGB")<<3<<MCO_RGB<<QImage::Format_RGB32<<qRgb(254,1,0);
    QTest::newRow("QImage::Format_RGB32 ==> C4_BGRA")<<4<<MCO_BGR<<QImage::Format_RGB32<<qRgb(254,1,0);
    QTest::newRow("QImage::Format_RGB32 ==> C4_RGBA")<<4<<MCO_RGB<<QImage::Format_RGB32<<qRgb(254,1,0);

    QTest::newRow("QImage::Format_RGB888 ==> C1")<<1<<MCO_BGR<<QImage::Format_RGB888<<qRgb(254,1,0);
    QTest::newRow("QImage::Format_RGB888 ==> C3_BGR")<<3<<MCO_BGR<<QImage::Format_RGB888<<qRgb(254,1,0);
    QTest::newRow("QImage::Format_RGB888 ==> C3_RGB")<<3<<MCO_RGB<<QImage::Format_RGB888<<qRgb(254,1,0);
    QTest::newRow("QImage::Format_RGB888 ==> C4_BGRA")<<4<<MCO_BGR<<QImage::Format_RGB888<<qRgb(254,1,0);
    QTest::newRow("QImage::Format_RGB888 ==> C4_RGBA")<<4<<MCO_RGB<<QImage::Format_RGB888<<qRgb(254,1,0);

    QTest::newRow("QImage::Format_Indexed8 ==> C1")<<1<<MCO_BGR<<QImage::Format_Indexed8<<qRgb(254,1,0);
    QTest::newRow("QImage::Format_Indexed8 ==> C3_BGR")<<3<<MCO_BGR<<QImage::Format_Indexed8<<qRgb(254,1,0);
    QTest::newRow("QImage::Format_Indexed8 ==> C3_RGB")<<3<<MCO_RGB<<QImage::Format_Indexed8<<qRgb(254,1,0);
    QTest::newRow("QImage::Format_Indexed8 ==> C4_BGRA")<<4<<MCO_BGRA<<QImage::Format_Indexed8<<qRgb(254,1,0);
    QTest::newRow("QImage::Format_Indexed8 ==> C4_RGBA")<<4<<MCO_RGBA<<QImage::Format_Indexed8<<qRgb(254,1,0);
}

void CvMatAndImageTest::testQImage2MatChannelsOrder(){
    QFETCH(int, channels);
    QFETCH(MatChannelOrder, rgbOrder);
    QFETCH(QImage::Format, format);
    QFETCH(unsigned int, fillValue);

    const int gray = cv::saturate_cast<int>(qRed(fillValue) * 0.299 + qGreen(fillValue) * 0.587 + qBlue(fillValue) * 0.114);

    //This is red image
    QImage img(400, 300, format);
    if (format == QImage::Format_Indexed8) {
        QVector<QRgb> table;
        for (int i=0; i<256; ++i)
            table.push_back(qRgb(i,i,i));
        img.fill(gray);
    } else {
        QColor c;
        c.setRgba(fillValue);
        img.fill(c);
        //img.fill(QColor(fillValue)); //This won't work, as it equals c.setRgb(fillValue);
    }

    //(1) CV_8U
    cv::Mat mat = image2Mat(img, CV_8UC(channels), rgbOrder);
    QCOMPARE (mat.channels(), channels);

    if (format == QImage::Format_Indexed8) {
        if (channels == 3) {
            QCOMPARE(mat.at<cv::Vec3b>(1,1), cv::Vec3b(gray,gray,gray));
        } else if (channels == 4) {
            QCOMPARE(mat.at<cv::Vec4b>(1,1), cv::Vec4b(gray,gray,gray, 255));
        } else {
            QVERIFY(abs(mat.at<uchar>(1,1) - uchar(gray)) < 2);
        }
    } else if (channels == 1) {
        qWarning()<<mat.at<uchar>(1,1)<<gray;
        QVERIFY(abs(mat.at<uchar>(1,1) - uchar(gray)) < 2);
    } else if (channels == 3) {
        QCOMPARE(mat.at<cv::Vec3b>(1,1), rgbOrder==MCO_BGR ? cv::Vec3b(0,1,254): cv::Vec3b(254,1,0));
    } else if (channels == 4) {
        int alpha = format == QImage::Format_ARGB32 ? 128 : 255;
        QCOMPARE(mat.at<cv::Vec4b>(1,1)[3], uchar(alpha));
        QCOMPARE(mat.at<cv::Vec4b>(1,1), rgbOrder==MCO_BGR ? cv::Vec4b(0,1,254,alpha):cv::Vec4b(254,1,0,alpha));
    }

    //(2) CV_16U
    cv::Mat mat_16U = image2Mat(img, CV_16UC(channels), rgbOrder);
    cv::Mat mat_16U_expected;
    mat.convertTo(mat_16U_expected, CV_16U, 65535.0/255.0);

    QVERIFY(lenientCompare<unsigned short>(mat_16U, mat_16U_expected));

    //(3) CV_32F
    cv::Mat mat_32F = image2Mat(img, CV_32FC(channels), rgbOrder);
    cv::Mat mat_32F_expected;
    mat.convertTo(mat_32F_expected, CV_32F, 1.0/255.0);

    QVERIFY(lenientCompare<float>(mat_32F, mat_32F_expected));
}

QTEST_MAIN(CvMatAndImageTest)

#include "tst_testcvmatandimagetest.moc"
