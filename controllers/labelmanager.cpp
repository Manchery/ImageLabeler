#include "labelmanager.h"
#include "annotationitem.h"
#include <QtDebug>

LabelManager::LabelManager(QObject *parent) : QObject(parent)
{
    currentMaxId=0;
}

void LabelManager::checkLabel(QString label) const {
    if (labels.find(label)==labels.end())
        throw "can not find label "+ label +" in label manager";
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
    QList<LabelProperty> items;
    for (int i=0;i<json.size();i++){
        QJsonValue value = json.at(i);
        if (value.isObject()){
            LabelProperty item;
            item.fromJsonObject(value.toObject());
            items.push_back(item);
        }
    }
    for (auto item: items){
        addLabel(item.label, item.color, item.visible, item.id);
    }
}

void LabelManager::fromJsonObject(QJsonObject json)
{
    if (json.contains("labels")){
        QJsonValue value = json.value("labels");
        if (value.isArray())
            fromJsonArray(value.toArray());
        else {
            throw JsonException("value of <labels> is illegal");
        }
    }else{
//        qDebug()<<"no content <labels> in json";
    }
}

void LabelManager::addLabel(QString label, QColor color, bool visible,int id){
    if (id==-1) id=++currentMaxId; else currentMaxId = std::max(id, currentMaxId);
    labels[label] = LabelProperty(label, color, visible, id);
    emit labelAdded(label, color, visible, id);
    emit labelChanged();
}

void LabelManager::removeLabel(QString label){
    labels.remove(label);
    emit labelRemoved(label);
    emit labelChanged();
}

void LabelManager::setColor(QString label, QColor color){
    checkLabel(label);
    if (labels[label].color != color){
        labels[label].color = color;
        emit colorChanged(label, color);
        emit labelChanged();
    }
}

void LabelManager::setVisible(QString label, bool visible){
    checkLabel(label);
    if (labels[label].visible != visible){
        labels[label].visible = visible;
        emit visibelChanged(label, visible);
        emit labelChanged();
    }
}

void LabelManager::allClear()
{
    labels.clear();
    currentMaxId=0;
    emit allCleared();
//    emit configChanged();
}

//---------------------------------LabelProperty-------------------------------------//

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
        }else{
            throw JsonException("value of <label> is illegal");
        }
    }else{
        throw JsonException("no data <label>");
    }

    if (json.contains("color")){
        QJsonValue value = json.value("color");
        if (value.isArray()){
            QJsonArray array = value.toArray();
            if (!array.at(0).isDouble() || !array.at(1).isDouble() || !array.at(2).isDouble()){
                throw JsonException("value of <color> is illegal");
            }
            int r=static_cast<int>(array.at(0).toDouble());
            int g=static_cast<int>(array.at(1).toDouble());
            int b=static_cast<int>(array.at(2).toDouble());
            color = QColor(r,g,b);
        }else{
            throw JsonException("value of <color> is illegal");
        }
    }else{
        throw JsonException("no data <color>");
    }

    if (json.contains("visible")){
        QJsonValue value = json.value("visible");
        if (value.isBool()){
            visible = value.toBool();
        }else{
            throw JsonException("value of <visible> is illegal");
        }
    }else{
        throw JsonException("no data <visible>");
    }

    if (json.contains("id")){
        QJsonValue value = json.value("id");
        if (value.isDouble()){
            id = static_cast<int>(value.toDouble());
        }else{
            throw JsonException("value of <id> is illegal");
        }
    }else{
        throw JsonException("no data <id>");
    }
}
