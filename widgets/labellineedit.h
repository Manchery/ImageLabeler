#ifndef LABELLINEEDIT_H
#define LABELLINEEDIT_H

#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QKeyEvent>
#include <QtDebug>

class LabelDialog;

class LabelLineEdit: public QLineEdit{
    friend LabelDialog;
public:
    explicit LabelLineEdit(QWidget *parent = nullptr);
    void setLabelListWidget(QListWidget* listWidget);
    void keyPressEvent(QKeyEvent *event);
private:
    QListWidget* labelListWidget;
};


#endif // LABELLINEEDIT_H
