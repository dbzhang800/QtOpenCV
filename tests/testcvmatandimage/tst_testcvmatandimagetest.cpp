#include "cvmatandqimage.h"
#include <QString>
#include <QtTest>
#include <QTemporaryFile>
#include <QDebug>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace QtOcv;

class CvMatAndImageTest : public QObject
{
    Q_OBJECT
    
public:
    CvMatAndImageTest();
    
private Q_SLOTS:
    void testQImageChannelsOrder(); //Just for ...
    void testMatChannelsOrder(); //Just for ...

    void testMat2QImage();
    void testMat2QImageShared();
    void testMat2QImageChannelsOrder();

    void testQImage2Mat();
    void testQImage2MatShared();
    void testQImage2MatChannelsOrder();
};

CvMatAndImageTest::CvMatAndImageTest()
{
}

void CvMatAndImageTest::testQImageChannelsOrder()
{
    //Note the channels order: R G B
    QImage redImage_rgb888(400, 300, QImage::Format_RGB888);
    redImage_rgb888.fill(qRgb(254,1,0));
    QCOMPARE(QByteArray(reinterpret_cast<char*>(redImage_rgb888.bits()), 6), QByteArray("\xfe\x01\x00\xfe\x01\x00", 6));

    //Note the channels order: B G R X
    QImage redImage_rgb32(400, 300, QImage::Format_RGB32);
    redImage_rgb32.fill(qRgb(254,1,0));
    QCOMPARE(QByteArray(reinterpret_cast<char*>(redImage_rgb32.bits()), 4), QByteArray("\x00\x01\xfe\xff", 4));

    //Note the channels order: B G R A
    QImage redImage_argb32(400, 300, QImage::Format_ARGB32);
    redImage_argb32.fill(qRgba(254,1,0, 128));
    QCOMPARE(QByteArray(reinterpret_cast<char*>(redImage_argb32.bits()), 4), QByteArray("\x00\x01\xfe\x80", 4));
}

void CvMatAndImageTest::testMatChannelsOrder()
{
    //generate a red-color image, then save as a png format image file
    QImage redImage(400, 300, QImage::Format_RGB888);
    redImage.fill(qRgb(255,0,0)); // * R G B *
    const char* redImage_filename = "tst_data_testchannelsorder.png";
    redImage.save(redImage_filename);

    //load this image with highgui's method, note that the order is B G R instead of R G B
    cv::Mat redMat = cv::imread(redImage_filename);
    QVERIFY(redMat.channels() == 3);
    QCOMPARE(redMat.at<cv::Vec3b>(1,1), cv::Vec3b(0,0,255)); // * B G R *
    QCOMPARE(redMat.at<cv::Vec3b>(1,1)[2], uchar(255));

    QCOMPARE(QByteArray(reinterpret_cast<char*>(redImage.bits()), 6), QByteArray("\xff\x00\x00\xff\x00\x00", 6));
    QCOMPARE(QByteArray(reinterpret_cast<char*>(redMat.data),     6), QByteArray("\x00\x00\xff\x00\x00\xff", 6));
}

void CvMatAndImageTest::testMat2QImage()
{
    cv::Mat mat_8UC1(100, 200, CV_8UC1);
    for (int i=0; i<100; ++i) {
        for (int j=0; j<200; ++j)
            mat_8UC1.at<uchar>(i,j) = (i*j)%255;
    }

    //CV_8UC1 to QImage::Format_Index8
    QImage img1_index8 = mat2Image(mat_8UC1, QImage::Format_Indexed8);
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
    QImage img3_rgb32 = mat2Image(mat_8UC3_bgr, QImage::Format_RGB32);
    QCOMPARE(img3_rgb32.pixel(1,1), qRgb(254, 1, 0));
}

void CvMatAndImageTest::testMat2QImageShared()
{
    cv::Mat mat_8UC1(100, 200, CV_8UC1);
    for (int i=0; i<100; ++i) {
        for (int j=0; j<200; ++j)
            mat_8UC1.at<uchar>(i,j) = (i*j)%255;
    }

    //CV_8UC1 to QImage::Format_Index8
    QImage img1_index8 = mat2Image_shared(mat_8UC1);
    QCOMPARE(img1_index8.pixelIndex(3,50), 150);
    QCOMPARE(img1_index8, mat2Image(mat_8UC1, QImage::Format_Indexed8));

    cv::Mat mat_8UC3;
    std::vector<cv::Mat> channels;
    channels.push_back(mat_8UC1);
    channels.push_back(mat_8UC1);
    channels.push_back(mat_8UC1);
    cv::merge(channels, mat_8UC3);

    //CV_8UC3 to QImage::Format_RGB888
    QImage img1_rgb888 = mat2Image_shared(mat_8UC3);
    QCOMPARE(img1_rgb888.format(), QImage::Format_RGB888);
    QCOMPARE(img1_rgb888.pixel(3, 50), qRgb(150, 150, 150));

    cv::Mat mat_8UC4;
    channels.push_back(cv::Mat(100, 200, CV_8UC1, cv::Scalar_<uchar>(128)));
    cv::merge(channels, mat_8UC4);

    QImage img1_argb32 = mat2Image_shared(mat_8UC4);
    QCOMPARE(img1_argb32.format(), QImage::Format_ARGB32);
    QCOMPARE(img1_argb32.pixel(3, 50), qRgba(150, 150, 150, 128));
}

void CvMatAndImageTest::testMat2QImageChannelsOrder()
{
    //This is a red image. channels in B G R order
    cv::Mat mat_8UC3_bgr(400, 300, CV_8UC3, cv::Scalar_<uchar>(0, 0, 255));
    //This is another red image, channels in R G B order
    cv::Mat mat_8UC3_rgb(400, 300, CV_8UC3, cv::Scalar_<uchar>(255, 0, 0));

    QImage image_rgb888 = mat2Image(mat_8UC3_bgr, QImage::Format_RGB888, BGR);
    QCOMPARE(image_rgb888.pixel(1,1), qRgb(255, 0, 0));

    image_rgb888 = mat2Image(mat_8UC3_rgb, QImage::Format_RGB888, RGB);
    QCOMPARE(image_rgb888.pixel(1,1), qRgb(255, 0, 0));

    QImage image_rgb32 = mat2Image(mat_8UC3_bgr, QImage::Format_RGB32, BGR);
    QCOMPARE(image_rgb32.pixel(1,1), qRgb(255, 0, 0));

    image_rgb32 = mat2Image(mat_8UC3_rgb, QImage::Format_RGB32, RGB);
    QCOMPARE(image_rgb32.pixel(1,1), qRgb(255, 0, 0));

    QImage image2 = mat2Image_shared(mat_8UC3_rgb);
    QCOMPARE (image2.pixel(1,1), qRgb(255, 0, 0));
}

void CvMatAndImageTest::testQImage2Mat()
{
    QImage img_rgb32 = QImage(100, 100, QImage::Format_RGB32);
    img_rgb32.fill(QColor(254, 1, 0));

    cv::Mat mat = image2Mat(img_rgb32);
    QCOMPARE (mat.type(), CV_8UC4);

    mat = image2Mat(img_rgb32, 1);
    QCOMPARE (mat.type(), CV_8UC1);

    QImage img_rgb888 = QImage(100, 100, QImage::Format_RGB888);
    img_rgb888.fill(QColor(254, 1, 0));
    mat = image2Mat(img_rgb888);
    QCOMPARE (mat.type(), CV_8UC3);

    mat = image2Mat(img_rgb888, 1);
    QCOMPARE (mat.type(), CV_8UC1);
}


void CvMatAndImageTest::testQImage2MatShared()
{
    QImage img_rgb32 = QImage(100, 100, QImage::Format_RGB32);
    img_rgb32.fill(QColor(254, 1, 0));
    cv::Mat mat_8UC4 = image2Mat_shared(img_rgb32);
    QCOMPARE(mat_8UC4.type(), CV_8UC4);

    QImage img_rgb888 = QImage(100, 100, QImage::Format_RGB888);
    img_rgb888.fill(QColor(254, 1, 0));
    cv::Mat mat_8UC3 = image2Mat_shared(img_rgb888);
    QCOMPARE(mat_8UC3.type(), CV_8UC3);
}

void CvMatAndImageTest::testQImage2MatChannelsOrder()
{
    //This is a blue image
    QImage img_rgb32 = QImage(100, 100, QImage::Format_RGB32);
    img_rgb32.fill(QColor(254, 1, 0));

    cv::Mat mat = image2Mat(img_rgb32, 0, BGRA);
    QCOMPARE (mat.channels(), 4);
    QCOMPARE (mat.at<cv::Vec4b>(50,50), cv::Vec4b(0,1,254,255));

    mat = image2Mat(img_rgb32, 0, RGBA);
    QCOMPARE (mat.channels(), 4);
    QCOMPARE (mat.at<cv::Vec4b>(50,50), cv::Vec4b(254,1,0,255));

    mat = image2Mat_shared(img_rgb32);
    QCOMPARE (mat.channels(), 4);
    QCOMPARE (mat.at<cv::Vec4b>(50,50), cv::Vec4b(0,1,254,255));
}

QTEST_MAIN(CvMatAndImageTest)

#include "tst_testcvmatandimagetest.moc"
