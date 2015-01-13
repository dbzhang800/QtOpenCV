#include "dialog.h"
#include "ui_dialog.h"

#include "cvmatandqimage.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QFileDialog>
#include <QSettings>
#include <QDebug>

#include <vector>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    connect(ui->openButton, SIGNAL(clicked()), SLOT(onOpenButtonClicked()));
    connect(ui->lowThresholdEdit, SIGNAL(valueChanged(int)), SLOT(onLowThresHoldEditValueChanged()));
    connect(ui->groupBox, SIGNAL(toggled(bool)), SLOT(onGroupBoxToggled()));

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
    if (img.isNull())
        return;

    settings.setValue("lastPath", filename);
    currentImage = img;

    QtOcv::MatColorOrder order;
    cv::Mat mat0 = QtOcv::image2Mat(img, CV_8U, &order);

    //Make sure currentMat is gray
    if (mat0.channels() == 3)
        cv::cvtColor(mat0, currentMat, order == QtOcv::MCO_BGR ? CV_BGR2GRAY : CV_RGB2GRAY);
    else if (mat0.channels() == 4)
        cv::cvtColor(mat0, currentMat, order == QtOcv::MCO_BGRA ? CV_BGRA2GRAY : CV_RGBA2GRAY);
    else
        currentMat = mat0;

    updateImage();
}

void Dialog::onLowThresHoldEditValueChanged()
{
    ui->maxThresholdEdit->setValue(ui->lowThresholdEdit->value()*3);
    updateImage();
}

void Dialog::onGroupBoxToggled()
{
    updateImage();
}

void Dialog::updateImage()
{
    if (currentImage.isNull()) {
        ui->imageWidget->setPixmap(QPixmap());
        return;
    }

    if (!ui->groupBox->isChecked()) {
        ui->imageWidget->setImage(currentImage);
    } else {
        int kernel_size = 3;

        cv::Mat detected_edges;
        /// Reduce noise with a kernel 3x3
        cv::blur(currentMat, detected_edges, cv::Size(3,3));
        /// Canny detector
        cv::Canny(detected_edges, detected_edges
                  , ui->lowThresholdEdit->value()
                  , ui->maxThresholdEdit->value()
                  , kernel_size);

        std::vector<std::vector<cv::Point> > contours;
        cv::findContours(detected_edges, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
        cv::Mat cimage = cv::Mat::zeros(detected_edges.size(), CV_8UC3);
        for(size_t i = 0; i < contours.size(); i++)
        {
            size_t count = contours[i].size();
            if( count < 6 )
                continue;

            cv::Mat pointsf;
            cv::Mat(contours[i]).convertTo(pointsf, CV_32F);
            cv::RotatedRect box = cv::fitEllipse(pointsf);

            if( MAX(box.size.width, box.size.height) > MIN(box.size.width, box.size.height)*30 )
                continue;
            cv::drawContours(cimage, contours, (int)i, cv::Scalar::all(255), 1, 8);

            cv::ellipse(cimage, box, cv::Scalar(0,0,255), 1, CV_AA);
            cv::ellipse(cimage, box.center, box.size*0.5f, box.angle, 0, 360, cv::Scalar(0,255,255), 1, CV_AA);
            cv::Point2f vtx[4];
            box.points(vtx);
            for( int j = 0; j < 4; j++ )
                cv::line(cimage, vtx[j], vtx[(j+1)%4], cv::Scalar(0,255,0), 1, CV_AA);
//            qDebug()<< "Center(" << box.center.x << "," << box.center.y << ")";
//            qDebug()<< "Size(" << box.size.width << ", " << box.size.height << ")";
//            qDebug()<< "Angle " << box.angle;
        }


        ui->imageWidget->setImage(QtOcv::mat2Image(cimage));
    }
}
