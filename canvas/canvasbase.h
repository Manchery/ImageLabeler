#ifndef CANVASBASE_H
#define CANVASBASE_H

#include <QObject>
#include <QWidget>

enum TaskMode{
    DETECTION, SEGMENTATION, DETECTION3D, SEGMENTATION3D
};
enum CanvasMode{
    DRAW, SELECT, MOVE
};
enum DrawMode{
    RECTANGLE, //! for detection
    CONTOUR, SQUAREPEN, CIRCLEPEN, POLYGEN //! for segmentation
};

class AnnotationContainer;
class LabelManager;

class CanvasBase : public QWidget
{
    Q_OBJECT
public:
    explicit CanvasBase(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent = nullptr);

    TaskMode getTaskMode() const { return task; }
    CanvasMode getCanvasMode() const { return mode; }
    DrawMode getDrawMode() const { return drawMode; }
    qreal getScale() const { return scale; }

    // these two functions are required by adjustSize()
    virtual QSize sizeHint() const { return minimumSizeHint(); }
    virtual QSize minimumSizeHint() const = 0;

    // required for fit window
    virtual QSize sizeUnscaled() const = 0;

    virtual QString modeString() const;

    int getLastPenWidth() const { return lastPenWidth; }
signals:
    void modeChanged(QString mode);
public slots:
    virtual void setScale(qreal newScale) = 0;
    virtual void changeTask(TaskMode _task) = 0;
    virtual void changeCanvasMode(CanvasMode _mode) = 0;
    virtual void changeDrawMode(DrawMode _draw) = 0;

    virtual void setPenWidth(int) = 0;

protected:
    qreal scale;

    //! mode
    TaskMode task;
    CanvasMode mode;
    DrawMode drawMode;

    //! related data
    const AnnotationContainer *pAnnoContainer;
    const LabelManager* pLabelManager;

    //! pen width for segmentation
    int lastPenWidth;
    int curPenWidth;
};

#endif // CANVASBASE_H
