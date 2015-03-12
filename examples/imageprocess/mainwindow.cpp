#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "recentfiles.h"
#include "convert.h"

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QCloseEvent>
#include <QSettings>
#include <QFileDialog>
#include <QScrollBar>

enum
{
    E_ConvertToGray,
    //vvvvvvvvv
    E_BilateralFilter,
    E_Blur,
    E_BoxFilter,
    E_Filter2D,
    E_GaussianBlur,
    E_MedianBlur,
    E_THRESHOLD,
    E_Canny,
    E_Dilate,
    E_Erode,

    E_HoughCircles,
    E_FitEllipse,
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), m_recentFiles(new RecentFiles(this))
{
    ui->setupUi(this);
    ui->actionRecentFiles->setMenu(m_recentFiles->menu());

    createImageActions();

    connect(m_recentFiles, SIGNAL(activated(QString)), SLOT(onRecentFilesTriggered(QString)));
    connect(ui->actionOpen, SIGNAL(triggered()), SLOT(onFileOpenActionTriggered()));
    connect(ui->actionSave, SIGNAL(triggered()), SLOT(onFileSaveActionTriggered()));
    connect(ui->actionSaveAs, SIGNAL(triggered()), SLOT(onFileSaveAsActionTriggered()));
    connect(ui->filterApplyButton, SIGNAL(clicked()), SLOT(onFilterApplyButtonClicked()));
    connect(ui->filterPreviewButton, SIGNAL(clicked()), SLOT(onFilterPreviewButtonClicked()));
    connect(ui->originalView, SIGNAL(colorUnderMouseChanged(QColor)), SLOT(onColorUnderMouseChanged(QColor)));
    connect(ui->processView, SIGNAL(colorUnderMouseChanged(QColor)), SLOT(onColorUnderMouseChanged(QColor)));
    connect(ui->originalView, SIGNAL(realScaleChanged(double)), ui->processView, SLOT(setCurrentScale(double)));
    connect(ui->processView, SIGNAL(realScaleChanged(double)), ui->originalView, SLOT(setCurrentScale(double)));
    //Todo
    connect(ui->originalView->horizontalScrollBar(), SIGNAL(valueChanged(int)), ui->processView->horizontalScrollBar(), SLOT(setValue(int)));
    connect(ui->originalView->verticalScrollBar(), SIGNAL(valueChanged(int)), ui->processView->verticalScrollBar(), SLOT(setValue(int)));

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

void MainWindow::onFileSaveActionTriggered()
{
    if (ui->originalView->pixmap().isNull())
        return;
    ui->originalView->pixmap().save(m_recentFiles->mostRecentFile());
}

void MainWindow::onFileSaveAsActionTriggered()
{
    if (ui->originalView->pixmap().isNull())
        return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), m_recentFiles->mostRecentFile(), "Images(*.png *.bmp *.jpg *.gif)");
    if (fileName.isEmpty())
        return;
    ui->originalView->pixmap().save(fileName);
}

void MainWindow::onImageActionTriggered()
{
    if (m_originalMat.empty())
        return;

    QAction *act = qobject_cast<QAction *>(sender());
    int id = act->property("id").toInt();
    m_convert.clear();

    switch (id) {
    case E_ConvertToGray:
        m_convert = QSharedPointer<AbstractConvert>(new Gray());
        break;
    case E_Blur:
        m_convert = QSharedPointer<AbstractConvert>(new Blur());
        break;
    case E_BilateralFilter:
        m_convert = QSharedPointer<AbstractConvert>(new BilateralFilter);
        break;
    case E_BoxFilter:
        m_convert = QSharedPointer<AbstractConvert>(new BoxFilter);
        break;
    case E_MedianBlur:
        m_convert = QSharedPointer<AbstractConvert>(new MedianBlur);
        break;
    case E_GaussianBlur:
        m_convert = QSharedPointer<AbstractConvert>(new GaussianBlur);
        break;
    case E_Filter2D:
        break;
    case E_THRESHOLD:
        m_convert = QSharedPointer<AbstractConvert>(new Threshold);
        break;
    case E_Canny:
        m_convert = QSharedPointer<AbstractConvert>(new Canny);
        break;
    case E_Dilate:
        m_convert = QSharedPointer<AbstractConvert>(new Dilate);
        break;
    case E_Erode:
        m_convert = QSharedPointer<AbstractConvert>(new Erode);
        break;

    case E_HoughCircles:
        m_convert = QSharedPointer<AbstractConvert>(new HoughCircles);
        break;
    case E_FitEllipse:
        m_convert = QSharedPointer<AbstractConvert>(new FitEllipse);
        break;
    default:
        break;
    }

    ui->processView->setPixmap(QPixmap());
    ui->filterDockWidget->setEnabled(m_convert);
    if (m_convert) {
        ui->filterDockWidget->setWindowTitle(act->text());
        ui->paramsWidget->layout()->addWidget(m_convert->paramsWidget());
    }
}

void MainWindow::onFilterPreviewButtonClicked()
{
    if (!m_convert)
        return;
    qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
    if (m_convert->applyTo(m_originalMat, m_processMat))
        ui->processView->setImage(QtOcv::mat2Image_shared(m_processMat));
    else
        statusBar()->showMessage(m_convert->errorString(), 3000);
    qApp->restoreOverrideCursor();
}

void MainWindow::onFilterApplyButtonClicked()
{
    if (ui->processView->pixmap().isNull())
        onFilterPreviewButtonClicked();

    ui->originalView->setPixmap(ui->processView->pixmap());
    m_originalMat = m_processMat;
    m_processMat = cv::Mat();
    ui->processView->setPixmap(QPixmap());
}

void MainWindow::onColorUnderMouseChanged(const QColor &c)
{
    if (c.isValid())
        statusBar()->showMessage(QString("R %1 G %2 B %3").arg(c.red()).arg(c.green()).arg(c.blue()));
    else
        statusBar()->clearMessage();
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
    createImageAction(E_ConvertToGray, "Convert To Gray");
    ui->menuImage->addSeparator();
    createImageAction(E_THRESHOLD, "Threshold");

    createImageAction(E_BilateralFilter, "Bilateral Filter");
    createImageAction(E_Blur, "Blur");
    createImageAction(E_BoxFilter, "Box Filter");
    createImageAction(E_Filter2D, "Filter2D");
    createImageAction(E_GaussianBlur, "Gaussian Blur");
    createImageAction(E_MedianBlur, "Median Blur");
    createImageAction(E_Canny, "Canny");
    createImageAction(E_Dilate, "Dilate");
    createImageAction(E_Erode, "Erode");

    ui->menuImage->addSeparator();
    createImageAction(E_HoughCircles, "HoughCircles");
    createImageAction(E_FitEllipse, "FitEllipse");
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
    setWindowTitle(QString("%1[*] - Image Process").arg(filePath));
}
