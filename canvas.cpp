#include "canvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtDebug>
#include <algorithm>

Canvas::Canvas(const LabelManager *pLabelConfig, const RectAnnotations *pLabelData, QWidget *parent) :
    QWidget(parent),
    pixmap(), scale(1.0),
    pRectAnno(pLabelData),
    pLabelManager(pLabelConfig)

{
    task = TaskMode::DETECTION;
    mode = CanvasMode::DRAW;
    createMode = CreateMode::RECTANGLE;

    setMouseTracking(true);
}

QSize Canvas::sizeHint() const
{
    return minimumSizeHint();
}

QSize Canvas::minimumSizeHint() const
{
    if (!pixmap.isNull())
        return scale * pixmap.size();
    return QWidget::minimumSizeHint();
}

qreal Canvas::getScale() const { return scale; }

const QPixmap &Canvas::getPixmap() const { return pixmap; }

int Canvas::selectShape(QPoint pos)
{
    for (int i=pRectAnno->length()-1;i>=0;i--){
        QRect rect=(*pRectAnno)[i].rect;
        // consider for really small bounding box
        rect.setTopLeft(rect.topLeft()-QPoint(2,2));
        rect.setBottomRight(rect.bottomRight()+QPoint(2,2));
        if (rect.contains(pos))
            return i;
    }
    return -1;
}

QPoint Canvas::pixelPos(QPoint pos)
{
    return pos / scale - offsetToCenter();
}

QPoint Canvas::boundedPixelPos(QPoint pos)
{
    pos = pos / scale - offsetToCenter();
    pos.setX(std::min(std::max(pos.x(), 0), pixmap.width()-1));
    pos.setY(std::min(std::max(pos.y(), 0), pixmap.height()-1));
    return pos;
}

bool Canvas::outOfPixmap(QPoint pos)
{
    int w = pixmap.width(), h= pixmap.height();
    return !(0<=pos.x() && pos.x()<w && 0<=pos.y() && pos.y()<h);
}


void Canvas::mousePressEvent(QMouseEvent *event)
{
    if (pixmap.isNull()){
        QWidget::mousePressEvent(event);
        return;
    }
    QPoint pixPos = pixelPos(event->pos());
    emit mouseMoved(pixPos);
    if (!outOfPixmap(pixPos)){
        if (task == TaskMode::DETECTION){
            if (mode == CanvasMode::DRAW){
                if (createMode == CreateMode::RECTANGLE){
                    if (event->button() == Qt::LeftButton){
                        if (curPoints.length()==0){
                            curPoints.push_back(pixPos);
                            curPoints.push_back(pixPos);
                            update();
                        } else if (curPoints.length()==2){
                            curPoints[1] = pixPos;
                            emit newRectangleAnnotated(QRect(curPoints[0], curPoints[1]).normalized());
                            curPoints.clear();
                        } else {
                            throw "Anomaly length of curPoints of new rectangle";
                        }
                    }else if (event->button() == Qt::RightButton){
                        if (curPoints.length()==0){
                            int selectedIdx = selectShape(pixPos);
                            if (selectedIdx!=-1)
                                emit removeRectRequest(selectedIdx);
                        } else if (curPoints.length()==2){
                            curPoints.clear();
                            update();
                        } else {
                            throw "Anomaly length of curPoints of new rectangle";
                        }
                    }
                }
            }
        }
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if (pixmap.isNull()){
        QWidget::mouseMoveEvent(event);
        return;
    }
    QPoint pixPos = boundedPixelPos(event->pos());
    emit mouseMoved(pixPos);
    if (task == TaskMode::DETECTION){
        if (mode == CanvasMode::DRAW){
            if (createMode == CreateMode::RECTANGLE){
                if (curPoints.length()==2){
                    curPoints[1] = pixPos;
                    update();
                } else if (curPoints.length()!=0){
                    throw "Anomaly length of curPoints of new rectangle";
                }
            }
        }
    }
}

void Canvas::loadPixmap(QPixmap new_pixmap)
{
    pixmap = new_pixmap;
    adjustSize();
    update();
}

void Canvas::setScale(qreal new_scale)
{
    scale = new_scale;
    adjustSize();
    update();
}

void Canvas::paintEvent(QPaintEvent *event)
{
    if (pixmap.isNull()){
        QWidget::paintEvent(event);
        return;
    }
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    p.scale(scale, scale);
    p.translate(offsetToCenter());
    p.drawPixmap(0, 0, pixmap);

    for (int i=0;i<pRectAnno->length();i++){
        QRect rect=(*pRectAnno)[i].rect;
        QString label=(*pRectAnno)[i].label;
        if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].visible==false)
            continue;
        if (!(*pRectAnno)[i].visible)
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

        QFont font("Helvetica");
        font.setFamily("Times");
        font.setPixelSize(10);
        p.drawText(rect.topLeft()-QPoint(0,10), label);
    }
    if (curPoints.length()>0){
        p.drawRect(QRect(curPoints[0], curPoints[1]).normalized());
    }
    p.end();
}

QPoint Canvas::offsetToCenter()
{
    qreal s = scale;
    int w = int(pixmap.width() * s), h=int(pixmap.height() * s);
    int aw = this->size().width(), ah = this->size().height();
    int x = aw > w ? int((aw - w) / (2 * s)) : 0;
    int y = ah > h ? int((ah - h) / (2 * s)) : 0;
    return QPoint(x,y);
}
