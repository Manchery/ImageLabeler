#ifndef CANVAS_H
#define CANVAS_H

#include "annotationcontainer.h"
#include "segannotationitem.h"
#include "labelmanager.h"
#include <QWidget>
#include <QRect>

enum TaskMode{
    DETECTION, SEGMENTATION, DETECTION3D, SEGMENTATION3D
};
enum CanvasMode{
    DRAW, SELECT
};
enum DrawMode{
    RECTANGLE, //! for detection
    CONTOUR, SQUAREPEN, CIRCLEPEN, POLYGEN //! for segmentation
};

enum EditingRectEdge{
    TOP, BOTTOM, LEFT, RIGHT
};

class Canvas : public QWidget
{
    Q_OBJECT    
public:
    explicit Canvas(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent=nullptr);

    const QPixmap& getPixmap() const;
    qreal getScale() const;
    TaskMode getTaskMode() const { return task; }
    int getLastPenWidth() const  { return lastPenWidth; }

    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

    //! for coord & zoom
    // these two functions are required by adjustSize()
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    //! for bbox editing
    int selectShape(QPoint pos);

    //! mode
    QString modeString() const;
    void changeTask(TaskMode _task);
    void changeDrawMode(DrawMode _draw);

signals:
    void mouseMoved(QPoint pos);
    void zoomRequest(qreal delta, QPoint pos);

    void newRectangleAnnotated(QRect newRect);
    void newStrokesAnnotated(const QList<SegStroke> &strokes);

    void modifySelectedRectRequest(int idx, QRect rect);
    void removeRectRequest(int idx);
    void modeChanged(QString mode);

public slots:
    void loadPixmap(QPixmap);
    void setScale(qreal);
    void paintEvent(QPaintEvent*);
    void changeCanvasModeRequest();
    void setPenWidth(int width) {
        curPenWidth = width;
        if (drawMode==CIRCLEPEN || drawMode==SQUAREPEN)
            lastPenWidth = width;
        update();
    }

private:
    QPixmap pixmap;

    QPoint mousePos;

    //! mode
    TaskMode task;
    CanvasMode mode;
    DrawMode drawMode;

    //! related data
    const AnnotationContainer *pAnnoContainer;
    const LabelManager* pLabelManager;

    //! for coord & zoom
    qreal scale;
    QPoint offsetToCenter();

    QPoint pixelPos(QPoint pos);
    QPoint boundedPixelPos(QPoint pos);
    bool outOfPixmap(QPoint pos);

    //! for bbox drawing
    QList<QPoint> curPoints;
    //! for bbox editing
    QRect editingRect;
    bool editing;
    EditingRectEdge editingRectEdge;

    //! for seg drawing
    bool strokeDrawing;
    int lastPenWidth;
    int curPenWidth;
    QList<SegStroke> curStrokes;
};

#endif // CANVAS_H
