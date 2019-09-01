#include "canvas3d.h"
#include <QGridLayout>
#include <QPixmap>
#include <QtDebug>
#include <QPainter>

Canvas3D::Canvas3D(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent):
    QWidget(parent),
    pAnnoContainer(pAnnoContainer),
    pLabelManager(pLabelManager)
{
    scale = 1.0;
    mouseX = mouseY = mouseZ = 0;

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

    connect(canvasX, &ChildCanvas3D::mouseMoved, [this](QPoint pos){
        mouseZ = pos.x(); mouseY = pos.y();
        _syncMousePos();
    });
    connect(canvasY, &ChildCanvas3D::mouseMoved, [this](QPoint pos){
        mouseX = pos.x(); mouseZ = pos.y();
        _syncMousePos();
    });
    connect(canvasZ, &ChildCanvas3D::mouseMoved, [this](QPoint pos){
        mouseX = pos.x(); mouseY = pos.y();
        _syncMousePos();
    });
}

QSize Canvas3D::sizeHint() const
{
    return minimumSizeHint();
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
    mouseX = mouseY = mouseZ = 0;

    canvasZ->loadImage(imagesZ[mouseZ]);
    canvasX->loadImage(getXSlides(imagesZ, mouseX));
    canvasY->loadImage(getYSlides(imagesZ, mouseY));
    canvasZ->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    canvasX->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    canvasY->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}


void Canvas3D::paintEvent(QPaintEvent *event)
{
    if (imagesZ.length()>0){
        canvasZ->loadImage(imagesZ[mouseZ]);
        canvasX->loadImage(getXSlides(imagesZ, mouseX));
        canvasY->loadImage(getYSlides(imagesZ, mouseY));
    }else{
        QWidget::paintEvent(event);
    }
}

void Canvas3D::_syncMousePos()
{
    canvasY->mouseSetRequest(QPoint(mouseX,mouseZ));
    canvasX->mouseSetRequest(QPoint(mouseZ,mouseY));
    canvasZ->mouseSetRequest(QPoint(mouseX,mouseY));
    emit mouse3DMoved(mouseX,mouseY,mouseZ);
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
        break;
    case SEGMENTATION3D:
        task = TaskMode::SEGMENTATION3D;
        break;
    default:
        break;
    }
//    emit modeChanged(modeString());
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
//    emit modeChanged(modeString());
}
