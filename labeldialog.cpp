#include "labeldialog.h"
#include "ui_labeldialog.h"
#include <QCompleter>
#include <QListWidgetItem>
#include <QIcon>

LabelDialog::LabelDialog(const LabelManager &labels, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LabelDialog)
{
    ui->setupUi(this);

    for (auto label:labels.getLabels()){
        QPixmap pixmap(16,16);
        if (label.color.isValid()){
            pixmap.fill(label.color);
        }else{
            pixmap.fill(Qt::white);
        }
        QListWidgetItem *item = new QListWidgetItem(QIcon(pixmap),label.label, ui->listWidget);
        ui->listWidget->addItem(item);
    }

    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(labelSelected(QListWidgetItem*))) ;

    ui->lineEdit->setLabelListWidget(ui->listWidget);

    QCompleter* completer = new QCompleter();
    completer->setCompletionMode(QCompleter::InlineCompletion);
    completer->setModel(ui->listWidget->model());
    ui->lineEdit->setCompleter(completer);
}

LabelDialog::~LabelDialog()
{
    delete ui;
}

QString LabelDialog::getLabel() const
{
    return ui->lineEdit->text();
}

void LabelDialog::labelSelected(QListWidgetItem* item)
{
    ui->lineEdit->setText(item->text());
}
