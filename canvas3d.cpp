#include "canvas3d.h"
#include <QGridLayout>
#include <QPixmap>
#include <QtDebug>

Canvas3D::Canvas3D(QWidget *parent) : QWidget(parent)
{
    mouseX = mouseY = mouseZ = 0;

    QGridLayout* layout = new QGridLayout(this);
    this->setLayout(layout);

    canvasZ = new Canvas2D(this);
    canvasX = new Canvas2D(this);
    canvasY = new Canvas2D(this);

    layout->addWidget(canvasZ, 0, 0);
    layout->addWidget(canvasX, 0, 1);
    layout->addWidget(canvasY, 1, 0);

    for (int i=0;i<159;i++){
        imagesZ.push_back(QImage("/Users/manchery/Desktop/labeler_test/ct/"+
                                 QString("%1").arg(i, 4, 10, QLatin1Char('0'))+
                                 ".jpg"));
    }

    connect(canvasX, &Canvas2D::mouseMoved, [this](QPoint pos){
        mouseZ = pos.x(); mouseY = pos.y();
        _syncMousePos();
    });
    connect(canvasY, &Canvas2D::mouseMoved, [this](QPoint pos){
        mouseX = pos.x(); mouseZ = pos.y();
        _syncMousePos();
    });
    connect(canvasZ, &Canvas2D::mouseMoved, [this](QPoint pos){
        mouseX = pos.x(); mouseY = pos.y();
        _syncMousePos();
    });

    canvasZ->loadImage(imagesZ[mouseZ]);
    canvasX->loadImage(getXSlides(mouseX));
    canvasY->loadImage(getYSlides(mouseY));
}

void Canvas3D::paintEvent(QPaintEvent *)
{
    canvasZ->loadImage(imagesZ[mouseZ]);
    canvasX->loadImage(getXSlides(mouseX));
    canvasY->loadImage(getYSlides(mouseY));
}

void Canvas3D::_syncMousePos()
{
    canvasY->mouseSetRequest(QPoint(mouseX,mouseZ));
    canvasX->mouseSetRequest(QPoint(mouseZ,mouseY));
    canvasZ->mouseSetRequest(QPoint(mouseX,mouseY));
    emit mouse3DMoved(mouseX,mouseY,mouseZ);
}

QImage Canvas3D::getYSlides(int y)
{
    int row = imagesZ[0].width();
    QImage image(QSize(row, imagesZ.length()), imagesZ[0].format());
    for (int z=0;z<imagesZ.length();z++){
        for (int i=0;i<row;i++){
            image.setPixel(i,z, imagesZ[z].pixel(i,y));
        }
    }
    return image;
}

QImage Canvas3D::getXSlides(int x)
{
    int row = imagesZ[0].height();
    QImage image(QSize(imagesZ.length(), row), imagesZ[0].format());
    for (int z=0;z<imagesZ.length();z++){
        for (int i=0;i<row;i++)
            image.setPixel(z,i, imagesZ[z].pixel(x,i));
    }
    return image;
}
