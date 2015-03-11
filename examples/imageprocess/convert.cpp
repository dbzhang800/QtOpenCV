#include "convert.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPlainTextEdit>

static QSpinBox *createSpinBox(int v, int min=0, int max=99)
{
    QSpinBox *edit = new QSpinBox;
    edit->setRange(min, max);
    edit->setValue(v);
    return edit;
}

AbstractConvert::AbstractConvert()
{

}

AbstractConvert::~AbstractConvert()
{
    if (!m_widget.isNull())
        delete m_widget.data();
}

QWidget *AbstractConvert::paramsWidget()
{
    if (m_widget.isNull()) {
        m_widget = new QWidget();
        initParamsWidget();
    }
    return m_widget.data();
}

Blur::Blur()
{

}

Blur::~Blur()
{

}

void Blur::applyTo(const cv::Mat &input, cv::Mat &output) const
{
    cv::blur(input, output, cv::Size(kSizeXEdit->value(), kSizeYEdit->value()),
             cv::Point(anchorXEdit->value(), anchorYEdit->value()),
             borderTypeEdit->currentData().toInt());
}

void Blur::initParamsWidget()
{
    kSizeXEdit = createSpinBox(3);
    kSizeYEdit = createSpinBox(3);
    anchorXEdit = createSpinBox(-1, -10, 255);
    anchorYEdit = createSpinBox(-1, -10, 255);

    borderTypeEdit = new QComboBox;
    borderTypeEdit->addItem("BORDER_DEFAULT", cv::BORDER_DEFAULT);
    borderTypeEdit->addItem("BORDER_REPLICATE", cv::BORDER_REPLICATE);
    borderTypeEdit->addItem("BORDER_REFLECT", cv::BORDER_REFLECT);
    borderTypeEdit->addItem("BORDER_REFLECT_101", cv::BORDER_REFLECT_101);
    borderTypeEdit->addItem("BORDER_WRAP", cv::BORDER_WRAP);
    borderTypeEdit->addItem("BORDER_CONSTANT", cv::BORDER_CONSTANT);

    QFormLayout *layout = new QFormLayout(m_widget.data());
    layout->addRow("kSize X", kSizeXEdit);
    layout->addRow("kSize Y", kSizeYEdit);
    layout->addRow("anchor X", anchorXEdit);
    layout->addRow("anchor Y", anchorYEdit);
    layout->addRow("border", borderTypeEdit);
}


BilateralFilter::BilateralFilter()
{

}

BilateralFilter::~BilateralFilter()
{

}

void BilateralFilter::applyTo(const cv::Mat &input, cv::Mat &output) const
{
    cv::bilateralFilter(input, output, dEdit->value(), sigmaColorEdit->value(),
                        sigmaSpaceEdit->value(), borderTypeEdit->currentData().toInt());

}

void BilateralFilter::initParamsWidget()
{
    dEdit = createSpinBox(5);
    sigmaColorEdit = createSpinBox(150, -100, 255);
    sigmaSpaceEdit = createSpinBox(150, -100, 255);

    borderTypeEdit = new QComboBox;
    borderTypeEdit->addItem("BORDER_DEFAULT", cv::BORDER_DEFAULT);
    borderTypeEdit->addItem("BORDER_REPLICATE", cv::BORDER_REPLICATE);
    borderTypeEdit->addItem("BORDER_REFLECT", cv::BORDER_REFLECT);
    borderTypeEdit->addItem("BORDER_REFLECT_101", cv::BORDER_REFLECT_101);
    borderTypeEdit->addItem("BORDER_WRAP", cv::BORDER_WRAP);
    borderTypeEdit->addItem("BORDER_CONSTANT", cv::BORDER_CONSTANT);

    QFormLayout *layout = new QFormLayout(m_widget.data());
    layout->addRow("d", dEdit);
    layout->addRow("sigmaColor", sigmaColorEdit);
    layout->addRow("sigmaSpace", sigmaSpaceEdit);
    layout->addRow("border", borderTypeEdit);
}

BoxFilter::BoxFilter()
{

}

BoxFilter::~BoxFilter()
{

}

void BoxFilter::applyTo(const cv::Mat &input, cv::Mat &output) const
{
    cv::boxFilter(input, output, -1, cv::Size(kSizeXEdit->value(), kSizeYEdit->value()),
                  cv::Point(anchorXEdit->value(), anchorYEdit->value()), normalizeEdit->currentData().toBool(),
                  borderTypeEdit->currentData().toInt());
}

void BoxFilter::initParamsWidget()
{
    Blur::initParamsWidget();

    normalizeEdit = new QComboBox;
    normalizeEdit->addItem("TRUE", true);
    normalizeEdit->addItem("FALSE", false);

    static_cast<QFormLayout *>(m_widget->layout())->addRow("normalized", normalizeEdit);
}



MedianBlur::MedianBlur()
{

}

MedianBlur::~MedianBlur()
{

}

void MedianBlur::applyTo(const cv::Mat &input, cv::Mat &output) const
{
    cv::medianBlur(input, output, kSizeEdit->value());
}

void MedianBlur::initParamsWidget()
{
    kSizeEdit = createSpinBox(3);

    QFormLayout *layout = new QFormLayout(m_widget.data());
    layout->addRow("kSize", kSizeEdit);
}


void GaussianBlur::applyTo(const cv::Mat &input, cv::Mat &output) const
{
    cv::GaussianBlur(input, output, cv::Size(kSizeXEdit->value(), kSizeYEdit->value()),
                     sigmaXEdit->value(), sigmaYEdit->value(),
                     borderTypeEdit->currentData().toInt());
}

void GaussianBlur::initParamsWidget()
{
    kSizeXEdit = createSpinBox(3);
    kSizeYEdit = createSpinBox(3);
    sigmaXEdit = createSpinBox(-1, -10, 255);
    sigmaYEdit = createSpinBox(-1, -10, 255);

    borderTypeEdit = new QComboBox;
    borderTypeEdit->addItem("BORDER_DEFAULT", cv::BORDER_DEFAULT);
    borderTypeEdit->addItem("BORDER_REPLICATE", cv::BORDER_REPLICATE);
    borderTypeEdit->addItem("BORDER_REFLECT", cv::BORDER_REFLECT);
    borderTypeEdit->addItem("BORDER_REFLECT_101", cv::BORDER_REFLECT_101);
    borderTypeEdit->addItem("BORDER_WRAP", cv::BORDER_WRAP);
    borderTypeEdit->addItem("BORDER_CONSTANT", cv::BORDER_CONSTANT);

    QFormLayout *layout = new QFormLayout(m_widget.data());
    layout->addRow("kSize X", kSizeXEdit);
    layout->addRow("kSize Y", kSizeYEdit);
    layout->addRow("anchor X", sigmaXEdit);
    layout->addRow("anchor Y", sigmaYEdit);
    layout->addRow("border", borderTypeEdit);
}


void Threshold::applyTo(const cv::Mat &input, cv::Mat &output) const
{
    int type = typeEdit->currentData().toInt();
    if (otsuButton->isChecked())
        type |= cv::THRESH_OTSU;
    cv::threshold(input, output, threshEdit->value(), maxvalEdit->value(), type);
}

void Threshold::initParamsWidget()
{
    threshEdit = createSpinBox(100, 0, 255);
    maxvalEdit = createSpinBox(255, 0, 255);

    typeEdit = new QComboBox;
    typeEdit->addItem("THRESH_BINARY", cv::THRESH_BINARY);
    typeEdit->addItem("THRESH_BINARY_INV", cv::THRESH_BINARY_INV);
    typeEdit->addItem("THRESH_TRUNC", cv::THRESH_TRUNC);
    typeEdit->addItem("THRESH_TOZERO", cv::THRESH_TOZERO);
    typeEdit->addItem("THRESH_TOZERO_INV", cv::THRESH_TOZERO_INV);

    otsuButton = new QCheckBox("THRESH_OTSU");

    QFormLayout *layout = new QFormLayout(m_widget.data());
    layout->addRow("thresh", threshEdit);
    layout->addRow("maxval", maxvalEdit);
    layout->addRow("type", typeEdit);
    layout->addRow(otsuButton);
}


void Canny::applyTo(const cv::Mat &input, cv::Mat &output) const
{
    cv::Canny(input, output, threshold1Edit->value(), threshold2Edit->value(),
              apertureSizeEdit->value(), l2gradientButton->isChecked());
}

void Canny::initParamsWidget()
{
    threshold1Edit = createSpinBox(20, 0, 255);
    threshold2Edit = createSpinBox(60, 0, 255);

    apertureSizeEdit = new QSpinBox;
    apertureSizeEdit->setValue(3);

    l2gradientButton = new QCheckBox("L2Gradient");

    QFormLayout *layout = new QFormLayout(m_widget.data());
    layout->addRow("threshold1", threshold1Edit);
    layout->addRow("threshold2", threshold2Edit);
    layout->addRow("apertureSize", apertureSizeEdit);
    layout->addRow(l2gradientButton);
}


void Dilate::applyTo(const cv::Mat &input, cv::Mat &output) const
{
    cv::Mat kernel = cv::getStructuringElement(kShapeEdit->currentData().toInt(),
                                               cv::Size(kSizeXEdit->value(), kSizeYEdit->value()),
                                               cv::Point(anchorXEdit->value(), anchorYEdit->value()));
    cv::dilate(input, output, kernel, cv::Point(anchorXEdit->value(), anchorYEdit->value()),
               iterationsEdit->value(), borderTypeEdit->currentData().toInt());
}

void Dilate::initParamsWidget()
{
    kSizeXEdit = createSpinBox(3);
    kSizeYEdit = createSpinBox(3);
    kShapeEdit = new QComboBox;
    kShapeEdit->addItem("MORPH_RECT", cv::MORPH_RECT);
    kShapeEdit->addItem("MORPH_ELLIPSE", cv::MORPH_ELLIPSE);
    kShapeEdit->addItem("MORPH_CROSS", cv::MORPH_CROSS);

    anchorXEdit = createSpinBox(-1, -10, 255);
    anchorYEdit = createSpinBox(-1, -10, 255);
    iterationsEdit = createSpinBox(1);
    borderTypeEdit = new QComboBox;
    borderTypeEdit->addItem("BORDER_DEFAULT", cv::BORDER_DEFAULT);
    borderTypeEdit->addItem("BORDER_REPLICATE", cv::BORDER_REPLICATE);
    borderTypeEdit->addItem("BORDER_REFLECT", cv::BORDER_REFLECT);
    borderTypeEdit->addItem("BORDER_REFLECT_101", cv::BORDER_REFLECT_101);
    borderTypeEdit->addItem("BORDER_WRAP", cv::BORDER_WRAP);
    borderTypeEdit->addItem("BORDER_CONSTANT", cv::BORDER_CONSTANT);
    borderTypeEdit->setCurrentIndex(5);

    QFormLayout *layout = new QFormLayout(m_widget.data());
    layout->addRow("kSize X", kSizeXEdit);
    layout->addRow("kSize Y", kSizeYEdit);
    layout->addRow("kShape", kShapeEdit);
    layout->addRow("anchor X", anchorXEdit);
    layout->addRow("anchor Y", anchorYEdit);
    layout->addRow("iterations", iterationsEdit);
    layout->addRow("border", borderTypeEdit);
}


void Erode::applyTo(const cv::Mat &input, cv::Mat &output) const
{
    cv::Mat kernel = cv::getStructuringElement(kShapeEdit->currentData().toInt(),
                                               cv::Size(kSizeXEdit->value(), kSizeYEdit->value()),
                                               cv::Point(anchorXEdit->value(), anchorYEdit->value()));
    cv::erode(input, output, kernel, cv::Point(anchorXEdit->value(), anchorYEdit->value()),
               iterationsEdit->value(), borderTypeEdit->currentData().toInt());

}


void HoughCircles::applyTo(const cv::Mat &input, cv::Mat &output) const
{
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(input, circles, CV_HOUGH_GRADIENT, dpEdit->value(), minDistEdit->value(), param1Edit->value(), param2Edit->value(),
                     minRadiusEdit->value(), maxRadiusEdit->value());

    if (input.channels() == 1)
        cv::cvtColor(input, output, CV_GRAY2RGB);
    else
        output = input.clone();

    for( size_t i = 0; i < circles.size(); i++ ) {
        cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        // draw the circle center
        cv::circle(output, center, 3, cv::Scalar(0,255,0), -1, 8, 0 );
        // draw the circle outline
        cv::circle(output, center, radius, cv::Scalar(0,0,255), 3, 8, 0 );

        QString info = QString("Center(%1, %2) Radius %3")
                .arg(center.x).arg(center.y)
                .arg(radius);
        infoEdit->appendPlainText(info);
    }
}

void HoughCircles::initParamsWidget()
{
    methodEdit = new QComboBox;
    methodEdit->addItem("CV_HOUGH_GRADIENT");

    dpEdit = createSpinBox(1);
    minDistEdit = createSpinBox(2, 0, 10000);
    param1Edit = createSpinBox(100, 0 ,255);
    param2Edit = createSpinBox(100, 0, 255);
    minRadiusEdit = createSpinBox(0, 0, 10000);
    maxRadiusEdit = createSpinBox(0, 0, 10000);
    infoEdit = new QPlainTextEdit;

    QFormLayout *layout = new QFormLayout(m_widget.data());
    layout->addRow("method", methodEdit);
    layout->addRow("dp", dpEdit);
    layout->addRow("minDist", minDistEdit);
    layout->addRow("param1", param1Edit);
    layout->addRow("param2", param2Edit);
    layout->addRow("minRadius", minRadiusEdit);
    layout->addRow("maxRadius", maxRadiusEdit);
    layout->addRow(infoEdit);
}


void FitEllipse::applyTo(const cv::Mat &input, cv::Mat &output) const
{

    //Find countours
    std::vector<std::vector<cv::Point> > contours;
    int find_contours_mode = CV_CHAIN_APPROX_TC89_L1; //CV_RETR_LIST
    cv::findContours(input, contours, find_contours_mode, CV_CHAIN_APPROX_NONE);
    std::sort(contours.begin(), contours.end(), [](std::vector<cv::Point> a, std::vector<cv::Point> b) {
        return b.size() < a.size();
    });

    if (input.channels() == 1)
        cv::cvtColor(input, output, CV_GRAY2RGB);
    else
        output = input.clone();

    size_t maxEllipseCount = 4;
    for(size_t i = 0, foundCount = 0; (i < contours.size()) && (foundCount < maxEllipseCount); i++) {
        size_t count = contours[i].size();

        if( count < 6 )
            continue;

        cv::Mat pointsf;
        cv::Mat(contours[i]).convertTo(pointsf, CV_32F);
        cv::RotatedRect box = cv::fitEllipse(pointsf);

        if( qMax(box.size.width, box.size.height) > qMin(box.size.width, box.size.height)*30 )
            continue;

        foundCount++;

        cv::ellipse(output, box, cv::Scalar(255,0,0), 1, CV_AA);
        cv::ellipse(output, box.center, box.size*0.5f, box.angle, 0, 360, cv::Scalar(255,255,0), 1, CV_AA);
        QString ellipseInfo = QString("Center(%1, %2) Size(%3, %4) Angle %5")
                .arg(box.center.x).arg(box.center.y)
                .arg(box.size.width).arg(box.size.height)
                .arg(box.angle);
        infoEdit->appendPlainText(ellipseInfo);
    }
}

void FitEllipse::initParamsWidget()
{
    infoEdit = new QPlainTextEdit;

    QFormLayout *layout = new QFormLayout(m_widget.data());
    layout->addRow(infoEdit);
}
