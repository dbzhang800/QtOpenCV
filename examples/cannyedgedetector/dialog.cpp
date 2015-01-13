#include "dialog.h"
#include "ui_dialog.h"

#include "cvmatandqimage.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QFileDialog>
#include <QSettings>
#include <QDebug>
#include <QTime>

#include <vector>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    connect(ui->openButton, SIGNAL(clicked()), SLOT(onOpenButtonClicked()));
    connect(ui->lowThresholdEdit, SIGNAL(valueChanged(int)), SLOT(onLowThresHoldEditValueChanged()));

    ui->lowThresholdEdit->setValue(10);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::onOpenButtonClicked()
{
    QSettings settings("QtOpenCV_example.ini");
    QString lastPath = settings.value("lastPath").toString();

    QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"), lastPath, "Images(*.png *.bmp *.jpg *.gif)");
    if (filename.isEmpty())
        return;

    QImage img(filename);
    if (img.isNull()) {
        currentImage_RGB888 = QImage();
    } else {
        settings.setValue("lastPath", filename);
        currentImage_RGB888 = img.convertToFormat(QImage::Format_RGB888);
        currentMat_8UC1 = QtOcv::image2Mat(currentImage_RGB888, CV_8UC1);
    }
    ui->imageWidget->setImage(img);
    updateImage();
}

void Dialog::onLowThresHoldEditValueChanged()
{
    ui->maxThresholdEdit->setValue(ui->lowThresholdEdit->value()*3);
    updateImage();
}

void Dialog::updateImage()
{
    ui->ellipseInfoEdit->clear();

    if (currentImage_RGB888.isNull()) {
        ui->imageWidget_canny->setPixmap(QPixmap());
        ui->imageWidget_ellipse->setPixmap(QPixmap());
        return;
    }

    QTime t;
    t.start();

    int kernel_size = 3;

    cv::Mat detected_edges;
    /// Reduce noise with a kernel 3x3
    cv::blur(currentMat_8UC1, detected_edges, cv::Size(3,3));
    /// Canny detector
    cv::Canny(detected_edges, detected_edges
              , ui->lowThresholdEdit->value()
              , ui->maxThresholdEdit->value()
              , kernel_size);

    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(detected_edges, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

    cv::Mat cannyImage = QtOcv::image2Mat(currentImage_RGB888, CV_8UC3, QtOcv::MCO_BGR);
    cv::Mat ellipseImage = cannyImage.clone();
    for(size_t i = 0; i < contours.size(); i++)
    {
        cv::drawContours(cannyImage, contours, (int)i, cv::Scalar::all(255), 1, 8);

        size_t count = contours[i].size();
        if( count < 10 )
            continue;

        cv::Mat pointsf;
        cv::Mat(contours[i]).convertTo(pointsf, CV_32F);
        cv::RotatedRect box = cv::fitEllipse(pointsf);

        if( MAX(box.size.width, box.size.height) > MIN(box.size.width, box.size.height)*30 )
            continue;

        cv::ellipse(ellipseImage, box, cv::Scalar(0,0,255), 1, CV_AA);
        cv::ellipse(ellipseImage, box.center, box.size*0.5f, box.angle, 0, 360, cv::Scalar(0,255,255), 1, CV_AA);
        cv::Point2f vtx[4];
        box.points(vtx);
        for( int j = 0; j < 4; j++ )
            cv::line(ellipseImage, vtx[j], vtx[(j+1)%4], cv::Scalar(0,255,0), 1, CV_AA);

        QString ellipseInfo = QString("Center(%1, %2) Size(%3, %4) Angle %5")
                .arg(box.center.x).arg(box.center.y)
                .arg(box.size.width).arg(box.size.height)
                .arg(box.angle);
        ui->ellipseInfoEdit->appendPlainText(ellipseInfo);
    }
    qDebug()<<t.elapsed();

    ui->imageWidget_canny->setImage(QtOcv::mat2Image(cannyImage));
    ui->imageWidget_ellipse->setImage(QtOcv::mat2Image(ellipseImage));
}
