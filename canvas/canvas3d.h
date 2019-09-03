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

    Point3D getFocusPos() const { return focusPos; }

signals:
    void focus3dMoved(Point3D focusPos);
    void cursor3dMoved(Point3D cursorPos);
    void newCubeAnnotated(Cuboid cube);
    void removeCubeRequest(int idx);
    void modifySelectedCubeRequest(int idx, Cuboid cube);

public slots:
    /*--------------------from CanvasBase (parent class)-------------------*/
    void changeDrawMode(DrawMode _draw) override;
    void changeCanvasMode(CanvasMode _mode) override;
    void changeTask(TaskMode _task) override;
    void setScale(qreal newScale) override;
    /*--------------------from CanvasBase (parent class) END----------------*/

    void setFocusPos(Point3D pos) { focusPos = pos; update(); }

    void mousePressedWhenSelected(Point3D cursorPos, ChildCanvas3D *child);
    void mouseMovedWhenSelected(Point3D cursorPos);
    void mouseReleasedWhenSelected();

    void close();
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

    //! for bbox editing
    Cuboid editingCube;
    bool editing;
    EditingCubeFace editingCubeFace;

};

#endif // CANVAS3D_H
