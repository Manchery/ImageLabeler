#ifndef CUBEANNOTATIONITEM_H
#define CUBEANNOTATIONITEM_H

#include "annotationitem.h"
#include <QRect>
#include <algorithm>
#include <QJsonArray>

struct Point3D {
    int x,y,z;
    Point3D(int x=0,int y=0,int z=0):x(x),y(y),z(z) { }
    void fromJsonArray(const QJsonArray &array);
    QJsonArray toJsonArray() const;
};

class Cuboid{
public:
    Cuboid() { }
    Cuboid(Point3D topLeft, Point3D bottomRight):_topLeft(topLeft), _bottomRight(bottomRight) {}
    Point3D topLeft() const { return _topLeft; }
    Point3D bottomRight() const { return _bottomRight; }
    int minX() const { return _topLeft.x; }
    int minY() const { return _topLeft.y; }
    int minZ() const { return _topLeft.z; }
    int maxX() const { return _bottomRight.x; }
    int maxY() const { return _bottomRight.y; }
    int maxZ() const { return _bottomRight.z; }
    Cuboid normalized() const {
        Point3D newTopLeft(_topLeft), newBottomRight(_bottomRight);
        if (newTopLeft.x > newBottomRight.x) std::swap(newTopLeft.x, newBottomRight.x);
        if (newTopLeft.y > newBottomRight.y) std::swap(newTopLeft.y, newBottomRight.y);
        if (newTopLeft.z > newBottomRight.z) std::swap(newTopLeft.z, newBottomRight.z);
        return Cuboid(newTopLeft, newBottomRight);
    }
    void fromJsonArray(const QJsonArray &array);
    QJsonArray toJsonArray() const;
private:
    Point3D _topLeft,_bottomRight;
};

class CubeAnnotationItem : public AnnotationItem
{
public:
    static std::shared_ptr<CubeAnnotationItem> castPointer(std::shared_ptr<AnnotationItem> ptr);
    Cuboid cube;
    CubeAnnotationItem();
    CubeAnnotationItem(Cuboid cube, QString label, int id);
    QString toStr();
    QJsonObject toJsonObject() const;
    void fromJsonObject(const QJsonObject &json);
};

#endif // CUBEANNOTATIONITEM_H
