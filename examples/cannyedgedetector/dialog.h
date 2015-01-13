#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <opencv2/core/core.hpp>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
    void onOpenButtonClicked();
    void onLowThresHoldEditValueChanged();

private:
    void updateImage();

    Ui::Dialog *ui;
    QImage currentImage_RGB888;
    cv::Mat currentMat_8UC1;
};

#endif // DIALOG_H
