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

    connect(ui->originImageWidget, SIGNAL(colorUnderMouseChanged(QColor)), SLOT(onColorUnderMouseChanged(QColor)));
    connect(ui->rImageWidget, SIGNAL(colorUnderMouseChanged(QColor)), SLOT(onColorUnderMouseChanged(QColor)));
    connect(ui->gImageWidget, SIGNAL(colorUnderMouseChanged(QColor)), SLOT(onColorUnderMouseChanged(QColor)));
    connect(ui->bImageWidget, SIGNAL(colorUnderMouseChanged(QColor)), SLOT(onColorUnderMouseChanged(QColor)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onColorUnderMouseChanged(const QColor &c)
{
    if (c.isValid())
        statusBar()->showMessage(QString("R %1 G %2 B %3").arg(c.red()).arg(c.green()).arg(c.blue()));
    else
        statusBar()->clearMessage();
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
    setWindowTitle(fileName);
    ui->originImageWidget->setImage(img);
    ui->originImageWidget->setCurrentScale(0);

    QImage image = img.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat = QtOcv::image2Mat_shared(image);
#if 0
    std::vector<cv::Mat> channels;
    cv::split(mat, channels);

    ui->rImageWidget->setImage(QtOcv::mat2Image_shared(channels[0]));
    ui->gImageWidget->setImage(QtOcv::mat2Image_shared(channels[1]));
    ui->bImageWidget->setImage(QtOcv::mat2Image_shared(channels[2]));
#else
    cv::Mat rMat(mat.rows, mat.cols, mat.type(), cv::Scalar(0, 0, 0));
    cv::Mat gMat = rMat.clone();
    cv::Mat bMat = rMat.clone();
    int r[] = {0, 0};
    int g[] = {1, 1};
    int b[] = {2, 2};
    cv::mixChannels(&mat, 1, &rMat, 1, r, 1);
    cv::mixChannels(&mat, 1, &gMat, 1, g, 1);
    cv::mixChannels(&mat, 1, &bMat, 1, b, 1);

    ui->rImageWidget->setImage(QtOcv::mat2Image_shared(rMat));
    ui->gImageWidget->setImage(QtOcv::mat2Image_shared(gMat));
    ui->bImageWidget->setImage(QtOcv::mat2Image_shared(bMat));
#endif

    ui->rImageWidget->setCurrentScale(0);
    ui->gImageWidget->setCurrentScale(0);
    ui->bImageWidget->setCurrentScale(0);
}
