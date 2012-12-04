#ifndef CAMERADEVICE_H
#define CAMERADEVICE_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QTimer;
class QImage;
QT_END_NAMESPACE

namespace cv{
    class VideoCapture;
    class Mat;
}

class CameraDevice : public QObject
{
    Q_OBJECT
public:
    explicit CameraDevice(QObject *parent = 0);
    ~CameraDevice();

signals:
    void imageReady(const QImage& image);

public slots:
    bool start();
    bool stop();

private slots:
    void onTimeout();

private:
    cv::VideoCapture * m_capture;
    QTimer * m_timer;
};

#endif // CAMERADEVICE_H
