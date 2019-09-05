#ifndef CUBEANNOTATIONITEM_H
#define CUBEANNOTATIONITEM_H

#include "annotationitem.h"
#include <QRect>
#include <QJsonArray>
#include <memory>

struct Point3D {
    int x,y,z;
    Point3D(int x=0,int y=0,int z=0):x(x),y(y),z(z) { }
    void fromJsonArray(const QJsonArray &array);
    QJsonArray toJsonArray() const;
};

class Cuboid{
public:
    Cuboid();
    Cuboid(Point3D topLeft, Point3D bottomRight);
    Point3D topLeft() const { return _topLeft; }
    Point3D bottomRight() const { return _bottomRight; }
    int minX() const { return _topLeft.x; }
    int minY() const { return _topLeft.y; }
    int minZ() const { return _topLeft.z; }
    int maxX() const { return _bottomRight.x; }
    int maxY() const { return _bottomRight.y; }
    int maxZ() const { return _bottomRight.z; }
    void setminX(int x) { _topLeft.x = x; }
    void setminY(int y) { _topLeft.y = y; }
    void setminZ(int z) { _topLeft.z = z; }
    void setmaxX(int x) { _bottomRight.x = x; }
    void setmaxY(int y) { _bottomRight.y = y; }
    void setmaxZ(int z) { _bottomRight.z = z; }
    QRect rectZ() const { return QRect(QPoint(minX(),minY()), QPoint(maxX(), maxY())); }
    QRect rectX() const { return QRect(QPoint(minZ(),minY()), QPoint(maxZ(), maxY())); }
    QRect rectY() const { return QRect(QPoint(minX(),minZ()), QPoint(maxX(), maxZ())); }
    Cuboid normalized() const;
    bool contains(Point3D pos, int margin=0) const;
    Point3D center() const {
        return Point3D((minX()+maxX())/2, (minY()+maxY())/2, (minZ()+maxZ())/2);
    }

    void setTopLeft(Point3D pos) { _topLeft = pos; }
    void setTopLeft(int x,int y,int z) { setTopLeft(Point3D(x,y,z)); }
    void setBottomRight(Point3D pos) { _bottomRight = pos; }
    void setBottomRight(int x,int y,int z) { setBottomRight(Point3D(x,y,z)); }

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
    QString toStr() const;
    QJsonObject toJsonObject() const;
    void fromJsonObject(const QJsonObject &json);
};

#endif // CUBEANNOTATIONITEM_H
