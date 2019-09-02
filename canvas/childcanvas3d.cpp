#include "childcanvas3d.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPen>

ChildCanvas3D::ChildCanvas3D(QWidget *parent) : QWidget(parent)
{
    mousePos = QPoint(0,0);
    scale=1.0;
    setMouseTracking(true);
}

void ChildCanvas3D::loadImage(const QImage &newImage)
{
    image = newImage;
    setFixedSize(image.size()*scale);
    update();
}

void ChildCanvas3D::setScale(qreal new_scale)
{
    scale = new_scale;
    setFixedSize(image.size()*scale);
    update();
}

void ChildCanvas3D::paintEvent(QPaintEvent *event)
{
    if (image.isNull()){
        QWidget::paintEvent(event);
        return;
    }
    QPainter p(this);
//    p.setRenderHint(QPainter::Antialiasing);
//    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    p.scale(scale, scale);
//    p.translate(offsetToCenter());
    p.drawImage(0,0,image);
    p.setPen(QPen(Qt::white));
    p.drawLine(0,mousePos.y(),getImageWidth()-1,mousePos.y());
    p.drawLine(mousePos.x(),0,mousePos.x(),getImageHeight()-1);
    p.end();
}

void ChildCanvas3D::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pixPos = pixelPos(event->pos());
    if (!outOfPixmap(pixPos)){
        mousePos = pixPos;
        update();
        emit mouseMoved(mousePos);
    }
}

void ChildCanvas3D::mouseSetRequest(QPoint pos){
    if (pos!=mousePos){
        mousePos = pos;
        update();
    }
}

QPoint ChildCanvas3D::pixelPos(QPoint pos)
{
    return pos / scale;
}

QPoint ChildCanvas3D::boundedPixelPos(QPoint pos)
{
    pos = pos / scale;
    pos.setX(std::min(std::max(pos.x(), 0), getImageWidth()-1));
    pos.setY(std::min(std::max(pos.y(), 0), getImageHeight()-1));
    return pos;
}


bool ChildCanvas3D::outOfPixmap(QPoint pos)
{
    int w = getImageWidth(), h= getImageHeight();
    return !(0<=pos.x() && pos.x()<w && 0<=pos.y() && pos.y()<h);
}

