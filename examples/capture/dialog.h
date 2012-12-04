#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
    class Dialog;
}

class CameraDevice;

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void onImageArrival(const QImage & image);

private:
    Ui::Dialog *ui;
    CameraDevice * m_camera;
};

#endif // DIALOG_H
