#ifndef CANVAS2D_H
#define CANVAS2D_H

#include "cubeannotationitem.h"
#include <QWidget>
#include <QPixmap>
#include <QImage>

enum Axis{
    X,Y,Z
};

class Canvas3D;

class ChildCanvas3D : public QWidget
{
    Q_OBJECT
public:
    explicit ChildCanvas3D(Canvas3D* parentCanvas, Axis axis, QWidget *parent = nullptr);

    QSize getImageSize() const { return image.size(); }
    int getImageHeight() const { return image.height(); }
    int getImageWidth() const { return image.width(); }

    void loadImage(const QImage &newImage);

    bool outOfPixmap(QPoint pos);
    void setScale(qreal new_scale);

    bool drawingRect(){
        return curPoints.length()==2;
    }
    void resetMousePressing() { mousePressing = false; }
signals:
    void focusMoved(QPoint pos);
    void newRectAnnotated(QRect rect);

public slots:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseSetRequest(QPoint pos);

private:
    const Canvas3D *parentCanvas;
    Axis axis;

    qreal scale;
    QImage image;

    QPoint mousePos;

    bool mousePressing;

    QPoint pixelPos(QPoint pos);
    QPoint boundedPixelPos(QPoint pos);

    //! for bbox drawing
    QList<QPoint> curPoints;

    bool _showableForCube(Cuboid cube) const;
    QRect _rectForCube(Cuboid cube) const;
};

#endif // CANVAS2D_H
