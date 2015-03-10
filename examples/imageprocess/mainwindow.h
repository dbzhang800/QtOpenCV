#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include "cvmatandqimage.h"

namespace Ui {
class MainWindow;
}
class RecentFiles;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onFileOpenActionTriggered();
    void onRecentFilesTriggered(const QString &filePath);
    void onImageActionTriggered();
    void onFilterApplyButtonClicked();

private:
    void closeEvent(QCloseEvent *evt);
    void loadSettings();
    void saveSettings();
    void createImageAction(int id, const QString &text);
    void createImageActions();
    void createFilterWidgets();
    void doOpen(const QString &filePath);

    Ui::MainWindow *ui;
    RecentFiles *m_recentFiles;
    QMap<int, QAction*> m_imageActions;
    cv::Mat m_originalMat;
    cv::Mat m_processMat;
    int m_filterId;
};

#endif // MAINWINDOW_H
