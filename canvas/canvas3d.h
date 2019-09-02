#ifndef CANVAS3D_H
#define CANVAS3D_H

#include "canvas.h"
#include "annotationcontainer.h"
#include "segannotationitem.h"
#include "labelmanager.h"
#include "childcanvas3d.h"
#include <QWidget>
#include <QList>
#include <QGridLayout>

class Canvas3D : public QWidget
{
    Q_OBJECT
public:
    explicit Canvas3D(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent=nullptr);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setScale(qreal newScale);

    void loadImagesZ(QStringList imagesFile);

    void changeTask(TaskMode _task);
    TaskMode getTaskMode() const { return task; }

signals:
    void mouse3DMoved(int x,int y,int z);
public slots:
    void paintEvent(QPaintEvent*);
    void changeDrawMode(DrawMode _draw);
private:
    qreal scale;

    QGridLayout *layout;
    ChildCanvas3D *canvasZ, *canvasX, *canvasY;
    QList<QImage> imagesZ;

    //! related data
    const AnnotationContainer *pAnnoContainer;
    const LabelManager* pLabelManager;

    int mouseX,mouseY,mouseZ;

    TaskMode task;
    DrawMode drawMode;

    void _syncMousePos();
    QImage getYSlides(const QList<QImage>& _imageZ, int y);
    QImage getXSlides(const QList<QImage>& _imageZ, int x);
};

#endif // CANVAS3D_H
