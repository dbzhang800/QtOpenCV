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

template<typename T>
QImage mat2Image_(const cv::Mat & mat, QImage::Format format, QtOcv::MatChannelOrder matRgbOrder, double scalefactor)
{
    Q_ASSERT(QSysInfo::ByteOrder == QSysInfo::LittleEndian);
    Q_ASSERT(mat.channels()==1 || mat.channels()==3 || mat.channels()==4);
    Q_ASSERT(format == QImage::Format_ARGB32 || format == QImage::Format_RGB32 \
             || format == QImage::Format_RGB888 || format == QImage::Format_Indexed8 \
             || format == QImage::Format_Invalid);

    QImage outImage(mat.cols, mat.rows, format);
    const int mat_channels = mat.channels();
    const int mat_red = matRgbOrder == QtOcv::MCO_BGR ? 2 : 0;
    const int mat_blue = 2 - mat_red;

    if (format == QImage::Format_ARGB32 || format == QImage::Format_RGB32 /*B G R A*/) {
        for (int i=0; i<mat.rows; ++i) { //deal with one line by one lne
            uchar * data = outImage.scanLine(i);
            if (mat_channels == 1) {
                for (int j=0; j<mat.cols; ++j) {
                    T val = cv::saturate_cast<uchar>(mat.at<T>(i, j) * scalefactor);
                    *data++ = val;
                    *data++ = val;
                    *data++ = val;
                    *data++ = 255;
                }
            } else if (mat_channels == 3) {
                for (int j=0; j<mat.cols; ++j) {
                    const cv::Vec<T, 3> & vec = mat.at<cv::Vec<T,3> >(i, j);
                    *data++ = cv::saturate_cast<uchar>(vec[mat_blue] * scalefactor);
                    *data++ = cv::saturate_cast<uchar>(vec[1] * scalefactor);
                    *data++ = cv::saturate_cast<uchar>(vec[mat_red] * scalefactor);
                    *data++ = 255;
                }
            } else { //channels == 4
                for (int j=0; j<mat.cols; ++j) {
                    const cv::Vec<T, 4> & vec = mat.at<cv::Vec<T,4> >(i, j);
                    *data++ = cv::saturate_cast<uchar>(vec[mat_blue] * scalefactor);
                    *data++ = cv::saturate_cast<uchar>(vec[1] * scalefactor);
                    *data++ = cv::saturate_cast<uchar>(vec[mat_red] * scalefactor);
                    *data++ = format == QImage::Format_ARGB32 ? cv::saturate_cast<uchar>(vec[3] * scalefactor) : 255;
                }
            }
        }
    } else if (format == QImage::Format_RGB888 /*R G B*/){
        for (int i=0; i<mat.rows; ++i) {
            uchar * data = outImage.scanLine(i);
            if (mat_channels == 1) {
                for (int j=0; j<mat.cols; ++j) {
                    T val = cv::saturate_cast<uchar>(mat.at<T>(i, j) * scalefactor);
                    *data++ = val;
                    *data++ = val;
                    *data++ = val;
                }
            } else if (mat_channels == 3) {
                for (int j=0; j<mat.cols; ++j) {
                    const cv::Vec<T, 3> & vec = mat.at<cv::Vec<T,3> >(i, j);
                    *data++ = cv::saturate_cast<uchar>(vec[mat_red] * scalefactor);
                    *data++ = cv::saturate_cast<uchar>(vec[1] * scalefactor);
                    *data++ = cv::saturate_cast<uchar>(vec[mat_blue] * scalefactor);
                }
            } else {
                for (int j=0; j<mat.cols; ++j) {
                    const cv::Vec<T, 4> & vec = mat.at<cv::Vec<T,4> >(i, j);
                    *data++ = cv::saturate_cast<uchar>(vec[mat_red] * scalefactor);
                    *data++ = cv::saturate_cast<uchar>(vec[1] * scalefactor);
                    *data++ = cv::saturate_cast<uchar>(vec[mat_blue] * scalefactor);
                }
            }
        }
    } else if (format == QImage::Format_Indexed8) {
        QVector<QRgb> colorTable;
        for (int i=0; i<256; ++i)
            colorTable.append(qRgb(i,i,i));
        outImage.setColorTable(colorTable);

        for (int i=0; i<mat.rows; ++i) {
            uchar * data = outImage.scanLine(i);
            if (mat_channels == 1) {
                for (int j=0; j<mat.cols; ++j)
                    *data++ = cv::saturate_cast<uchar>(mat.at<T>(i, j) * scalefactor);
            } else if (mat_channels == 3) {
                for (int j=0; j<mat.cols; ++j) {
                    const cv::Vec<T, 3> & vec = mat.at<cv::Vec<T,3> >(i, j);
                    *data++  = cv::saturate_cast<uchar>((vec[mat_red] * 0.299 + vec[1] * 0.587 + vec[mat_blue]*0.114)*scalefactor);
                }
            } else {
                for (int j=0; j<mat.cols; ++j) {
                    const cv::Vec<T, 4> & vec = mat.at<cv::Vec<T,4> >(i, j);
                    *data++  = cv::saturate_cast<uchar>((vec[mat_red] * 0.299 + vec[1] * 0.587 + vec[mat_blue]*0.114)*scalefactor);
                }
            }
        }
    }

    return outImage;
}

#if 1
template<>
QImage mat2Image_<uchar>(const cv::Mat & mat, QImage::Format format, QtOcv::MatChannelOrder matRgbOrder, double /*scalefactor*/)
{
    Q_ASSERT(QSysInfo::ByteOrder == QSysInfo::LittleEndian);
    Q_ASSERT(mat.channels()==1 || mat.channels()==3 || mat.channels()==4);
    Q_ASSERT(format == QImage::Format_ARGB32 || format == QImage::Format_RGB32 \
             || format == QImage::Format_RGB888 || format == QImage::Format_Indexed8 \
             || format == QImage::Format_Invalid);

    QImage outImage(mat.cols, mat.rows, format);
    const int mat_channels = mat.channels();
    const int mat_red = matRgbOrder == QtOcv::MCO_BGR ? 2 : 0;
    const int mat_blue = 2 - mat_red;

    if (format == QImage::Format_ARGB32 || format == QImage::Format_RGB32 /*B G R A*/) {
        if (mat_channels == 4 && matRgbOrder == QtOcv::MCO_RGBA && format == QImage::Format_ARGB32)
            cv::cvtColor(mat, mat, CV_RGBA2BGRA);

        for (int i=0; i<mat.rows; ++i) {
            uchar * data = outImage.scanLine(i);
            if (mat_channels == 1) {
                for (int j=0; j<mat.cols; ++j) {
                    uchar val = mat.at<uchar>(i, j);
                    *data++ = val;
                    *data++ = val;
                    *data++ = val;
                    *data++ = 255;
                }
            } else if (mat_channels == 3) {
                for (int j=0; j<mat.cols; ++j) {
                    const cv::Vec3b & vec = mat.at<cv::Vec3b>(i, j);
                    *data++ = vec[mat_blue];
                    *data++ = vec[1];
                    *data++ = vec[mat_red];
                    *data++ = 255;
                }
            } else if (mat_channels == 4 && format == QImage::Format_ARGB32) {
                std::memcpy(data, mat.row(i).data, mat.cols*4);
            } else { //CV_8UC4, QImage::Format_RGB32
                for (int j=0; j<mat.cols; ++j) {
                    const cv::Vec4b & vec = mat.at<cv::Vec4b>(i, j);
                    *data++ = vec[mat_blue];
                    *data++ = vec[1];
                    *data++ = vec[mat_red];
                    *data++ = 255;
                }
            }
        }
    } else if (format == QImage::Format_RGB888 /*R G B*/){
        if (mat_channels == 3 && matRgbOrder == QtOcv::MCO_BGR)
            cv::cvtColor(mat, mat, CV_BGR2RGB);

        for (int i=0; i<mat.rows; ++i) {
            uchar * data = outImage.scanLine(i);
            if (mat_channels == 1) {
                for (int j=0; j<mat.cols; ++j) {
                    uchar val = mat.at<uchar>(i, j);
                    *data++ = val;
                    *data++ = val;
                    *data++ = val;
                }
            } else if (mat_channels == 3) {
                std::memcpy(data, mat.row(i).data, mat.cols*3);
            } else if (mat_channels == 4) {
                for (int j=0; j<mat.cols; ++j) {
                    const cv::Vec4b & vec = mat.at<cv::Vec4b>(i, j);
                    *data++ = vec[mat_red];
                    *data++ = vec[1];
                    *data++ = vec[mat_blue];
                }
            }
        }
    } else if (format == QImage::Format_Indexed8) {
        cv::Mat grayMat = mat;
        if (mat_channels == 3)
            cv::cvtColor(mat, grayMat, matRgbOrder == QtOcv::MCO_BGR ? CV_BGR2GRAY : CV_RGB2GRAY, 1);
        else if (mat_channels == 4)
            cv::cvtColor(mat, grayMat, matRgbOrder == QtOcv::MCO_BGR ? CV_BGRA2GRAY : CV_RGBA2GRAY, 1);

        QVector<QRgb> colorTable;
        for (int i=0; i<256; ++i)
            colorTable.append(qRgb(i,i,i));
        outImage.setColorTable(colorTable);

        for (int i=0; i<grayMat.rows; ++i) {
            uchar * data = outImage.scanLine(i);
            std::memcpy(data, grayMat.row(i).data, grayMat.cols);
        }
    }

    return outImage;
}
#endif

template<typename T>
cv::Mat image2Mat_(const QImage &image, int matType, QtOcv::MatChannelOrder matRgbOrder, double scaleFactor)
{
    Q_ASSERT(QSysInfo::ByteOrder == QSysInfo::LittleEndian);
    Q_ASSERT(format == QImage::Format_ARGB32 || format == QImage::Format_RGB32 \
             || format == QImage::Format_RGB888 || format == QImage::Format_Indexed8);

    const int channels = CV_MAT_CN(matType);
    cv::Mat mat = cv::Mat(image.height(), image.width(), matType);

    if (channels == 1) {
        for (int i=0; i<mat.rows; ++i) {
            const uchar * data = image.scanLine(i);
            if (image.format() == QImage::Format_Indexed8) {
                for (int j=0; j<mat.cols; ++j)
                    mat.at<T>(i, j) = *data * scaleFactor;
            } else if (image.format() == QImage::Format_RGB888) {
                for (int j=0; j<mat.cols; ++j, data+=3) {
                    uchar val = static_cast<uchar>((data[0] * 3728 + data[1] * 19238 + data[2]*9798)/32768);
                    mat.at<T>(i, j) = val * scaleFactor;
                }
            } else {
                for (int j=0; j<mat.cols; ++j, data+=4) {
                    uchar val = static_cast<uchar>((data[2] * 3728 + data[1] * 19238 + data[0]*9798)/32768);
                    mat.at<uchar>(i, j) = val * scaleFactor;
                }
            }
        }
    } else if (channels == 3) {
        for (int i=0; i<mat.rows; ++i) {
            const uchar * data = image.scanLine(i);
            if (image.format() == QImage::Format_Indexed8) {
                for (int j=0; j<mat.cols; ++j, ++data) {
                    T val = cv::saturate_cast<T>(*data * scaleFactor);
                    mat.at<cv::Vec<T,3> >(i, j) = cv::Vec<T,3>(val, val, val);
                }
            } else if (image.format() == QImage::Format_RGB888) {
                int first = matRgbOrder == QtOcv::MCO_BGR ? 2 : 0;
                int third = 2 - first;
                for (int j=0; j<mat.cols; ++j, data+=3) {
                    T v0 = cv::saturate_cast<T>(data[first] * scaleFactor);
                    T v1 = cv::saturate_cast<T>(data[1] * scaleFactor);
                    T v2 = cv::saturate_cast<T>(data[third] * scaleFactor);
                    mat.at<cv::Vec<T,3> >(i, j) = cv::Vec<T,3>(v0, v1, v2);
                }
            } else { //QImage::Format_RGB32 || QImage::Format_ARGB32
                int first = matRgbOrder == QtOcv::MCO_BGR ? 0 : 2;
                int third = 2 - first;
                for (int j=0; j<mat.cols; ++j, data+=4) {
                    T v0 = cv::saturate_cast<T>(data[first] * scaleFactor);
                    T v1 = cv::saturate_cast<T>(data[1] * scaleFactor);
                    T v2 = cv::saturate_cast<T>(data[third] * scaleFactor);
                    mat.at<cv::Vec<T,3> >(i, j) = cv::Vec<T,3>(v0, v1, v2);
                }
            }
        }
    } else if (channels == 4) {
        for (int i=0; i<mat.rows; ++i) {
            const uchar * data = image.scanLine(i);
            if (image.format() == QImage::Format_Indexed8) {
                for (int j=0; j<mat.cols; ++j, ++data)
                    mat.at<cv::Vec<T,4> >(i, j) = cv::Vec<T,4>(*data, *data, *data, 255);
            } else if (image.format() == QImage::Format_RGB888) {
                int first = matRgbOrder == QtOcv::MCO_BGRA ? 2 : 0;
                int third = 2 - first;
                for (int j=0; j<mat.cols; ++j, data+=3)
                    mat.at<cv::Vec<T,4> >(i, j) = cv::Vec<T,4>(data[first], data[1], data[third], 255);
            } else { //QImage::Format_RGB32 || QImage::Format_ARGB32
                int first = matRgbOrder == QtOcv::MCO_BGRA ? 0 : 2;
                int third = 2 - first;
                for (int j=0; j<mat.cols; ++j, data+=4)
                    mat.at<cv::Vec<T,4> >(i, j) = cv::Vec<T,4>(data[first], data[1], data[third], data[3]);
            }
        }
    }
    return mat;
}

#if 1
template<>
cv::Mat image2Mat_<uchar>(const QImage &image, int matType, QtOcv::MatChannelOrder matRgbOrder, double /*scaleFactor*/ )
{
    Q_ASSERT(QSysInfo::ByteOrder == QSysInfo::LittleEndian);
    Q_ASSERT(CV_MAT_DEPTH(matType) == CV_8U);
    Q_ASSERT(format == QImage::Format_ARGB32 || format == QImage::Format_RGB32 \
             || format == QImage::Format_RGB888 || format == QImage::Format_Indexed8);

    const int channels = CV_MAT_CN(matType);
    cv::Mat mat = cv::Mat(image.height(), image.width(), matType);

    if (channels == 1) {
        for (int i=0; i<mat.rows; ++i) {
            const uchar * data = image.scanLine(i);
            if (image.format() == QImage::Format_Indexed8) {
                std::memcpy(mat.row(i).data, data, image.width());
            } else if (image.format() == QImage::Format_RGB888) {
                for (int j=0; j<mat.cols; ++j, data+=3)
                    mat.at<uchar>(i, j) = (data[0] * 3728 + data[1] * 19238 + data[2]*9798)/32768;
            } else {
                for (int j=0; j<mat.cols; ++j, data+=4)
                    mat.at<uchar>(i, j) = (data[2] * 3728 + data[1] * 19238 + data[0]*9798)/32768;
            }
        }
    } else if (channels == 3) {
        for (int i=0; i<mat.rows; ++i) {
            const uchar * data = image.scanLine(i);
            if (image.format() == QImage::Format_Indexed8) {
                for (int j=0; j<mat.cols; ++j, ++data)
                    mat.at<cv::Vec3b>(i, j) = cv::Vec3b(*data, *data, *data);
            } else if (image.format() == QImage::Format_RGB888) {
                if (matRgbOrder == QtOcv::MCO_RGB) {
                    std::memcpy(mat.row(i).data, data, image.width()*3);
                } else if (image.format() == QImage::Format_RGB888) { // BGR
                    for (int j=0; j<mat.cols; ++j, data+=3)
                        mat.at<cv::Vec3b>(i, j) = cv::Vec3b(data[2], data[1], data[0]);
                }
            } else { //QImage::Format_RGB32 || QImage::Format_ARGB32
                int first = matRgbOrder == QtOcv::MCO_BGR ? 0 : 2;
                int third = 2 - first;
                for (int j=0; j<mat.cols; ++j, data+=4)
                    mat.at<cv::Vec3b>(i, j) = cv::Vec3b(data[first], data[1], data[third]);
            }
        }
    } else if (channels == 4) {
        for (int i=0; i<mat.rows; ++i) {
            const uchar * data = image.scanLine(i);
            if (image.format() == QImage::Format_Indexed8) {
                for (int j=0; j<mat.cols; ++j, ++data)
                    mat.at<cv::Vec4b>(i, j) = cv::Vec4b(*data, *data, *data, 255);
            } else if (image.format() == QImage::Format_RGB888) {
                int first = matRgbOrder == QtOcv::MCO_BGRA ? 2 : 0;
                int third = 2 - first;
                for (int j=0; j<mat.cols; ++j, data+=3)
                    mat.at<cv::Vec4b>(i, j) = cv::Vec4b(data[first], data[1], data[third], 255);
            } else { //QImage::Format_RGB32 || QImage::Format_ARGB32
                if (matRgbOrder == QtOcv::MCO_BGRA) {
                    std::memcpy(mat.row(i).data, data, image.width()*4);
                } else {// RGBA
                    for (int j=0; j<mat.cols; ++j, data+=4)
                        mat.at<cv::Vec4b>(i, j) = cv::Vec4b(data[2], data[1], data[0], data[3]);
                }
            }
        }
    }
    return mat;
}
#endif

} //namespace

namespace QtOcv {

/* Convert QImage to cv::Mat
 *
 * - Supported mat channels are 0, 1, 3, 4,  where 0 means selecting based on QImage's format
 * - Depth of generated cv::Mat is CV_8U
 */
cv::Mat image2Mat(const QImage &img, int matType, MatChannelOrder matRgbOrder)
{
    Q_ASSERT(QSysInfo::ByteOrder == QSysInfo::LittleEndian);
    Q_ASSERT(CV_MAT_CN(matType) == 0 || CV_MAT_CN(matType)==1 \
             || CV_MAT_CN(matType)==3 || CV_MAT_CN(matType)==4);

    if (img.isNull())
        return cv::Mat();

    QImage image;
    switch (img.format()) {
    case QImage::Format_Indexed8:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_RGB888:
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
        image = img.convertToFormat(QImage::Format_RGB32);
        break;
    }

    const int channels = CV_MAT_CN(matType)==CV_CN_MAX ? image.depth()/8 : CV_MAT_CN(matType);
    const int type = CV_MAKETYPE(matType, channels);
    cv::Mat mat;

    switch (CV_MAT_DEPTH(matType)) {
    case CV_8U:
        mat = image2Mat_<uchar>(image, type, matRgbOrder, 1.0);
        break;
    case CV_16U:
        mat = image2Mat_<quint16>(image, type, matRgbOrder, 65535./255.);
        break;
    case CV_32S:
        mat = image2Mat_<qint32>(image, type, matRgbOrder, 65535./255.);
        break;
    case CV_32F:
        mat = image2Mat_<float>(image, type, matRgbOrder, 1./255.);
        break;
    case CV_64F:
        mat = image2Mat_<double>(image, type, matRgbOrder, 1./255.);
        break;
    default:
        break;
    }

    return mat;
}

/* Convert cv::Mat to QImage
 *
 * Channels of cv::Mat should be 1, 3, 4
 * Format of QImage should be ARGB32,RGB32,RGB888,Indexed8 or Invalid(means auto selection),
 */
QImage mat2Image(const cv::Mat & mat, QImage::Format format, MatChannelOrder matRgbOrder)
{
    Q_ASSERT(QSysInfo::ByteOrder == QSysInfo::LittleEndian);
    Q_ASSERT(mat.channels()==1 || mat.channels()==3 || mat.channels()==4);
    Q_ASSERT(format == QImage::Format_ARGB32 || format == QImage::Format_RGB32 \
             || format == QImage::Format_RGB888 || format == QImage::Format_Indexed8 \
             || format == QImage::Format_Invalid);

    if (mat.empty())
        return QImage();

    if (format == QImage::Format_Invalid) {
        if (mat.channels() == 1)
            format = QImage::Format_Indexed8;
        else if (mat.channels() == 3)
            format = QImage::Format_RGB888;
        else
            format = QImage::Format_ARGB32;
    }

    QImage outImage;
    switch (mat.depth()) {
    case CV_8U:
        outImage = mat2Image_<uchar>(mat, format, matRgbOrder, 1.0);
        break;
    case CV_16U:
        outImage = mat2Image_<quint16>(mat, format, matRgbOrder, 255./65535.);
        break;
    case CV_32S:
        outImage = mat2Image_<qint32>(mat, format, matRgbOrder, 255./65535.);
        break;
    case CV_32F:
        outImage = mat2Image_<float>(mat, format, matRgbOrder, 255.);
        break;
    case CV_64F:
        outImage = mat2Image_<double>(mat, format, matRgbOrder, 255.);
        break;
    default:
        break;
    }

    return outImage;
}

/* Convert QImage to cv::Mat without data copy
 *
 * - Supported QImage format is QImage::Format_Indexed8, Format_RGB888, Format_RGB32, Format_ARGB32
 * - Type of generated cv::Mat is CV_8UC1, CV_8UC3(R G B order), CV_8UC4 (B G R A order)
 */
cv::Mat image2Mat_shared(const QImage &img)
{
    Q_ASSERT(QSysInfo::ByteOrder == QSysInfo::LittleEndian);
    Q_ASSERT(img.format() == QImage::Format_Indexed8 || img.format() == QImage::Format_RGB888
             || img.format() == QImage::Format_RGB32 || img.format() == QImage::Format_ARGB32);

    if (img.isNull())
        return cv::Mat();

    switch (img.format()) {
    case QImage::Format_Indexed8:
    case QImage::Format_RGB888:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
        return cv::Mat(img.height(), img.width(), CV_8UC(img.depth()/8), (uchar*)img.bits(), img.bytesPerLine());
    default:
        return cv::Mat();
    }
}

/* Convert  cv::Mat to QImage without data copy
 *
 * - Supported type of cv::Mat is CV_8UC1, CV_8UC3(R G B order), CV_8UC4 (B G R A order)
 * - QImage format is QImage::Format_Indexed8, Format_RGB888, Format_ARGB32
 */
QImage mat2Image_shared(const cv::Mat &mat)
{
    Q_ASSERT(QSysInfo::ByteOrder == QSysInfo::LittleEndian);
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
        img = QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
    }

    return img;
}

} //namespace QtOcv
