#ifndef SEGANNOTATIONITEM_H
#define SEGANNOTATIONITEM_H

#include "annotationitem.h"
#include <QPoint>
#include <QList>
#include <QJsonArray>
#include <QJsonObject>
#include <QtDebug>
#include <QColor>
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include <QPainter>

struct SegStroke{
    QString type;
    int penWidth;
    QList<QPoint> points;
    SegStroke();
    void fromJsonObject(QJsonObject json);
    QJsonObject toJsonObject();
    void drawSelf(QPainter &p,QColor color, bool fill=true);
};

class SegAnnotationItem : public AnnotationItem
{
public:
    static std::shared_ptr<SegAnnotationItem> castPointer(std::shared_ptr<AnnotationItem> ptr);
    QList<SegStroke> strokes;
    SegAnnotationItem();
    SegAnnotationItem(const QList<SegStroke>& strokes, QString label, int id);
    QString toStr();
    QJsonObject toJsonObject();
    void fromJsonObject(const QJsonObject &json);
};

class AnnotationContainer;
extern QImage drawColorImage(const QSize &size, const AnnotationContainer *pAnnoContainer, LabelManager *pLabelManager);
extern QImage drawLabelIdImage(const QSize &size, const AnnotationContainer *pAnnoContainer, LabelManager *pLabelManager);


#endif // SEGANNOTATIONITEM_H
