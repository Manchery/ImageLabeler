#ifndef RECTANNOTATIONITEM_H
#define RECTANNOTATIONITEM_H

#include "annotationitem.h"
#include <QRect>
#include <QString>
#include <QJsonObject>
#include <memory>

class RectAnnotationItem: public AnnotationItem {
public:
    static std::shared_ptr<RectAnnotationItem> castPointer(std::shared_ptr<AnnotationItem> ptr);
    QRect rect;
    RectAnnotationItem();
    RectAnnotationItem(QRect rect, QString label, int id);
    QString toStr();
    QJsonObject toJsonObject() const;
    void fromJsonObject(const QJsonObject &json);
};

#endif // RECTANNOTATIONITEM_H
