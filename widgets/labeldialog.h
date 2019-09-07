#ifndef LABELDIALOG_H
#define LABELDIALOG_H

#include "labellineedit.h"
#include "labelmanager.h"
#include <QDialog>

namespace Ui {
class LabelDialog;
}

/* LabelDialog
 * 简介：供玩家选择label的一个对话框
 * 界面：ui->lineEdit，LabelLineEdit 类型，绑定了 ui->listWidget（详见 LabelLineEdit 的声明）
 *      ui->listWidget，用于显示label，且有一个纯色的icon代表label对应的颜色
 */

class LabelDialog : public QDialog
{
    Q_OBJECT

public:
    // 该类从labelManager中获取所有 label 并显示在 listWidget 上
    explicit LabelDialog(const LabelManager& labelManager, QWidget *parent = nullptr);
    ~LabelDialog();

    // 获取lineEdit中输入的label
    QString getLabel() const;

private:
    Ui::LabelDialog *ui;
};

#endif // LABELDIALOG_H
