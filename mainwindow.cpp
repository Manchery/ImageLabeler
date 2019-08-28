#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "labeldialog.h"
#include "canvas.h"
#include "utils.h"
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QtDebug>
#include <QColorDialog>
#include <QtGlobal>
#include <QKeyEvent>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    canvas = new Canvas(&labelManager, &rectAnno, ui->scrollArea);
    ui->scrollArea->setWidget(canvas);
//    ui->scrollArea->setWidgetResizable(true);

    canvas->setEnabled(true);
    QPixmap pixmap(":/pictures/img/seg_01.jpg");
    canvas->loadPixmap(pixmap);

    // label config
    // signal-slot from-to: ui->list => labelconfig => canvas
    ui->labelListWidget->setSortingEnabled(true);
    ui->labelListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->labelListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->labelListWidget, &QListWidget::customContextMenuRequested,
            this, &MainWindow::provideLabelContextMenu);

    connect(ui->labelListWidget, &QListWidget::itemChanged,
            [this](QListWidgetItem *item){ // changeLabelVisble
                if (item->checkState()==Qt::Checked){
                    labelManager.setVisible(item->text(),true);
                }else{
                    labelManager.setVisible(item->text(),false);
                }
            });

    connect(&labelManager, &LabelManager::configChanged, canvas, qOverload<>(&QWidget::update));
    connect(&labelManager, &LabelManager::labelAdded,
            ui->labelListWidget, &LabelListWidget::addCustomItem);
    connect(&labelManager, &LabelManager::labelRemoved,
            ui->labelListWidget, &LabelListWidget::removeCustomItem);
    connect(&labelManager, &LabelManager::colorChanged,
            ui->labelListWidget, &LabelListWidget::changeIconColor);
    connect(&labelManager, &LabelManager::visibelChanged,
            ui->labelListWidget, &LabelListWidget::changeCheckState);

    //! TODO: replaced by read a file;
    labelManager.addLabel("people", Qt::green);
    labelManager.addLabel("cat", Qt::blue, false);
    labelManager.addLabel("dog", Qt::yellow);
    // end label config

    // label data
    // signal-slot from-to: ui->action/canvas => labeldata => canvas
    ui->annoListWidget->setSortingEnabled(false);
    ui->annoListWidget->setSelectionMode(QAbstractItemView::NoSelection);

    ui->annoListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->annoListWidget, &QListWidget::customContextMenuRequested,
            this, &MainWindow::provideAnnoContextMenu);

    connect(ui->annoListWidget, &QListWidget::itemChanged,
            [this](QListWidgetItem *item){ // changeLabelVisble
                if (item->checkState()==Qt::Checked){
                    rectAnno.setVisible(ui->annoListWidget->row(item),true);
                }else{
                    rectAnno.setVisible(ui->annoListWidget->row(item),false);
                }
            });

    connect(ui->actionUndo, &QAction::triggered, &rectAnno, &RectAnnotations::undo);
    connect(ui->actionRedo, &QAction::triggered, &rectAnno, &RectAnnotations::redo);

    connect(canvas, &Canvas::newRectangleAnnotated, this, &MainWindow::getNewRect);

    connect(canvas, &Canvas::removeRectRequest, &rectAnno, &RectAnnotations::remove);


    connect(&labelManager, &LabelManager::colorChanged, [this](QString label, QColor color){
        for (int i=0;i<ui->annoListWidget->count();i++){
            auto item = ui->annoListWidget->item(i);
            if (item->text().split(' ')[0]==label)
                ui->annoListWidget->changeIconColorByIdx(i, color);
        }
    });

    connect(&rectAnno, &RectAnnotations::labelGiveBack, this, &MainWindow::newLabelRequest);
    connect(&rectAnno, &RectAnnotations::dataChanged, canvas, qOverload<>(&QWidget::update));
    connect(&rectAnno, &RectAnnotations::AnnotationAdded,[this](RectAnnotationItem item){
        ui->annoListWidget->addCustomItem(item.toStr(), labelManager.getColor(item.label),
                                          item.visible);
    });
    connect(&rectAnno, &RectAnnotations::AnnotationInserted,[this](RectAnnotationItem item, int idx){
        ui->annoListWidget->insertCustomItem(item.toStr(), labelManager.getColor(item.label),
                                          item.visible, idx);
    });
    connect(&rectAnno, &RectAnnotations::AnnotationRemoved,[this](int idx){
        ui->annoListWidget->removeCustomItemByIdx(idx);
    });
    // end label data

    connect(canvas, &Canvas::mouseMoved, this, &MainWindow::reportMouseMoved);
    connect(canvas, &Canvas::zoomRequest, this, &MainWindow::zoomRequest);
    connect(ui->actionFit_Window, &QAction::triggered, this, &MainWindow::adjustFitWindow);

    connect(ui->pushButton_addLabel, &QPushButton::clicked, [this](){
        newLabelRequest(ui->lineEdit_addLabel->text());
    });
    connect(ui->lineEdit_addLabel, &QLineEdit::returnPressed, [this](){
        newLabelRequest(ui->lineEdit_addLabel->text());
        ui->lineEdit_addLabel->setText("");
    });
}

MainWindow::~MainWindow()
{
    delete ui;
    delete canvas;
}

void MainWindow::getNewRect(QRect rect)
{
    QString curLabel = getCurrentLabel();
    if (curLabel==""){
        LabelDialog dialog(labelManager, this);
        if(dialog.exec() == QDialog::Accepted) {
            QString newLabel = dialog.getLabel();
            newLabelRequest(newLabel);
            rectAnno.push_back(rect, newLabel, true);
        }
    }else{
        rectAnno.push_back(rect, curLabel, true);
    }
}

void MainWindow::newLabelRequest(QString newLabel)
{
    if (newLabel.isNull() || newLabel.isEmpty()) return;
    if (!labelManager.hasLabel(newLabel)){
        QColor newColor = randomColor();
        labelManager.addLabel(newLabel, newColor);
    }
}

void MainWindow::removeLabelRequest(QString label)
{
    if (rectAnno.hasData(label)){
        QMessageBox::warning(this, "Warning", "This label has existing data! Please remove them first.");
    }else{
        labelManager.removeLabel(label);
    }
}

void MainWindow::provideLabelContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->labelListWidget->mapToGlobal(pos);
    QModelIndex modelIdx = ui->labelListWidget->indexAt(pos);
    if (!modelIdx.isValid()) return;
    int row = modelIdx.row();
    auto item = ui->labelListWidget->item(row);

    QMenu submenu;
    submenu.addAction("Change Color");
    submenu.addAction("Delete");
    QAction* rightClickItem = submenu.exec(globalPos);
    if (rightClickItem){
        if (rightClickItem->text().contains("Delete")){
            removeLabelRequest(item->text());
        }else if (rightClickItem->text().contains("Change Color")){
            QColor color = QColorDialog::getColor(Qt::white, this, "Choose Color");
            if (color.isValid()){
                labelManager.setColor(item->text(),color);
            }
        }
    }
}

void MainWindow::provideAnnoContextMenu(const QPoint &pos)
{
    QPoint globalPos = ui->annoListWidget->mapToGlobal(pos);
    QModelIndex modelIdx = ui->annoListWidget->indexAt(pos);
    if (!modelIdx.isValid()) return;
    int row = modelIdx.row();
//    auto item = ui->annoListWidget->item(row);

    QMenu submenu;
    submenu.addAction("Delete");
    QAction* rightClickItem = submenu.exec(globalPos);
    if (rightClickItem){
        if (rightClickItem->text().contains("Delete")){
            rectAnno.remove(row);
        }
    }
}

QString MainWindow::getCurrentLabel()
{
    auto selected = ui->labelListWidget->selectedItems();
    if (selected.length()==1){
        return selected[0]->text();
    }else if (selected.length()==0){
        return QString();
    }else {
        throw "selected mutiple label in the list";
    }
}

void MainWindow::on_actionOpen_File_triggered()
{
    QString file_name = QFileDialog::getOpenFileName(this, "open a file", "/", "Image Files(*.jpg *.png);;JPEG Files(*.jpg);;PNG Files(*.png)");
    if (file_name!=""){
        canvas->loadPixmap(file_name);
        adjustFitWindow();
    }
}

void MainWindow::on_actionZoom_in_triggered()
{
    canvas->setScale(canvas->getScale()*1.1);
}

void MainWindow::on_actionZoom_out_triggered()
{
    canvas->setScale(canvas->getScale()*0.9);
}

qreal MainWindow::scaleFitWindow()
{
    int w1 = ui->scrollArea->width() - 2; // So that no scrollbars are generated.
    int h1 = ui->scrollArea->height() - 2;
    qreal a1 = static_cast<qreal>(w1)/h1;
    int w2 = canvas->getPixmap().width();
    int h2 = canvas->getPixmap().height();
    qreal a2 = static_cast<qreal>(w2)/h2;
    return a2>=a1 ? static_cast<qreal>(w1)/w2 : static_cast<qreal>(h1)/h2;
}

void MainWindow::adjustFitWindow()
{
    canvas->setScale(scaleFitWindow());
}

void MainWindow::zoomRequest(qreal delta, QPoint pos)
{
    int oldWidth = canvas->width();
    if (delta>0){
        canvas->setScale(canvas->getScale()*1.1);
    } else {
        canvas->setScale(canvas->getScale()*0.9);
    }
    int newWidth = canvas->width();
    if (newWidth!=oldWidth){
        qreal scale = static_cast<qreal>(newWidth) / oldWidth;
        int xShift = static_cast<int>(round(pos.x() * scale)) - pos.x();
        int yShift = static_cast<int>(round(pos.y() * scale)) - pos.y();
        auto verticalBar = ui->scrollArea->verticalScrollBar();
        verticalBar->setValue(verticalBar->value()+xShift);
        auto horizontalBar = ui->scrollArea->horizontalScrollBar();
        horizontalBar->setValue(horizontalBar->value()+yShift);
    }
}

void MainWindow::reportMouseMoved(QPoint pos)
{
    ui->statusBar->showMessage("("+ QString::number(pos.x())+","+QString::number(pos.y())+")");
}
