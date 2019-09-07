#include "labellineedit.h"
#include <QListWidgetItem>
#include <QKeyEvent>
#include <QtDebug>
#include <algorithm>
#include <QCompleter>

LabelLineEdit::LabelLineEdit(QWidget *parent):QLineEdit(parent), labelListWidget(nullptr)
{
}

void LabelLineEdit::setLabelListWidget(QListWidget *listWidget){
    labelListWidget = listWidget;

    // 功能 a: 输入时根据 labelListWidget 的 items 的 text 提供自动补全
    QCompleter* completer = new QCompleter(this);
    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setModel(labelListWidget->model());
    this->setCompleter(completer);

    // 功能 b: 当 labelListWidget 的选中项改变时，相应的改变输入框中的text
    connect(labelListWidget, &QListWidget::currentItemChanged, [this](QListWidgetItem *currentItem){
       this->setText(currentItem->text());
    });
}

void LabelLineEdit::keyPressEvent(QKeyEvent *event){
    // 功能 c: 按下 up和down 键，可改变 labelListWidget 的选中项
    if (event->key() == Qt::Key_Up){
        if (labelListWidget!=nullptr && labelListWidget->count()>0){
            int newRow = std::max(0, std::min(labelListWidget->currentRow() - 1, labelListWidget->count()-1));
            labelListWidget->setCurrentRow(newRow);
            this->setText(labelListWidget->currentItem()->text());
        }
    }else if (event->key() == Qt::Key_Down){
        if (labelListWidget!=nullptr && labelListWidget->count()>0){
            int newRow = std::max(0, std::min(labelListWidget->currentRow() + 1, labelListWidget->count()-1));
            labelListWidget->setCurrentRow(newRow);
            this->setText(labelListWidget->currentItem()->text());
        }
    }else{
        QLineEdit::keyPressEvent(event);
    }
}
