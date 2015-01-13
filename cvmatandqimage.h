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

#ifndef CVMATANDQIMAGE_H
#define CVMATANDQIMAGE_H

#include <QtGui/qimage.h>
#include <opencv2/core/core.hpp>

namespace cv {
class Mat;
}

namespace QtOcv {

enum MatColorOrder {
    MCO_BGR,
    MCO_RGB,
    MCO_BGRA = MCO_BGR,
    MCO_RGBA = MCO_RGB,
    MCO_ARGB
};


/* Convert QImage to/from cv::Mat
 *
 * - Channels of cv::Mat should be 1, 3, 4
 * - Depth of cv::Mat should be 8U, 16U or 32F
 */
cv::Mat image2Mat(const QImage &img, int matType = CV_8UC(0), MatColorOrder order=MCO_BGR);
QImage mat2Image(const cv::Mat &mat, MatColorOrder order=MCO_BGR, QImage::Format formatHint = QImage::Format_Invalid);


/* Convert QImage to/from cv::Mat
 *
 * - Channels of cv::Mat should be 1, 3, 4
 *
 * - All other image formats should be converted to one of following formats.
 *   - QImage::Format_Indexed8  <==> 8UC1 16UC1 32FC1
 *   - QImage::Format_RGB888    <==> 8UC3 16UC3 32FC3 (R G B)
 *   - QImage::Format_ARGB32    <==> 8UC4 16UC4 32FC4 (B G R A) or (A R G B)
 *   - QImage::Format_RGB32     <==> 8UC4 16UC4 32FC4 (B G R A) or (A R G B)
 *   - QImage::Format_RGBA8888  <==> 8UC4 16UC4 32FC4 (R G B A)
 *   - QImage::Format_RGBX8888  <==> 8UC4 16UC4 32FC4 (R G B A)
 *   - QImage::Format_Invalid(means auto selection)
 *
 * - For QImage::Format_RGB32 and QImage::Format_ARGB32,
 *   the color channel order of cv::Mat will be (B G R A) in
 *   little endian system or (A R G B) in big endian system.
 *
 * - User must make sure that the color channels order is the same as
 *   the color channels order requried by QImage.
 */
cv::Mat image2Mat(const QImage &img, MatColorOrder *order, int matDepth = CV_8U);
QImage mat2Image(const cv::Mat &mat, QImage::Format formatHint);

/* Convert QImage to/from cv::Mat without data copy
 *
 * - Supported QImage formats and cv::Mat types are:
 *   - QImage::Format_Indexed8 <==> CV_8UC1
 *   - QImage::Format_RGB888   <==> CV_8UC3 (R G B)
 *   - QImage::Format_RGB32    <==> CV_8UC4 (B G R A) or (A R G B)
 *   - QImage::Format_ARGB32   <==> CV_8UC4 (B G R A) or (A R G B)
 *   - QImage::Format_RGBX8888 <==> CV_8UC4 (R G B A)
 *   - QImage::Format_RGBA8888 <==> CV_8UC4 (R G B A)
 *
 * - For QImage::Format_RGB32 and QImage::Format_ARGB32, the
 *   color channel order of cv::Mat will be (B G R A) in little
 *   endian system or (A R G B) in big endian system.
 *
 * - User must make sure that the color channels order is the same as
 *   the color channels order requried by QImage.
 */
cv::Mat image2Mat_shared(const QImage &img, MatColorOrder *order=0);
QImage mat2Image_shared(const cv::Mat &mat, QImage::Format formatHint = QImage::Format_Invalid);

} //namespace QtOcv

#endif // CVMATANDQIMAGE_H
