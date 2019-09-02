#ifndef LABELDIALOG_H
#define LABELDIALOG_H

#include "widgets/labellineedit.h"
#include "labelmanager.h"
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QKeyEvent>
#include <QtDebug>

namespace Ui {
class LabelDialog;
}

class LabelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LabelDialog(const LabelManager& labels, QWidget *parent = nullptr);
    ~LabelDialog();
    QString getLabel() const;

private slots:
    void labelSelected(QListWidgetItem *item);

private:
    Ui::LabelDialog *ui;
};

#endif // LABELDIALOG_H
