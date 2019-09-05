#ifndef SEGANNOTATIONITEM_H
#define SEGANNOTATIONITEM_H

#include "annotationitem.h"
#include <QPoint>
#include <QList>
#include <QJsonArray>
#include <QJsonObject>
#include <memory>

struct SegStroke{
    QString type;
    int penWidth;
    QList<QPoint> points;
    SegStroke();
    void fromJsonObject(QJsonObject json);
    QJsonObject toJsonObject();
    void drawSelf(QPainter &p,QColor color, bool fill=true);
};

struct SegStroke3D: public SegStroke{
    int z;
    SegStroke3D();
    void fromJsonObject(QJsonObject json);
    QJsonObject toJsonObject();
};

template<typename stroke_type>
class Basic_SegAnnotationItem : public AnnotationItem
{
public:
    static std::shared_ptr<Basic_SegAnnotationItem> castPointer(std::shared_ptr<AnnotationItem> ptr){
        return std::static_pointer_cast<Basic_SegAnnotationItem>(ptr);
    }
    QList<stroke_type> strokes;
    Basic_SegAnnotationItem() { }
    Basic_SegAnnotationItem(const QList<stroke_type>& strokes, QString label, int id):
        AnnotationItem(label,id),strokes(strokes) {}
    QString toStr() const { return label+" "+QString::number(id); }
    QJsonObject toJsonObject() const{
        QJsonObject json = AnnotationItem::toJsonObject();
        QJsonArray array;
        for (auto stroke: strokes){
            array.append(stroke.toJsonObject());
        }
        json.insert("strokes", array);
        return json;
    }
    void fromJsonObject(const QJsonObject &json){
        AnnotationItem::fromJsonObject(json);
        if (json.contains("strokes")){
            QJsonValue value = json.value("strokes");
            if (value.isArray()){
                strokes.clear();
                QJsonArray array = value.toArray();
                for (int i=0;i<array.size();i++){
                    stroke_type stroke;
                    stroke.fromJsonObject(array[i].toObject());
                    strokes.push_back(stroke);
                }
            }
        }
    }
};

using SegAnnotationItem = Basic_SegAnnotationItem<SegStroke>;
using Seg3DAnnotationItem = Basic_SegAnnotationItem<SegStroke3D>;

class AnnotationContainer;
extern QImage drawColorImage(const QSize &size, const AnnotationContainer *pAnnoContainer, const LabelManager *pLabelManager);
extern QImage drawLabelIdImage(const QSize &size, const AnnotationContainer *pAnnoContainer, const LabelManager *pLabelManager);

extern QImage drawColorImage3d(int zCoordinate, bool *hasContent, const QSize &size, const AnnotationContainer *pAnnoContainer, const LabelManager *pLabelManager);
extern QImage drawLabelIdImage3d(int zCoordinate, bool *hasContent, const QSize &size, const AnnotationContainer *pAnnoContainer, const LabelManager *pLabelManager);


#endif // SEGANNOTATIONITEM_H
