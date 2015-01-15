#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/core/core.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_action_Open_triggered();
    void updateImage();

private:
    Ui::MainWindow *ui;

    QImage currentImage_RGB888;
    cv::Mat currentMat_8UC1;
};

#endif // MAINWINDOW_H
