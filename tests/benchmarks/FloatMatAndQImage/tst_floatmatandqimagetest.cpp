#include "../../../cvmatandqimage.cpp"
#include <QString>
#include <QtTest>
#include <QImage>
#include <QDebug>
#include <opencv2/core/core.hpp>

using namespace QtOcv;

class FloatMatAndQImageTest : public QObject
{
    Q_OBJECT
    
public:
    FloatMatAndQImageTest();
    
private Q_SLOTS:
    void floatMat2QImage_data();
    void floatMat2QImage();
    void qImage2floatMat_data();
    void qImage2floatMat();
};

FloatMatAndQImageTest::FloatMatAndQImageTest()
{
}

void FloatMatAndQImageTest::floatMat2QImage_data()
{
    QTest::addColumn<bool>("useShared");
    QTest::addColumn<bool>("directly");

    QTest::newRow("Shared Version")<<true<<false;
    QTest::newRow("Normal Direct Version")<<false<<true;
    QTest::newRow("First 8U Then 32F")<<false<<false;
}

void FloatMatAndQImageTest::floatMat2QImage()
{
    QFETCH(bool, useShared);
    QFETCH(bool, directly);

    cv::Size2i size(4096, 4096);
    const int channel = 3;
    cv::Mat mat_rgb(size, CV_32FC(channel), cv::Scalar_<float>(0.9f, 0.01f, 0.0f, 0.5f));

    QImage img0;

    if (directly){
        QBENCHMARK {
            img0 = mat2Image(mat_rgb, QImage::Format_RGB888, QtOcv::MCO_RGB);
        }
    } else {
        QBENCHMARK {
            mat_rgb.convertTo(mat_rgb, CV_8U, 255.0);
            img0 = useShared ? mat2Image_shared(mat_rgb)
                      : mat2Image(mat_rgb, QImage::Format_RGB888, QtOcv::MCO_RGB);
        }
    }
}

void FloatMatAndQImageTest::qImage2floatMat_data()
{
    QTest::addColumn<bool>("useShared");

    QTest::newRow("Shared Version")<<true;
    QTest::newRow("Normal Version")<<false;
}

void FloatMatAndQImageTest::qImage2floatMat()
{
    QFETCH(bool, useShared);

    QImage img(4096, 4096, QImage::Format_RGB888);

    cv::Mat mat;
    if (useShared) {
        QBENCHMARK {
            mat = image2Mat_shared(img);
            mat.convertTo(mat, CV_32F, 1.0/255.0);
        }
    } else {
        QBENCHMARK {
            mat = image2Mat(img, CV_32FC(3), MCO_RGB);
        }
    }
}

QTEST_MAIN(FloatMatAndQImageTest)

#include "tst_floatmatandqimagetest.moc"
