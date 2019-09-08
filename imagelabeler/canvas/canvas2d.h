#ifndef CANVAS2D_H
#define CANVAS2D_H

#include "canvasbase.h"
#include "segannotationitem.h"
#include "common.h"
#include <QRect>

/* Canvas2D
 * 简介：用于2D检测和分割的画布类，显示单张图片，并可在上面绘制矩形或“笔画”
 */
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

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // 鼠标双击后，“多边形轮廓” 类型的一个“笔画”绘制完毕
    void mouseDoubleClickEvent(QMouseEvent *event) override;
public:
    // 分割模式下按下回车键，一个分割标注绘制完毕，由已绘制的所有笔画组成
    void keyPressEvent(QKeyEvent *event) override;

signals:
    // 鼠标像素坐标的移动
    void mouseMoved(QPoint pos);
    // 一个新的矩形标注绘制完毕，待MainWindow处理
    void newRectangleAnnotated(QRect newRect);
    // 一个新的分割标注绘制完毕，待MainWindow处理
    void newStrokesAnnotated(const QList<SegStroke> &strokes);
    // 修改一个矩形标注的请求，待MainWindow处理
    void modifySelectedRectRequest(int idx, QRect rect);
    // 删除一个矩形标注的请求，待MainWindow处理
    void removeRectRequest(int idx);

public slots:
    /*--------------------from CanvasBase (parent class)-------------------*/
    void setScale(qreal) override;
    void setPenWidth(int width) override;
    void changeTask(TaskMode _task) override;           // emit modeChanged(modeString());
    void changeCanvasMode(CanvasMode _mode) override;   // emit modeChanged(modeString());
    void changeDrawMode(DrawMode _draw) override;       // emit modeChanged(modeString());
    void close() override;
    /*--------------------from CanvasBase (parent class) END----------------*/

    void loadPixmap(QPixmap);

private:
    QPixmap pixmap;     // 被绘制和标注的图像
    QPoint mousePos;    // 当前鼠标的像素坐标

    /* 坐标相关的函数 */
    // pixmap绘制位置的偏移量，这是由于当pixmap的大小小于画布（窗口）大小时，将pixmap绘制在中央
    QPoint offsetToCenter();
    // 将画布坐标转化为pixmap的像素坐标，具体为 pos / scale - offsetToCenter();
    QPoint pixelPos(QPoint pos);
    // 将画布坐标转化为pixmap的像素坐标，当超出pixmap的范围时，将其限制在pixmap边界上
    QPoint boundedPixelPos(QPoint pos);
    // 判断像素坐标pos是否超出pixmap的范围
    bool outOfPixmap(QPoint pos);

    /* 用于矩形标注的绘制 */
    QList<QPoint> curPoints;        // cur 是 current 的缩写

    /* 用于矩形标注的编辑（平移某条边） */
    bool rectEditing;
    QRect editedRect;
    CanvasUtils::EditingRectEdge editedRectEdge;

    /* 用于矩形标注的删除 */
    // 返回包含pos的矩形的idx，若存在多个，则返回添加时间最近的一个
    int selectShape(QPoint pos);

    /* 用于分割标注的绘制 */
    bool strokeDrawing;
    // int lastPenWidth;            // 父类的成员
    // int curPenWidth;             // 父类的成员
    QList<SegStroke> curStrokes;    // cur 是 current 的缩写
};

#endif // CANVAS2D_H
