#ifndef CANVAS_H
#define CANVAS_H

#include "annotationcontainer.h"
#include "labelmanager.h"
#include <QWidget>
#include <QRect>

enum TaskMode{
    DETECTION, SEGMENTATION, DETECTION3D, SEGMENTATION3D
};
enum CanvasMode{
    DRAW, EDIT
};
enum DrawMode{
    RECTANGLE
};

enum EditingRectEdge{
    TOP, BOTTOM, LEFT, RIGHT
};

class Canvas : public QWidget
{
    Q_OBJECT    
public:
    explicit Canvas(const LabelManager *pLabelManager, const AnnotationContainer *pRectAnno, QWidget *parent=nullptr);

    // these two functions are required by adjustSize()
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    qreal getScale() const;
    const QPixmap& getPixmap() const;
    int selectShape(QPoint pos);

    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
//    void wheelEvent(QWheelEvent *event);

    QString modeString() const {
        QString modeStr("");
        if (task == DETECTION) modeStr+="Detection, ";
        else if (task == SEGMENTATION) modeStr+="Segmentation, ";

        if (mode == DRAW) modeStr+="Draw, ";
        else if (mode == EDIT) modeStr+="Edit, ";

        if (drawMode == RECTANGLE) modeStr+="2D Rectangle";

        return modeStr;
    }

signals:
    void mouseMoved(QPoint pos);
    void zoomRequest(qreal delta, QPoint pos);
    void newRectangleAnnotated(QRect newRect);
    void modifySelectedRectRequest(int idx, QRect rect);
    void removeRectRequest(int idx);
    void modeChanged(QString mode);

public slots:
    void loadPixmap(QPixmap);
    void setScale(qreal);
    void paintEvent(QPaintEvent*);
    void changeCanvasModeRequest(){
        if (pRectAnno->getSelectedIdx()==-1)
            mode = DRAW;
        else {
            mode = EDIT;
        }
        emit modeChanged(modeString());
    }

private:
    QPixmap pixmap;

    QPoint pixelPos(QPoint pos);
    QPoint boundedPixelPos(QPoint pos);
    bool outOfPixmap(QPoint pos);

    qreal scale;
    QPoint offsetToCenter();

    TaskMode task;
    CanvasMode mode;
    DrawMode drawMode;

    QList<QPoint> curPoints;

    const AnnotationContainer *pRectAnno;
    const LabelManager* pLabelManager;

    QRect editingRect;
    bool editing;
    EditingRectEdge editingRectEdge;
};

#endif // CANVAS_H
