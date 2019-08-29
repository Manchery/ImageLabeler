#include "rectannotations.h"
#include <QtDebug>

QString RectAnnotationItem::toStr(){
    QString topLeftStr = "("+QString::number(rect.topLeft().x())+","+
            QString::number(rect.topLeft().y())+")";
    QString bottomRightStr = "("+QString::number(rect.bottomRight().x())+","+
            QString::number(rect.bottomRight().y())+")";
    return label+" ("+topLeftStr+","+bottomRightStr+")";
}

QJsonObject RectAnnotationItem::toJsonObject(){
    QJsonArray points, point1, point2;
    point1.append(rect.topLeft().x());
    point1.append(rect.topLeft().y());
    point2.append(rect.bottomRight().x());
    point2.append(rect.bottomRight().y());
    points.append(point1);
    points.append(point2);
    QJsonObject json;
    json.insert("points", points);
    json.insert("label", label);
    json.insert("visible", visible);
    return json;
}

void RectAnnotationItem::fromJsonObject(const QJsonObject &json){
    if (json.contains("points")){
        QJsonValue value = json.value("points");
        if (value.isArray()){
            QJsonArray array = value.toArray();
            QJsonValue point1 = array.at(0);
            if (point1.isArray()){
                QJsonArray point1Array = point1.toArray();
                int x=point1Array.at(0).isDouble()?static_cast<int>(point1Array.at(0).toDouble()):0;
                int y=point1Array.at(1).isDouble()?static_cast<int>(point1Array.at(1).toDouble()):0;
                qDebug()<<"point1: "<<x<<" "<<y;
                rect.setTopLeft(QPoint(x,y));
            }
            QJsonValue point2 = array.at(1);
            if (point2.isArray()){
                QJsonArray point2Array = point2.toArray();
                int x=point2Array.at(0).isDouble()?static_cast<int>(point2Array.at(0).toDouble()):0;
                int y=point2Array.at(1).isDouble()?static_cast<int>(point2Array.at(1).toDouble()):0;
                qDebug()<<"point2: "<<x<<" "<<y;
                rect.setBottomRight(QPoint(x,y));
            }
        }
    }
    if (json.contains("label")){
        QJsonValue value = json.value("label");
        if (value.isString()){
            label = value.toString();
            qDebug()<<"label: "<<label;
        }
    }
    if (json.contains("visible")){
        QJsonValue value = json.value("visible");
        if (value.isBool()){
            visible = value.toBool();
            qDebug()<<"visible: "<<visible;
        }
    }
}

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

QJsonArray RectAnnotations::toJsonArray(){
    QJsonArray json;
    for (auto item:items){
        json.append(item.toJsonObject());
    }
    return json;
}

void RectAnnotations::fromJsonObject(QJsonObject json)
{
    if (json.contains("annotations")){
        QJsonValue value = json.value("annotations");
        if (value.isArray())
            fromJsonArray(value.toArray());
        else {
            throw "content <annotations> in json is not array";
        }
    }else{
        qDebug()<<"no content <annotations> in json";
    }
}

void RectAnnotations::fromJsonArray(QJsonArray json)
{
    for (int i=0;i<json.size();i++){
        QJsonValue value = json.at(i);
        if (value.isObject()){
            RectAnnotationItem item;
            item.fromJsonObject(value.toObject());
            this->push_back(item);
        }
    }
}

void RectAnnotations::allClear(){
    items.clear();
    ops.clear();
    curVersion=-1;
//    emit dataChanged();
    emitUndoRedoEnable();
    emit allCleared();
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


