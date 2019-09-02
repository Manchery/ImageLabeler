#include "canvas3d.h"
#include <QGridLayout>
#include <QPixmap>
#include <QtDebug>
#include <QPainter>
#include <cmath>
#include <algorithm>

Canvas3D::Canvas3D(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent):
    CanvasBase (pLabelManager, pAnnoContainer, parent), focusPos(0,0,0)
{
    //! layout
    layout = new QGridLayout(this);
    this->setLayout(layout);

    canvasZ = new ChildCanvas3D(this, Z, this);
    canvasX = new ChildCanvas3D(this, X, this);
    canvasY = new ChildCanvas3D(this, Y, this);
    layout->addWidget(canvasZ, 0, 0);
    layout->addWidget(canvasX, 0, 1);
    layout->addWidget(canvasY, 1, 0);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    //! end layout

    connect(canvasX, &ChildCanvas3D::focusMoved, [this](QPoint pos){
        qDebug()<<"canvasX mouse move: "<<pos.x()<<" "<<pos.y();
        focusPos.z = pos.x(); focusPos.y = pos.y();
        _updateFocusPos();
    });
    connect(canvasY, &ChildCanvas3D::focusMoved, [this](QPoint pos){
        qDebug()<<"canvasY mouse move: "<<pos.x()<<" "<<pos.y();
        focusPos.x = pos.x(); focusPos.z = pos.y();
        _updateFocusPos();
    });
    connect(canvasZ, &ChildCanvas3D::focusMoved, [this](QPoint pos){
        qDebug()<<"canvasZ mouse move: "<<pos.x()<<" "<<pos.y();
        focusPos.x = pos.x(); focusPos.y = pos.y();
        _updateFocusPos();
    });

    connect(canvasX, &ChildCanvas3D::newRectAnnotated, [this](QRect rect){
        Cuboid cube;
        cube.setTopLeft(std::max(focusPos.x-10, 0), rect.top(), rect.left());
        cube.setBottomRight(std::min(focusPos.x+10, sizeX()-1), rect.bottom(), rect.right());
        emit newCubeAnnotated(cube);
    });
    connect(canvasY, &ChildCanvas3D::newRectAnnotated, [this](QRect rect){
        Cuboid cube;
        cube.setTopLeft(rect.left(), std::max(focusPos.y-10, 0), rect.top());
        cube.setBottomRight(rect.right(), std::min(focusPos.y+10, sizeY()-1), rect.bottom());
        emit newCubeAnnotated(cube);
    });
    connect(canvasZ, &ChildCanvas3D::newRectAnnotated, [this](QRect rect){
        Cuboid cube;
        cube.setTopLeft(rect.left(), rect.top(), std::max(focusPos.z-10, 0));
        cube.setBottomRight(rect.right(), rect.bottom(), std::min(focusPos.z+10, sizeZ()-1));
        emit newCubeAnnotated(cube);
    });
}

QSize Canvas3D::minimumSizeHint() const
{
    if (imagesZ.length()>0){
        return layout->minimumSize();
    }
    return QWidget::minimumSizeHint();
}

void Canvas3D::setScale(qreal newScale)
{
    scale = newScale;
    canvasX->setScale(scale);
    canvasY->setScale(scale);
    canvasZ->setScale(scale);
}

void Canvas3D::loadImagesZ(QStringList imagesFile)
{
    imagesZ.clear();
    for (auto file: imagesFile)
        imagesZ.push_back(QImage(file));
    focusPos = Point3D(imagesZ[0].width()/2,imagesZ[0].height()/2,0);
    repaint();
    _sizeUnscaled = layout->minimumSize();
}

void Canvas3D::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control){
        lastMode = mode;
        //!!! TODO: need some if to avoid breaking current operation
        changeCanvasMode(MOVE);
    }else{
        QWidget::keyPressEvent(event);
    }
}

void Canvas3D::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key()==Qt::Key_Control){
        if (mode==MOVE){
            changeCanvasMode(lastMode);
        }
    }else{
        QWidget::keyReleaseEvent(event);
    }
}

void Canvas3D::paintEvent(QPaintEvent *event)
{
    if (imagesZ.length()>0){
        canvasZ->loadImage(imagesZ[focusPos.z]);
        canvasX->loadImage(getXSlides(imagesZ, focusPos.x));
        canvasY->loadImage(getYSlides(imagesZ, focusPos.y));
    }else{
        QWidget::paintEvent(event);
    }
}

void Canvas3D::_updateFocusPos()
{
    canvasY->mouseSetRequest(QPoint(focusPos.x,focusPos.z));
    canvasX->mouseSetRequest(QPoint(focusPos.z,focusPos.y));
    canvasZ->mouseSetRequest(QPoint(focusPos.x,focusPos.y));
    update();
    emit focusMoved(focusPos);
}

QImage Canvas3D::getYSlides(const QList<QImage> &_imageZ, int y)
{
    int row = _imageZ[0].width();
    QImage image(QSize(row, _imageZ.length()), _imageZ[0].format());
    for (int z=0;z<_imageZ.length();z++){
        for (int i=0;i<row;i++){
            image.setPixel(i,z, _imageZ[z].pixel(i,y));
        }
    }
    return image;
}

QImage Canvas3D::getXSlides(const QList<QImage> &_imageZ, int x)
{
    int row = _imageZ[0].height();
    QImage image(QSize(_imageZ.length(), row), _imageZ[0].format());
    for (int z=0;z<_imageZ.length();z++){
        for (int i=0;i<row;i++)
            image.setPixel(z,i, _imageZ[z].pixel(x,i));
    }
    return image;
}

void Canvas3D::changeTask(TaskMode _task) {
    switch(_task){
    case DETECTION3D:
        task = TaskMode::DETECTION3D;
        mode = CanvasMode::DRAW;
        drawMode = DrawMode::RECTANGLE;
        break;
    case SEGMENTATION3D:
        task = TaskMode::SEGMENTATION3D;
        break;
    default:
        throw "abnormal 2d task set to canvas 3d";
    }
    emit modeChanged(modeString());
}

void Canvas3D::changeCanvasMode(CanvasMode _mode)
{
    //!</TODO
    if (mode == SELECT){
        if (task == DETECTION3D){
            auto item = CubeAnnotationItem::castPointer(pAnnoContainer->getSelectedItem());
            setFocusPos(item->cube.center());
        }
    }
    //!TODO/>
    if (mode == _mode) return;
    mode = _mode;
    if (mode == MOVE){
        canvasX->resetMousePressing();
        canvasY->resetMousePressing();
        canvasZ->resetMousePressing();
    }
    emit modeChanged(modeString());
}

void Canvas3D::changeDrawMode(DrawMode _draw)
{
    drawMode=_draw;
    switch (drawMode) {
    case RECTANGLE:
        break;
    case CIRCLEPEN:
    case SQUAREPEN:
        break;
    case CONTOUR:
    case POLYGEN:
        break;
    }
    emit modeChanged(modeString());
}

