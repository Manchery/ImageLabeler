#include "segannotationitem.h"
#include "annotationcontainer.h"
#include <QColor>
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QtDebug>

SegStroke::SegStroke(): type(),penWidth(-1), points() { }

void SegStroke::fromJsonObject(QJsonObject json){
    if (json.contains("type")){
        QJsonValue value = json.value("type");
        if (value.isString()){
            type = value.toString();
            qDebug()<<"type: "<<type;
        }
    }
    if (json.contains("pen_width")){
        QJsonValue value = json.value("pen_width");
        if (value.isDouble()){
            penWidth = static_cast<int>(value.toDouble());
            qDebug()<<"pen_width: "<<penWidth;
        }
    }
    if (json.contains("points")){
        QJsonValue value = json.value("points");
        if (value.isArray()){
            QJsonArray pointsArray = value.toArray();
            points.clear();
            for (int i=0;i<pointsArray.size();i++){
                QJsonArray point = pointsArray.at(i).toArray();
                int x=point.at(0).isDouble()?static_cast<int>(point.at(0).toDouble()):0;
                int y=point.at(1).isDouble()?static_cast<int>(point.at(1).toDouble()):0;
                points << QPoint(x,y);
                qDebug()<<"point: "<<x<<" "<<y;
            }
        }
    }
}

QJsonObject SegStroke::toJsonObject(){
    QJsonObject json;
    json.insert("type", type);
    QJsonArray array;
    for (auto point:points){
        QJsonArray pointArray;
        pointArray.append(point.x());
        pointArray.append(point.y());
        array.append(pointArray);
    }
    json.insert("points", array);
    if (penWidth!=-1)
        json.insert("pen_width", penWidth);
    return json;
}

void SegStroke::drawSelf(QPainter &p, QColor color, bool fill)
{
    QPainterPath path;
    path.moveTo(points[0]);
    for (int i=1;i<points.length();i++)
        path.lineTo(points[i]);
    if (fill) path.setFillRule(Qt::WindingFill);
    p.save();
    if (type=="contour"){
        p.setPen(QPen(color));
        if (fill)
            p.setBrush(QBrush(color));
    }else if (type == "square_pen"){
        p.setPen(QPen(color, penWidth, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
    }else if (type == "circle_pen"){
        p.setPen(QPen(color, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    }
    p.drawPath(path);
    p.restore();
}

SegStroke3D::SegStroke3D(): SegStroke (), z(-1) { }

void SegStroke3D::fromJsonObject(QJsonObject json)
{
    SegStroke::fromJsonObject(json);
    if (json.contains("z_coordinate")){
        QJsonValue value = json.value("z_coordinate");
        if (value.isDouble()){
            z = static_cast<int>(value.toDouble());
            qDebug()<<"z_coordinate: "<<z;
        }
    }
}

QJsonObject SegStroke3D::toJsonObject()
{
    QJsonObject json = SegStroke::toJsonObject();
    json.insert("z_coordinate", z);
    return json;
}

QImage drawColorImage(const QSize &size, const AnnotationContainer *pAnnoContainer, LabelManager *pLabelManager)
{
    QImage image(size, QImage::Format_RGB32);
    image.fill(QColor(0,0,0));
    QPainter p(&image);
    for (int i=0;i<pAnnoContainer->length();i++){
        auto item = SegAnnotationItem::castPointer((*pAnnoContainer)[i]);
        QString label = item->label;
        QColor color = (*pLabelManager)[label].color;
        for (auto stroke: item->strokes)
            stroke.drawSelf(p,color);
    }
    p.end();
    return image;
}

QImage drawLabelIdImage(const QSize &size, const AnnotationContainer *pAnnoContainer, LabelManager *pLabelManager)
{
    QImage image(size, QImage::Format_Grayscale8);
    image.fill(QColor(0,0,0));
    QPainter p(&image);
    for (int i=0;i<pAnnoContainer->length();i++){
        auto item = SegAnnotationItem::castPointer((*pAnnoContainer)[i]);
        QString label = item->label;
        int labelId = (*pLabelManager)[label].id;
        QColor color = QColor(labelId, labelId, labelId);
        for (auto stroke: item->strokes)
            stroke.drawSelf(p,color);
    }
    p.end();
    return image;
}

