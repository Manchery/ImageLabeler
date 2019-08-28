#include "rectannotations.h"
#include <QtDebug>

RectAnnotations::RectAnnotations(QObject *parent) : QObject(parent), curVersion(-1) {
}

void RectAnnotations::push_back(const RectAnnotationItem &item) {
    pushBackOp(RectAnnotationOp{PUSH, 0, item, RectAnnotationItem()});
    items.push_back(item);

    emit AnnotationAdded(item);
    emit dataChanged();
    emitUndoRedoEnable();
}

void RectAnnotations::remove(int idx){
    checkIdx(idx);
    pushBackOp(RectAnnotationOp{REMOVE, idx, items[idx], RectAnnotationItem()});
    items.removeAt(idx);

    emit AnnotationRemoved(idx);
    emit dataChanged();
    emitUndoRedoEnable();
}

void RectAnnotations::modify(int idx, const RectAnnotationItem &item){
    checkIdx(idx);
    pushBackOp(RectAnnotationOp{MODIFY, idx, items[idx], item}); // before and after
    items[idx] = item;
    //! TODO: modify signal
    emit dataChanged();
    emitUndoRedoEnable();
}

//void RectAnnotations::modify(int idx, const QRect &rect, const QString &label){
//    modify(idx, RectAnnotationItem{rect, label});
//}

void RectAnnotations::push_back(const QRect &rect, const QString &label, bool visible){
    push_back(RectAnnotationItem{rect, label, visible});
}

void RectAnnotations::redo(){
    if (curVersion==ops.length()-1){
        qDebug()<<"the last state";
        return;
    }
    RectAnnotationOp op = ops[++curVersion];
    if (op.opClass==PUSH){
        items.push_back(op.item);
        emit AnnotationAdded(op.item);
    }else if (op.opClass==REMOVE){
        items.removeAt(op.idx);
        emit AnnotationRemoved(op.idx);
    }else if (op.opClass==MODIFY){
        //! TODO: modify signal
        items[op.idx] = op.item2;
    }
    emit dataChanged();
    emitUndoRedoEnable();
}

void RectAnnotations::undo(){
    if (curVersion==-1){
        qDebug()<<"the first state";
        return;
    }
    RectAnnotationOp op = ops[curVersion--];
    if (op.opClass==PUSH){
        items.pop_back();
        emit AnnotationRemoved(items.length());
    }else if (op.opClass==REMOVE){
        items.insert(op.idx, op.item);
        emit AnnotationInserted(op.item, op.idx);
        emit labelGiveBack(op.item.label);
    }else if (op.opClass==MODIFY){
        //! TODO: modify signal
        items[op.idx] = op.item2;
    }
    emit dataChanged();
    emitUndoRedoEnable();
}

void RectAnnotations::setVisible(int idx, bool visible){
    items[idx].visible = visible;
    emit dataChanged();
}

int RectAnnotations::length() const{
    return items.length();
}

RectAnnotationItem RectAnnotations::operator [](int idx) const{
    checkIdx(idx);
    return items[idx];
}

bool RectAnnotations::hasData(QString label){
    for (auto item: items)
        if (item.label==label)
            return true;
    return false;
}

void RectAnnotations::allClear(){
    items.clear();
    ops.clear();
    curVersion=-1;
    emit dataChanged();
    emitUndoRedoEnable();
}

void RectAnnotations::checkIdx(int idx) const{
    if (!(idx>=0 && idx<items.length()))
        throw "idx out of range";
}

void RectAnnotations::pushBackOp(RectAnnotationOp op){
    while (curVersion!=ops.length()-1)
        ops.pop();
    ops.push_back(op);
    ++curVersion;
    emitUndoRedoEnable();
}

void RectAnnotations::emitUndoRedoEnable()
{
    emit UndoEnableChanged(curVersion!=-1);
    emit RedoEnableChanged(curVersion!=ops.length()-1);
}
