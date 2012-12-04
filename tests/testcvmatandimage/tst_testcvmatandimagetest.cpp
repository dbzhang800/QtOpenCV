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
    void testMatb2QImage();
    void testMatf2QImage();
    void testMat2QImageChannelsOrder();

    void testQImage2Mat();
    void testQImage2MatChannelsOrder();

    void testMat2QImageShared();
    void testQImage2MatShared();
};

CvMatAndImageTest::CvMatAndImageTest()
{
}

void CvMatAndImageTest::testMatb2QImage()
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

    cv::Mat mat_8UC3(100, 200, CV_8UC3, cv::Scalar_<uchar>(50, 100, 200));
    cv::Mat mat_8UC3_gray(100, 100, CV_8UC1);
    cv::cvtColor(mat_8UC3, mat_8UC3_gray, CV_BGR2GRAY, 1);

    //CV_8UC3 to QImage::Format_Index8
    QImage img3_index8 = mat2Image(mat_8UC3, QImage::Format_Indexed8);
    QCOMPARE(img3_index8.pixelIndex(1,1), int(mat_8UC3_gray.at<uchar>(1,1)));

    //CV_8UC3 to QImage::Format_RGB32
    QImage img3_rgb32 = mat2Image(mat_8UC3, QImage::Format_RGB32);
    QCOMPARE(img3_rgb32.pixel(1,1), qRgb(200, 100, 50));
}


void CvMatAndImageTest::testMatf2QImage()
{
    cv::Mat mat_32FC1(100, 200, CV_32FC1, cv::Scalar_<float>(0.5));

    QCOMPARE(cv::saturate_cast<uchar>(0.5f*255), uchar(128));

    //CV_32FC1 to QImage::Format_Index8
    QImage img1_index8 = mat2Image(mat_32FC1, QImage::Format_Indexed8);
    QCOMPARE(img1_index8.pixelIndex(50,50), 128);

    //CV_32FC1 to QImage::Format_RGB32
    QImage img1_rgb32 = mat2Image(mat_32FC1, QImage::Format_RGB32);
    QCOMPARE(img1_rgb32.pixel(50,50), qRgb(128, 128, 128));

    cv::Mat mat_32FC3(100, 200, CV_32FC3, cv::Scalar_<float>(0.1f, 0.3f, 0.5f));
    cv::Mat mat_32FC3_gray(100, 100, CV_32FC1);
    cv::cvtColor(mat_32FC3, mat_32FC3_gray, CV_BGR2GRAY, 1);

    //CV_32FC3 to QImage::Format_Index8
    QImage img3_index8 = mat2Image(mat_32FC3, QImage::Format_Indexed8);
    QCOMPARE(img3_index8.pixelIndex(1,1), cv::saturate_cast<int>(mat_32FC3_gray.at<float>(1,1)*255));

    //CV_32FC3 to QImage::Format_RGB32
    QImage img3_rgb32 = mat2Image(mat_32FC3, QImage::Format_RGB32);
    QCOMPARE(qRed(img3_rgb32.pixel(1,1)), cv::saturate_cast<int>(0.5*255));
    QCOMPARE(qBlue(img3_rgb32.pixel(1,1)), cv::saturate_cast<int>(0.1*255));
}

void CvMatAndImageTest::testMat2QImageChannelsOrder()
{
    //This is a red image.
    cv::Mat mat_8UC3(100, 200, CV_8UC3, cv::Scalar_<uchar>(1, 2, 200));
    const char * filename = "tst_data_testchannelsorder.png";
    cv::imwrite(filename, mat_8UC3);

    QImage image0(filename);
    QImage image1 = mat2Image(mat_8UC3, QImage::Format_RGB32);
    QCOMPARE (qRed(image1.pixel(1,1)), 200);
    QCOMPARE (image0.pixel(1,1), image1.pixel(1,1));
}

void CvMatAndImageTest::testQImage2Mat()
{
    QImage img_rgb32 = QImage(100, 100, QImage::Format_RGB32);
    img_rgb32.fill(QColor(2, 1, 255));

    cv::Mat mat = image2Mat(img_rgb32);
    QCOMPARE (mat.type(), CV_8UC4);

    mat = image2Mat(img_rgb32, 1);
    QCOMPARE (mat.type(), CV_8UC1);
}

void CvMatAndImageTest::testQImage2MatChannelsOrder()
{
    //This is a blue image
    QImage img_rgb32 = QImage(100, 100, QImage::Format_RGB32);
    img_rgb32.fill(QColor(2, 1, 255));

    cv::Mat mat = image2Mat(img_rgb32);
    QCOMPARE (mat.channels(), 4);
    QCOMPARE (mat.at<cv::Vec4b>(50,50)[0], uchar(255));
    QCOMPARE (mat.at<cv::Vec4b>(50,50)[2], uchar(2));

    const char * filename = "tst_data_testchannelsorder2.png";
    img_rgb32.save(filename);
    cv::Mat mat2 = cv::imread(filename);
    QCOMPARE (mat2.channels(), 3);
    QCOMPARE (mat2.at<cv::Vec3b>(50,50)[0], uchar(255));
    QCOMPARE (mat2.at<cv::Vec3b>(50,50)[2], uchar(2));
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

void CvMatAndImageTest::testQImage2MatShared()
{
    QImage img_rgb32 = QImage(100, 100, QImage::Format_RGB32);
    img_rgb32.fill(QColor(2, 1, 255));
    cv::Mat mat_8UC4 = image2Mat_shared(img_rgb32);
    QCOMPARE(mat_8UC4.type(), CV_8UC4);

    QImage img_rgb888 = QImage(100, 100, QImage::Format_RGB888);
    img_rgb888.fill(QColor(2, 1, 255));
    cv::Mat mat_8UC3 = image2Mat_shared(img_rgb888);
    QCOMPARE(mat_8UC3.type(), CV_8UC3);
}

QTEST_MAIN(CvMatAndImageTest)

#include "tst_testcvmatandimagetest.moc"
