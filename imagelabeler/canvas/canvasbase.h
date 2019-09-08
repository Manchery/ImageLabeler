#ifndef CANVASBASE_H
#define CANVASBASE_H

#include <QObject>
#include <QWidget>

// 用于表示当前任务的枚举类型：分别为2D检测，2D分割，3D检测，3D分割
enum TaskMode{
    DETECTION, SEGMENTATION, DETECTION3D, SEGMENTATION3D
};
// 用于表示当前canvas的模式的枚举类型：分别表示绘制模式，选中模式，移动模式
// DRAW 用于绘制一个标注
// SELECT 用于选中一个标注并编辑之
// MOVE 仅限3D任务，用于移动焦点的位置，对应移动三个切面的坐标
enum CanvasMode{
    DRAW, SELECT, MOVE
};
// 用于表示当前绘制模式的枚举类型：分别为矩形（或立方体），轮廓，多边形轮廓，圆形画笔，方形画笔
enum DrawMode{
    RECTANGLE,
    CONTOUR, SQUAREPEN, CIRCLEPEN, POLYGEN
};

class AnnotationContainer;
class LabelManager;

/* CanvasBase
 * 简介：画布类的基类，用于在窗口中显示被标注的图片以及提供标注操作
 *      画布可缩放
 *      该基类声明了2D画布与3D画布的公共的借口，便于与MainWindow的交互的统一
 */

class CanvasBase : public QWidget
{
    Q_OBJECT
public:
    explicit CanvasBase(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent = nullptr);

    TaskMode getTaskMode() const { return task; }
    CanvasMode getCanvasMode() const { return mode; }
    DrawMode getDrawMode() const { return drawMode; }
    qreal getScale() const { return scale; }
    int getLastPenWidth() const { return lastPenWidth; }

    // 这两个重载函数对于adjustSize()以及layout等是必须的
    QSize sizeHint() const override { return minimumSizeHint(); }
    QSize minimumSizeHint() const override = 0;
    // 返回未缩放情况下的画布大小，可用于适应窗口大小（fitWindow）等
    virtual QSize sizeUnscaled() const = 0;
    // 返回一个字符串描述当前的模式信息（包括task，mode，drawMode），可用于状态栏等
    virtual QString modeString() const;

signals:
    void modeChanged(QString mode);

public slots:
    virtual void setScale(qreal newScale) = 0;
    virtual void setPenWidth(int) = 0;
    virtual void changeTask(TaskMode _task) = 0;
    virtual void changeCanvasMode(CanvasMode _mode) = 0;
    virtual void changeDrawMode(DrawMode _draw) = 0;
    // 清空画布
    virtual void close() = 0;

protected:
    // 缩放比例，默认为 1.0
    qreal scale;

    // canvas的模式，drawMode只在mode为Draw时有意义
    TaskMode task;
    CanvasMode mode;
    DrawMode drawMode;

    // 相关数据的指针
    const AnnotationContainer *pAnnoContainer;
    const LabelManager* pLabelManager;

    // 分割时的画笔大小，lastPenWidth用于存储上一次分割时的画笔大小，在本次切换至画笔时还原上次的大小
    int lastPenWidth;
    int curPenWidth;
};

#endif // CANVASBASE_H
