#ifndef ABSTRACTCONVERT_H
#define ABSTRACTCONVERT_H

#include <QPointer>
#include <QString>

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

    virtual bool applyTo(const cv::Mat &input, cv::Mat &output) = 0;
    QWidget *paramsWidget();
    QString errorString() const;

protected:
    virtual void initParamsWidget() = 0;

    QPointer<QWidget> m_widget;
    QString m_errorString;
};

class Gray : public AbstractConvert
{
public:
    Gray(){}
    ~Gray(){}

    bool applyTo(const cv::Mat &input, cv::Mat &output);

private:
    void initParamsWidget(){}
};

class Blur : public AbstractConvert
{
public:
    Blur();
    ~Blur();

    bool applyTo(const cv::Mat &input, cv::Mat &output);

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

    bool applyTo(const cv::Mat &input, cv::Mat &output);

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

    bool applyTo(const cv::Mat &input, cv::Mat &output);

private:
    void initParamsWidget();

    QComboBox *normalizeEdit;
};

class MedianBlur : public AbstractConvert
{
public:
    MedianBlur();
    ~MedianBlur();

    bool applyTo(const cv::Mat &input, cv::Mat &output);

private:
    void initParamsWidget();

    QSpinBox *kSizeEdit;
};

class GaussianBlur : public AbstractConvert
{
public:
    GaussianBlur() {}
    ~GaussianBlur() {}

    bool applyTo(const cv::Mat &input, cv::Mat &output);

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

    bool applyTo(const cv::Mat &input, cv::Mat &output);

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

    bool applyTo(const cv::Mat &input, cv::Mat &output);

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

    bool applyTo(const cv::Mat &input, cv::Mat &output);

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

    bool applyTo(const cv::Mat &input, cv::Mat &output);
};

class HoughCircles : public AbstractConvert
{
public:
    HoughCircles() {}
    ~HoughCircles() {}

    bool applyTo(const cv::Mat &input, cv::Mat &output);

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

    bool applyTo(const cv::Mat &input, cv::Mat &output);

protected:
    void initParamsWidget();

    QPlainTextEdit *infoEdit;
};

#endif // ABSTRACTCONVERT_H
