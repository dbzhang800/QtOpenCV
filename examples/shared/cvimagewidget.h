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
#ifndef QTOCVIMAGEWIDGET_H
#define QTOCVIMAGEWIDGET_H

#include <qgraphicsview.h>

namespace QtOcv {
class ImageWidgetPrivate;
class ImageWidget : public QGraphicsView
{
    Q_OBJECT
public:
    ImageWidget(QWidget *parent=0);
    ~ImageWidget();

    double currentScale() const;
    double currentRealScale() const;
    bool isMouseWheelEnabled() const;

    QPixmap pixmap() const;
    QColor colorUnderMouse() const;

public slots:
    void setCurrentScale(double currentScale);
    void setScaleRange(double min, double max);
    void setMouseWheelEnabled(bool enable);

    void setImage(const QImage &image);
    void setPixmap(const QPixmap &pixmap);

signals:
    void scaleChanged(double scale);
    void realScaleChanged(double scale);
    void colorUnderMouseChanged(const QColor &color);

protected:
    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    friend class ImageWidgetPrivate;
    ImageWidgetPrivate *d;
};

} //namespace QtOcv
#endif // QTOCVIMAGEWIDGET_H
