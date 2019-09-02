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
        focusPos.z = pos.x(); focusPos.y = pos.y();
        update();
        emit focus3dMoved(focusPos);
    });
    connect(canvasY, &ChildCanvas3D::focusMoved, [this](QPoint pos){
        focusPos.x = pos.x(); focusPos.z = pos.y();
        update();
        emit focus3dMoved(focusPos);
    });
    connect(canvasZ, &ChildCanvas3D::focusMoved, [this](QPoint pos){
        focusPos.x = pos.x(); focusPos.y = pos.y();
        update();
        emit focus3dMoved(focusPos);
    });

    connect(canvasX, &ChildCanvas3D::cursorMoved, this, &Canvas3D::cursor3dMoved);
    connect(canvasY, &ChildCanvas3D::cursorMoved, this, &Canvas3D::cursor3dMoved);
    connect(canvasZ, &ChildCanvas3D::cursorMoved, this, &Canvas3D::cursor3dMoved);

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

    connect(canvasX, &ChildCanvas3D::removeCubeRequest, this, &Canvas3D::removeCubeRequest);
    connect(canvasY, &ChildCanvas3D::removeCubeRequest, this, &Canvas3D::removeCubeRequest);
    connect(canvasZ, &ChildCanvas3D::removeCubeRequest, this, &Canvas3D::removeCubeRequest);

    connect(canvasX, &ChildCanvas3D::mousePressWhenSelected,
            [this](Point3D cursorPos){ this->mousePressedWhenSelected(cursorPos, canvasX); });
    connect(canvasY, &ChildCanvas3D::mousePressWhenSelected,
            [this](Point3D cursorPos){ this->mousePressedWhenSelected(cursorPos, canvasY); });
    connect(canvasZ, &ChildCanvas3D::mousePressWhenSelected,
            [this](Point3D cursorPos){ this->mousePressedWhenSelected(cursorPos, canvasZ); });

    connect(canvasX, &ChildCanvas3D::mouseMoveWhenSelected, this, &Canvas3D::mouseMovedWhenSelected);
    connect(canvasY, &ChildCanvas3D::mouseMoveWhenSelected, this, &Canvas3D::mouseMovedWhenSelected);
    connect(canvasZ, &ChildCanvas3D::mouseMoveWhenSelected, this, &Canvas3D::mouseMovedWhenSelected);

    connect(canvasX, &ChildCanvas3D::mouseReleaseWhenSelected, this, &Canvas3D::mouseReleasedWhenSelected);
    connect(canvasY, &ChildCanvas3D::mouseReleaseWhenSelected, this, &Canvas3D::mouseReleasedWhenSelected);
    connect(canvasZ, &ChildCanvas3D::mouseReleaseWhenSelected, this, &Canvas3D::mouseReleasedWhenSelected);
}


void Canvas3D::mousePressedWhenSelected(Point3D cursorPos, ChildCanvas3D *child)
{
    auto item = CubeAnnotationItem::castPointer(pAnnoContainer->getSelectedItem());
    Cuboid selectedCube = item->cube;
    if (onCubeFront(cursorPos, selectedCube) && (child==canvasZ || child == canvasX)){
        editing = true; editingCube = selectedCube; editingCubeFace = FRONTf;
    } else if (onCubeBack(cursorPos, selectedCube) && (child==canvasZ || child == canvasX)){
        editing = true; editingCube = selectedCube; editingCubeFace = BACKf;
    } else if (onCubeLeft(cursorPos, selectedCube) && (child==canvasZ || child == canvasY)){
        editing = true; editingCube = selectedCube; editingCubeFace = LEFTf;
    } else if (onCubeRight(cursorPos, selectedCube) && (child==canvasZ || child == canvasY)){
        editing = true; editingCube = selectedCube; editingCubeFace = RIGHTf;
    } else if (onCubeTop(cursorPos, selectedCube) && (child==canvasX || child == canvasY)){
        editing = true; editingCube = selectedCube; editingCubeFace = TOPf;
    } else if (onCubeBottom(cursorPos, selectedCube) && (child==canvasX || child == canvasY)){
        editing = true; editingCube = selectedCube; editingCubeFace = BOTTOMf;
    }
}

void Canvas3D::mouseMovedWhenSelected(Point3D cursorPos)
{
    switch(editingCubeFace){
    case FRONTf:
        editingCube.setmaxY(cursorPos.y); break;
    case BACKf:
        editingCube.setminY(cursorPos.y); break;
    case LEFTf:
        editingCube.setminX(cursorPos.x); break;
    case RIGHTf:
        editingCube.setmaxX(cursorPos.x); break;
    case TOPf:
        editingCube.setminZ(cursorPos.z); break;
    case BOTTOMf:
        editingCube.setmaxZ(cursorPos.z); break;
    }
    canvasX->update();
    canvasY->update();
    canvasZ->update();
}

void Canvas3D::mouseReleasedWhenSelected()
{
    editing = false;
    emit modifySelectedCubeRequest(pAnnoContainer->getSelectedIdx(), editingCube.normalized());
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

