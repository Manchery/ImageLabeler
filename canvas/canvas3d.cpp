#include "canvas3d.h"
#include <QGridLayout>
#include <QPixmap>
#include <QtDebug>
#include <QPainter>
#include <cmath>

Canvas3D::Canvas3D(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent):
    CanvasBase (pLabelManager, pAnnoContainer, parent), focusPos(0,0,0)
{
    //! layout
    layout = new QGridLayout(this);
    this->setLayout(layout);

    canvasZ = new ChildCanvas3D(this);
    canvasX = new ChildCanvas3D(this);
    canvasY = new ChildCanvas3D(this);
    layout->addWidget(canvasZ, 0, 0);
    layout->addWidget(canvasX, 0, 1);
    layout->addWidget(canvasY, 1, 0);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    //! end layout

    connect(canvasX, &ChildCanvas3D::mouseMoved, [this](QPoint pos, bool mousePressed){
        qDebug()<<"canvasX mouse move: "<<pos.x()<<" "<<pos.y();
        if (mode == MOVE && mousePressed){
            focusPos.z = pos.x(); focusPos.y = pos.y();
            _updateFocusPos();
        }
    });
    connect(canvasY, &ChildCanvas3D::mouseMoved, [this](QPoint pos, bool mousePressed){
        qDebug()<<"canvasY mouse move: "<<pos.x()<<" "<<pos.y();
        if (mode == MOVE && mousePressed){
            focusPos.x = pos.x(); focusPos.z = pos.y();
            _updateFocusPos();
        }
    });
    connect(canvasZ, &ChildCanvas3D::mouseMoved, [this](QPoint pos, bool mousePressed){
        qDebug()<<"canvasZ mouse move: "<<pos.x()<<" "<<pos.y();
        if (mode == MOVE && mousePressed){
            focusPos.x = pos.x(); focusPos.y = pos.y();
            _updateFocusPos();
        }
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
    focusPos = Point3D(0,0,0);
    repaint();
    _sizeUnscaled = layout->minimumSize();
}

void Canvas3D::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control){
        lastMode = mode;
        changeCanvasMode(MOVE);
    }else{
        QWidget::keyPressEvent(event);
    }
}

void Canvas3D::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key()==Qt::Key_Control){
        changeCanvasMode(lastMode);
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
    if (mode == _mode) return;
    mode = _mode;
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

