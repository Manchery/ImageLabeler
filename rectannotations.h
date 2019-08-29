#ifndef RECTLABELDATA_H
#define RECTLABELDATA_H

#include <QObject>
#include <QList>
#include <QRect>
#include <QStack>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>

struct RectAnnotationItem{
    QRect rect;
    QString label;
    QString toStr();
    QJsonObject toJsonObject();
    void fromJsonObject(const QJsonObject &json);
};

// Op means Operator
enum ContainerOp{
    PUSH, REMOVE, MODIFY
};

struct RectAnnotationOp{
    ContainerOp opClass;
    int idx;
    // op == PUSH: item = the item pushed
    // op == REMOVE: idx == index of the removed, item = the removed
    // op == MODIFY: idx == index of the modified, item = before modify, item2 == after modify
    RectAnnotationItem item, item2;
};

class RectAnnotations: public QObject
{
    Q_OBJECT
public:
    explicit RectAnnotations(QObject *parent = nullptr);
    int length() const;
    RectAnnotationItem operator [](int idx) const;
    bool hasData(QString label);
    QJsonArray toJsonArray();
    void fromJsonObject(QJsonObject json);
    void fromJsonArray(QJsonArray json);

    int getSelectedIdx() const { return selectedIdx; }
    RectAnnotationItem getSelectedItem() const { return items[selectedIdx]; }

signals:
    void dataChanged();
    void labelGiveBack(QString label);

    void UndoEnableChanged(bool);
    void RedoEnableChanged(bool);

    void AnnotationAdded(RectAnnotationItem item);
    void AnnotationInserted(RectAnnotationItem item, int idx);
    void AnnotationModified(RectAnnotationItem item, int idx);
    void AnnotationRemoved(int idx);
    void allCleared();
public slots:
    void push_back(const RectAnnotationItem &item);
    void remove(int idx);
    void modify(int idx, const RectAnnotationItem &item);

    void push_back(const QRect &rect, const QString &label);

    void allClear();

    void redo();
    void undo();

    void setSelected(int idx);

private:
    QList<RectAnnotationItem> items;
    QStack<RectAnnotationOp> ops;
    // ops at [0,curVersion] are done, curVersion are valid in [-1, ops.len-1]
    int curVersion;

    int selectedIdx;

    void checkIdx(int idx) const;
    void pushBackOp(RectAnnotationOp op);
    void emitUndoRedoEnable();
};

#endif // RECTLABELDATA_H
