#include "canvas.h"
#include "segannotationitem.h"
#include "rectannotationitem.h"
#include "utils.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtDebug>
#include <algorithm>

Canvas::Canvas(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent) :
    QWidget(parent),
    pixmap(),
    pAnnoContainer(pAnnoContainer),
    pLabelManager(pLabelManager),
    scale(1.0)
{
    lastPenWidth=DEFAULT_PEN_WIDTH;
    mousePos=QPoint(0,0);
    setMouseTracking(true);
}

void Canvas::paintEvent(QPaintEvent *event)
{
    if (pixmap.isNull()){
        QWidget::paintEvent(event);
        return;
    }
    QPainter p(this);
//    p.setRenderHint(QPainter::Antialiasing);
//    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    p.scale(scale, scale);
    p.translate(offsetToCenter());
    p.drawPixmap(0, 0, pixmap);

    if (task == DETECTION){
        if (mode == DRAW){
            for (int i=0;i<pAnnoContainer->length();i++){
                auto item = RectAnnotationItem::castPointer((*pAnnoContainer)[i]);
                QRect rect=item->rect;
                QString label=item->label;

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

                QFont font("Helvetica");
                font.setFamily("Times");
                font.setPixelSize(10);
                p.drawText(rect.topLeft()-QPoint(0,10), label);
            }
            if (curPoints.length()>0){ // drawing
                p.drawRect(QRect(curPoints[0], curPoints[1]).normalized());
            }
            p.end();
        }else if (mode==SELECT){
            for (int i=0;i<pAnnoContainer->length();i++){
                if (i==pAnnoContainer->getSelectedIdx()) continue;

                auto item = RectAnnotationItem::castPointer((*pAnnoContainer)[i]);
                QRect rect=item->rect;
                QString label=item->label;

                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].visible==false)
                    continue;

                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].color.isValid()){
                    p.save();
                    QColor color = (*pLabelManager)[label].color;
                    color.setAlphaF(0.1); QBrush brush(color); p.setBrush(brush);
                    color.setAlphaF(0.2); QPen pen(color); p.setPen(pen);
                    p.drawRect(rect);
                    p.restore();
                }else{
                    p.drawRect(rect);
                }
            }

            QString selectedLabel = pAnnoContainer->getSelectedItem()->label;
            QRect drawedRect = editing?editingRect:RectAnnotationItem::castPointer(pAnnoContainer->getSelectedItem())->rect;
            p.save();
            QColor color = (*pLabelManager)[selectedLabel].color;
            color.setAlphaF(0.2); QBrush brush(color); p.setBrush(brush);
            color.setAlphaF(0.5); QPen pen(Qt::white); pen.setStyle(Qt::DashLine); p.setPen(pen);
            p.drawRect(drawedRect);
            p.restore();

            QFont font("Helvetica");
            font.setFamily("Times");
            font.setPixelSize(10);
            p.drawText(drawedRect.topLeft()-QPoint(0,10), selectedLabel);
        }
    }else if (task == SEGMENTATION){
        if (mode == DRAW){
            QPixmap colorMap(pixmap.size());
            colorMap.fill(QColor(0,0,0,0));
            QPainter p0(&colorMap);

            for (int i=0;i<pAnnoContainer->length();i++){
                auto item = SegAnnotationItem::castPointer((*pAnnoContainer)[i]);
                QString label = item->label;
                QColor color = (*pLabelManager)[label].color;
                for (auto stroke: item->strokes)
                    stroke.drawSelf(p0,color);
            }
            if (curStrokes.length()>0){
                QColor color = Qt::white;
                for (int i=0;i<curStrokes.length()-1;i++)
                    curStrokes[i].drawSelf(p0,color);
                if (strokeDrawing)
                    curStrokes.back().drawSelf(p0, color, false);
                else
                    curStrokes.back().drawSelf(p0, color, true);
            }
            p0.end();

            p.setOpacity(0.5);
            p.drawPixmap(0,0,colorMap);

            if (!strokeDrawing && (drawMode==SQUAREPEN || drawMode==CIRCLEPEN) && !outOfPixmap(mousePos)){
                p.save();
                if (drawMode==SQUAREPEN){
                    p.setPen(QPen(Qt::white, curPenWidth, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
                }else if (drawMode==CIRCLEPEN){
                    p.setPen(QPen(Qt::white, curPenWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                }
                p.drawPoint(mousePos);
                p.restore();
            }

            p.end();
        }else if (mode == SELECT){
            QPixmap colorMap(pixmap.size());
            colorMap.fill(QColor(0,0,0,0));
            QPainter p0(&colorMap);

            for (int i=0;i<pAnnoContainer->length();i++){
                auto item = SegAnnotationItem::castPointer((*pAnnoContainer)[i]);
                QString label = item->label;
                QColor color = (i==pAnnoContainer->getSelectedIdx())?(*pLabelManager)[label].color:QColor(0,0,0,0);
                for (auto stroke: item->strokes)
                    stroke.drawSelf(p0,color);
            }
            p0.end();

            p.setOpacity(0.5);
            p.drawPixmap(0,0,colorMap);
            p.end();
        }
    }
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    if (pixmap.isNull()){
        QWidget::mousePressEvent(event);
        return;
    }
    QPoint pixPos = pixelPos(event->pos());
//    qDebug()<<"mouse press"<<pixPos.x()<<" "<<pixPos.y();
    emit mouseMoved(pixPos);
    if (task == TaskMode::DETECTION){
        if (mode == CanvasMode::DRAW){
            if (drawMode == DrawMode::RECTANGLE){
                if (event->button() == Qt::LeftButton){
                    if (curPoints.length()==0){
                        if (!outOfPixmap(pixPos)){
                            curPoints.push_back(pixPos);
                            curPoints.push_back(pixPos);
                            update();
                        }
                    } else if (curPoints.length()==2){
                        curPoints[1] = boundedPixelPos(event->pos());
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
        } else if (mode == CanvasMode::SELECT){
            if (event->button() == Qt::LeftButton){
                auto item = RectAnnotationItem::castPointer((*pAnnoContainer)[pAnnoContainer->getSelectedIdx()]);
                QRect selectedRect = item->rect;
                if (onRectTop(pixPos, selectedRect)){
                    editing=true; editingRect = selectedRect;
                    editingRectEdge = TOP;
                }else if (onRectBottom(pixPos, selectedRect)){
                    editing=true; editingRect = selectedRect;
                    editingRectEdge = BOTTOM;
                }else if (onRectLeft(pixPos, selectedRect)){
                    editing=true; editingRect = selectedRect;
                    editingRectEdge = LEFT;
                }else if (onRectRight(pixPos, selectedRect)){
                    editing=true; editingRect = selectedRect;
                    editingRectEdge = RIGHT;
                }
            }
        }
    }else if (task == TaskMode::SEGMENTATION){
        if (mode == DRAW){
            if (event->button()==Qt::LeftButton){
                if (drawMode!=POLYGEN){
                    SegStroke stroke;
                    stroke.penWidth = curPenWidth;
                    switch(drawMode){
                    case CONTOUR: stroke.type="contour"; break;
                    case SQUAREPEN: stroke.type="square_pen"; break;
                    case CIRCLEPEN: stroke.type="circle_pen"; break;
                    default: throw "abnormal draw mode when segmentation";
                    }
                    stroke.points.push_back(pixPos);
                    curStrokes.push_back(stroke);
                    strokeDrawing=true;
                    update();
                }else{ // drawMode == POLYGEN
                    if (event->button()==Qt::LeftButton){
                        if (!strokeDrawing){
                            SegStroke stroke;
                            stroke.penWidth = curPenWidth;
                            stroke.type = "contour";
                            stroke.points.push_back(pixPos);
                            stroke.points.push_back(pixPos);
                            curStrokes.push_back(stroke);
                            strokeDrawing=true;
                            update();
                        }else{
                            curStrokes.back().points.push_back(pixPos);
                            update();
                        }
                    }
                }
            }else if (event->button()==Qt::RightButton){
                if (drawMode!=POLYGEN || strokeDrawing==false){
                    if (curStrokes.length()>0){
                        curStrokes.pop_back();
                        update();
                    }
                }else{ //drawMode == POLYGEN && strokeDrawing
                    strokeDrawing=false;
                    curStrokes.pop_back();
                    update();
                }
            }
        }
    }
    setFocus();
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if (pixmap.isNull()){
        QWidget::mouseMoveEvent(event);
        return;
    }
    mousePos = pixelPos(event->pos());
    QPoint pixPos = boundedPixelPos(event->pos());
    emit mouseMoved(pixPos);
    if (task == TaskMode::DETECTION){
        if (mode == CanvasMode::DRAW){
            if (drawMode == DrawMode::RECTANGLE){
                if (curPoints.length()==2){
                    curPoints[1] = pixPos;
                    update();
                } else if (curPoints.length()!=0){
                    throw "Anomaly length of curPoints of new rectangle";
                }
            }
        }else if (mode == CanvasMode::SELECT){
            if (editing){
                switch (editingRectEdge) {
                case TOP:
                    editingRect.setTop(pixPos.y());
                    break;
                case BOTTOM:
                    editingRect.setBottom(pixPos.y());
                    break;
                case LEFT:
                    editingRect.setLeft(pixPos.x());
                    break;
                case RIGHT:
                    editingRect.setRight(pixPos.x());
                    break;
                }
                update();
            }
        }
    }else if (task == SEGMENTATION){
        if (mode == DRAW){
            if (drawMode!=POLYGEN){
                if (strokeDrawing){
                    curStrokes.back().points.push_back(pixPos);
                    update();
                }
                if (!strokeDrawing && (drawMode==SQUAREPEN || drawMode==CIRCLEPEN)){
                    update(); // track mouse pos;
                }
            }else{ // drawMode == POLYGEN
                if (strokeDrawing){
                    curStrokes.back().points.back()=pixPos;
                    update();
                }
            }
        }
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event){
    if (pixmap.isNull()){
        QWidget::mouseReleaseEvent(event);
        return;
    }
//    QPoint pixPos = boundedPixelPos(event->pos());
    if (task == TaskMode::DETECTION){
        if (mode == CanvasMode::SELECT){
            if (editing){
                emit modifySelectedRectRequest(pAnnoContainer->getSelectedIdx(), editingRect.normalized());
                editing = false;
            }
        }
    }else if (task == TaskMode::SEGMENTATION){
        if (mode == DRAW){
            if (drawMode!=POLYGEN){
                strokeDrawing=false;
                update();
            }
        }
    }
}

void Canvas::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (pixmap.isNull()){
        QWidget::mouseDoubleClickEvent(event);
        return;
    }
    //    QPoint pixPos = boundedPixelPos(event->pos());
    if (task==SEGMENTATION)
        if (mode == DRAW){
            if (drawMode==POLYGEN){
                strokeDrawing=false;
                update();
            }
        }
}

void Canvas::keyPressEvent(QKeyEvent *event)
{
    if (task == SEGMENTATION){
        if (mode == DRAW){
            if (event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter){
                if (drawMode==POLYGEN){
                    strokeDrawing=false;
                }
                if (curStrokes.length()>0){
                    emit newStrokesAnnotated(curStrokes);
                    curStrokes.clear();
                    update();
                }
                return;
            }
        }
    }
    QWidget::keyPressEvent(event);
}

QString Canvas::modeString() const {
    QString modeStr("");

    switch(task){
    case DETECTION:  modeStr+="Detection"; break;
    case SEGMENTATION: modeStr+="Segmentation"; break;
    default: modeStr+="Unknown task"; break;
    }

    switch (mode) {
    case DRAW: modeStr+=", Draw"; break;
    case SELECT: modeStr+=", Select"; break;
//    default: modeStr+="Unknown mode"; break;
    }

    if (mode==DRAW){
        switch (drawMode) {
        case RECTANGLE: modeStr+=", Rectangle"; break;
        case CONTOUR: modeStr+=", Contour"; break;
        case SQUAREPEN: modeStr+=", Square Pen"; break;
        case CIRCLEPEN: modeStr+=", Circle Pen"; break;
        case POLYGEN: modeStr+=", Polygen Contour"; break;
//        default: modeStr+="Unknown mode"; break;
        }
    }
    return modeStr;
}

void Canvas::changeTask(TaskMode _task) {
    switch(_task){
    case DETECTION:
        task = TaskMode::DETECTION;
        mode = CanvasMode::DRAW;
        drawMode = DrawMode::RECTANGLE;
        editing=false;
        strokeDrawing=false;
        break;
    case SEGMENTATION:
        task = TaskMode::SEGMENTATION;
        mode = CanvasMode::DRAW;
        drawMode = DrawMode::POLYGEN;
        editing = false;
        curPenWidth=1;
        strokeDrawing=false;
        break;
    default:
        break;
    }
    emit modeChanged(modeString());
}

void Canvas::changeDrawMode(DrawMode _draw)
{
    drawMode=_draw;
    switch (drawMode) {
    case RECTANGLE:
        curPoints.clear();
        editing=false;
        break;
    case CIRCLEPEN:
    case SQUAREPEN:
        if (strokeDrawing==true){
            curStrokes.pop_back();
            strokeDrawing=false;
        }
        curPenWidth = lastPenWidth;
        break;
    case CONTOUR:
    case POLYGEN:
        if (strokeDrawing==true){
            curStrokes.pop_back();
            strokeDrawing=false;
        }
        curPenWidth=1;
        break;
    }
    emit modeChanged(modeString());
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

void Canvas::changeCanvasModeRequest(){
    if (pAnnoContainer->getSelectedIdx()==-1)
        mode = DRAW;
    else {
        mode = SELECT;
    }
    emit modeChanged(modeString());
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
    if (task==DETECTION){
        for (int i=pAnnoContainer->length()-1;i>=0;i--){
            auto item = RectAnnotationItem::castPointer((*pAnnoContainer)[i]);
            QRect rect=item->rect;
            // consider for really small bounding box
            rect.setTopLeft(rect.topLeft()-QPoint(2,2));
            rect.setBottomRight(rect.bottomRight()+QPoint(2,2));
            if (rect.contains(pos))
                return i;
        }
        return -1;
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
