#ifndef RECTANNOTATIONITEM_H
#define RECTANNOTATIONITEM_H

#include "annotationitem.h"
#include <QRect>
#include <QString>
#include <QJsonObject>
#include <QPainter>
#include <QColor>
#include <memory>

/* RectAnnotationItem
 * 简介：用于2D Detection的标注类型，比父类多记录了一个像素坐标的矩形 rect 表示bounding box
 *
 * Json：该类的数据与json相互转化时的格式如下，points表示两个矩形的左上和右下顶点
 *  {
 *      "label": String
 *      "id": Double
 *      "points": [
 *          [
 *              Double,
 *              Double
 *          ],
 *          [
 *              Double,
 *              Double
 *          ]
 *      ]
 *  }
 */

class AnnotationContainer;
class RectAnnotationItem: public AnnotationItem {
    friend AnnotationContainer;
public:
    // 将一个父类型的指针转化为该类型的指针
    static std::shared_ptr<RectAnnotationItem> castPointer(std::shared_ptr<AnnotationItem> ptr);

    RectAnnotationItem();
    RectAnnotationItem(QRect rect, QString label, int id);

    QRect getRect() const { return rect; }

    // 重载函数的说明详见父类
    QString toStr() const override;
    QJsonObject toJsonObject() const override;
    void fromJsonObject(const QJsonObject &json) override;

protected:
    QRect rect;
};

// 在QPainter $p$ 上画一个矩形 $rect$: brush设置为颜色 $brushColor$,透明度 $brushAlphaF$; pen的设置类似
extern void drawRectAnnotation(QPainter &p, const QRect &rect,
                               QColor brushColor, qreal brushAlphaF,
                               QColor penColor, qreal penAlphaF);

// 在QPainter $p$ 上画一个矩形 $rect$: brush设置为 $brush$；pen设置为 $pen$
extern void drawRectAnnotation(QPainter &p, const QRect &rect, const QBrush &brush, const QPen &pen);


#endif // RECTANNOTATIONITEM_H
