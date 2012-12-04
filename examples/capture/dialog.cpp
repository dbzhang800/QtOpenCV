#include "dialog.h"
#include "ui_dialog.h"
#include "cameradevice.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::Dialog), m_camera(new CameraDevice(this))
{
    ui->setupUi(this);

    connect(m_camera, SIGNAL(imageReady(QImage)), this, SLOT(onImageArrival(QImage)));
    connect(ui->startButton, SIGNAL(clicked()), m_camera, SLOT(start()));
    connect(ui->stopButton, SIGNAL(clicked()), m_camera, SLOT(stop()));
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::onImageArrival(const QImage &image)
{
    ui->view->setPixmap(QPixmap::fromImage(image));
}
