#ifndef SEGANNOTATIONITEM_H
#define SEGANNOTATIONITEM_H

#include "annotationitem.h"
#include <QPoint>
#include <QList>
#include <QJsonArray>
#include <QJsonObject>
#include <memory>

/* SegStroke
 * 简介：用于表示2D分割标注中一 “笔画” 的数据类型
 * 笔画：笔画共分为三种，分别为圆形画笔、方形画笔、轮廓
 *      对应的type的值分别为 "circle_pen" "square_pen" "contour"
 *      当type为画笔时，points中记录的是画笔中心经过路径，penWidth记录画笔的大小；
 *      当type为轮廓时，points依次记录轮廓上的点，并用多边形连接它们，特别地，当点比较稠密时，可视为光滑闭合曲线
 *      当type为轮廓时，penWidth无意义，一般记为1，也可不记
 *
 * Json：该类的数据与json相互转化时的格式如下，
 *  {
 *      "type": String      // 取值为 "circle_pen" 或 "square_pen" 或 "contour"
 *      "penWidth": Double
 *      "points": [ [Double, Double], [Double, Double], ... ] //Array类型，表示xy坐标
 *  }
 */
struct SegStroke{
    QString type;
    int penWidth;
    QList<QPoint> points;
    SegStroke();
    void fromJsonObject(QJsonObject json);
    QJsonObject toJsonObject();
    // 将自身（一个“笔画”）以颜色color绘制在QPainter上，当类型为轮廓时，fill表示是否填充中间区域
    void drawSelf(QPainter &p,QColor color, bool fill=true);
};

/* SegStroke3D
 * 简介：用于表示3D分割标注中一 “笔画” 的数据类型
 *      继承自SegStroke，多记录了一个 z 来表示该笔画是在z坐标下哪一层画的
 *
 * Json：该类的数据与json相互转化时的格式如下，
 *  {
 *      "type": String      // 取值为 "circle_pen" 或 "square_pen" 或 "contour"
 *      "penWidth": Double
 *      "points": [ [Double, Double], [Double, Double], ... ] //Array类型，表示xy坐标
 *      "z_coordinate": Double
 *  }
 */
struct SegStroke3D: public SegStroke{
    int z;
    SegStroke3D();
    void fromJsonObject(QJsonObject json);
    QJsonObject toJsonObject();
};

/* Basic_SegAnnotationItem
 * 简介：用于表示Segmentation标注的类模板，该项目中认为一个分割标注可由多个笔画（stroke_type类型）合并组成
 *      对应2D和3D分割标注，stroke_type分别为SegStroke和SegStroke3D
 *
 * Json：该类的数据与json相互转化时的格式如下
 *  {
 *      "label": String
 *      "id": Double
 *      "strokes": [ stroke_type, stroke_type, ... ] // Array的元素的格式为SegStroke或SegStroke3D对应的格式
 *  }
 */

class AnnotationContainer;
template<typename stroke_type>
class Basic_SegAnnotationItem : public AnnotationItem
{
    friend AnnotationContainer;
public:
    // 将一个父类型的指针转化为该类型的指针
    static std::shared_ptr<Basic_SegAnnotationItem> castPointer(std::shared_ptr<AnnotationItem> ptr){
        return std::static_pointer_cast<Basic_SegAnnotationItem>(ptr);
    }

    Basic_SegAnnotationItem() { }
    Basic_SegAnnotationItem(const QList<stroke_type>& strokes, QString label, int id):
        AnnotationItem(label,id),strokes(strokes) {}

    const QList<stroke_type>& getStrokes() const { return strokes; }

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

protected:
    QList<stroke_type> strokes;
};

// 用于表示2D Segmentation标注的类型
using SegAnnotationItem = Basic_SegAnnotationItem<SegStroke>;
// 用于表示3D Segmentation标注的类型
using Seg3DAnnotationItem = Basic_SegAnnotationItem<SegStroke3D>;

class AnnotationContainer;
// 构建一张size为 $size$ 的QImage，并将pAnnoContainer中的所有标注画在上面，构成一张 Color Map
// 具体的方式为，每个标注用其对应label的颜色来绘制其每一个笔画，后画会覆盖先画的笔画
extern QImage drawColorImage(const QSize &size, const AnnotationContainer *pAnnoContainer, const LabelManager *pLabelManager);
// 构建一张size为 $size$ 的QImage，并将pAnnoContainer中的所有标注画在上面，构成一张 LabelId Map
// 具体的方式为，每个标注用其对应label的id（注意不是instance id）来绘制其每一个笔画，构成一张灰度图，后画会覆盖先画的笔画
extern QImage drawLabelIdImage(const QSize &size, const AnnotationContainer *pAnnoContainer, const LabelManager *pLabelManager);

// 构建一张size为 $size$ 的QImage，并将pAnnoContainer中的所有z坐标为 $zCoordinate$ 的标注画在上面，构成一张 Color Map
// 具体的方式为，每个标注用其对应label的颜色来绘制其每一个笔画，后画会覆盖先画的笔画
// hasContent为true当且仅当至少有一个标注的z坐标为 $zCoordinate$
extern QImage drawColorImage3d(int zCoordinate, bool *hasContent, const QSize &size, const AnnotationContainer *pAnnoContainer, const LabelManager *pLabelManager);

// 构建一张size为 $size$ 的QImage，并将pAnnoContainer中的所有z坐标为 $zCoordinate$ 的标注画在上面，构成一张 LabelId Map
// 具体的方式为，每个标注用其对应label的id（注意不是instance id）来绘制其每一个笔画，构成一张灰度图，后画会覆盖先画的笔画
// hasContent为true当且仅当至少有一个标注的z坐标为 $zCoordinate$
extern QImage drawLabelIdImage3d(int zCoordinate, bool *hasContent, const QSize &size, const AnnotationContainer *pAnnoContainer, const LabelManager *pLabelManager);


#endif // SEGANNOTATIONITEM_H
