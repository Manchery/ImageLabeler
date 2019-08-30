#ifndef ANNOTATIONITEM_H
#define ANNOTATIONITEM_H

#include <QString>
#include <QJsonObject>

class AnnotationItem
{
public:
    QString label;
    int id; // instance id
    AnnotationItem();
    AnnotationItem(QString label, int id);
    virtual ~AnnotationItem();
    virtual QString toStr()=0;
    virtual QJsonObject toJsonObject()=0;
    virtual void fromJsonObject(const QJsonObject &json)=0;
};

#endif // ANNOTATIONITEM_H
