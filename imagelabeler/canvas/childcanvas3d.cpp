#include "childcanvas3d.h"
#include "canvas3d.h"
#include "common.h"
#include "annotationcontainer.h"
#include "cubeannotationitem.h"
#include "rectannotationitem.h"
#include "labelmanager.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QtDebug>

#include <cmath>

using namespace CanvasUtils;

ChildCanvas3D::ChildCanvas3D(Canvas3D *parentCanvas, Axis axis, QWidget *parent) :
    QWidget(parent),
    parentCanvas(parentCanvas),
    axis(axis)
{
    scale=1.0;
    setMouseTracking(true);
    focusMoving=false;
    mousePressingWhenSelected=false;

    strokeDrawing = strokeDrawable = false;
    curStroke = SegStroke3D();
}

void ChildCanvas3D::paintEvent(QPaintEvent *event)
{
    if (image.isNull()){
        QWidget::paintEvent(event);
        return;
    }
    QPainter p(this);
    // 由于分割需要进行逐像素标注，故不开启防锯齿
    // p.setRenderHint(QPainter::Antialiasing);
    // p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    p.scale(scale, scale);
    // 之后的绘制在像素坐标系下进行

    p.drawImage(0,0,image);

    p.setPen(QPen(Qt::white));

    // 绘制focus的十字架
    QPoint focus2d = _getFocus2dFromParent();
    p.drawLine(focus2d.x(),0,focus2d.x(),getImageHeight()-1);
    p.drawLine(0,focus2d.y(),getImageWidth()-1,focus2d.y());

    const AnnotationContainer *pAnnoContainer = parentCanvas->pAnnoContainer;
    const LabelManager *pLabelManager = parentCanvas->pLabelManager;

    if (parentCanvas->task==DETECTION3D){
        if (parentCanvas->mode == DRAW || (parentCanvas->mode == MOVE && parentCanvas->lastMode==DRAW)){
            // 已有的矩形标注
            for (int i=0;i<pAnnoContainer->length();i++){
                auto item = CubeAnnotationItem::castPointer((*pAnnoContainer)[i]);
                Cuboid cube = item->getCube();
                if (!_showableForCube(cube)) continue;

                QRect rect = _rectForCube(cube);
                QString label = item->getLabel();
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

                QFont font("Helvetica"); font.setFamily("Times"); font.setPixelSize(LABEL_PIXEL_SIZE);
                p.setFont(font);
                p.drawText(rect.topLeft()-QPoint(0,LABEL_PIXEL_SIZE/2), label);
            }
            // 正在绘制的矩形标注
            if (curPoints.length()>0){
                p.drawRect(QRect(curPoints[0], curPoints[1]).normalized());
            }

        }else if (parentCanvas->mode == SELECT || (parentCanvas->mode == MOVE && parentCanvas->lastMode==SELECT)){
            // 已有的矩形标注，淡化显示，选中的除外
            for (int i=0;i<pAnnoContainer->length();i++){
                if (i==pAnnoContainer->getSelectedIdx()) continue;

                auto item = CubeAnnotationItem::castPointer((*pAnnoContainer)[i]);
                Cuboid cube = item->getCube();
                if (!_showableForCube(cube)) continue;

                QRect rect = _rectForCube(cube);
                QString label = item->getLabel();
                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].visible==false)
                    continue;

                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].color.isValid()){
                    QColor color = (*pLabelManager)[label].color;
                    drawRectAnnotation(p, rect, color, 0.1, color, 0.2);
                }else{
                    p.drawRect(rect);
                }
            }
            // 选中的矩形标注，突出显示
            auto selectedItem = CubeAnnotationItem::castPointer(pAnnoContainer->getSelectedItem());
            QString selectedLabel = selectedItem->getLabel();
            Cuboid drawedCube = parentCanvas->cubeEditing?parentCanvas->editedCube:selectedItem->getCube();
            if (_showableForCube(drawedCube)){
                QRect drawedRect = _rectForCube(drawedCube);
                p.save();
                QColor color = (*pLabelManager)[selectedLabel].color;
                color.setAlphaF(0.2); QBrush brush(color); p.setBrush(brush);
                QPen pen(Qt::white); pen.setStyle(Qt::DashLine); p.setPen(pen);
                p.drawRect(drawedRect);
                p.restore();

                QFont font("Helvetica"); font.setFamily("Times"); font.setPixelSize(LABEL_PIXEL_SIZE);
                p.setFont(font);
                p.drawText(drawedRect.topLeft()-QPoint(0,LABEL_PIXEL_SIZE/2), selectedLabel);
            }
        }
    } else if (parentCanvas->task == SEGMENTATION3D){
        if (parentCanvas->mode == DRAW){
            if (strokeDrawable){
                DrawMode drawMode = parentCanvas->drawMode;
                int curPenWidth = parentCanvas->curPenWidth;

                p.setOpacity(0.5);

                // 已有的分割标注已被烘焙进image中
                // 正在绘制的分割标注
                if (strokeDrawing){
                    curStroke.drawSelf(p, Qt::white, false); // 正在绘制的轮廓不填充
                }
                // 显示当前画笔
                if (!strokeDrawing && (drawMode==SQUAREPEN || drawMode==CIRCLEPEN) && !outOfPixmap(cursorPos)){
                    p.save();
                    if (parentCanvas->drawMode==SQUAREPEN){
                        p.setPen(QPen(Qt::white, curPenWidth, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
                    }else if (parentCanvas->drawMode==CIRCLEPEN){
                        p.setPen(QPen(Qt::white, curPenWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                    }
                    //! TODO: cursorPos maybe stick on the boundary of image
                    p.drawPoint(cursorPos);
                    p.restore();
                }
            }
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
    QPoint pixPos = pixelPos(event->pos());
    if (outOfPixmap(pixPos)) return;
    if (parentCanvas->mode == MOVE){                        // MOVE模式下按下鼠标，若离focus近则开始拖动focus
        QPoint focus = _getFocus2dFromParent();
        if (abs(pixPos.x()-focus.x())<pixEps && abs(pixPos.y()-focus.y())<pixEps){
            focusMoving = true;
            movingFocusAxis = Z;
            editedFocus = focus;
        }else if (abs(pixPos.x()-focus.x())<pixEps){
            focusMoving = true;
            movingFocusAxis = X;
            editedFocus = focus;
        }else if (abs(pixPos.y()-focus.y())<pixEps){
            focusMoving = true;
            movingFocusAxis = Y;
            editedFocus = focus;
        }
        return;
    }
    if (parentCanvas->task == DETECTION3D){
        if (parentCanvas->mode == DRAW){
            if (event->button()==Qt::LeftButton){           // 左键开始绘制矩形
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
            }else if (event->button()==Qt::RightButton){    // 右键删除cube或取消当前正在绘制的矩形
                if (curPoints.length()==0){
                    int selectedIdx = selectShape(cursorPos3d());
                    if (selectedIdx!=-1)
                        emit removeCubeRequest(selectedIdx);
                } else if (curPoints.length()==2){
                    curPoints.clear();
                    update();
                } else {
                    throw "Anomaly length of curPoints of new rectangle";
                }
            }
        } else if (parentCanvas->mode == SELECT){
            if (event->button() == Qt::LeftButton){         // 左键开始拖动cube的边，发送给Canvas3D处理
                mousePressingWhenSelected = true;
                emit mousePressWhenSelected(cursorPos3d());
            }
        }
    } else if (parentCanvas->task == SEGMENTATION3D){
        if (parentCanvas->mode == DRAW){
            DrawMode drawMode = parentCanvas->drawMode;
            int curPenWidth = parentCanvas->curPenWidth;
            if (event->button()==Qt::LeftButton){           // 左键开始绘制“笔画”
                if (drawMode!=POLYGEN){
                    curStroke.points.clear();

                    curStroke.penWidth = curPenWidth;
                    switch(drawMode){
                    case CONTOUR: curStroke.type="contour"; break;
                    case SQUAREPEN: curStroke.type="square_pen"; break;
                    case CIRCLEPEN: curStroke.type="circle_pen"; break;
                    default: throw "abnormal draw mode when segmentation";
                    }
                    curStroke.points.push_back(pixPos);
                    curStroke.z = parentCanvas->focusPos.z;
                    strokeDrawing=true;
                    update();
                }else{ // drawMode == POLYGEN
                    if (!strokeDrawing){
                        curStroke.points.clear();

                        curStroke.penWidth = curPenWidth;
                        curStroke.type = "contour";
                        curStroke.points.push_back(pixPos);
                        curStroke.points.push_back(pixPos);
                        curStroke.z = parentCanvas->focusPos.z;
                        strokeDrawing=true;
                        update();
                    }else{
                        curStroke.points.push_back(pixPos);
                        update();
                    }
                }
            }else if (event->button() == Qt::RightButton){
                if (drawMode!=POLYGEN || strokeDrawing==false){ // 右键删除最近绘制的一个笔画或者取消正在绘制的多边形
                    emit removeLatestStrokeRequest();
                }else{ //drawMode == POLYGEN && strokeDrawing
                    strokeDrawing=false;
                    curStroke = SegStroke3D();
                    update();
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

    if (!outOfPixmap(pixPos)){
        cursorPos = pixPos;
        emit cursorMoved(cursorPos3d());
    }

    if (parentCanvas->mode==MOVE){                      // 鼠标拖动focus
        if (focusMoving && !outOfPixmap(pixPos)){
            if (movingFocusAxis == X)
                editedFocus.setX(pixPos.x());
            else if (movingFocusAxis == Y)
                editedFocus.setY(pixPos.y());
            else if (movingFocusAxis == Z)
                editedFocus = pixPos;
            emit focusMoved(editedFocus);
        }
        return;
    }
    if (parentCanvas->task == TaskMode::DETECTION3D){
        if (parentCanvas->mode == CanvasMode::DRAW){    // 矩形绘制中，移动鼠标说明移动矩形的一个顶点
            if (curPoints.length()==2){
                curPoints[1] = pixPos;
                update();
            } else if (curPoints.length()!=0){
                throw "Anomaly length of curPoints of new rectangle";
            }
        }else if (parentCanvas->mode == CanvasMode::SELECT){    // 矩形编辑中，按下时移动鼠标是在拖动cube的一条边，交给Canvas3D处理
            if (mousePressingWhenSelected){
                emit mouseMoveWhenSelected(cursorPos3d());
            }
        }
    }else if (parentCanvas->task == SEGMENTATION3D){
        if (parentCanvas->mode == DRAW){
            DrawMode drawMode = parentCanvas->drawMode;
            if (drawMode!=POLYGEN){                     // 多边形轮廓模式除外，移动鼠标是在绘制一个笔画
                if (strokeDrawing){
                    if (curStroke.points.length()==0 || curStroke.points.back()!=pixPos){
                        curStroke.points.push_back(pixPos);
                        update();
                    }
                }
                if (!strokeDrawing && (drawMode==SQUAREPEN || drawMode==CIRCLEPEN)){
                    update();                           // 画笔的显示跟踪鼠标的移动
                }
            }else{                                      // 多边形模式，移动鼠标是在移动当前多边形的最后一个点
                if (strokeDrawing){
                    curStroke.points.back()=pixPos;
                    update();
                }
            }
        }
    }
}

void ChildCanvas3D::mouseReleaseEvent(QMouseEvent *event)
{
    if (image.isNull()){
        QWidget::mouseReleaseEvent(event);
        return;
    }
    focusMoving=false;
    if (parentCanvas->task == TaskMode::DETECTION3D){
        if (parentCanvas->mode == CanvasMode::SELECT){  // 选中编辑模式下，鼠标释放说明对矩形边的拖动完毕，交给Canvas3D处理
            mousePressingWhenSelected=false;
            emit mouseReleaseWhenSelected();
        }
    }else if (parentCanvas->task == TaskMode::SEGMENTATION3D){
        if (parentCanvas->mode == DRAW){
            if (parentCanvas->drawMode!=POLYGEN){       // 除了多边形轮廓模式，鼠标释放说明一个笔画绘制完毕
                strokeDrawing=false;
                emit newStrokeRequest(curStroke);
                curStroke = SegStroke3D();
                update();
            }
        }
    }
}

void ChildCanvas3D::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (image.isNull()){
        QWidget::mouseDoubleClickEvent(event);
        return;
    }
    if (parentCanvas->task==SEGMENTATION3D)
        if (parentCanvas->mode == DRAW){                // 鼠标双击后，“多边形轮廓” 类型的一个“笔画”绘制完毕
            if (parentCanvas->drawMode==POLYGEN){
                strokeDrawing=false;
                emit newStrokeRequest(curStroke);
                curStroke = SegStroke3D();
                update();
            }
        }
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

void ChildCanvas3D::close(){
    image = QImage();
    curPoints.clear();
    strokeDrawing=false;
}

/*-----------------------------------二维像素坐标相关------------------------------------*/

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


/*----------------------三维坐标相关，坐标的转换关系详见Canvas3D----------------------------*/


Point3D ChildCanvas3D::cursorPos3d() const
{
    Point3D focusPos = parentCanvas->focusPos;
    switch (axis){
    case X: return Point3D(focusPos.x, cursorPos.y(), cursorPos.x());
    case Y: return Point3D(cursorPos.x(), focusPos.y, cursorPos.y());
    case Z: return Point3D(cursorPos.x(), cursorPos.y(), focusPos.z);
    }
}

int ChildCanvas3D::selectShape(Point3D pos)
{
    if (parentCanvas->task==DETECTION3D){
        const AnnotationContainer *pAnnoContainer = parentCanvas->pAnnoContainer;
        for (int i=pAnnoContainer->length()-1;i>=0;i--){
            auto item = CubeAnnotationItem::castPointer((*pAnnoContainer)[i]);
            Cuboid cube=item->getCube();
            // 对于一些很小的矩形，要允许一些偏差，否则将很难点中
            if (cube.contains(pos, 2))
                return i;
        }
        return -1;
    }
    return -1;
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

QPoint ChildCanvas3D::_getFocus2dFromParent() const {
    int focusU,focusV;
    switch(axis){
    case X: focusU = parentCanvas->focusPos.z; focusV = parentCanvas->focusPos.y; break;
    case Y: focusU = parentCanvas->focusPos.x; focusV = parentCanvas->focusPos.z; break;
    case Z: focusU = parentCanvas->focusPos.x; focusV = parentCanvas->focusPos.y; break;
    }
    return QPoint(focusU,focusV);
}
