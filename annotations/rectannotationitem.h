#ifndef RECTANNOTATIONITEM_H
#define RECTANNOTATIONITEM_H

#include "annotationitem.h"
#include <QRect>
#include <QString>
#include <QJsonObject>
#include <QPainter>
#include <QColor>
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

extern void drawRectAnnotation(QPainter &p, const QRect &rect,
                               QColor brushColor, qreal brushAlphaF,
                               QColor penColor, qreal penAlphaF);

extern void drawRectAnnotation(QPainter &p, const QRect &rect, const QBrush &brush, const QPen &pen);


#endif // RECTANNOTATIONITEM_H
