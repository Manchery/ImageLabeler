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

// Op means Operator
enum ContainerOp{
    PUSH, REMOVE, MODIFY, SWAP
};

struct AnnotationOp{
    ContainerOp opClass;
    int idx;
    // op == PUSH: item = the item pushed
    // op == REMOVE: idx == index of the removed, item = the removed
    // op == MODIFY: idx == index of the modified, item = before modify, item2 == after modify
    // op == SWAP: idx & idx+1 are swapped
    AnnoItemPtr item, item2;
};

class AnnotationContainer: public QObject
{
    Q_OBJECT
public:
    explicit AnnotationContainer(QObject *parent = nullptr);
    int length() const{ return items.length(); }
    AnnoItemPtr operator [](int idx) const;
    AnnoItemPtr at(int idx) const { checkIdx(idx); return items[idx]; }
    bool hasData(QString label);

    QJsonArray toJsonArray();
    void fromJsonObject(QJsonObject json, TaskMode task);
    void fromJsonArray(QJsonArray json, TaskMode task);

    int getSelectedIdx() const { return selectedIdx; }
    AnnoItemPtr getSelectedItem() const { return (selectedIdx>=0 && selectedIdx<items.length())?items[selectedIdx]:nullptr; }

    int newInstanceIdForLabel(QString label);

signals:
    void selectedChanged();
    void annoChanged();
    void labelGiveBack(QString label);

    void UndoEnableChanged(bool);
    void RedoEnableChanged(bool);

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
    void swap(int idx); // swap idx & idx+1

    void allClear();

    void redo();
    void undo();

    void setSelected(int idx);

private:
    QList<AnnoItemPtr> items;
    QStack<AnnotationOp> ops;
    // ops at [0,curVersion] are done, curVersion are valid in [-1, ops.len-1]
    int curVersion;

    int selectedIdx; // -1

    void checkIdx(int idx) const;
    void pushBackOp(AnnotationOp op);
    void emitUndoRedoEnable();
};

#endif // ANNOTATIONCONTAINER_H
