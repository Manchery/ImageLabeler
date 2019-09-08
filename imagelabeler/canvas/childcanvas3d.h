#ifndef CHILDCANVAS3D_H
#define CHILDCANVAS3D_H

#include "cubeannotationitem.h"
#include "segannotationitem.h"
#include "common.h"
#include <QWidget>
#include <QPixmap>
#include <QImage>

// 用于表示坐标轴的枚举类型
enum Axis{
    X,Y,Z
};

/* Canvas2D
 * 简介：用于3D检测和分割的画布类的组成部分，只显示沿一个坐标平面的切面
 *      3D坐标与切面的2D坐标的对应关系详见Canvas3D
 */

class Canvas3D;
class ChildCanvas3D : public QWidget
{
    Q_OBJECT
    friend class Canvas3D;
public:
    explicit ChildCanvas3D(Canvas3D* parentCanvas, Axis axis, QWidget *parent = nullptr);

    QSize getImageSize() const { return image.size(); }
    int getImageHeight() const { return image.height(); }
    int getImageWidth() const { return image.width(); }
    bool isDrawingRect() const { return curPoints.length()==2; }
    Point3D cursorPos3d() const;    // 返回当前鼠标的3D坐标

    void loadImage(const QImage &newImage);

protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

signals:
    void focusMoved(QPoint pos);
    void cursorMoved(Point3D pos);

    void newRectAnnotated(QRect rect);
    void removeCubeRequest(int idx);

    /* 相关：cube的编辑（即拖动平移某个面），待Canvas3D处理 */
    void mousePressWhenSelected(Point3D cursorPos);
    void mouseMoveWhenSelected(Point3D cursorPos);
    void mouseReleaseWhenSelected();

    void removeLatestStrokeRequest();
    void newStrokeRequest(SegStroke3D stroke);

public slots:
    void setScale(qreal new_scale);
    void close();

private:
    const Canvas3D *parentCanvas;
    Axis axis;              // 该切面对应的坐标轴，详见Canvas3D的说明
    qreal scale;            // 缩放比例
    QPoint cursorPos;       // 鼠标在该切面的像素坐标系下的坐标
    QImage image;

    /* 相关：坐标 */
    // 将画布坐标转化为pixmap的像素坐标，具体为 pos / scale - offsetToCenter();
    QPoint pixelPos(QPoint pos);
    // 将画布坐标转化为pixmap的像素坐标，当超出pixmap的范围时，将其限制在pixmap边界上
    QPoint boundedPixelPos(QPoint pos);
    // 判断像素坐标pos是否超出pixmap的范围
    bool outOfPixmap(QPoint pos);

    /* 相关：焦点的移动 */
    bool focusMoving;
    Axis movingFocusAxis; // 正在移动focus的哪个坐标，若为Z则同时移动x,y，这里的xy是指当前切面下像素坐标系的xy
    QPoint editedFocus;

    /* 相关：cube的绘制 */
    QList<QPoint> curPoints;                    // cur是current的缩写
    bool _showableForCube(Cuboid cube) const;   // cube与当前切面是否有交
    QRect _rectForCube(Cuboid cube) const;      // cube在当前切面的投影矩形

    /* 相关：cube的编辑（即拖动平移某个面） */
    bool mousePressingWhenSelected;
    // 返回包含pos的cube的idx，若存在多个，则返回添加时间最近的一个
    int selectShape(Point3D pos);

    /* 相关：笔画的绘制 */
    bool strokeDrawable;    // 只有对应Z轴的切面上能够绘制笔画，其余只能同步显示，详见Canvas3D
    bool strokeDrawing;
    SegStroke3D curStroke;  // cur是current的缩写

    // 从parent的3d focus坐标得到该切面下的2d focus坐标
    QPoint _getFocus2dFromParent() const;
};

#endif // CHILDCANVAS3D_H
