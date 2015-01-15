#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "cvmatandqimage.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <QFileDialog>
#include <QSettings>
#include <QTime>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->lowThresholdEdit, SIGNAL(valueChanged(int)), SLOT(updateImage()));
    connect(ui->maxThresholdEdit, SIGNAL(valueChanged(int)), SLOT(updateImage()));
    connect(ui->ellipseMaxCountEdit, SIGNAL(valueChanged(int)), SLOT(updateImage()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_Open_triggered()
{
    QSettings settings("QtOpenCV_example.ini", QSettings::IniFormat);
    QString lastPath = settings.value("lastPath").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), lastPath, "Images(*.png *.bmp *.jpg *.gif)");
    if (fileName.isEmpty())
        return;

    QImage img(fileName);
    if (img.isNull()) {
        currentImage_RGB888 = QImage();
    } else {
        settings.setValue("lastPath", fileName);
        currentImage_RGB888 = img.convertToFormat(QImage::Format_RGB888);
        currentMat_8UC1 = QtOcv::image2Mat(currentImage_RGB888, CV_8UC1);
    }
    ui->imageWidget->setImage(img);
    setWindowTitle(fileName);
    updateImage();
}

void MainWindow::updateImage()
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

    for(size_t i = 0; i < contours.size(); i++) {
        //Draw contours on the cannyImage.
        cv::drawContours(cannyImage, contours, (int)i, cv::Scalar::all(255), 1, 8);
    }

    std::sort(contours.begin(), contours.end(), [](std::vector<cv::Point> a, std::vector<cv::Point> b) {
        return b.size() < a.size();
    });

    for(size_t i = 0, foundCount = 0; (i < contours.size()) && (int(foundCount) < ui->ellipseMaxCountEdit->value()); i++) {
        size_t count = contours[i].size();

        if( count < 10 )
            continue;

        cv::Mat pointsf;
        cv::Mat(contours[i]).convertTo(pointsf, CV_32F);
        cv::RotatedRect box = cv::fitEllipse(pointsf);

        if( qMax(box.size.width, box.size.height) > qMin(box.size.width, box.size.height)*30 )
            continue;

        foundCount++;

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
    qDebug()<<"Run... "<<t.elapsed()<<"ms";

    ui->imageWidget_canny->setImage(QtOcv::mat2Image(cannyImage));
    ui->imageWidget_ellipse->setImage(QtOcv::mat2Image(ellipseImage));
}

