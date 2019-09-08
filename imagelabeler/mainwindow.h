#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "canvas2d.h"
#include "canvas3d.h"
#include "labeldialog.h"
#include "labelmanager.h"
#include "annotationcontainer.h"
#include "filemanager.h"
#include <QMainWindow>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>


namespace Ui {
class MainWindow;
}

/* MainWindow
 * 简介：提供ui界面，
 *      并将labelManager，annoContainer，fileManager，Canvas通过signal-slot机制连接起来
 */

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 根据canvas的sizeUnscaled和scrollArea的大小计算应该缩放的scale
    qreal scaleFitWindow() const;
    // 返回labelmanager中被选中的label，否则返回 ""
    QString getCurrentLabel() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

public slots:
    // 在状态栏显示鼠标的坐标
    void reportMouse2dMoved(QPoint pos);
    void reportMouse3dMoved();
    // 在状态栏显示模式的改变
    void reportCanvasMode(QString mode);

    // 将canvas缩放到适合窗口大小
    void adjustFitWindow();

    // 相应来自canvas的新标注的请求，将其加入annoContainer中
    void getNewRect(QRect rect);
    void getNewStrokes(const QList<SegStroke> &strokes);
    void getNewCube(Cuboid cube);
    void getNewStrokes3D(const QList<SegStroke3D> &strokes);

    // 尝试添加一个新的label，会检查是否已经存在
    void newLabelRequest(QString newLabel);
    // 尝试删除一个label，会检查是否仍然存在该label的标注，若是则不能删除
    void removeLabelRequest(QString label);

    // 切换到序号idx对应的图像
    bool switchFile(int idx);

    // 对canvas进行update操作，由于canvas可以为2d或3d，具体的操作各不相同
    void canvasUpdate();

private slots:
    void on_actionOpen_File_triggered();    // 先调用on_actionClose_triggered检查未保存的修改
    void on_actionOpen_Dir_triggered();     // 先调用on_actionClose_triggered检查未保存的修改
    void on_actionClose_triggered();
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();
    void on_actionLoad_triggered();
    void on_actionExit_triggered();         // 先调用on_actionClose_triggered
    void on_actionAbout_triggered();

    // 将 fileRelatedActions 都设为 enable
    void enableFileActions();
    // 将 fileRelatedActions 都设为 disable
    void unableFileActions();

    // 依据 taskComboBox 的text改变canvas的task
    void taskModeChanged();
    // 依据 drawComboBox 的text改变canvas的drawMode
    void drawModeChanged();

private:
    Ui::MainWindow *ui;

    // 状态栏以及工具栏中的一些手动添加的部件
    QLabel *mousePosLabel;
    QComboBox *taskComboBox;
    QComboBox *drawComboBox;
    QSpinBox *penWidthBox;

    QList<QAction*> fileRelatedActions;

    CanvasBase *curCanvas; // 取值一定为 canvas2d 或 canvas3d
    Canvas2D *canvas2d;
    Canvas3D *canvas3d;

    // 相关的数据类型
    LabelManager labelManager;
    AnnotationContainer annoContainer;
    FileManager fileManager;

    // 从文件中读取json并给labelmanager和annocontainer读取
    void _loadJsonFile(QString fileName);
    // 关闭文件前，检查是否存在修改未保存，若是且不是autosave的，弹出消息框
    bool _checkUnsaved();
    // 保存2D分割的图像结果，包含 color map 和 label id map
    void _saveSegmentImageResults();
    // 保存3D分割的图像结果，包含z方向上每个切面的 color map 和 label id map
    void _saveSegment3dImageResults();

    // 在构造时调用，创建与之相关的signal-slot连接
    void _setupToolBarAndStatusBar();
    void _setupLabelManager();
    void _setupAnnotationContainer();
    void _setupFileManager();

    // 提供labellist的右键菜单，包含 更改颜色和删除 操作
    void provideLabelContextMenu(const QPoint &pos);
    // 提供annolist的右键菜单，包含 删除 操作
    void provideAnnoContextMenu(const QPoint &pos);

    // 若getCurrentLabel()存在则返回之，否则弹出labelDialog供用户选取label
    QString _labelRequest();
};

#endif // MAINWINDOW_H
