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
#include "cvimagewidget.h"

#include <QDir>
#include <QWheelEvent>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPointF>
#include <QDebug>

#include <math.h>

namespace QtOcv {

class ImageWidgetPrivate
{
public:
    ImageWidgetPrivate(ImageWidget *q);
    ~ImageWidgetPrivate(){}

    void dealWithScaleChanged(double rSacle, bool causedByWheel=true);
    void doAutoFit();
    QColor getColorUnderMouse();

    double m_scale;  //on work when view rotate 0, 90, 180, 270
    double m_scaleMax;
    double m_scaleMin;

    QColor m_lastColor;

    bool m_wheelScaleEnabled;
    bool m_autoAdjustEnabled;
    QGraphicsPixmapItem *m_pixmapItem;

    ImageWidget *q;
};

ImageWidgetPrivate::ImageWidgetPrivate(ImageWidget *q) :
    q(q)
{
    m_scale = 1;
    m_scaleMin = 0.01;
    m_scaleMax = 64;
    m_wheelScaleEnabled = true;
    m_autoAdjustEnabled = false;
}

/*!
   This is the only place where the scale changed!
   When scale changed, a signal will be emitted with the new scale.
   Note: the function only works when the view does not rotate, or rotate n*90 degree.
*/
void ImageWidgetPrivate::dealWithScaleChanged(double rScale, bool causedByWheel)
{
    if (fabs(rScale - 1)<10e-4)
        return;

    if (causedByWheel)
        q->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    else
        q->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    q->scale(rScale, rScale);
    m_scale = qMax(fabs(q->transform().m11()),fabs(q->transform().m12()));
    emit q->scaleChanged(m_scale);
    emit q->realScaleChanged(m_scale);
}

void ImageWidgetPrivate::doAutoFit()
{
    if (!m_autoAdjustEnabled || !q->scene())
        return;

    q->fitInView(q->scene()->sceneRect(), Qt::KeepAspectRatio);
    m_scale = qMax(fabs(q->transform().m11()),fabs(q->transform().m12()));
    emit q->realScaleChanged(m_scale);
}

QColor ImageWidgetPrivate::getColorUnderMouse()
{
    QColor c;
    if (!m_pixmapItem->pixmap().isNull()) {
        QPoint pos = q->mapToScene(q->mapFromGlobal(QCursor::pos())).toPoint();
        if (q->scene()->sceneRect().contains(pos))
            c = QColor(q->pixmap().copy(pos.x(), pos.y(), 1, 1).toImage().pixel(0, 0));
    }
    return c;
}

/*!
  \class QtOcv::ImageWidget
*/

/*!
  Default constructor.
*/
ImageWidget::ImageWidget(QWidget *parent)
        :QGraphicsView(parent), d(new ImageWidgetPrivate(this))
{
    QGraphicsScene *sc = new QGraphicsScene(this);
    d->m_pixmapItem = new QGraphicsPixmapItem;
    sc->addItem(d->m_pixmapItem);
    setScene(sc);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setDragMode(ScrollHandDrag);
    setMouseTracking(true);

//    QPixmap pix(512, 512);
//    pix.fill(Qt::gray);
//    setPixmap(pix);
}

ImageWidget::~ImageWidget()
{
    delete d;
}

QPixmap ImageWidget::pixmap() const
{
    return d->m_pixmapItem->pixmap();
}

QColor ImageWidget::colorUnderMouse() const
{
    return d->m_lastColor;
}

void ImageWidget::setPixmap(const QPixmap &pixmap)
{
    d->m_pixmapItem->setPixmap(pixmap);

    if (scene()->sceneRect() != d->m_pixmapItem->boundingRect()) {
        //Be careful.
        //Note that, when pixmap isNull, the sceneRect() is not null,
        //though setSceneRect(QRectF()) is called.
        scene()->setSceneRect(d->m_pixmapItem->boundingRect());

        if (d->m_autoAdjustEnabled)
            d->doAutoFit();
    }

    QColor c = d->getColorUnderMouse();
    if (c != d->m_lastColor) {
        d->m_lastColor = c;
        emit colorUnderMouseChanged(c);
    }
}

void ImageWidget::setImage(const QImage &image)
{
    setPixmap(QPixmap::fromImage(image));
}

double ImageWidget::currentScale() const
{
    if (d->m_autoAdjustEnabled)
        return 0;

    return d->m_scale;
}

double ImageWidget::currentRealScale() const
{
    return d->m_scale;
}

bool ImageWidget::isMouseWheelEnabled() const
{
    return d->m_wheelScaleEnabled;
}

/*!
  Set a scale value to the View.
  When the value is out of the scale range, the value will be adjusted.

  Note that, scale 0 mean auto fit.

  \sa setScaleRange()
*/
void ImageWidget::setCurrentScale(double factor)
{
    if (factor == currentScale())
        return;

    if (factor == 0) {
        if (!d->m_autoAdjustEnabled) {
            d->m_autoAdjustEnabled = true;
            d->doAutoFit();
            emit scaleChanged(0);
        }
        return;
    }

    if (d->m_autoAdjustEnabled)
        d->m_autoAdjustEnabled = false;

    if (factor > d->m_scaleMax)
        factor = d->m_scaleMax;
    else if (factor < d->m_scaleMin)
        factor = d->m_scaleMin;

    d->dealWithScaleChanged(factor/d->m_scale);
}

void ImageWidget::setMouseWheelEnabled(bool enable)
{
    d->m_wheelScaleEnabled = enable;
}

/*!
  Set the range of scale.
  When current scale value not in the range, the value will be adjusted.
  \sa setCurrentScale()
*/
void ImageWidget::setScaleRange(double min, double max)
{
    d->m_scaleMin = min;
    d->m_scaleMax = max;
    if (!d->m_autoAdjustEnabled) {
        if (d->m_scale < d->m_scaleMin)
            setCurrentScale(d->m_scaleMin);
        else if (d->m_scale > d->m_scaleMax)
            setCurrentScale(d->m_scaleMax);
    }
}

/*!
  deal with wheel event.
*/
void ImageWidget::wheelEvent(QWheelEvent *event)
{
    if (d->m_wheelScaleEnabled) {
        //Disable auto fit!!
        d->m_autoAdjustEnabled = false;

        double numDegrees = -event->delta() / 8.0;
        double numSteps = numDegrees / 15.0;
        double factor = pow(1.125, numSteps);

        if (numSteps > 0)
            factor = qMin(factor, d->m_scaleMax/d->m_scale);
        else
            factor = qMax(factor, d->m_scaleMin/d->m_scale);

        d->dealWithScaleChanged(factor);
    }

    //QGraphicsView::wheelEvent(event);
}

void ImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    QColor c = d->getColorUnderMouse();
    if (c != d->m_lastColor) {
        d->m_lastColor = c;
        emit colorUnderMouseChanged(c);
    }

    QGraphicsView::mouseMoveEvent(event);
}

void ImageWidget::leaveEvent(QEvent *event)
{
    if (d->m_lastColor.isValid()) {
        d->m_lastColor = QColor();
        emit colorUnderMouseChanged(QColor());
    }
    QGraphicsView::leaveEvent(event);
}

void ImageWidget::resizeEvent(QResizeEvent *event)
{
    if (d->m_autoAdjustEnabled) {
        //Auto fit enabled.
        d->doAutoFit();
    }
    QGraphicsView::resizeEvent(event);
}

} //namespace QtOcv
