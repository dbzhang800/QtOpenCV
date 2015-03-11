#ifndef ABSTRACTCONVERT_H
#define ABSTRACTCONVERT_H

#include <QPointer>

namespace cv {
class Mat;
}
class QWidget;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QPlainTextEdit;

class AbstractConvert
{
public:
    AbstractConvert();
    virtual ~AbstractConvert();

    virtual void applyTo(const cv::Mat &input, cv::Mat &output) const = 0;
    QWidget *paramsWidget();

protected:
    virtual void initParamsWidget() = 0;

    QPointer<QWidget> m_widget;
};

class Blur : public AbstractConvert
{
public:
    Blur();
    ~Blur();

    void applyTo(const cv::Mat &input, cv::Mat &output) const;

protected:
    void initParamsWidget();

    QSpinBox *kSizeXEdit;
    QSpinBox *kSizeYEdit;
    QSpinBox *anchorXEdit;
    QSpinBox *anchorYEdit;
    QComboBox *borderTypeEdit;
};

class BilateralFilter : public AbstractConvert
{
public:
    BilateralFilter();
    ~BilateralFilter();

    void applyTo(const cv::Mat &input, cv::Mat &output) const;

private:
    void initParamsWidget();

    QSpinBox *dEdit;
    QSpinBox *sigmaColorEdit;
    QSpinBox *sigmaSpaceEdit;
    QComboBox *borderTypeEdit;
};

class BoxFilter : public Blur
{
public:
    BoxFilter();
    ~BoxFilter();

    void applyTo(const cv::Mat &input, cv::Mat &output) const;

private:
    void initParamsWidget();

    QComboBox *normalizeEdit;
};

class MedianBlur : public AbstractConvert
{
public:
    MedianBlur();
    ~MedianBlur();

    void applyTo(const cv::Mat &input, cv::Mat &output) const;

private:
    void initParamsWidget();

    QSpinBox *kSizeEdit;
};

class GaussianBlur : public AbstractConvert
{
public:
    GaussianBlur() {}
    ~GaussianBlur() {}

    void applyTo(const cv::Mat &input, cv::Mat &output) const;

protected:
    void initParamsWidget();

    QSpinBox *kSizeXEdit;
    QSpinBox *kSizeYEdit;
    QSpinBox *sigmaXEdit;
    QSpinBox *sigmaYEdit;
    QComboBox *borderTypeEdit;
};

class Threshold : public AbstractConvert
{
public:
    Threshold() {}
    ~Threshold() {}

    void applyTo(const cv::Mat &input, cv::Mat &output) const;

protected:
    void initParamsWidget();

    QSpinBox *threshEdit;
    QSpinBox *maxvalEdit;
    QComboBox *typeEdit;
    QCheckBox *otsuButton;
};

class Canny : public AbstractConvert
{
public:
    Canny() {}
    ~Canny() {}

    void applyTo(const cv::Mat &input, cv::Mat &output) const;

protected:
    void initParamsWidget();

    QSpinBox *threshold1Edit;
    QSpinBox *threshold2Edit;
    QSpinBox *apertureSizeEdit;
    QCheckBox *l2gradientButton;
};

class Dilate : public AbstractConvert
{
public:
    Dilate() {}
    ~Dilate() {}

    void applyTo(const cv::Mat &input, cv::Mat &output) const;

protected:
    void initParamsWidget();

    QSpinBox *kSizeXEdit;
    QSpinBox *kSizeYEdit;
    QComboBox *kShapeEdit;

    QSpinBox *anchorXEdit;
    QSpinBox *anchorYEdit;
    QSpinBox *iterationsEdit;
    QComboBox *borderTypeEdit;
};

class Erode : public Dilate
{
public:
    Erode() {}
    ~Erode() {}

    void applyTo(const cv::Mat &input, cv::Mat &output) const;
};

class HoughCircles : public AbstractConvert
{
public:
    HoughCircles() {}
    ~HoughCircles() {}

    void applyTo(const cv::Mat &input, cv::Mat &output) const;

protected:
    void initParamsWidget();

    QComboBox *methodEdit;
    QSpinBox *dpEdit;
    QSpinBox *minDistEdit;

    QSpinBox *param1Edit;
    QSpinBox *param2Edit;
    QSpinBox *minRadiusEdit;
    QSpinBox *maxRadiusEdit;
    QPlainTextEdit *infoEdit;
};

class FitEllipse : public AbstractConvert
{
public:
    FitEllipse() {}
    ~FitEllipse() {}

    void applyTo(const cv::Mat &input, cv::Mat &output) const;

protected:
    void initParamsWidget();

    QPlainTextEdit *infoEdit;
};

#endif // ABSTRACTCONVERT_H
