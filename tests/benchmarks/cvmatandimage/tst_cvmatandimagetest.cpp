#include "../../../cvmatandqimage.cpp"
#include <QString>
#include <QtTest>
#include <QImage>
#include <QDebug>
#include <opencv2/core/core.hpp>

using namespace QtOcv;

class CvMatAndImageTest : public QObject
{
    Q_OBJECT
    
public:
    CvMatAndImageTest();
    
private Q_SLOTS:
    void mat2ImageTemplateUChar_data();
    void mat2ImageTemplateUChar();
    void mat2ImageTemplateFloat_data();
    void mat2ImageTemplateFloat();

    void image2MatTemplateUChar_data();
    void image2MatTemplateUChar();
    /*
    void testMatToImage_data();
    void testMatToImage();

    void testImageToMat_data();
    void testImageToMat();
    */
};

CvMatAndImageTest::CvMatAndImageTest()
{
}

void CvMatAndImageTest::mat2ImageTemplateUChar_data()
{
    QTest::addColumn<bool>("useTemplate");

    QTest::newRow("Template Version")<<true;
    QTest::newRow("Normal Version")<<false;
}

void CvMatAndImageTest::mat2ImageTemplateUChar()
{
    QFETCH(bool, useTemplate);

    cv::Size2i size(4096, 4096);
    const int channel = 3;
    cv::Mat mat(size, CV_8UC(channel), cv::Scalar_<uchar>(254, 1, 0, 128));

    QImage img;
    if (useTemplate) {
        QBENCHMARK {
            img = mat2Image_<uchar>(mat, QImage::Format_ARGB32, QtOcv::MCO_BGRA, 1.0);
        }
    } else {
        QBENCHMARK {
            img = mat2Image(mat, QImage::Format_ARGB32, QtOcv::MCO_BGRA);
        }
    }
}

void CvMatAndImageTest::mat2ImageTemplateFloat_data()
{
    QTest::addColumn<bool>("useTemplate");

    QTest::newRow("Template Version")<<true;
    QTest::newRow("Normal Version")<<false;
}

void CvMatAndImageTest::mat2ImageTemplateFloat()
{
    QFETCH(bool, useTemplate);

    cv::Size2i size(2048, 2048);
    cv::Mat mat(size, CV_32FC4, cv::Scalar_<float>(0.9f, 0.1f, 0.0f, 0.5f));

    QImage img;
    if (useTemplate) {
        QBENCHMARK {
            img = mat2Image_<float>(mat, QImage::Format_ARGB32, QtOcv::MCO_BGRA, 255.);
        }
    } else {
        QBENCHMARK {
            mat.convertTo(mat, CV_8U, 255.);
            img = mat2Image(mat, QImage::Format_ARGB32, QtOcv::MCO_BGRA);
        }
    }
}

void CvMatAndImageTest::image2MatTemplateUChar_data()
{
    QTest::addColumn<bool>("useTemplate");

    QTest::newRow("Template Version")<<true;
    QTest::newRow("Normal Version")<<false;
}

void CvMatAndImageTest::image2MatTemplateUChar()
{
    QFETCH(bool, useTemplate);

    QImage image(2048, 2048, QImage::Format_RGB888);

    if (useTemplate) {
        QBENCHMARK {
            cv::Mat mat = image2Mat_<uchar>(image, CV_8UC4, QtOcv::MCO_BGR, 1.0);
        }
    } else {
        QBENCHMARK {
            cv::Mat mat = image2Mat(image, CV_8UC4, QtOcv::MCO_BGR);
        }
    }
}

/*
void CvMatAndImageTest::testMatToImage_data()
{
    QTest::addColumn<bool>("useShared");
    QTest::addColumn<int>("channels");
    QTest::newRow("shared data convert: Gray") << true << 1;
    QTest::newRow("standard convert: Gray") << false << 1;
    QTest::newRow("shared data convert: RGB888") << true << 3;
    QTest::newRow("standard convert: RGB888") << false << 3;
    QTest::newRow("shared data convert: RGB32") << true << 4;
    QTest::newRow("standard convert:RGB32") << false << 4;
}

void CvMatAndImageTest::testMatToImage()
{
    QFETCH(bool, useShared);
    QFETCH(int, channels);

    const int width = 512;
    cv::Mat mat_8UC1(width, width, CV_8UC1, 10);
    cv::Mat mat_8UC3(width, width, CV_8UC3, cv::Scalar_<uchar>(10,20,30));
    cv::Mat mat_8UC4(width, width, CV_8UC4, cv::Scalar_<uchar>(10,20,30,40));

    QImage img;

    if (!useShared) {
        if (channels == 1) {
            QBENCHMARK {
                img = mat2Image(mat_8UC1, QImage::Format_Indexed8);
            }
        } else if (channels == 3) {
            QBENCHMARK {
                img = mat2Image(mat_8UC3, QImage::Format_RGB888);
            }
        } else {
            QBENCHMARK {
                img = mat2Image(mat_8UC4, QImage::Format_RGB32);
            }
        }
    } else {
        if (channels == 1) {
            QBENCHMARK {
                img = mat2Image_shared(mat_8UC1);
            }
        } else if (channels == 3) {
            QBENCHMARK {
                img = mat2Image_shared(mat_8UC3);
            }
        } else {
            QBENCHMARK {
                img = mat2Image_shared(mat_8UC4);
            }
        }
    }
}

void CvMatAndImageTest::testImageToMat_data()
{
    QTest::addColumn<bool>("useShared");
    QTest::addColumn<int>("channels");
    QTest::newRow("shared data convert: Gray") << true << 1;
    QTest::newRow("standard convert: Gray") << false << 1;
    QTest::newRow("shared data convert: RGB888") << true << 3;
    QTest::newRow("standard convert: RGB888") << false << 3;
    QTest::newRow("shared data convert: RGB32") << true << 4;
    QTest::newRow("standard convert:RGB32") << false << 4;
}

void CvMatAndImageTest::testImageToMat()
{
    QFETCH(bool, useShared);
    QFETCH(int, channels);

    QImage index8 = QImage(512, 512, QImage::Format_Indexed8);
    QImage rgb888 = QImage(512, 512, QImage::Format_RGB888);
    QImage rgb32 = QImage(512, 512, QImage::Format_RGB32);

    cv::Mat mat;
    if (!useShared) {
        if (channels == 1) {
            QBENCHMARK {
                mat = image2Mat(index8);
            }
        } else if (channels == 3) {
            QBENCHMARK {
                mat = image2Mat(rgb888);
            }
        } else {
            QBENCHMARK {
                mat = image2Mat(rgb32);
            }
        }
    } else {
        if (channels == 1) {
            QBENCHMARK {
                mat = image2Mat_shared(index8);
            }
        } else if (channels == 3) {
            QBENCHMARK {
                mat = image2Mat_shared(rgb888);
            }
        } else {
            QBENCHMARK {
                mat = image2Mat_shared(rgb32);
            }
        }
    }
}
*/
QTEST_MAIN(CvMatAndImageTest)

#include "tst_cvmatandimagetest.moc"
