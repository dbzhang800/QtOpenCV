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

#ifndef CVMATANDQIMAGE_H
#define CVMATANDQIMAGE_H

#include <QImage>
#include <opencv2/core/core.hpp>

namespace cv {
class Mat;
}

namespace QtOcv {

enum MatChannelOrder
{
    MCO_BGR,
    MCO_RGB,
    MCO_RGBA = MCO_RGB,
    MCO_BGRA = MCO_BGR
};

//Standard convert, MatChannelOrder will be skipped if cv::Mat has only one channel
cv::Mat image2Mat(const QImage &img, int matType = CV_8UC(0), MatChannelOrder matRgbOrder = MCO_BGR);
QImage mat2Image(const cv::Mat &mat, QImage::Format format = QImage::Format_Invalid, MatChannelOrder matRgbOrder = MCO_BGR);

//Convert without data copy. MatChannelOrder should be R G B (3 channels) ,B G R A(4 channels in little endian system)
//or A R G B (4 channels in big endian system)
cv::Mat image2Mat_shared(const QImage &img);
QImage mat2Image_shared(const cv::Mat &mat);

} //namespace QtOcv

#endif // CVMATANDQIMAGE_H
