#include "customlistwidget.h"
#include "common.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <algorithm>

CustomListWidget::CustomListWidget(QWidget *parent):QListWidget(parent){
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void CustomListWidget::mousePressEvent(QMouseEvent *event){
    QModelIndex idx = indexAt(event->pos());
    if (!idx.isValid()){
        clearSelection();
    } else {
        QListWidget::mousePressEvent(event);
    }
}

void CustomListWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key()==Qt::Key_Up || event->key()==Qt::Key_Down){
        auto items = selectedItems();
        if (items.length()==1){
            int idx = row(items[0]);
            int newIdx = event->key()==Qt::Key_Down ? std::min(idx+1, count()-1) : std::max(idx-1,0);
            item(newIdx)->setSelected(true);
            return;
        }
    }
    QListWidget::keyPressEvent(event);
}

void CustomListWidget::addCustomItem(QString label, QColor color, bool checked){
    QListWidgetItem *item = new QListWidgetItem(iconFromColor(color), label);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(checked?Qt::Checked:Qt::Unchecked);
    //! note: use QListWidget::itemChanged() and check there if your item is checked or unchecked
    this->addItem(item);
}

void CustomListWidget::insertCustomItem(QString label, QColor color, bool checked, int idx)
{
    QListWidgetItem *item = new QListWidgetItem(iconFromColor(color), label);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(checked?Qt::Checked:Qt::Unchecked);
    //! note: use QListWidget::itemChanged() and check there if your item is checked or unchecked
    this->insertItem(idx, item);
}

void CustomListWidget::addCustomItemUncheckable(QString label, QColor color)
{
    QListWidgetItem *item = new QListWidgetItem(iconFromColor(color), label);
    this->addItem(item);
}

void CustomListWidget::insertCustomItemUncheckable(QString label, QColor color, int idx)
{
    QListWidgetItem *item = new QListWidgetItem(iconFromColor(color), label);
    this->insertItem(idx, item);
}

void CustomListWidget::removeCustomItem(QString label){
    auto item = _findItemByText(label);
    delete this->takeItem(this->row(item));
    clearSelection(); //! note: 删除item后的selection情况是较复杂的
}

void CustomListWidget::removeCustomItemByIdx(int idx)
{
    delete this->takeItem(idx);
    clearSelection(); //! note: 删除item后的selection情况是较复杂的
}

void CustomListWidget::changeCheckState(QString label, bool checked){
    auto item = _findItemByText(label);
    item->setCheckState(checked?Qt::Checked:Qt::Unchecked);
}

void CustomListWidget::changeIconColor(QString label, QColor color){
    auto item = _findItemByText(label);
    item->setIcon(iconFromColor(color));
}

void CustomListWidget::changeIconColorByIdx(int idx, QColor color)
{
    auto item = this->item(idx);
    item->setIcon(iconFromColor(color));
}

void CustomListWidget::changeTextByIdx(int idx, QString text)
{
    auto item = this->item(idx);
    item->setText(text);
}

QListWidgetItem *CustomListWidget::_findItemByText(QString label){
    auto items = this->findItems(label, Qt::MatchExactly);
    if (items.length()>0){
        return items[0];
    } else {
        return nullptr;
    }
}
