#ifndef ANNOTATIONITEM_H
#define ANNOTATIONITEM_H

#include "labelmanager.h"
#include <QString>
#include <QJsonObject>

/* JsonException
 * 简介：异常类，用于抛出由于json的读取等造成的异常
 */
class JsonException: public std::exception{
public:
    JsonException(std::string message);
    // 返回 message
    const char * what() const noexcept;
private:
    // 用于存储异常的提示信息
    std::string message;
};

/* AnnotationItem
 * 简介：AnnotationContainer中表示标注（Annotation，有时简写作Anno）的基本类型；
 *      仅记录label以及instance id；
 *      是其他标注类型（如RectAnnotationItem等）的基类；
 *      支持json格式的读写
 *
 * Json：该类的数据与json相互转化时的格式为
 *  {
 *      "label": String
 *      "id": Double
 *  }
 */

class AnnotationContainer;
class AnnotationItem
{
    friend AnnotationContainer;
public:
    AnnotationItem();
    AnnotationItem(QString label, int id);
    virtual ~AnnotationItem();

    QString getLabel() const { return label; }
    int getId() const { return id; }

    // 将重要信息描述为一个字符串，用于显示在ListWidget等
    virtual QString toStr() const = 0;
    // 将数据转换为一个QJsonObject
    virtual QJsonObject toJsonObject() const;
    // 从一个QJsonObject中读取数据，可能抛出 JsonException
    virtual void fromJsonObject(const QJsonObject &json);

protected:
    QString label;
    int id; // instance id
};

#endif // ANNOTATIONITEM_H
