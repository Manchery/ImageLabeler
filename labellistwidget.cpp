#include "labellistwidget.h"


LabelListWidget::LabelListWidget(QWidget *parent):QListWidget(parent){}

void LabelListWidget::mousePressEvent(QMouseEvent *event){
    QModelIndex idx = indexAt(event->pos());
    if (!idx.isValid()){
        clearSelection();
    } else {
        QListWidget::mousePressEvent(event);
    }
}

void LabelListWidget::addCustomItem(QString label, QColor color, bool visible){
    QPixmap pixmap(16,16);
    if (color.isValid()){
        pixmap.fill(color);
    }else{
        throw "invalid color when add custom colored item.";
    }
    QListWidgetItem *item = new QListWidgetItem(QIcon(pixmap),label);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(visible?Qt::Checked:Qt::Unchecked);
    //! use QListWidget::itemChanged() and check there if your item is checked or unchecked
    this->addItem(item);
}

void LabelListWidget::insertCustomItem(QString label, QColor color, bool visible, int idx)
{
    QPixmap pixmap(16,16);
    if (color.isValid()){
        pixmap.fill(color);
    }else{
        throw "invalid color when add custom colored item.";
    }
    QListWidgetItem *item = new QListWidgetItem(QIcon(pixmap),label);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(visible?Qt::Checked:Qt::Unchecked);
    //! use QListWidget::itemChanged() and check there if your item is checked or unchecked
    this->insertItem(idx, item);
}

void LabelListWidget::addCustomItemUncheckable(QString label, QColor color){
    QPixmap pixmap(16,16);
    if (color.isValid()){
        pixmap.fill(color);
    }else{
        throw "invalid color when add custom colored item.";
    }
    QListWidgetItem *item = new QListWidgetItem(QIcon(pixmap),label);

    this->addItem(item);
}

void LabelListWidget::insertCustomItemUncheckable(QString label, QColor color, int idx)
{
    QPixmap pixmap(16,16);
    if (color.isValid()){
        pixmap.fill(color);
    }else{
        throw "invalid color when add custom colored item.";
    }
    QListWidgetItem *item = new QListWidgetItem(QIcon(pixmap),label);
    this->insertItem(idx, item);
}

void LabelListWidget::removeCustomItem(QString label){
    auto item = _findItemByText(label);
    delete this->takeItem(this->row(item));
    clearSelection(); //! warning: only because this fixes a confuse bug
}

void LabelListWidget::removeCustomItemByIdx(int idx)
{
    delete this->takeItem(idx);
    clearSelection(); //! warning: only because this fixes a confuse bug
}

void LabelListWidget::changeCheckState(QString label, bool visible){
    auto item = _findItemByText(label);
    item->setCheckState(visible?Qt::Checked:Qt::Unchecked);
}

void LabelListWidget::changeIconColor(QString label, QColor color){
    auto item = _findItemByText(label);
    QPixmap pixmap(16,16); pixmap.fill(color); item->setIcon(pixmap);
}

void LabelListWidget::changeIconColorByIdx(int idx, QColor color)
{
    auto item = this->item(idx);
    QPixmap pixmap(16,16); pixmap.fill(color); item->setIcon(pixmap);
}

void LabelListWidget::changeTextByIdx(int idx, QString text)
{
    auto item = this->item(idx);
    item->setText(text);
}

QListWidgetItem *LabelListWidget::_findItemByText(QString label){
    auto items = this->findItems(label, Qt::MatchExactly);
    if (items.length()!=1)
        throw "abnormal counts of label in the list";
    return items[0];
}
