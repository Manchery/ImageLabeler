#include "annotationcontainer.h"
#include "rectannotationitem.h"
#include "segannotationitem.h"
#include "cubeannotationitem.h"
#include <QtDebug>
#include <algorithm>
using std::shared_ptr;

AnnotationContainer::AnnotationContainer(QObject *parent) : QObject(parent), curVersion(-1), selectedIdx(-1) {

}

void AnnotationContainer::push_back(const AnnoItemPtr &item) {
    pushBackOp(AnnotationOp{PUSH, 0, item, nullptr});
    items.push_back(item);

    emit AnnotationAdded(item);
    emit annoChanged();
    emitUndoRedoEnable();
}

void AnnotationContainer::remove(int idx){
    checkIdx(idx);
    pushBackOp(AnnotationOp{REMOVE, idx, items[idx],nullptr});
    items.removeAt(idx);

    emit AnnotationRemoved(idx);
    emit annoChanged();
    emitUndoRedoEnable();
}

void AnnotationContainer::modify(int idx, const AnnoItemPtr &item){
    checkIdx(idx);
    pushBackOp(AnnotationOp{MODIFY, idx, items[idx], item}); // before and after
    items[idx] = item;

    emit AnnotationModified(item, idx);
    emit annoChanged();
    emitUndoRedoEnable();
}

// swap idx & idx+1
void AnnotationContainer::swap(int idx)
{
    checkIdx(idx); checkIdx(idx+1);
    pushBackOp(AnnotationOp{SWAP, idx, nullptr, nullptr});
    std::swap(items[idx], items[idx+1]);
    if (selectedIdx == idx) selectedIdx = idx+1;
    else if (selectedIdx == idx+1) selectedIdx = idx;

    emit AnnotationSwap(idx);
    emit annoChanged();
    emitUndoRedoEnable();
}

void AnnotationContainer::redo(){
    if (curVersion==ops.length()-1){
        qDebug()<<"the last state";
        return;
    }
    AnnotationOp op = ops[++curVersion];
    if (op.opClass==PUSH){
        items.push_back(op.item);
        emit AnnotationAdded(op.item);
    }else if (op.opClass==REMOVE){
        items.removeAt(op.idx);
        emit AnnotationRemoved(op.idx);
    }else if (op.opClass==MODIFY){
        emit AnnotationModified(op.item2, op.idx);
        items[op.idx] = op.item2;
    }else if (op.opClass==SWAP){
        std::swap(items[op.idx], items[op.idx+1]);
        if (selectedIdx == op.idx) selectedIdx = op.idx+1;
        else if (selectedIdx == op.idx+1) selectedIdx = op.idx;
        emit AnnotationSwap(op.idx);
    }
    emit annoChanged();
    emitUndoRedoEnable();
}

void AnnotationContainer::undo(){
    if (curVersion==-1){
        qDebug()<<"the first state";
        return;
    }
    AnnotationOp op = ops[curVersion--];
    if (op.opClass==PUSH){
        items.pop_back();
        emit AnnotationRemoved(items.length());
    }else if (op.opClass==REMOVE){
        items.insert(op.idx, op.item);
        emit AnnotationInserted(op.item, op.idx);
        emit labelGiveBack(op.item->label);
    }else if (op.opClass==MODIFY){
        emit AnnotationModified(op.item, op.idx);
        items[op.idx] = op.item;
    }else if (op.opClass==SWAP){
        std::swap(items[op.idx], items[op.idx+1]);
        if (selectedIdx == op.idx) selectedIdx = op.idx+1;
        else if (selectedIdx == op.idx+1) selectedIdx = op.idx;
        emit AnnotationSwap(op.idx);
    }
    emit annoChanged();
    emitUndoRedoEnable();
}

void AnnotationContainer::setSelected(int idx)
{
    selectedIdx=idx;
//    qDebug()<<"Select "<< idx;
    emit annoChanged();
}

AnnoItemPtr AnnotationContainer::operator [](int idx) const{
    checkIdx(idx);
    return items[idx];
}

bool AnnotationContainer::hasData(QString label){
    for (auto item: items)
        if (item->label==label)
            return true;
    return false;
}

QJsonArray AnnotationContainer::toJsonArray(){
    QJsonArray json;
    for (auto item:items){
        json.append(item->toJsonObject());
    }
    return json;
}

void AnnotationContainer::fromJsonObject(QJsonObject json, TaskMode task)
{
    if (json.contains("annotations")){
        QJsonValue value = json.value("annotations");
        if (value.isArray())
            fromJsonArray(value.toArray(), task);
        else {
            throw "content <annotations> in json is not array";
        }
    }else{
        qDebug()<<"no content <annotations> in json";
    }
}

void AnnotationContainer::fromJsonArray(QJsonArray json, TaskMode task)
{
    for (int i=0;i<json.size();i++){
        QJsonValue value = json.at(i);
        if (value.isObject()){
            if (task == DETECTION){
                shared_ptr<RectAnnotationItem> item = std::make_shared<RectAnnotationItem>();
                item->fromJsonObject(value.toObject());
                this->push_back(std::static_pointer_cast<AnnotationItem>(item));
            }else if (task == SEGMENTATION){
                shared_ptr<SegAnnotationItem> item = std::make_shared<SegAnnotationItem>();
                item->fromJsonObject(value.toObject());
                this->push_back(std::static_pointer_cast<AnnotationItem>(item));
            }else if (task == DETECTION3D){
                shared_ptr<CubeAnnotationItem> item = std::make_shared<CubeAnnotationItem>();
                item->fromJsonObject(value.toObject());
                this->push_back(std::static_pointer_cast<AnnotationItem>(item));
            }else if (task == SEGMENTATION3D){
                shared_ptr<Seg3DAnnotationItem> item = std::make_shared<Seg3DAnnotationItem>();
                item->fromJsonObject(value.toObject());
                this->push_back(std::static_pointer_cast<AnnotationItem>(item));
            }
        }
    }
}

int AnnotationContainer::newInstanceIdForLabel(QString label){
    int maxId=-1;
    for (auto item:items)
        if (item->label==label)
            maxId = std::max(item->id, maxId);
    for (auto op: ops){
        if (op.item->label==label)
            maxId = std::max(op.item->id, maxId);
        if (op.item2!=nullptr && op.item2->label==label)
            maxId = std::max(op.item2->id, maxId);
    }
    if (maxId+1>255) throw "new instance id out of range [0,255]";
    return maxId+1;
}

void AnnotationContainer::allClear(){
    items.clear();
    ops.clear();
    curVersion=-1;
    selectedIdx=-1;
    //    emit dataChanged();
    emitUndoRedoEnable();
    emit allCleared();
}

void AnnotationContainer::checkIdx(int idx) const{
    if (!(idx>=0 && idx<items.length()))
        throw "idx out of range";
}

void AnnotationContainer::pushBackOp(AnnotationOp op){
    while (curVersion!=ops.length()-1)
        ops.pop();
    ops.push_back(op);
    ++curVersion;
    emitUndoRedoEnable();
}

void AnnotationContainer::emitUndoRedoEnable()
{
    emit UndoEnableChanged(curVersion!=-1);
    emit RedoEnableChanged(curVersion!=ops.length()-1);
}


