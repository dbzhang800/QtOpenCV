#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "recentfiles.h"

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QCloseEvent>
#include <QSettings>
#include <QFileDialog>

enum
{
    ConvertToGray,
    //vvvvvvvvv
    BilateralFilter,
    Blur,
    BoxFilter,
    Filter2D,
    GaussianBlur,
    MedianBlur,
    //^^^^^^^^^
    ReplaceOriginal
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_recentFiles(new RecentFiles(this)), m_filterId(-1)
{
    ui->setupUi(this);
    ui->actionRecentFiles->setMenu(m_recentFiles->menu());

    createImageActions();
    createFilterWidgets();

    connect(m_recentFiles, SIGNAL(activated(QString)), SLOT(onRecentFilesTriggered(QString)));
    connect(ui->actionOpen, SIGNAL(triggered()), SLOT(onFileOpenActionTriggered()));
    connect(ui->filterApplyButton, SIGNAL(clicked()), SLOT(onFilterApplyButtonClicked()));
    ui->filterDockWidget->setVisible(false);
    ui->filterDockWidget->setEnabled(false);
    loadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onFileOpenActionTriggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), m_recentFiles->mostRecentFile(), "Images(*.png *.bmp *.jpg *.gif)");
    if (fileName.isEmpty())
        return;
    doOpen(fileName);
}

void MainWindow::onRecentFilesTriggered(const QString &filePath)
{
    doOpen(filePath);
}

void MainWindow::onImageActionTriggered()
{
    if (m_originalMat.empty())
        return;

    QAction *act = qobject_cast<QAction *>(sender());
    m_filterId = act->property("id").toInt();
    switch (m_filterId) {
    case ConvertToGray:
        cv::cvtColor(m_originalMat, m_processMat, CV_RGB2GRAY);
        ui->processView->setImage(QtOcv::mat2Image_shared(m_processMat));
        break;
    case Blur:
        break;
    case ReplaceOriginal:
        ui->originalView->setPixmap(ui->processView->pixmap());
        m_originalMat = m_processMat.clone();
        break;
    default:
        break;
    }

    bool visible = m_filterId != ConvertToGray && m_filterId != ReplaceOriginal;
    ui->filterDockWidget->setVisible(visible);
    ui->filterDockWidget->setEnabled(visible);
    ui->filterDockWidget->setWindowTitle(act->text());

    //Update actions status.
    m_imageActions[ConvertToGray]->setEnabled(m_originalMat.channels() > 1);
}

void MainWindow::onFilterApplyButtonClicked()
{
    switch (m_filterId) {
    case Blur:
        cv::blur(m_originalMat, m_processMat, cv::Size(ui->filter_kSizeXEdit->value(), ui->filter_kSizeYEdit->value()),
                 cv::Point(ui->filter_anchorXEdit->value(), ui->filter_anchorYEdit->value()),
                 ui->filter_borderTypeEdit->currentData().toInt());
        break;
    case BilateralFilter:
        cv::bilateralFilter(m_originalMat, m_processMat, ui->filter_dEdit->value(), ui->filter_sigmaColorEdit->value(),
                            ui->filter_sigmaSpaceEdit->value(), ui->filter_borderTypeEdit->currentData().toInt());
        break;
    case BoxFilter:
        cv::boxFilter(m_originalMat, m_processMat, -1, cv::Size(ui->filter_kSizeXEdit->value(), ui->filter_kSizeYEdit->value()),
                      cv::Point(ui->filter_anchorXEdit->value(), ui->filter_anchorYEdit->value()), ui->filter_normalizeEdit->currentIndex(),
                      ui->filter_borderTypeEdit->currentData().toInt());
        break;
    case Filter2D:
    {
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(ui->filter_kSizeXEdit->value(), ui->filter_kSizeYEdit->value()),
                      cv::Point(ui->filter_anchorXEdit->value(), ui->filter_anchorYEdit->value()));
        cv::filter2D(m_originalMat, m_processMat, -1, kernel,
                     cv::Point(ui->filter_anchorXEdit->value(), ui->filter_anchorYEdit->value()),
                     ui->filter_deltaEdit->value(),
                     ui->filter_borderTypeEdit->currentData().toInt());
    }
        break;
    case GaussianBlur:
        cv::GaussianBlur(m_originalMat, m_processMat, cv::Size(ui->filter_kSizeXEdit->value(), ui->filter_kSizeYEdit->value()),
                         ui->filter_sigmaXEdit->value(), ui->filter_sigmaYEdit->value(),
                         ui->filter_borderTypeEdit->currentData().toInt());
        break;
    case MedianBlur:
        cv::medianBlur(m_originalMat, m_processMat, ui->filter_kSizeXEdit->value());
        break;
    default:
        break;
    }

    ui->processView->setImage(QtOcv::mat2Image_shared(m_processMat));
}

void MainWindow::closeEvent(QCloseEvent *evt)
{
    saveSettings();
    evt->accept();
}

void MainWindow::loadSettings()
{
    QSettings settings;
    settings.beginGroup("RecentFiles");
    m_recentFiles->setFileList(settings.value("list").toStringList());
    settings.endGroup();
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.beginGroup("RecentFiles");
    settings.setValue("list", m_recentFiles->fileList());
    settings.endGroup();
}

void MainWindow::createImageAction(int id, const QString &text)
{
    QAction *act = ui->menuImage->addAction(text, this, SLOT(onImageActionTriggered()));
    act->setProperty("id", id);
    m_imageActions.insert(id, act);
}

void MainWindow::createImageActions()
{
    createImageAction(ConvertToGray, "Convert To Gray");
    ui->menuImage->addSeparator();

    createImageAction(BilateralFilter, "Bilateral Filter");
    createImageAction(Blur, "Blur");
    createImageAction(BoxFilter, "Box Filter");
    createImageAction(Filter2D, "Filter2D");
    createImageAction(GaussianBlur, "Gaussian Blur");
    createImageAction(MedianBlur, "Median Blur");

    ui->menuImage->addSeparator();
    createImageAction(ReplaceOriginal, "Replace Original");
}

void MainWindow::createFilterWidgets()
{
    ui->filter_borderTypeEdit->addItem("BORDER_DEFAULT", cv::BORDER_DEFAULT);
    ui->filter_borderTypeEdit->addItem("BORDER_REPLICATE", cv::BORDER_REPLICATE);
    ui->filter_borderTypeEdit->addItem("BORDER_REFLECT", cv::BORDER_REFLECT);
    ui->filter_borderTypeEdit->addItem("BORDER_REFLECT_101", cv::BORDER_REFLECT_101);
    ui->filter_borderTypeEdit->addItem("BORDER_WRAP", cv::BORDER_WRAP);
    ui->filter_borderTypeEdit->addItem("BORDER_CONSTANT", cv::BORDER_CONSTANT);
}

void MainWindow::doOpen(const QString &filePath)
{
    QImage image(filePath);
    if (image.isNull()) {
        m_recentFiles->remove(filePath);
        return;
    }

    m_recentFiles->add(filePath);
    ui->originalView->setImage(image);
    ui->originalView->setCurrentScale(0);//Auto scale
    ui->processView->setPixmap(QPixmap());
    ui->processView->setCurrentScale(0);
    bool isGray = image.isGrayscale();
    m_originalMat = QtOcv::image2Mat(image, CV_8UC(isGray ? 1 : 3), QtOcv::MCO_RGB);
    m_imageActions[ConvertToGray]->setDisabled(isGray);
    setWindowTitle(QString("%1[*] - Image Process").arg(filePath));
}
