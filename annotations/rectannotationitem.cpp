#include "rectannotationitem.h"
#include <QtDebug>
#include <QJsonArray>

std::shared_ptr<RectAnnotationItem> RectAnnotationItem::castPointer(std::shared_ptr<AnnotationItem> ptr){
    return std::static_pointer_cast<RectAnnotationItem>(ptr);
}

RectAnnotationItem::RectAnnotationItem():AnnotationItem(), rect()  { }

RectAnnotationItem::RectAnnotationItem(QRect rect, QString label, int id):AnnotationItem (label, id), rect(rect) { }

QString RectAnnotationItem::toStr() const{
    QString topLeftStr = "("+QString::number(rect.topLeft().x())+","+
            QString::number(rect.topLeft().y())+")";
    QString bottomRightStr = "("+QString::number(rect.bottomRight().x())+","+
            QString::number(rect.bottomRight().y())+")";
    return label+" "+QString::number(id)+" ("+topLeftStr+","+bottomRightStr+")";
}

QJsonObject RectAnnotationItem::toJsonObject() const{
    QJsonObject json = AnnotationItem::toJsonObject();
    QJsonArray points, point1, point2;
    point1.append(rect.topLeft().x());
    point1.append(rect.topLeft().y());
    point2.append(rect.bottomRight().x());
    point2.append(rect.bottomRight().y());
    points.append(point1);
    points.append(point2);
    json.insert("points", points);
    return json;
}

void RectAnnotationItem::fromJsonObject(const QJsonObject &json){
    AnnotationItem::fromJsonObject(json);
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

}

void drawRectAnnotation(QPainter &p, const QRect &rect,
                        QColor brushColor, qreal brushAlphaF,
                        QColor penColor, qreal penAlphaF)
{
    p.save();
    brushColor.setAlphaF(brushAlphaF); QBrush brush(brushColor); p.setBrush(brush);
    penColor.setAlphaF(penAlphaF); QPen pen(penColor); p.setPen(pen);
    p.drawRect(rect);
    p.restore();
}

void drawRectAnnotation(QPainter &p, const QRect &rect, const QBrush &brush, const QPen &pen)
{
    p.save();
    p.setBrush(brush); p.setPen(pen);
    p.drawRect(rect);
    p.restore();
}
