#ifndef CANVAS3D_H
#define CANVAS3D_H

#include "cubeannotationitem.h"
#include "segannotationitem.h"
#include "canvasbase.h"
#include "childcanvas3d.h"
#include <QWidget>
#include <QList>
#include <QGridLayout>

/* Canvas3D
 * 简介：用于3D检测和分割的画布类，分三个切面显示3D物体，可在上面绘制矩形或“笔画”
 *
 * 界面：  +-----x-----+ +--z--+
 *        |           | |     |
 *        y  axis=Z   | y axis|
 *        |           | |  =X |
 *        +-----------+ + ----+
 *        +-----x-----+
 *        z    axis   |
 *        |     =Y    |
 *        +-----------+
 *
 *      界面如上，由三个ChildCanvas3D组成，对应axis的值如图所示
 *      图上还表明了各个切面的坐标轴对应的3D坐标轴，坐标轴的方向都是从左往右从上到下为正方向
 */

class Canvas3D : public CanvasBase
{
    Q_OBJECT
    friend ChildCanvas3D;
public:
    explicit Canvas3D(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent=nullptr);

    /*--------------------from CanvasBase (parent class)-------------------*/
    QSize minimumSizeHint() const override;
    QSize sizeUnscaled() const override { return _sizeUnscaled; }
    /*--------------------from CanvasBase (parent class) END----------------*/

    // 载入一系列沿着z方向形成的切面，并设置 _sizeUnscaled
    void loadImagesZ(QStringList imagesFile);

    QSize imageZSize() const { return canvasZ->image.size(); }
    int sizeX() const { return imagesZ[0].width(); }
    int sizeY() const { return imagesZ[0].height(); }
    int sizeZ() const { return imagesZ.length(); }
    Point3D getFocusPos() const { return focusPos; }
    Point3D getCursorPos() const { return cursorPos; }

    // 返回正在绘制矩形的子画布
    ChildCanvas3D* childDrawingRect() const {
        if (canvasX->isDrawingRect()) return canvasX;
        if (canvasY->isDrawingRect()) return canvasY;
        if (canvasZ->isDrawingRect()) return canvasZ;
        return nullptr;
    }

    // 按下回车表示一个分割标注绘制完毕，按下alt键进入move模式
    void keyPressEvent(QKeyEvent *event) override;
    // 释放alt键退出move模式
    void keyReleaseEvent(QKeyEvent *event) override;

    // 对canvasX, canvasY, canvasZ 分别执行update
    void updateChildren();

signals:
    void focus3dMoved(Point3D focusPos);
    void cursor3dMoved(Point3D cursorPos);
    void newCubeAnnotated(Cuboid cube);                             // 新绘制一个cube的标注，发送到MainWindow处理
    void removeCubeRequest(int idx);                                // 删除一个cube标注的请求，发送到MainWindow处理
    void modifySelectedCubeRequest(int idx, Cuboid cube);           // 修改一个cube标注的请求，发送到MainWind处理
    void newStrokes3DAnnotated(const QList<SegStroke3D>& strokes);  // 新绘制一个3D分割标注的请求，发送到MainWindow处理

public slots:
    /*--------------------from CanvasBase (parent class)-------------------*/
    void setScale(qreal newScale) override;
    void setPenWidth(int width) override;
    void changeDrawMode(DrawMode _draw) override;       // emit modeChanged(modeString());
    void changeCanvasMode(CanvasMode _mode) override;   // emit modeChanged(modeString());
    void changeTask(TaskMode _task) override;           // emit modeChanged(modeString());
    void close() override;
    /*--------------------from CanvasBase (parent class) END----------------*/

    // 往最初的imagesZ上绘制已有的分割标注，这样在setImageForChild时另两个切面也能绘制有标注，类似“烘焙”
    void repaintSegAnnotation();
    // 从imagesZ这一前面的集合中根据focus切割出另两个切面对应的image并设置给三个子画布
    void setImageForChild();
    // 设置focus并调用 setImageForChild
    void setFocusPos(Point3D pos);  //emit focus3dMoved(pos);

    /* 相关：cube编辑，处理ChildCanvas发送来的signal */
    void mousePressedWhenSelected(Point3D cursorPos, ChildCanvas3D *child);
    void mouseMovedWhenSelected(Point3D cursorPos);
    void mouseReleasedWhenSelected();

private:
    QGridLayout *layout;        // 布局三个子画布
    ChildCanvas3D *canvasZ, *canvasX, *canvasY;
    QList<QImage> imagesZ;      // Z方向上的切面，可能经过烘焙（repaintSegAnnotation
    QList<QImage> initImagesZ;  // 最初的未经烘焙的Z方向上的切面

    Point3D focusPos;           // 焦点的坐标，指目前三个切面的交点的坐标
    Point3D cursorPos;          // 鼠标光标指向的坐标

    QSize _sizeUnscaled;

    CanvasMode lastMode;        // 进入move模式之前的mode，便于退出move模式后还原

    // 根据Z方向的切面集合生成对应方向上对应坐标的切面
    QImage getYSlides(const QList<QImage>& _imageZ, int y);
    QImage getXSlides(const QList<QImage>& _imageZ, int x);


    /* 相关：cube的编辑 */
    Cuboid editedCube;
    bool cubeEditing;
    CanvasUtils::EditingCubeFace editedCubeFace;

    /* 相关：分割标注的绘制 */
    // int lastPenWidth; // 来自父类
    // int curPenWidth;  // 来自父类
    QList<SegStroke3D> curStrokes;
};

#endif // CANVAS3D_H
