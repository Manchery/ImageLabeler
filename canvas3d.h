#ifndef CANVAS3D_H
#define CANVAS3D_H

#include "canvas2d.h"
#include <QWidget>
#include <QList>

class Canvas3D : public QWidget
{
    Q_OBJECT
public:
    explicit Canvas3D(QWidget *parent = nullptr);

signals:
    void mouse3DMoved(int x,int y,int z);
public slots:
    void paintEvent(QPaintEvent*);
private:
    Canvas2D *canvasZ, *canvasX, *canvasY;
    QList<QImage> imagesZ;

    int mouseX,mouseY,mouseZ;

    void _syncMousePos();
    QImage getYSlides(int y);
    QImage getXSlides(int x);
};

#endif // CANVAS3D_H
