#include "dialog.h"
#include "ui_dialog.h"
#include <QFileDialog>
#include <QPixmap>
#include <QImage>
#include <QDebug>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "cvmatandqimage.h"

namespace {

void calcFFT(const cv::Mat &input, cv::Mat &output)
{
    cv::Mat padded;                            //expand input image to optimal size
    int m = cv::getOptimalDFTSize( input.rows );
    int n = cv::getOptimalDFTSize( input.cols ); // on the border add zero values
    cv::copyMakeBorder(input, padded, 0, m - input.rows, 0, n - input.cols, cv::BORDER_CONSTANT, cv::Scalar_<float>::all(0));

    cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat complexI;
    cv::merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

    cv::dft(complexI, complexI);            // this way the result may fit in the source matrix

    // compute the magnitude and switch to logarithmic scale
    // => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
    cv::split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
    cv::magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
    output = planes[0];

    output += cv::Scalar_<float>::all(1);                    // switch to logarithmic scale
    cv::log(output, output);

    // crop the spectrum, if it has an odd number of rows or columns
    output = output(cv::Rect(0, 0, output.cols & -2, output.rows & -2));

    // rearrange the quadrants of Fourier image  so that the origin is at the image center
    int cx = output.cols/2;
    int cy = output.rows/2;

    cv::Mat q0(output, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
    cv::Mat q1(output, cv::Rect(cx, 0, cx, cy));  // Top-Right
    cv::Mat q2(output, cv::Rect(0, cy, cx, cy));  // Bottom-Left
    cv::Mat q3(output, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

    cv::Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);

    q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
    q2.copyTo(q1);
    tmp.copyTo(q2);

    cv::normalize(output, output, 0, 1, CV_MINMAX); // Transform the matrix with float values into a
                                            // viewable image form (float between values 0 and 1).
    return;
}
} //namespace

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    connect(ui->openButton, SIGNAL(clicked()), SLOT(onOpenButtonClicked()));
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::onOpenButtonClicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"), QString(), "Images(*.png *.bmp *.jpg *.gif)");
    if (filename.isEmpty())
        return;

    QImage img(filename);
    if (img.isNull())
        return;

    ui->label->setPixmap(QPixmap::fromImage(img));

    cv::Mat mat = QtOcv::image2Mat(img, CV_32FC1);
    cv::Mat out_mat;
    calcFFT(mat, out_mat);

    QLabel *fftLabel = new QLabel;
    fftLabel->setWindowTitle(tr("FFT result window"));
    fftLabel->setAttribute(Qt::WA_DeleteOnClose);
    QImage out_img = QtOcv::mat2Image(out_mat);
    fftLabel->setPixmap(QPixmap::fromImage(out_img));
    fftLabel->show();
}
