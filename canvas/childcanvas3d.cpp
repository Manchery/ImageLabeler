#include "childcanvas3d.h"
#include "canvas3d.h"
#include "annotationcontainer.h"
#include "cubeannotationitem.h"
#include "labelmanager.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QtDebug>

ChildCanvas3D::ChildCanvas3D(Canvas3D *parentCanvas, Axis axis, QWidget *parent) :
    QWidget(parent),
    parentCanvas(parentCanvas),
    axis(axis)
{
    mousePos = QPoint(0,0);
    scale=1.0;
    setMouseTracking(true);
    mousePressing=false;
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

    int focusU, focusV;
    switch(axis){
    case X: focusU = parentCanvas->focusPos.z; focusV = parentCanvas->focusPos.y; break;
    case Y: focusU = parentCanvas->focusPos.x; focusV = parentCanvas->focusPos.z; break;
    case Z: focusU = parentCanvas->focusPos.x; focusV = parentCanvas->focusPos.y; break;
    }

    p.drawLine(focusU,0,focusU,getImageHeight()-1);
    p.drawLine(0,focusV,getImageWidth()-1,focusV);

    const AnnotationContainer *pAnnoContainer = parentCanvas->pAnnoContainer;
    const LabelManager *pLabelManager = parentCanvas->pLabelManager;

    if (parentCanvas->task==DETECTION3D){
        if (parentCanvas->mode == DRAW || (parentCanvas->mode == MOVE && parentCanvas->lastMode==DRAW)){
            for (int i=0;i<pAnnoContainer->length();i++){
                auto item = CubeAnnotationItem::castPointer((*pAnnoContainer)[i]);
                Cuboid cube = item->cube;

                if (!_showableForCube(cube)) continue;

                QRect rect = _rectForCube(cube);
                QString label = item->label;
                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].visible==false)
                    continue;

                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].color.isValid()){
                    p.save();
                    QColor color = (*pLabelManager)[label].color;
                    color.setAlphaF(0.2); QBrush brush(color); p.setBrush(brush);
                    color.setAlphaF(0.5); QPen pen(color); p.setPen(pen);
                    p.drawRect(rect);
                    p.restore();
                }else{
                    p.drawRect(rect);
                }
            }

            if (curPoints.length()>0){ // drawing
                p.drawRect(QRect(curPoints[0], curPoints[1]).normalized());
            }

        }else if (parentCanvas->mode == SELECT || (parentCanvas->mode == MOVE && parentCanvas->lastMode==SELECT)){
            //! TODO
            for (int i=0;i<pAnnoContainer->length();i++){
                auto item = CubeAnnotationItem::castPointer((*pAnnoContainer)[i]);
                Cuboid cube = item->cube;

                if (!_showableForCube(cube)) continue;

                QRect rect = _rectForCube(cube);
                QString label = item->label;
                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].visible==false)
                    continue;

                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].color.isValid()){
                    p.save();
                    QColor color = (*pLabelManager)[label].color;
                    color.setAlphaF(0.2); QBrush brush(color); p.setBrush(brush);
                    color.setAlphaF(0.5); QPen pen(color); p.setPen(pen);
                    p.drawRect(rect);
                    p.restore();
                }else{
                    p.drawRect(rect);
                }
            }
            //!TODO
        }
    }

    p.end();
}

void ChildCanvas3D::mousePressEvent(QMouseEvent *event)
{
    if (image.isNull()){
        QWidget::mousePressEvent(event);
        return;
    }
    if (parentCanvas->mode == MOVE){
        mousePressing = true;
    }
    QPoint pixPos = pixelPos(event->pos());
    if (parentCanvas->task == DETECTION3D){
        if (parentCanvas->mode == DRAW){
            if (event->button()==Qt::LeftButton){
                if (curPoints.length()==0){
                    if (parentCanvas->childDrawingRect()==nullptr && !outOfPixmap(pixPos)){
                        curPoints.push_back(pixPos);
                        curPoints.push_back(pixPos);
                        update();
                    }
                } else if (curPoints.length()==2){
                    curPoints[1] = boundedPixelPos(event->pos());
                    emit newRectAnnotated(QRect(curPoints[0], curPoints[1]).normalized());
                    curPoints.clear();
                } else {
                    throw "Anomaly length of curPoints of new rectangle";
                }
            }
        }
    }
}

void ChildCanvas3D::mouseMoveEvent(QMouseEvent *event)
{
    if (image.isNull()){
        QWidget::mouseMoveEvent(event);
        return;
    }
    QPoint pixPos = pixelPos(event->pos());
    if (parentCanvas->mode==MOVE){
        if (mousePressing && !outOfPixmap(pixPos)){
            emit focusMoved(pixPos);
        }
        return;
    }
    if (parentCanvas->task == TaskMode::DETECTION3D){
        if (parentCanvas->mode == CanvasMode::DRAW){
            if (curPoints.length()==2){
                curPoints[1] = pixPos;
                update();
            } else if (curPoints.length()!=0){
                throw "Anomaly length of curPoints of new rectangle";
            }
        }
    }
}

void ChildCanvas3D::mouseReleaseEvent(QMouseEvent *event)
{
    mousePressing=false;
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

bool ChildCanvas3D::_showableForCube(Cuboid cube) const
{
    bool showable;
    Point3D focusPos = parentCanvas->focusPos;
    switch (axis) {
    case X: showable = cube.minX() <= focusPos.x &&  focusPos.x <= cube.maxX(); break;
    case Y: showable = cube.minY() <= focusPos.y &&  focusPos.y <= cube.maxY(); break;
    case Z: showable = cube.minZ() <= focusPos.z &&  focusPos.z <= cube.maxZ(); break;
    }
    return showable;
}

QRect ChildCanvas3D::_rectForCube(Cuboid cube) const
{
    QRect rect;
    switch (axis) {
    case X: rect = cube.rectX(); break;
    case Y: rect = cube.rectY(); break;
    case Z: rect = cube.rectZ(); break;
    }
    return rect;
}
