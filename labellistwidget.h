#ifndef ESCAPABLELISTWIDGET_H
#define ESCAPABLELISTWIDGET_H

#include <QListWidget>
#include <QMouseEvent>
#include <QString>
#include <QColor>

class LabelListWidget : public QListWidget
{
public:
    LabelListWidget(QWidget *parent=nullptr);
    void mousePressEvent(QMouseEvent *event);
public slots:
    void addCustomItem(QString label, QColor color, bool visible);
    void insertCustomItem(QString label, QColor color, bool visible, int idx);
//    void addCustomItem(QString label, bool visible);
    void removeCustomItem(QString label);
    void removeCustomItemByIdx(int idx);
    void changeCheckState(QString label, bool visible);
    void changeIconColor(QString label,QColor color);
    void changeIconColorByIdx(int idx,QColor color);
    void changeTextByIdx(int idx, QString text);
private:
    QListWidgetItem* _findItemByText(QString label);
};

#endif // ESCAPABLELISTWIDGET_H
