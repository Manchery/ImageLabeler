#include "cubeannotationitem.h"

void Point3D::fromJsonArray(const QJsonArray &array){
    x=array.at(0).isDouble()?static_cast<int>(array.at(0).toDouble()):0;
    y=array.at(1).isDouble()?static_cast<int>(array.at(1).toDouble()):0;
    z=array.at(2).isDouble()?static_cast<int>(array.at(2).toDouble()):0;
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

bool Cuboid::contains(Point3D pos) const {
    return minX()<=pos.x && pos.x<=maxX() &&
            minY()<=pos.y && pos.y<=maxY() &&
            minZ()<=pos.z && pos.z<=maxZ();
}

void Cuboid::fromJsonArray(const QJsonArray &array)
{
    if (array.size()!=2) throw "abnormal array length for cuboid";
    if (array.at(0).isArray()){
        _topLeft.fromJsonArray(array.at(0).toArray());
    }else
        throw "array[0] is not array for cuboid";
    if (array.at(1).isArray()){
        _bottomRight.fromJsonArray(array.at(1).toArray());
    }else
        throw "array[1] is not array for cuboid";
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

QString CubeAnnotationItem::toStr()
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
            throw "points is not array for cubeAnnotationItem";
        }
    }
}





