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
cv::Mat image2Mat(const QImage &img, MatColorOrder *order, int matDepth)
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

/* Convert QImage to cv::Mat
 */
cv::Mat image2Mat(const QImage &img, int matType, MatColorOrder order)
{
    Q_ASSERT(CV_MAT_CN(matType) == CV_CN_MAX || CV_MAT_CN(matType)==1 \
             || CV_MAT_CN(matType)==3 || CV_MAT_CN(matType)==4);

    MatColorOrder imageOrder;
    cv::Mat mat0 = image2Mat(img, &imageOrder, CV_MAT_DEPTH(matType));

    cv::Mat mat;

    //Adjust mat if needed.
    int targetChannels = CV_MAT_CN(matType) == CV_CN_MAX ? mat0.channels() : CV_MAT_CN(matType);
    switch(targetChannels) {
    case 1:
        if (mat0.channels() == 1) {
            mat = mat0;
        } else if (mat0.channels() == 3) {
            cv::cvtColor(mat0, mat, CV_RGB2GRAY);
        } else if (mat0.channels() == 4) {
            if (imageOrder == MCO_BGRA) {
                cv::cvtColor(mat0, mat, CV_BGRA2GRAY);
            } else if (imageOrder == MCO_RGBA) {
                cv::cvtColor(mat0, mat, CV_RGBA2GRAY);
            } else { //MCO_ARGB
                qWarning("Not support yet.");
            }
        }
        break;
    case 3:
        if (mat0.channels() == 1) {
            cv::cvtColor(mat0, mat, order == MCO_BGR ? CV_GRAY2BGR : CV_GRAY2RGB);
        } else if (mat0.channels() == 3) {
            if (order != imageOrder)
                cv::cvtColor(mat0, mat, CV_RGB2BGR);
            else
                mat = mat0;
        } else if (mat0.channels() == 4) {
            if (imageOrder == MCO_ARGB) {
                qWarning("Not support at present");
            } else if (imageOrder == MCO_BGRA) {
                cv::cvtColor(mat0, mat, order == MCO_BGR ? CV_BGRA2BGR : CV_BGRA2RGB);
            } else {//RGBA
                cv::cvtColor(mat0, mat, order == MCO_BGR ? CV_RGBA2BGR : CV_RGBA2RGB);
            }
        }
        break;
    case 4:
        if (mat0.channels() == 1) {
            if (order == MCO_ARGB)
                qWarning("Not support yet.");
            else if (order == MCO_RGBA)
                cv::cvtColor(mat0, mat, CV_GRAY2RGBA);
            else //MCO_BGRA
                cv::cvtColor(mat0, mat, CV_GRAY2BGRA);
        } else if (mat0.channels() == 3) {
            if (order == MCO_ARGB)
                qWarning("Not support yet.");
            else if (order == MCO_RGBA)
                cv::cvtColor(mat0, mat, CV_RGB2RGBA);
            else //MCO_BGRA
                cv::cvtColor(mat0, mat, CV_RGB2BGRA);
        } else if (mat0.channels() == 4) {
            if (imageOrder == order) {
                mat = mat0;
            } else {
                if (imageOrder == MCO_ARGB || order == MCO_ARGB)
                    qWarning("Not support yet.");
                else
                    cv::cvtColor(mat0, mat, CV_BGRA2RGBA);
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
        if (formatHint != QImage::Format_RGB32 && formatHint != QImage::Format_ARGB32
        #if QT_VERSION >= 0x050200
                && formatHint != QImage::Format_RGBA8888 && formatHint != QImage::Format_RGBX8888
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
    case CV_32S:
        outImage = mat2Image_<qint32>(mat, format, 255./65535.);
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
    if (mat.channels() == 3) {
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
                else
                    qWarning("RGBA is not supported.");
#if QT_VERSION >= 0x050200
            }
#endif
        } else if (order == MCO_BGRA) {
            if (formatHint != QImage::Format_ARGB32 && formatHint != QImage::Format_RGB32)
                format = QImage::Format_ARGB32;
            if (QSysInfo::ByteOrder == QSysInfo::BigEndian)
                qWarning("BGRA is not supported.");
        } else if (order == MCO_ARGB) {
            if (formatHint != QImage::Format_ARGB32 && formatHint != QImage::Format_RGB32)
                format = QImage::Format_ARGB32;
            if (QSysInfo::ByteOrder == QSysInfo::LittleEndian)
                qWarning("ARGB is not supported.");
        }
    }
    if (newMat.empty())
        newMat = mat;

    return mat2Image(newMat, format);
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
