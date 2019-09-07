#ifndef CUSTOMLISTWIDGET_H
#define CUSTOMLISTWIDGET_H

#include <QListWidget>
#include <QString>
#include <QColor>

/* CustomListWidget
 * 简介：作为MainWindow中的部件，根据所需提供了更方便的函数接口以及事件响应
 *      是MainWindow中 annoListWidget、labelListWidget、fileListWidget的类型
 * 界面：每个 item 从左至右组成为：一个用于check的小框（可有可无），一个icon（可有可无），以及text
 * 事件：鼠标若点击到listwidget空白区域，即没有item的区域，则清除list的所有选中状态
 *      键盘按下 up/down 键改变list的选中项
 * 注意：a. 该类应为单选模式（SingleSelection），且不应该被更改;
 *      b. 该类不应出现不同的item的text相同的情况,
 *          这种情况在该项目中是不会发生的，因为：
 *          1. annoList中每个label相同的标注被一个各不相同的instance id标识
 *          2. fileList中不会出现文件重名的情况
 *          3. labelList在加入item时会根据labelManager检查label是否已经存在
 */

class CustomListWidget : public QListWidget
{
public:
    // 构造时设置为为单选模式（SingleSelection）
    CustomListWidget(QWidget *parent=nullptr);

protected:
    // 鼠标若点击到listwidget空白区域，即没有item的区域，则清除list的所有选中状态
    void mousePressEvent(QMouseEvent *event);
    // 键盘按下 up/down 键改变list的选中项
    void keyPressEvent(QKeyEvent *event);

public slots:
    // 往该listwidget的末尾添加一个checkable的有icon的item
    // text 设为参数 $label$，icon设为纯色的 $color$，默认check的状态设为参数 $checked$
    void addCustomItem(QString label, QColor color, bool checked);

    // 往该listwidget的 $idx$ 位置添加一个checkable的有icon的item
    // text 设为参数 $label$，icon设为纯色的 $color$，默认check的状态设为参数 $checked$
    void insertCustomItem(QString label, QColor color, bool checked, int idx);

    // 往该listwidget的末尾添加一个uncheckable的有icon的item
    // text 设为参数 $label$，icon设为纯色的 $color$
    void addCustomItemUncheckable(QString label, QColor color);

    // 往该listwidget的 $idx$ 位置添加一个uncheckable的有icon的item
    // text 设为参数 $label$，icon设为纯色的 $color$
    void insertCustomItemUncheckable(QString label, QColor color, int idx);

    // 删除对应text为 $label$ 的item
    void removeCustomItem(QString label);
    // 更改对应text为 $label$ 的item的icon的颜色为 $color$
    void changeIconColor(QString label,QColor color);
    // 更改对应text为 $label$ 的item的check状态为 $checked$
    void changeCheckState(QString label, bool checked);

    // 删除对应idx为 $idx$ 的item
    void removeCustomItemByIdx(int idx);
    // 更改对应idx为 $idx$ 的item的icon的颜色为 $color$
    void changeIconColorByIdx(int idx,QColor color);
    // 更改对应idx为 $idx$ 的item的text为 $text$
    void changeTextByIdx(int idx, QString text);

private:
    QListWidgetItem* _findItemByText(QString label);
};

#endif // CUSTOMLISTWIDGET_H
