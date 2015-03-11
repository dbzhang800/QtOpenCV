#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("OpenCV_ImageProcess");
    a.setOrganizationName("Debao");
    MainWindow w;
    w.showMaximized();

    return a.exec();
}
