/****************************************************************************
** Copyright (c) 2012-2015 Debao Zhang <hello@debao.me>
** All right reserved.
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
** LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
** OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
** WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
****************************************************************************/

#include "cvmatandqimage.h"
#include <QImage>
#include <QSysInfo>
#include <QDebug>
#include <cstring>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

namespace {

/* The caller must make sure the image format is valid.
*/
template<typename T>
QImage mat2Image_(const cv::Mat & mat, QImage::Format format, double scalefactor)
{
    Q_ASSERT(mat.channels()==1 || mat.channels()==3 || mat.channels()==4);

    QImage outImage(mat.cols, mat.rows, format);
    if (format == QImage::Format_Indexed8) {
        QVector<QRgb> colorTable;
        for (int i=0; i<256; ++i)
            colorTable.append(qRgb(i,i,i));
        outImage.setColorTable(colorTable);
    }

    for (int row=0; row<mat.rows; ++row) {
        uchar *imageData = outImage.scanLine(row);
        const T *matData = mat.ptr<T>(row);
        for (int col=0; col<mat.cols * mat.channels(); ++col)
            *imageData++ = cv::saturate_cast<uchar>(matData[col] * scalefactor);
    }

    return outImage;
}

/* The caller must make sure the image format and matType is valid.
*/
template<typename T>
cv::Mat image2Mat_(const QImage &image, int matType, double scaleFactor)
{
    const int channels = CV_MAT_CN(matType);
    cv::Mat mat = cv::Mat(image.height(), image.width(), matType);

    for (int row = 0; row < mat.rows; ++row) {
        const uchar *imageData = image.scanLine(row);
        T *matData = mat.ptr<T>(row);
        for (int col = 0; col < mat.cols * channels; ++col)
            *matData++ = cv::saturate_cast<T>(imageData[col] * scaleFactor);
    }
    return mat;
}

/*ARGB <==> BGRA
 */
cv::Mat argb2bgra(const cv::Mat &mat)
{
    Q_ASSERT(mat.channels()==4);

    cv::Mat newMat(mat.rows, mat.cols, mat.type());
    int from_to[] = {0,3, 1,2, 2,1, 3,0};
    cv::mixChannels(&mat, 1, &newMat, 1, from_to, 4);
    return newMat;
}
} //namespace

namespace QtOcv {

/* Convert QImage to cv::Mat
 */
cv::Mat image2Mat(const QImage &img, MatColorOrder *order, int matDepth)
{
    matDepth = CV_MAT_DEPTH(matDepth);
    Q_ASSERT(matDepth==CV_8U || matDepth==CV_16U || matDepth==CV_32F);

    if (img.isNull())
        return cv::Mat();

    QImage image;
    switch (img.format()) {
    case QImage::Format_Indexed8:
    case QImage::Format_RGB888:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
#if QT_VERSION >= 0x050200
    case QImage::Format_RGBX8888:
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
#endif
        image = img;
        break;
    case QImage::Format_Mono:
    case QImage::Format_MonoLSB:
        image = img.convertToFormat(QImage::Format_Indexed8);
        break;
    case QImage::Format_RGB444:
    case QImage::Format_RGB555:
    case QImage::Format_RGB666:
    case QImage::Format_RGB16:
        image = img.convertToFormat(QImage::Format_RGB888);
        break;
    case QImage::Format_ARGB4444_Premultiplied:
    case QImage::Format_ARGB6666_Premultiplied:
    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_ARGB8565_Premultiplied:
        image = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        break;
    default:
        image = img.convertToFormat(QImage::Format_ARGB32);
        break;
    }

    cv::Mat mat;
    const int matType = CV_MAKETYPE(matDepth, image.depth()/8);
    switch (matDepth) {
    case CV_8U:
        mat = image2Mat_<uchar>(image, matType, 1.0);
        break;
    case CV_16U:
        mat = image2Mat_<quint16>(image, matType, 65535./255.);
        break;
    case CV_32F:
        mat = image2Mat_<float>(image, matType, 1./255.);
        break;
    default:
        break;
    }

    if (order) {
        switch (image.format()) {
        case QImage::Format_RGB888:
            *order = MCO_RGB;
            break;
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
            *order = QSysInfo::ByteOrder == QSysInfo::LittleEndian ? MCO_BGRA : MCO_ARGB;
            break;
#if QT_VERSION >= 0x050200
        case QImage::Format_RGBX8888:
        case QImage::Format_RGBA8888:
        case QImage::Format_RGBA8888_Premultiplied:
            *order = MCO_RGBA;
            break;
#endif
        default:
            *order = MCO_BGR;
            break;
        }
    }

    return mat;
}

/* Convert QImage to cv::Mat
 */
cv::Mat image2Mat(const QImage &img, int requiredMatType, MatColorOrder requriedOrder)
{
    int targetDepth = CV_MAT_DEPTH(requiredMatType);
    int targetChannels = CV_MAT_CN(requiredMatType);
    Q_ASSERT(targetChannels==CV_CN_MAX || targetChannels==1 || targetChannels==3 || targetChannels==4);
    Q_ASSERT(targetDepth==CV_8U || targetDepth==CV_16U || targetDepth==CV_32F);

    MatColorOrder srcOrder;
    cv::Mat mat0 = image2Mat(img, &srcOrder, targetDepth);

    cv::Mat mat;
    const float maxAlpha = targetDepth==CV_8U ? 255 : (targetDepth==CV_16U ? 65535 : 1.0);
    if (targetChannels == CV_CN_MAX)
        targetChannels = mat0.channels();
    //Adjust mat if needed.
    switch(targetChannels) {
    case 1:
        if (mat0.channels() == 1) {
            mat = mat0;
        } else if (mat0.channels() == 3) {
            cv::cvtColor(mat0, mat, CV_RGB2GRAY);
        } else if (mat0.channels() == 4) {
            if (srcOrder == MCO_BGRA)
                cv::cvtColor(mat0, mat, CV_BGRA2GRAY);
            else if (srcOrder == MCO_RGBA)
                cv::cvtColor(mat0, mat, CV_RGBA2GRAY);
            else//MCO_ARGB
                cv::cvtColor(argb2bgra(mat0), mat, CV_BGRA2GRAY);
        }
        break;
    case 3:
        if (mat0.channels() == 1) {
            cv::cvtColor(mat0, mat, requriedOrder == MCO_BGR ? CV_GRAY2BGR : CV_GRAY2RGB);
        } else if (mat0.channels() == 3) {
            if (requriedOrder != srcOrder)
                cv::cvtColor(mat0, mat, CV_RGB2BGR);
            else
                mat = mat0;
        } else if (mat0.channels() == 4) {
            if (srcOrder == MCO_ARGB) {
                mat = cv::Mat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 3));
                int ARGB2RGB[] = {1,0, 2,1, 3,2};
                int ARGB2BGR[] = {1,2, 2,1, 3,0};
                cv::mixChannels(&mat0, 1, &mat, 1, requriedOrder == MCO_BGR ? ARGB2BGR : ARGB2RGB, 3);
            } else if (srcOrder == MCO_BGRA) {
                cv::cvtColor(mat0, mat, requriedOrder == MCO_BGR ? CV_BGRA2BGR : CV_BGRA2RGB);
            } else {//RGBA
                cv::cvtColor(mat0, mat, requriedOrder == MCO_BGR ? CV_RGBA2BGR : CV_RGBA2RGB);
            }
        }
        break;
    case 4:
        if (mat0.channels() == 1) {
            if (requriedOrder == MCO_ARGB) {
                cv::Mat alphaMat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 1), cv::Scalar(maxAlpha));
                mat = cv::Mat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 4));
                cv::Mat in[] = {alphaMat, mat0};
                int from_to[] = {0,0, 1,1, 1,2, 1,3};
                cv::mixChannels(in, 2, &mat, 1, from_to, 4);
            } else if (requriedOrder == MCO_RGBA) {
                cv::cvtColor(mat0, mat, CV_GRAY2RGBA);
            } else {//MCO_BGRA
                cv::cvtColor(mat0, mat, CV_GRAY2BGRA);
            }
        } else if (mat0.channels() == 3) {
            if (requriedOrder == MCO_ARGB) {
                cv::Mat alphaMat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 1), cv::Scalar(maxAlpha));
                mat = cv::Mat(mat0.rows, mat0.cols, CV_MAKE_TYPE(mat0.type(), 4));
                cv::Mat in[] = {alphaMat, mat0};
                int from_to[] = {0,0, 1,1, 2,2, 3,3};
                cv::mixChannels(in, 2, &mat, 1, from_to, 4);
            } else if (requriedOrder == MCO_RGBA) {
                cv::cvtColor(mat0, mat, CV_RGB2RGBA);
            } else {//MCO_BGRA
                cv::cvtColor(mat0, mat, CV_RGB2BGRA);
            }
        } else if (mat0.channels() == 4) {
            if (srcOrder == requriedOrder) {
                mat = mat0;
            } else {
                if ((srcOrder == MCO_ARGB && requriedOrder == MCO_BGRA)
                        ||(srcOrder == MCO_BGRA && requriedOrder == MCO_ARGB)) {
                    //ARGB <==> BGRA
                    mat = argb2bgra(mat0);
                } else if (srcOrder == MCO_ARGB && requriedOrder == MCO_RGBA) {
                    //ARGB ==> RGBA
                    mat = cv::Mat(mat0.rows, mat0.cols, mat0.type());
                    int from_to[] = {0,3, 1,0, 2,1, 3,2};
                    cv::mixChannels(&mat0, 1, &mat, 1, from_to, 4);
                } else if (srcOrder == MCO_RGBA && requriedOrder == MCO_ARGB) {
                    //RGBA ==> ARGB
                    mat = cv::Mat(mat0.rows, mat0.cols, mat0.type());
                    int from_to[] = {0,1, 1,2, 2,3, 3,0};
                    cv::mixChannels(&mat0, 1, &mat, 1, from_to, 4);
                } else {
                    //BGRA <==> RBGA
                    cv::cvtColor(mat0, mat, CV_BGRA2RGBA);
                }
            }
        }
        break;
    default:
        break;
    }

    return mat;
}

/* Convert cv::Mat to QImage, user should make sure that,
 * the color channels order is the same as the color channels
 * order requried by QImage.
 */
QImage mat2Image(const cv::Mat &mat, QImage::Format formatHint)
{
    if (mat.empty())
        return QImage();

    //Find proper image format based on Mat channels and format hint.
    QImage::Format format = formatHint;
    switch (mat.channels()) {
    case 1:
        format = QImage::Format_Indexed8;
        break;
    case 3:
        format = QImage::Format_RGB888;
        break;
    case 4:
        if (formatHint != QImage::Format_RGB32
                && formatHint != QImage::Format_ARGB32
                && formatHint != QImage::Format_ARGB32_Premultiplied
        #if QT_VERSION >= 0x050200
                && formatHint != QImage::Format_RGBX8888
                && formatHint != QImage::Format_RGBA8888
                && formatHint != QImage::Format_RGBA8888_Premultiplied
        #endif
                ) {
            format = QImage::Format_ARGB32;
        }
        break;
    default:
        return QImage();
    }

    QImage outImage;
    switch (mat.depth()) {
    case CV_8U:
        outImage = mat2Image_<uchar>(mat, format, 1.0);
        break;
    case CV_16U:
        outImage = mat2Image_<quint16>(mat, format, 255./65535.);
        break;
    case CV_32F:
        outImage = mat2Image_<float>(mat, format, 255.);
        break;
    default:
        break;
    }
    return outImage;
}

/* Convert cv::Mat to QImage
 */
QImage mat2Image(const cv::Mat &mat, MatColorOrder order, QImage::Format formatHint)
{
    Q_ASSERT(mat.channels()==1 || mat.channels()==3 || mat.channels()==4);
    Q_ASSERT(mat.depth()==CV_8U || mat.depth()==CV_16U || mat.depth()==CV_32S || mat.depth()==CV_32F);

    if (mat.empty())
        return QImage();

    QImage::Format format = formatHint;
    //Adjust mat channels if needed.
    cv::Mat newMat;
    if (mat.channels() == 3) {
        if (order == MCO_BGR)
            cv::cvtColor(mat, newMat, CV_BGR2RGB);
    } else if (mat.channels() == 4) {
        if (order == MCO_RGBA) {
#if QT_VERSION >= 0x050200
            if (formatHint != QImage::Format_RGBX8888
                    && formatHint != QImage::Format_RGBA8888
                    && formatHint != QImage::Format_RGBA8888_Premultiplied) {
                format = QImage::Format_RGBA8888;
            } else {
#endif
                if (formatHint != QImage::Format_RGB32
                        && formatHint != QImage::Format_ARGB32
                        && formatHint != QImage::Format_ARGB32_Premultiplied) {
                    format = QImage::Format_ARGB32;
                }
                if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
                    cv::cvtColor(mat, newMat, CV_RGBA2BGRA);
                } else {
                    //RGBA ==> ARGB
                    newMat = cv::Mat(mat.rows, mat.cols, mat.type());
                    int from_to[] = {0,1, 1,2, 2,3, 3,0};
                    cv::mixChannels(&mat, 1, &newMat, 1, from_to, 4);
                }
#if QT_VERSION >= 0x050200
            }
#endif
        } else {
            if (formatHint != QImage::Format_RGB32
                    && formatHint != QImage::Format_ARGB32
                    && formatHint != QImage::Format_ARGB32_Premultiplied ) {
                format = QImage::Format_ARGB32;
            }

            if ((order == MCO_BGRA && QSysInfo::ByteOrder == QSysInfo::BigEndian)
                    || (order == MCO_ARGB && QSysInfo::ByteOrder == QSysInfo::LittleEndian)) {
                newMat = argb2bgra(mat);
            }
        }
    }
    if (newMat.empty())
        newMat = mat;

    return mat2Image(newMat, format);
}

/* Convert QImage to cv::Mat without data copy
 */
cv::Mat image2Mat_shared(const QImage &img, MatColorOrder *order)
{
    if (img.isNull())
        return cv::Mat();

    switch (img.format()) {
    case QImage::Format_Indexed8:
    case QImage::Format_RGB888:
        if (order)
            *order = MCO_RGB;
        break;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        if (order)
            *order = QSysInfo::ByteOrder == QSysInfo::LittleEndian ? MCO_BGRA : MCO_ARGB;
        break;
#if QT_VERSION >= 0x050200
    case QImage::Format_RGBX8888:
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
        if (order)
            *order = MCO_RGBA;
        break;
#endif
    default:
        return cv::Mat();
    }
    return cv::Mat(img.height(), img.width(), CV_8UC(img.depth()/8), (uchar*)img.bits(), img.bytesPerLine());
}

/* Convert  cv::Mat to QImage without data copy
 */
QImage mat2Image_shared(const cv::Mat &mat, QImage::Format formatHint)
{
    Q_ASSERT(mat.type() == CV_8UC1 || mat.type() == CV_8UC3 || mat.type() == CV_8UC4);

    if (mat.empty())
        return QImage();

    QImage img;
    if (mat.type() == CV_8UC1) {
        img = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
        QVector<QRgb> colorTable;
        for (int i=0; i<256; ++i)
            colorTable.append(qRgb(i,i,i));
        img.setColorTable(colorTable);
    } else if (mat.type() == CV_8UC3) {
        img = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
    } else if (mat.type() == CV_8UC4) {
        if (formatHint != QImage::Format_RGB32
                && formatHint != QImage::Format_ARGB32
                && formatHint != QImage::Format_ARGB32_Premultiplied
        #if QT_VERSION >= 0x050200
                && formatHint != QImage::Format_RGBX8888
                && formatHint != QImage::Format_RGBA8888
                && formatHint != QImage::Format_RGBA8888_Premultiplied
        #endif
                ) {
            formatHint = QImage::Format_ARGB32;
        }
        img = QImage(mat.data, mat.cols, mat.rows, mat.step, formatHint);
    }

    return img;
}

} //namespace QtOcv
