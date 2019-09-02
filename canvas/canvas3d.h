#ifndef CANVAS3D_H
#define CANVAS3D_H

#include "cubeannotationitem.h"
#include "canvasbase.h"
#include "childcanvas3d.h"
#include <QWidget>
#include <QList>
#include <QGridLayout>

class Canvas3D : public CanvasBase
{
    Q_OBJECT
    friend ChildCanvas3D;
public:
    explicit Canvas3D(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent=nullptr);

    /*--------------------from CanvasBase (parent class)-------------------*/
    QSize minimumSizeHint() const override;
    QSize sizeUnscaled() const override { return _sizeUnscaled; }
    /*--------------------from CanvasBase (parent class) END----------------*/

    void loadImagesZ(QStringList imagesFile);

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    ChildCanvas3D* childDrawingRect() const {
        if (canvasX->drawingRect()) return canvasX;
        if (canvasY->drawingRect()) return canvasY;
        if (canvasZ->drawingRect()) return canvasZ;
        return nullptr;
    }

    int sizeX() const { return imagesZ[0].width(); }
    int sizeY() const { return imagesZ[0].height(); }
    int sizeZ() const { return imagesZ.length(); }

signals:
    void focusMoved(Point3D focusPos);
    void newCubeAnnotated(Cuboid cube);

public slots:
    /*--------------------from CanvasBase (parent class)-------------------*/
    void changeDrawMode(DrawMode _draw) override;
    void changeCanvasMode(CanvasMode _mode) override;
    void changeTask(TaskMode _task) override;
    void setScale(qreal newScale) override;
    /*--------------------from CanvasBase (parent class) END----------------*/

    void setFocusPos(Point3D pos) { focusPos = pos; update(); }
private:
    QGridLayout *layout;
    ChildCanvas3D *canvasZ, *canvasX, *canvasY;
    QList<QImage> imagesZ;

    Point3D focusPos;

    void _updateFocusPos();
    QImage getYSlides(const QList<QImage>& _imageZ, int y);
    QImage getXSlides(const QList<QImage>& _imageZ, int x);

    QSize _sizeUnscaled;

    CanvasMode lastMode;
};

#endif // CANVAS3D_H
