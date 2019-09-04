#ifndef CHILDCANVAS3D_H
#define CHILDCANVAS3D_H

#include "cubeannotationitem.h"
#include "segannotationitem.h"
#include "common.h"
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
    friend class Canvas3D;
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

    void removeLatestStrokeRequest();
    void newStrokeRequest(SegStroke3D stroke);

public slots:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    const Canvas3D *parentCanvas;
    Axis axis;

    QPoint cursorPos;

    qreal scale;
    QImage image;

    bool mousePressingWhenMove;
    bool mousePressingWhenSelected;

    QPoint pixelPos(QPoint pos);
    QPoint boundedPixelPos(QPoint pos);

    //! for bbox drawing
    QList<QPoint> curPoints;
    bool _showableForCube(Cuboid cube) const;
    QRect _rectForCube(Cuboid cube) const;

    //! for seg stroke drawing
    bool strokeDrawable;
    bool strokeDrawing;
    SegStroke3D curStroke;
};

#endif // CHILDCANVAS3D_H
