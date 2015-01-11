/****************************************************************************
** Copyright (c) 2012 Debao Zhang <hello@debao.me>
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
        uchar *data = outImage.scanLine(row);
        for (int col=0; col<mat.cols * mat.channels(); ++col)
            *data++ = cv::saturate_cast<uchar>(mat.at<T>(row, col) * scalefactor);
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
        const uchar *data = image.scanLine(row);
        for (int col = 0; col < mat.cols * channels; ++col)
            mat.at<T>(row, col) = cv::saturate_cast<T>(data[col] * scaleFactor);
    }
    return mat;
}

} //namespace

namespace QtOcv {

/* Convert QImage to cv::Mat
 */
cv::Mat image2Mat(const QImage &img, int matDepth, MatColorOrder *order)
{
    if (img.isNull())
        return cv::Mat();

    QImage image;
    switch (img.format()) {
    case QImage::Format_Indexed8:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_RGB888:
#if QT_VERSION >= 0x050200
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBX8888:
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
    default:
        image = img.convertToFormat(QImage::Format_ARGB32);
        break;
    }

    cv::Mat mat;
    const int matType = CV_MAKETYPE(matDepth, image.depth()/8);
    switch (CV_MAT_DEPTH(matDepth)) {
    case CV_8U:
        mat = image2Mat_<uchar>(image, matType, 1.0);
        break;
    case CV_16U:
        mat = image2Mat_<quint16>(image, matType, 65535./255.);
        break;
    case CV_32S:
        mat = image2Mat_<qint32>(image, matType, 65535./255.);
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
        case QImage::Format_ARGB32:
        case QImage::Format_RGB32:
            *order = QSysInfo::ByteOrder == QSysInfo::LittleEndian ? MCO_BGRA : MCO_ARGB;
            break;
#if QT_VERSION >= 0x050200
        case QImage::Format_RGBA8888:
        case QImage::Format_RGBX8888:
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

/* Convert cv::Mat to QImage
 *
 * Channels of cv::Mat should be 1, 3, 4
 *
 * Format of QImage should be:
 * - QImage::Format_Indexed8
 * - QImage::Format_RGB888
 * - QImage::Format_ARGB32
 * - QImage::Format_RGB32
 * - QImage::Format_RGBA8888
 * - QImage::Format_RGBX8888
 * - QImage::Format_Invalid(means auto selection),
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
    if (mat.channels() == 1) {
        format = QImage::Format_Indexed8;
    } else if (mat.channels() == 3) {
        format = QImage::Format_RGB888;
        if (order == MCO_BGR)
            cv::cvtColor(mat, newMat, CV_BGR2RGB);
    } else if (mat.channels() == 4) {
        if (order == MCO_RGBA) {
#if QT_VERSION >= 0x050200
            if (formatHint != QImage::Format_RGBA8888 && formatHint != QImage::Format_RGBX8888) {
                format = QImage::Format_RGBA8888;
            } else {
#endif
                if (formatHint != QImage::Format_ARGB32 && formatHint != QImage::Format_RGB32)
                    format = QImage::Format_ARGB32;
                if (QSysInfo::ByteOrder == QSysInfo::LittleEndian)
                    cv::cvtColor(mat, newMat, CV_RGBA2BGRA);
//                else
//                    cv::cvtColor(mat, newMat, CV_RGBA2ARGB);
#if QT_VERSION >= 0x050200
            }
#endif
        } else if (order == MCO_BGRA) {
            if (formatHint != QImage::Format_ARGB32 && formatHint != QImage::Format_RGB32)
                format = QImage::Format_ARGB32;
//            if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
//                cv::cvtColor(mat, newMat, CV_BGRA2ARGB);
        } else if (order == MCO_ARGB) {
            if (formatHint != QImage::Format_ARGB32 && formatHint != QImage::Format_RGB32)
                format = QImage::Format_ARGB32;
//            if (QSysInfo::ByteOrder == QSysInfo::LittleEndian)
//                cv::cvtColor(mat, newMat, CV_ARGB2BGRA);
        }
    }
    if (newMat.empty())
        newMat = mat;

    QImage outImage;
    switch (mat.depth()) {
    case CV_8U:
        outImage = mat2Image_<uchar>(newMat, format, 1.0);
        break;
    case CV_16U:
        outImage = mat2Image_<quint16>(newMat, format, 255./65535.);
        break;
    case CV_32S:
        outImage = mat2Image_<qint32>(newMat, format, 255./65535.);
        break;
    case CV_32F:
        outImage = mat2Image_<float>(newMat, format, 255.);
        break;
    default:
        break;
    }

    return outImage;
}

/* Convert QImage to cv::Mat without data copy
 *
 * - Supported QImage format are:
 *   - QImage::Format_Indexed8 ==> CV_8UC1
 *   - QImage::Format_RGB888   ==> CV_8UC3 (R G B)
 *   - QImage::Format_RGB32    ==> CV_8UC4 (A R G B or B G R A)
 *   - QImage::Format_ARGB32   ==> CV_8UC4 (A R G B or B G R A)
 *   - QImage::Format_RGBX8888 ==> CV_8UC4 (R G B A)
 *   - QImage::Format_RGBA8888 ==> CV_8UC4 (R G B A)
 *
 * - For QImage::Format_RGB32 and QImage::Format_ARGB32, the
 *   color channel order of generated cv::Mat will be (B G R A)
 *   in little endian system or (A R G B) in big endian system.
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
        if (order)
            *order = QSysInfo::ByteOrder == QSysInfo::LittleEndian ? MCO_BGRA : MCO_ARGB;
        break;
#if QT_VERSION >= 0x050200
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBX8888:
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
 *
 * - Supported type of cv::Mat are:
 *   - CV_8UC1            ==> QImage::Format_Indexed8
 *   - CV_8UC3 (R G B)    ==> QImage::Format_RGB888
 *   - CV_8UC4 (B G R A)  ==> QImage::Format_ARGB32 or QImage::Format_RGB32
 *   - CV_8UC4 (A R G B)  ==> QImage::Format_ARGB32 or QImage::Format_RGB32
 *   - CV_8UC4 (R G B A)  ==> QImage::Format_RGBA8888 or QImage::Format_RGBX8888
 *
 * - Note that,
 *   - CV_8UC4 (B G R A) works in little endian system only
 *   - CV_8UC4 (A R G B) works in big endian system only.
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
        if (formatHint != QImage::Format_RGB32 && formatHint != QImage::Format_ARGB32
        #if QT_VERSION >= 0x050200
                && formatHint != QImage::Format_RGBA8888 && formatHint != QImage::Format_RGBX8888
        #endif
                ) {
            formatHint = QImage::Format_RGB32;
        }
        img = QImage(mat.data, mat.cols, mat.rows, mat.step, formatHint);
    }

    return img;
}

} //namespace QtOcv
