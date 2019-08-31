#include "canvas2d.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPen>

Canvas2D::Canvas2D(QWidget *parent) : QWidget(parent)
{
    mousePos = QPoint(0,0);
//    setMouseTracking(true);
}

QSize Canvas2D::sizeHint() const
{
    return minimumSizeHint();
}

QSize Canvas2D::minimumSizeHint() const
{
    if (!image.isNull())
        return image.size();
    return QWidget::minimumSizeHint();
}

void Canvas2D::loadImage(const QImage &newImage)
{
    image = newImage;
    adjustSize();
    update();
}

//void Canvas2D::loadPixmap(QPixmap new_pixmap)
//{
//    pixmap = new_pixmap;
//    adjustSize();
//    update();
//}

void Canvas2D::paintEvent(QPaintEvent *event)
{
    if (image.isNull()){
        QWidget::paintEvent(event);
        return;
    }
    QPainter p(this);
//    p.setRenderHint(QPainter::Antialiasing);
//    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

//    p.scale(scale, scale);
//    p.translate(offsetToCenter());
//    p.drawPixmap(0, 0, pixmap);
    p.drawImage(0,0,image);
    p.setPen(QPen(Qt::white));
    p.drawLine(0,mousePos.y(),width()-1,mousePos.y());
    p.drawLine(mousePos.x(),0,mousePos.x(),height()-1);
    p.end();
}

void Canvas2D::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pixPos = event->pos();
    if (!outOfPixmap(pixPos)){
        mousePos = event->pos();
        update();
        emit mouseMoved(mousePos);
    }
}

void Canvas2D::mouseSetRequest(QPoint pos){
    if (pos!=mousePos){
        mousePos = pos;
        update();
    }
}

QPoint Canvas2D::boundedPixelPos(QPoint pos)
{
    pos.setX(std::min(std::max(pos.x(), 0), width()-1));
    pos.setY(std::min(std::max(pos.y(), 0), height()-1));
    return pos;
}


bool Canvas2D::outOfPixmap(QPoint pos)
{
    int w = width(), h= height();
    return !(0<=pos.x() && pos.x()<w && 0<=pos.y() && pos.y()<h);
}



