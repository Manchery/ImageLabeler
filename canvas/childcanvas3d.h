#ifndef CANVAS2D_H
#define CANVAS2D_H

#include "cubeannotationitem.h"
#include "utils.h"
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

    Point3D cursorPos3d() const;

    int selectShape(Point3D pos);
signals:
    void focusMoved(QPoint pos);
    void cursorMoved(Point3D pos);
    void newRectAnnotated(QRect rect);
    void removeCubeRequest(int idx);

    void mousePressWhenSelected(Point3D cursorPos);
    void mouseMoveWhenSelected(Point3D cursorPos);
    void mouseReleaseWhenSelected();

public slots:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    const Canvas3D *parentCanvas;
    Axis axis;

    QPoint cursorPos;

    qreal scale;
    QImage image;

    bool mousePressing;
    bool mousePressingWhenSelected;

    QPoint pixelPos(QPoint pos);
    QPoint boundedPixelPos(QPoint pos);

    //! for bbox drawing
    QList<QPoint> curPoints;

    bool _showableForCube(Cuboid cube) const;
    QRect _rectForCube(Cuboid cube) const;
};

#endif // CANVAS2D_H
