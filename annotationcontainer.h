#ifndef ANNOTATIONCONTAINER_H
#define ANNOTATIONCONTAINER_H

#include "annotationitem.h"
#include "canvasbase.h"
#include <QObject>
#include <QList>
#include <QRect>
#include <QStack>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>
#include <memory>

using AnnoItemPtr = std::shared_ptr<AnnotationItem>;

// 表示作用在AnnotationContainer上的操作类型的枚举类型
// Op means Operator
// PUSH 表示在末尾插入，SWAP表示交换相邻两个
enum ContainerOp{
    PUSH, REMOVE, MODIFY, SWAP
};

// 表示作用在AnnotationContainer上的操作的数据类型
//  op == PUSH: item = the item pushed
//  op == REMOVE: idx == index of the removed, item = the removed
//  op == MODIFY: idx == index of the modified, item = before modify, item2 == after modify
//  op == SWAP: idx & idx+1 are swapped
struct AnnotationOp{
    ContainerOp opClass;
    int idx;
    AnnoItemPtr item, item2;
};

/* AnnotationContainer
 * 简介：用于存储标注数据的数据结构，并能够实现撤销和重做的可持久化操作
 * 功能：a. 存储修改标注的数据，并支持撤销和重做
 *      b. 可设置某个标注为选中，与ui->annoListWidget同步，并能在画布中修改
 *
 * 注意：如果undo之后进行了新的操作，则之后undo再redo也只能重做新的操作，原先的操作在新的操作之后都被截断丢弃了
 *
 * Json: 该类的数据与json相互转化时的格式为
 *  [ AnnotationItem, AnnotationItem, ... ]  // Array中的元素为对应的AnnotationItem及其子类的json格式
 */

class AnnotationContainer: public QObject
{
    Q_OBJECT
public:
    explicit AnnotationContainer(QObject *parent = nullptr);

    int length() const{ return items.length(); }
    AnnoItemPtr operator [](int idx) const { checkIdx(idx); return items[idx]; }
    AnnoItemPtr at(int idx) const { checkIdx(idx); return items[idx]; }

    int getSelectedIdx() const { return selectedIdx; }
    AnnoItemPtr getSelectedItem() const { return (selectedIdx>=0 && selectedIdx<items.length())?items[selectedIdx]:nullptr; }

    QJsonArray toJsonArray();
    // 从JsonObject中找到key为 "annotations" 对应的value，并调用fromJsonArray
    void fromJsonObject(QJsonObject json, TaskMode task);
    void fromJsonArray(QJsonArray json, TaskMode task);

    // 返回是否存在至少一个标注的label是 $label$
    bool hasData(QString label) const;
    // 返回一个新的label为 $label$ 的标注的instance id
    // 具体的做法为所有已有的同一label的标注的 instance id 的最大值加 1
    int newInstanceIdForLabel(QString label);

signals:
    // 选中状态的改变
    void selectedChanged();
    // 标注数据的改变
    void annoChanged();
    // 执行undo的时候可能会造成已经被删除的label又重新存在的情况
    void labelGiveBack(QString label);

    void UndoEnableChanged(bool);   // 是否可以undo，依据是否已经是第一个状态
    void RedoEnableChanged(bool);   // 是否可以redo，依据是否已经是最后一个状态

    void AnnotationAdded(AnnoItemPtr item);
    void AnnotationInserted(AnnoItemPtr item, int idx);
    void AnnotationModified(AnnoItemPtr item, int idx);
    void AnnotationRemoved(int idx);
    void AnnotationSwap(int idx);
    void allCleared();
public slots:
    void push_back(const AnnoItemPtr &item);
    void remove(int idx);
    void modify(int idx,const AnnoItemPtr &item);
    void swap(int idx);             // swap idx & idx+1

    void allClear();

    void redo();
    void undo();

    void setSelected(int idx);

private:
    QList<AnnoItemPtr> items;   // 当前状态(curVersion)下的标注数据
    QStack<AnnotationOp> ops;   // 所有进行的操作
    // curVersion 表示items的状态是已经进行了 ops[0,curVersion] 的操作，而之后的操作都被undo了
    // curVersion 的合法范围应该是 [-1, ops.len-1]
    int curVersion;

    int selectedIdx;            // -1 表示未选中状态

    // 检查idx是否 (idx>=0 && idx<items.length()) 否则抛出异常
    void checkIdx(int idx) const;
    // 将新操作加入ops的末尾，如果有未redo的操作，则这些操作会被丢弃
    void pushBackOp(AnnotationOp op);
    // 根据当前的状态重新发出相应的 UndoEnableChanged 和 RedoEnableChanged 信号
    void emitUndoRedoEnable();
};

#endif // ANNOTATIONCONTAINER_H
