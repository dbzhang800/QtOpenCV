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
    void testMatToImage_data();
    void testMatToImage();

    void testImageToMat_data();
    void testImageToMat();
};

CvMatAndImageTest::CvMatAndImageTest()
{
}

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

    const int width = 2048;
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
                img = mat2Image(mat_8UC3, QImage::Format_RGB888, MCO_RGB);
            }
        } else {
            QBENCHMARK {
                img = mat2Image(mat_8UC4, QImage::Format_RGB32, MCO_BGRA);
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

    const int w = 2048;
    QImage index8 = QImage(w, w, QImage::Format_Indexed8);
    QImage rgb888 = QImage(w, w, QImage::Format_RGB888);
    QImage rgb32 = QImage(w, w, QImage::Format_RGB32);

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

QTEST_MAIN(CvMatAndImageTest)

#include "tst_cvmatandimagetest.moc"
