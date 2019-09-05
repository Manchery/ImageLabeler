#include "cubeannotationitem.h"
#include <algorithm>

void Point3D::fromJsonArray(const QJsonArray &array){
    if (!array.at(0).isDouble() || !array.at(1).isDouble() || !array.at(2).isDouble()){
        throw JsonException("value of point is illegal");
    }
    x=static_cast<int>(array.at(0).toDouble());
    y=static_cast<int>(array.at(1).toDouble());
    z=static_cast<int>(array.at(2).toDouble());
}

QJsonArray Point3D::toJsonArray() const {
    QJsonArray array;
    array.append(x);
    array.append(y);
    array.append(z);
    return array;
}

//-------------------------------------Point 3D END--------------------------------//


Cuboid::Cuboid() { }

Cuboid::Cuboid(Point3D topLeft, Point3D bottomRight):_topLeft(topLeft), _bottomRight(bottomRight) {}

Cuboid Cuboid::normalized() const {
    Point3D newTopLeft(_topLeft), newBottomRight(_bottomRight);
    if (newTopLeft.x > newBottomRight.x) std::swap(newTopLeft.x, newBottomRight.x);
    if (newTopLeft.y > newBottomRight.y) std::swap(newTopLeft.y, newBottomRight.y);
    if (newTopLeft.z > newBottomRight.z) std::swap(newTopLeft.z, newBottomRight.z);
    return Cuboid(newTopLeft, newBottomRight);
}

bool Cuboid::contains(Point3D pos, int margin) const {
    return minX()-margin<=pos.x && pos.x<=maxX()+margin &&
            minY()-margin<=pos.y && pos.y<=maxY()+margin &&
            minZ()-margin<=pos.z && pos.z<=maxZ()+margin;
}

void Cuboid::fromJsonArray(const QJsonArray &array)
{
    if (array.size()!=2) throw JsonException("abnormal array length of <points> for cuboid");
    if (array.at(0).isArray()){
        _topLeft.fromJsonArray(array.at(0).toArray());
    }else
        throw JsonException("points[0] is not array for cuboid");
    if (array.at(1).isArray()){
        _bottomRight.fromJsonArray(array.at(1).toArray());
    }else
        throw JsonException("points[1] is not array for cuboid");
}

QJsonArray Cuboid::toJsonArray() const
{
    QJsonArray json;
    json.append(_topLeft.toJsonArray());
    json.append(_bottomRight.toJsonArray());
    return json;
}

//-------------------------------------Cuboid END--------------------------------//

std::shared_ptr<CubeAnnotationItem> CubeAnnotationItem::castPointer(std::shared_ptr<AnnotationItem> ptr)
{
    return std::static_pointer_cast<CubeAnnotationItem>(ptr);
}

CubeAnnotationItem::CubeAnnotationItem():AnnotationItem (), cube() { }

CubeAnnotationItem::CubeAnnotationItem(Cuboid cube, QString label, int id): AnnotationItem (label, id), cube(cube) { }

QString CubeAnnotationItem::toStr() const
{
    QString topLeftStr = "("+QString::number(cube.topLeft().x)+","+
            QString::number(cube.topLeft().y)+","+
            QString::number(cube.topLeft().z)+")";
    QString bottomRightStr = "("+QString::number(cube.bottomRight().x)+","+
            QString::number(cube.bottomRight().y)+","+
            QString::number(cube.bottomRight().z)+")";
    return label+" "+QString::number(id)+" ("+topLeftStr+","+bottomRightStr+")";
}

QJsonObject CubeAnnotationItem::toJsonObject() const
{
    QJsonObject json = AnnotationItem::toJsonObject();
    json.insert("points", cube.toJsonArray());
    return json;
}

void CubeAnnotationItem::fromJsonObject(const QJsonObject &json)
{
    AnnotationItem::fromJsonObject(json);
    if (json.contains("points")){
        QJsonValue value = json.value("points");
        if (value.isArray()){
            cube.fromJsonArray(value.toArray());
        }else{
            throw JsonException("<points> is not array for cubeAnnotationItem");
        }
    }else{
        throw JsonException("no data <points>");
    }
}





