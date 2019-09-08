#ifndef CUBEANNOTATIONITEM_H
#define CUBEANNOTATIONITEM_H

#include "annotationitem.h"
#include <QRect>
#include <QJsonArray>
#include <memory>

/* Point3D
 * 简介：用于表示 3D 整点坐标的数据类型，可读写json格式的数据
 * Json：该类的数据与json相互转化时的格式如下
 *  [ Double, Double, Double ] // 分别表示xyz
 */
struct Point3D {
    int x,y,z;
    Point3D(int x=0,int y=0,int z=0):x(x),y(y),z(z) { }
    void fromJsonArray(const QJsonArray &array);
    QJsonArray toJsonArray() const;
};

/* Cuboid
 * 简介：用于表示 3D 整点立方体的数据类型，可读写json格式的数据
 *
 * Json：该类的数据与json相互转化时的格式如下
 * [
 *  [ Double, Double, Double ], // 一个顶点的xyz
 *  [ Double, Double, Double ]  // 另一个顶点的xyz
 * ]
 */
class Cuboid{
public:
    Cuboid();
    // 这里二维矩形的记法，对于一个normalized的cuboid，
    // topLeft表示分别由xyz坐标中较小值构成的三维点坐标
    // bottomRight表示分别由xyz坐标中较大值构成的三维点坐标
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
    // 返回该cuboid沿着各坐标轴投影到二维平面下形成的矩形
    QRect rectZ() const { return QRect(QPoint(minX(),minY()), QPoint(maxX(), maxY())); }
    QRect rectX() const { return QRect(QPoint(minZ(),minY()), QPoint(maxZ(), maxY())); }
    QRect rectY() const { return QRect(QPoint(minX(),minZ()), QPoint(maxX(), maxZ())); }
    // 将一个cuboid变为normalized的，即topLeft的三维坐标都分别不大于bottomRight，若不然通过分别交换对应坐标实现normalize
    Cuboid normalized() const;
    // 计算点坐标是否在cuboid的内部及其边界上，
    // 若margin不为0，则将各个面都向外平移margin个像素后再计算是否在平移后的cuboid中，可看做接受点坐标的一定的误差
    bool contains(Point3D pos, int margin=0) const;
    // 返回cuboid的中心坐标
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

/* CubeAnnotationItem
 * 简介：用于3D Detection的标注类型，比父类多记录了一个像素坐标的长方体 cube 表示 3D 的 bounding box
 *
 * Json：该类的数据与json相互转化时的格式如下，points表示两个矩形的左上和右下顶点
 *  {
 *      "label": String
 *      "id": Double
 *      "points": ... // 该key的value的格式为Cuboid对应的格式
 *  }
 */

class AnnotationContainer;
class CubeAnnotationItem : public AnnotationItem
{
    friend AnnotationContainer;
public:
    // 将一个父类型的指针转化为该类型的指针
    static std::shared_ptr<CubeAnnotationItem> castPointer(std::shared_ptr<AnnotationItem> ptr);

    CubeAnnotationItem();
    CubeAnnotationItem(Cuboid cube, QString label, int id);

    Cuboid getCube() const { return cube; }

    // 重载函数的说明详见父类
    QString toStr() const;
    QJsonObject toJsonObject() const;
    void fromJsonObject(const QJsonObject &json);

protected:
    Cuboid cube;
};

#endif // CUBEANNOTATIONITEM_H
