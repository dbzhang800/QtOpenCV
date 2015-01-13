#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cvmatandqimage.h"

#include <QFileDialog>
#include <QImage>
#include <QDebug>
#include <QSettings>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
    if (img.isNull())
        return;
    settings.setValue("lastPath", fileName);

    QImage image = img.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat = QtOcv::image2Mat_shared(image);
    std::vector<cv::Mat> channels;
    cv::split(mat, channels);

    ui->originImageWidget->setImage(image);
    ui->rImageWidget->setImage(QtOcv::mat2Image_shared(channels[0]));
    ui->gImageWidget->setImage(QtOcv::mat2Image_shared(channels[1]));
    ui->bImageWidget->setImage(QtOcv::mat2Image_shared(channels[2]));

    ui->originImageWidget->setCurrentScale(0);
    ui->rImageWidget->setCurrentScale(0);
    ui->gImageWidget->setCurrentScale(0);
    ui->bImageWidget->setCurrentScale(0);
}
