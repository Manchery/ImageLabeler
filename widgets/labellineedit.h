#ifndef LABELLINEEDIT_H
#define LABELLINEEDIT_H

#include <QLineEdit>
#include <QListWidget>

class LabelDialog;

/* LabelLineEdit
 * 简介：用作LabelDialog的一个部件，可绑定一个QListWidget实现一些联动的效果
 * 功能：a. 输入时根据 labelListWidget 的 items 的 text 提供自动补全
 *      b. 当 labelListWidget 的选中项改变时，相应的改变输入框中的text
 *      c. 按下 up和down 键，可改变 labelListWidget 的选中项。
 */

class LabelLineEdit: public QLineEdit{
    friend LabelDialog;

public:
    // labelListWidget 构造时默认为 nullptr
    explicit LabelLineEdit(QWidget *parent = nullptr);

    // 设置 labelListWidget，并实现功能 a 和 b (自动补全 和 相应list的选中)
    void setLabelListWidget(QListWidget* listWidget);

protected:
    // 处理 up 和 down 键的按下事件，实现功能 c (改变list的选中项)
    void keyPressEvent(QKeyEvent *event);

private:
    QListWidget* labelListWidget;
};


#endif // LABELLINEEDIT_H
