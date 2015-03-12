#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QSharedPointer>
#include "cvmatandqimage.h"

namespace Ui {
class MainWindow;
}
class RecentFiles;
class AbstractConvert;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onFileOpenActionTriggered();
    void onRecentFilesTriggered(const QString &filePath);
    void onFileSaveActionTriggered();
    void onFileSaveAsActionTriggered();
    void onImageActionTriggered();
    void onFilterPreviewButtonClicked();
    void onFilterApplyButtonClicked();
    void onColorUnderMouseChanged(const QColor &c);

private:
    void closeEvent(QCloseEvent *evt);
    void loadSettings();
    void saveSettings();
    void createImageAction(int id, const QString &text);
    void createImageActions();
    void doOpen(const QString &filePath);

    Ui::MainWindow *ui;
    RecentFiles *m_recentFiles;
    QMap<int, QAction*> m_imageActions;
    cv::Mat m_originalMat;
    cv::Mat m_processMat;
    QSharedPointer<AbstractConvert> m_convert;
};

#endif // MAINWINDOW_H
