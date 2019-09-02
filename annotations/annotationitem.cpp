#include "annotationitem.h"
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QtDebug>

AnnotationItem::AnnotationItem():label(), id(-1) {}

AnnotationItem::AnnotationItem(QString label, int id):label(label), id(id) {}

AnnotationItem::~AnnotationItem() {}

QJsonObject AnnotationItem::toJsonObject() {
    QJsonObject json;
    json.insert("label", label);
    json.insert("id", id);
    return json;
}

void AnnotationItem::fromJsonObject(const QJsonObject &json){
    if (json.contains("label")){
        QJsonValue value = json.value("label");
        if (value.isString()){
            label = value.toString();
            qDebug()<<"label: "<<label;
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

