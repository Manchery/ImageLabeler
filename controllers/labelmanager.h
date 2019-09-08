#ifndef LABELMANAGER_H
#define LABELMANAGER_H

#include <QObject>
#include <QColor>
#include <QList>
#include <QMap>
#include <QJsonArray>
#include <QJsonObject>

/* LabelProperty
 * 简介：用来存储label的相关性质的数据类型，存储了label、color、visible、id
 * Json：该类的数据与json相互转化时的格式为
 *  {
 *      "color": [
 *          Double,     // R
 *          Double,     // G
 *          Double      // B
 *      ],
 *      "id": Double,
 *      "label": String,
 *      "visible": Bool
 *  }
 */

class LabelProperty
{
public:
    LabelProperty(QString label, QColor color, bool visible, int id);
    LabelProperty();
    QString label;
    QColor color;
    bool visible;
    int id;

    QJsonObject toJsonObject();
    void fromJsonObject(QJsonObject json);
};

/* LabelManager
 * 简介：管理label相关的数据的类，主要与ui->labelListWidget同步
 * 注意：在addLabel前应检查是否已经存在该label
 *
 * Json：该类的数据与json相互转化时的格式为
 *  [ LabelProperty, LabelProperty, ... ] // Array中的元素的格式为LabelProperty的格式
 */

class LabelManager : public QObject
{
    Q_OBJECT
public:
    explicit LabelManager(QObject *parent = nullptr);

    LabelProperty operator[](QString label) const { return labels[label]; }
    bool hasLabel(QString label) const { return labels.find(label)!=labels.end(); }
    QList<LabelProperty> getLabels() const { return labels.values(); }
    QColor getColor(QString label) const { checkLabel(label); return labels[label].color; }

    QJsonArray toJsonArray();
    void fromJsonArray(QJsonArray json);
    // 从JsonObject中找到key为 "labels" 对应的value，并调用fromJsonArray
    void fromJsonObject(QJsonObject json);

signals:
    void labelChanged();
    void labelRemoved(QString label);
    void labelAdded(QString label, QColor color, bool visibile,int id);
    void visibelChanged(QString label, bool visible);
    void colorChanged(QString label, QColor color);
    void allCleared();
public slots:
    // 在addLabel前应检查是否已经存在该label，若id未被指定会被设为currentMaxId+1，并将currentMaxId加一
    // 注意，对于指定的id则不检查互异性，可能造成运行错误
    void addLabel(QString label, QColor color, bool visible, int id=-1);
    void removeLabel(QString label);
    void setColor(QString label, QColor color);
    void setVisible(QString label, bool visible);
    void allClear();

private:
    QMap<QString, LabelProperty> labels;
    // 检查是否存在label，否则抛出异常，不抛出异常的版本为共有函数 hasLabel
    void checkLabel(QString label) const;

    // 用来协助每次加入的label的id应该与之前的互不相同
    int currentMaxId;
};

#endif // LABELMANAGER_H
