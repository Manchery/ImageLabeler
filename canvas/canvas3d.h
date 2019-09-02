#ifndef CANVAS3D_H
#define CANVAS3D_H

#include "canvasbase.h"
#include "childcanvas3d.h"
#include <QWidget>
#include <QList>
#include <QGridLayout>

class Canvas3D : public CanvasBase
{
    Q_OBJECT
public:
    explicit Canvas3D(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent=nullptr);

    /*--------------------from CanvasBase (parent class)-------------------*/
    QSize minimumSizeHint() const override;
    QSize sizeUnscaled() const override { return _sizeUnscaled; }
    /*--------------------from CanvasBase (parent class) END----------------*/

    void loadImagesZ(QStringList imagesFile);

signals:
    void mouse3DMoved(int x,int y,int z);

public slots:
    /*--------------------from CanvasBase (parent class)-------------------*/
    void changeDrawMode(DrawMode _draw) override;
    void changeCanvasMode(CanvasMode _mode) override;
    void changeTask(TaskMode _task) override;
    void setScale(qreal newScale) override;
    /*--------------------from CanvasBase (parent class) END----------------*/

private:
    QGridLayout *layout;
    ChildCanvas3D *canvasZ, *canvasX, *canvasY;
    QList<QImage> imagesZ;

    int mouseX,mouseY,mouseZ;

    void _syncMousePos();
    QImage getYSlides(const QList<QImage>& _imageZ, int y);
    QImage getXSlides(const QList<QImage>& _imageZ, int x);

    QSize _sizeUnscaled;
};

#endif // CANVAS3D_H
