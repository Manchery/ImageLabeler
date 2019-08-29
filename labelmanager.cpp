#include "labelmanager.h"
#include <QtDebug>

LabelManager::LabelManager(QObject *parent) : QObject(parent)
{
    currentId=-1;
}

LabelProperty LabelManager::operator[](QString label) const {
    return labels[label];
}

bool LabelManager::hasLabel(QString label) const {
    return labels.find(label)!=labels.end();
}

void LabelManager::checkLabel(QString label) const {
    if (labels.find(label)==labels.end())
        throw "can not find label in labelconfig";
}

QList<LabelProperty> LabelManager::getLabels() const {
    return labels.values();
}

QColor LabelManager::getColor(QString label) const {
    checkLabel(label);
    return labels[label].color;
}

QJsonArray LabelManager::toJsonArray(){
    QJsonArray json;
    for (auto label:labels){
        json.append(label.toJsonObject());
    }
    return json;
}

void LabelManager::fromJsonArray(QJsonArray json)
{
    for (int i=0;i<json.size();i++){
        QJsonValue value = json.at(i);
        if (value.isObject()){
            LabelProperty item;
            item.fromJsonObject(value.toObject());
            addLabel(item.label, item.color, item.visible);
        }
    }
}

void LabelManager::fromJsonObject(QJsonObject json)
{
    if (json.contains("labels")){
        QJsonValue value = json.value("labels");
        if (value.isArray())
            fromJsonArray(value.toArray());
        else {
            throw "content <labels> in json is not array";
        }
    }else{
        qDebug()<<"no content <labels> in json";
    }
}

void LabelManager::addLabel(QString label, QColor color, bool visible){
    labels[label] = LabelProperty(label, color, visible, newLabelId());
    emit labelAdded(label, color, visible, currentId);
    emit configChanged();
}

void LabelManager::removeLabel(QString label){
    labels.remove(label);
    emit labelRemoved(label);
    emit configChanged();
}

void LabelManager::setColor(QString label, QColor color){
    checkLabel(label);
    if (labels[label].color != color){
        labels[label].color = color;
        emit colorChanged(label, color);
        emit configChanged();
    }
}

void LabelManager::setVisible(QString label, bool visible){
    checkLabel(label);
    if (labels[label].visible != visible){
        labels[label].visible = visible;
        emit visibelChanged(label, visible);
        emit configChanged();
    }
}

void LabelManager::allClear()
{
    labels.clear();
    currentId=-1;
    emit allCleared();
//    emit configChanged();
}

LabelProperty::LabelProperty(QString label, QColor color, bool visible, int id) :
    label(label), color(color), visible(visible),id(id) { }

LabelProperty::LabelProperty():label(), color(), visible(true), id(-1) {
}

QJsonObject LabelProperty::toJsonObject(){
    QJsonArray colorJson;
    colorJson.append(color.red());
    colorJson.append(color.green());
    colorJson.append(color.blue());
    QJsonObject json;
    json.insert("label", label);
    json.insert("id", id);
    json.insert("color", colorJson);
    json.insert("visible", visible);
    return json;
}

void LabelProperty::fromJsonObject(QJsonObject json)
{
    if (json.contains("label")){
        QJsonValue value = json.value("label");
        if (value.isString()){
            label = value.toString();
            qDebug()<<"label: "<<label;
        }
    }
    if (json.contains("color")){
        QJsonValue value = json.value("color");
        if (value.isArray()){
            QJsonArray array = value.toArray();
            int r=array.at(0).isDouble()?static_cast<int>(array.at(0).toDouble()):0;
            int g=array.at(1).isDouble()?static_cast<int>(array.at(1).toDouble()):0;
            int b=array.at(2).isDouble()?static_cast<int>(array.at(2).toDouble()):0;
            color = QColor(r,g,b);
            qDebug()<<"color: "<<r<<" "<<g<<" "<<b;
        }
    }
    if (json.contains("visible")){
        QJsonValue value = json.value("visible");
        if (value.isBool()){
            visible = value.toBool();
            qDebug()<<"visible: "<<visible;
        }
    }
    if (json.contains("id")){
        QJsonValue value = json.value("id");
        if (value.isDouble()){
            id = static_cast<int>(value.toDouble());
            qDebug()<<"id: "<<id;
        }
    }
}
