#include "annotationitem.h"
#include <QtDebug>

JsonException::JsonException(std::string message):message(message) {}

const char *JsonException::what() const noexcept {
    return message.c_str();
}

/*------------------------------------AnnotationItem---------------------------------------*/

AnnotationItem::AnnotationItem():label(), id(-1) {}

AnnotationItem::AnnotationItem(QString label, int id):label(label), id(id) {}

AnnotationItem::~AnnotationItem() {}

QJsonObject AnnotationItem::toJsonObject() const {
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
        }else {
            throw JsonException("value of <label> is illegal");
        }
    }else{
        throw JsonException("no data <label>");
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

