#include "labellineedit.h"
#include <algorithm>

LabelLineEdit::LabelLineEdit(QWidget *parent):QLineEdit(parent)
{
}

void LabelLineEdit::setLabelListWidget(QListWidget *listWidget){
    labelListWidget = listWidget;
}

void LabelLineEdit::keyPressEvent(QKeyEvent *event){
    if (event->key() == Qt::Key_Up){
        if (labelListWidget->count()>0){
            int newRow = std::max(0, std::min(labelListWidget->currentRow() - 1, labelListWidget->count()-1));
            labelListWidget->setCurrentRow(newRow);
        }
    }else if (event->key() == Qt::Key_Down){
        if (labelListWidget->count()>0){
            int newRow = std::max(0, std::min(labelListWidget->currentRow() + 1, labelListWidget->count()-1));
            labelListWidget->setCurrentRow(newRow);
        }
    }else{
        QLineEdit::keyPressEvent(event);
    }
}
