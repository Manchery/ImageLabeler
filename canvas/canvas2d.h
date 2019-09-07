#ifndef CANVAS2D_H
#define CANVAS2D_H

#include "canvasbase.h"
#include "segannotationitem.h"
#include "common.h"
#include <QRect>

class Canvas2D : public CanvasBase
{
    Q_OBJECT    
public:
    explicit Canvas2D(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent=nullptr);

    /*--------------------from CanvasBase (parent class)-------------------*/
    QSize minimumSizeHint() const override;
    QSize sizeUnscaled() const override { return pixmap.size(); }
    /*--------------------from CanvasBase (parent class) END----------------*/

    const QPixmap &getPixmap() const { return pixmap; }

    int getLastPenWidth() const  { return lastPenWidth; }

    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    //! for bbox removing
    int selectShape(QPoint pos);

signals:
    void mouseMoved(QPoint pos);

    void newRectangleAnnotated(QRect newRect);
    void newStrokesAnnotated(const QList<SegStroke> &strokes);

    void modifySelectedRectRequest(int idx, QRect rect);
    void removeRectRequest(int idx);
    void modeChanged(QString mode);

public slots:
    /*--------------------from CanvasBase (parent class)-------------------*/
    void setScale(qreal) override;
    void changeTask(TaskMode _task) override;
    void changeCanvasMode(CanvasMode _mode) override;
    void changeDrawMode(DrawMode _draw) override;
    /*--------------------from CanvasBase (parent class) END----------------*/

    void loadPixmap(QPixmap);
    void paintEvent(QPaintEvent*) override;
    void setPenWidth(int width) override {
        curPenWidth = width;
        if (drawMode==CIRCLEPEN || drawMode==SQUAREPEN)
            lastPenWidth = width;
        update();
    }
    void close() override;

private:
    QPixmap pixmap;

    QPoint mousePos;

    //! for coord & zoom
    QPoint offsetToCenter();

    QPoint pixelPos(QPoint pos);
    QPoint boundedPixelPos(QPoint pos);
    bool outOfPixmap(QPoint pos);

    //! for bbox drawing
    QList<QPoint> curPoints;
    //! for bbox editing
    QRect editingRect;
    bool editing;
    CanvasUtils::EditingRectEdge editingRectEdge;

    //! for seg drawing
    bool strokeDrawing;
//    int lastPenWidth;
//    int curPenWidth;
    QList<SegStroke> curStrokes;
};

#endif // CANVAS2D_H
