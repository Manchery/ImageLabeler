#ifndef CANVAS_H
#define CANVAS_H

#include "rectannotations.h"
#include "labelmanager.h"
#include <QWidget>

enum TaskMode{
    DETECTION,SEGMENTATION
};

enum CanvasMode{
    DRAW, EDIT
};
enum CreateMode{
    RECTANGLE
};
class Canvas : public QWidget
{
    Q_OBJECT    
public:
    explicit Canvas(const LabelManager *pLabelManager, const RectAnnotations *pRectAnno, QWidget *parent=nullptr);

    // these two functions are required by adjustSize()
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    qreal getScale() const;
    const QPixmap& getPixmap() const;
    int selectShape(QPoint pos);

    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
//    void mouseReleaseEvent(QMouseEvent *event);
//    void wheelEvent(QWheelEvent *event);

signals:
    void mouseMoved(QPoint pos);
    void zoomRequest(qreal delta, QPoint pos);
    void newRectangleAnnotated(QRect newRect);
    void removeRectRequest(int idx);

public slots:
    void loadPixmap(QPixmap);
    void setScale(qreal);
    void paintEvent(QPaintEvent*);

private:
    QPixmap pixmap;

    QPoint pixelPos(QPoint pos);
    QPoint boundedPixelPos(QPoint pos);
    bool outOfPixmap(QPoint pos);

    qreal scale;
    QPoint offsetToCenter();

    TaskMode task;
    CanvasMode mode;
    CreateMode createMode;

    QList<QPoint> curPoints;

    const RectAnnotations *pRectAnno;
    const LabelManager* pLabelManager;
};

#endif // CANVAS_H
