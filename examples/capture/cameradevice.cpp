#include "cameradevice.h"
#include <QTimer>
#include <QImage>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "cvmatandqimage.h"

CameraDevice::CameraDevice(QObject *parent) :
    QObject(parent)
{
    m_capture = new cv::VideoCapture;
    m_timer = new QTimer(this);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

CameraDevice::~CameraDevice()
{
    delete m_capture;
    m_capture = NULL;
}

bool CameraDevice::start()
{
    if (m_capture->isOpened())
        return true;

    m_capture->open(CV_CAP_ANY);
    if (m_capture->isOpened())
        m_timer->start(40);

    return m_capture->isOpened();
}

bool CameraDevice::stop()
{
    if (m_capture->isOpened())
        m_capture->release();

    return true;
}

void CameraDevice::onTimeout()
{
    if (!m_capture->isOpened())
        return;

    static cv::Mat frame;
    *m_capture >> frame;
    if (frame.cols)
        emit imageReady(QtOcv::mat2Image(frame));
}
